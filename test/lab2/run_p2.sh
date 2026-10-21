#!/bin/sh
# Run the Lab2 P2 Best-Fit black-box suite in a throw-away copy of lab2.
#
#   sh test/lab2/run_p2.sh              # all cases in one boot; on failure,
#                                       # re-run every case in its own boot
#   sh test/lab2/run_p2.sh --per-case   # always one boot per case
#   P2_PAGE_INIT_FROM=<commit> sh ...   # this branch lacks page_init (P1):
#                                       # take kern/mm/pmm.c and kern/init/entry.S
#                                       # from <commit>, re-apply this branch's
#                                       # pmm.c diff; best_fit_pmm.c stays as is
#   P2_MANAGER=default sh ...           # sanity check of the suite itself: run it
#                                       # against First-Fit, which must FAIL
#
# lab2/ itself is never written: the copy lives under $TMPDIR and is removed
# afterwards. Logs go to test/lab2/results/.

set -u

HERE=$(cd "$(dirname "$0")" && pwd)
REPO=$(cd "$HERE/../.." && pwd)
LAB2="$REPO/lab2"
RESULTS="$HERE/results"
QEMU=${QEMU:-qemu-system-riscv64}
BOOT_TIMEOUT=${BOOT_TIMEOUT:-60}
PER_CASE=0
[ "${1:-}" = "--per-case" ] && PER_CASE=1

mkdir -p "$RESULTS"
WORK=$(mktemp -d "${TMPDIR:-/tmp}/lab2_p2_blackbox.XXXXXX") || exit 1
trap 'rm -rf "$WORK"' EXIT INT TERM
STAMP=$(date +%Y%m%d-%H%M%S)
REPORT="$RESULTS/p2_blackbox_$STAMP.log"

(cd "$LAB2" && tar --exclude=./obj --exclude=./bin --exclude=.git -cf - .) |
    (cd "$WORK" && tar -xf -) || exit 1
OVERLAY=""
if [ -n "${P2_PAGE_INIT_FROM:-}" ]; then
    base=$(git -C "$REPO" merge-base HEAD "$P2_PAGE_INIT_FROM") || exit 1
    git -C "$REPO" show "$P2_PAGE_INIT_FROM:lab2/kern/mm/pmm.c" > "$WORK/kern/mm/pmm.c" || exit 1
    git -C "$REPO" show "$P2_PAGE_INIT_FROM:lab2/kern/init/entry.S" > "$WORK/kern/init/entry.S" || exit 1
    git -C "$REPO" diff "$base" HEAD -- lab2/kern/mm/pmm.c | (cd "$WORK" && patch -s -p2) || exit 1
    OVERLAY="page_init overlay from $(git -C "$REPO" rev-parse --short "$P2_PAGE_INIT_FROM") (merge-base $(git -C "$REPO" rev-parse --short "$base"))"
fi
if [ "${P2_MANAGER:-}" = "default" ]; then
    sed -i.bak 's/pmm_manager = &best_fit_pmm_manager/pmm_manager = \&default_pmm_manager/' \
        "$WORK/kern/mm/pmm.c" && rm -f "$WORK/kern/mm/pmm.c.bak"
    grep -q 'pmm_manager = &default_pmm_manager' "$WORK/kern/mm/pmm.c" || exit 1
    OVERLAY="${OVERLAY:+$OVERLAY; }SANITY: pmm_manager switched to default_pmm_manager (expected FAIL)"
fi
python3 "$HERE/inject_p2.py" "$WORK" || exit 1

# boot_case <P2_CASE> <log>: build with that case selected and boot QEMU until
# the suite prints its end marker, the kernel panics, or the timeout expires.
boot_case() {
    # make 3.81 compares mtimes in whole seconds; drop every product that
    # depends on P2_CASE so a fast previous boot cannot leave a stale image
    find "$WORK/obj" -name 'test_p2_blackbox*' -exec rm -f {} + 2>/dev/null
    rm -f "$WORK/bin/kernel" "$WORK/bin/ucore.img"
    if ! (cd "$WORK" && make --quiet --no-print-directory DEFS="-DP2_CASE=$1") \
            > "$2.build" 2>&1; then
        cat "$2.build" > "$2"
        echo "BUILD-FAILED" >> "$2"
        return
    fi
    # -kernel rather than the Makefile's "-device loader": with QEMU 11 /
    # OpenSBI 1.8 fw_dynamic the loader leaves "Next Address" at 0 and the
    # kernel never runs; -kernel puts the image at 0x80200000 and jumps there.
    "$QEMU" -machine virt -nographic -bios default \
        -kernel "$WORK/bin/ucore.img" \
        > "$2" 2>&1 < /dev/null &
    pid=$!
    t=0
    while [ "$t" -lt $((BOOT_TIMEOUT * 2)) ]; do
        if grep -q -e 'P2-BLACKBOX-DONE' -e 'kernel panic' "$2" 2>/dev/null; then
            sleep 0.2
            break
        fi
        sleep 0.5
        t=$((t + 1))
    done
    kill "$pid" 2>/dev/null
    wait "$pid" 2>/dev/null
    grep -q -e 'P2-BLACKBOX-DONE' -e 'kernel panic' "$2" || echo "TIMEOUT" >> "$2"
}

{
    echo "# Lab2 P2 Best-Fit black-box run $STAMP"
    echo "# branch: $(git -C "$REPO" rev-parse --abbrev-ref HEAD 2>/dev/null)" \
         "commit: $(git -C "$REPO" rev-parse --short HEAD 2>/dev/null)"
    [ -z "$OVERLAY" ] || echo "# $OVERLAY"
} > "$REPORT"

status=0
if [ "$PER_CASE" -eq 0 ]; then
    boot_case 0 "$WORK/all.log"
    echo "## all cases, one boot" >> "$REPORT"
    if grep -q 'BUILD-FAILED' "$WORK/all.log"; then
        grep -e 'error' -e 'BUILD-FAILED' "$WORK/all.log" >> "$REPORT"
        echo "RESULT: BUILD FAILED" >> "$REPORT"
        cp "$REPORT" "$RESULTS/latest.log"
        cat "$REPORT"
        exit 2
    fi
    grep -e '^\[P2\]' -e 'kernel panic' -e 'assertion failed' \
         -e 'TIMEOUT' -e '^ *+[0-9]*:' -e '^ *nr_free_pages' "$WORK/all.log" >> "$REPORT"
    if grep -q 'fail=0' "$WORK/all.log"; then
        echo "RESULT: PASS" >> "$REPORT"
    else
        PER_CASE=1
        status=1
        echo "## failure in one-boot run; re-running each case alone" >> "$REPORT"
    fi
fi

if [ "$PER_CASE" -eq 1 ]; then
    [ -f "$WORK/all.log" ] || boot_case 0 "$WORK/all.log"
    n=$(sed -n 's/.*P2_CASE=0, \([0-9]*\) cases.*/\1/p' "$WORK/all.log" | head -n 1)
    [ -n "$n" ] || { echo "cannot read case count" >> "$REPORT"; n=0; status=1; }
    pass=0
    fail=0
    i=1
    while [ "$i" -le "$n" ]; do
        boot_case "$i" "$WORK/case$i.log"
        if grep -q 'fail=0' "$WORK/case$i.log"; then
            pass=$((pass + 1))
            echo "case $i: $(grep '^\[P2\] PASS' "$WORK/case$i.log" | sed 's/^\[P2\] //')" >> "$REPORT"
        else
            fail=$((fail + 1))
            echo "case $i: FAIL" >> "$REPORT"
            grep -e '^\[P2\]' -e 'kernel panic' -e 'assertion failed' -e 'BUILD-FAILED' \
                 -e 'TIMEOUT' -e '^ *+[0-9]*:' -e '^ *nr_free_pages' \
                 "$WORK/case$i.log" | grep -v -e 'suite start' -e 'isolated:' \
                 | sed 's/^/    /' >> "$REPORT"
        fi
        i=$((i + 1))
    done
    echo "RESULT: per-case pass=$pass fail=$fail of $n" >> "$REPORT"
    [ "$fail" -eq 0 ] || status=1
fi

cp "$REPORT" "$RESULTS/latest.log"
cat "$REPORT"
exit "$status"
