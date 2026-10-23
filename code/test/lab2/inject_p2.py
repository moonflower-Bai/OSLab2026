#!/usr/bin/env python3
"""Inject the P2 black-box suite into a temporary copy of lab2.

Mirrors lab2/tools/inject.py: test sources go to <copy>/kern/test, the
Makefile gains kern/test, pmm.c includes the header and calls the suite at
the end of pmm_init(). Only the copy given on the command line is touched.
"""

import os
import re
import shutil
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.join(HERE, "p2_blackbox")
FILES = ("test_p2_blackbox.c", "test_p2_blackbox.h")


def die(msg):
    print("[p2 inject] ERROR: " + msg)
    raise SystemExit(1)


def read(path):
    with open(path, "r", encoding="utf-8") as f:
        return f.read()


def write(path, text):
    with open(path, "w", encoding="utf-8") as f:
        f.write(text)


def function_end(text, name):
    """Index of the closing brace of function `name` (comments/strings aware)."""
    pat = re.compile(
        r"(?ms)^[ \t]*(?:static[ \t]+)?[A-Za-z_][A-Za-z0-9_ \t\n\*]*?\b"
        + re.escape(name) + r"[ \t\n]*\([^;{}]*\)[ \t\n]*\{"
    )
    found = list(pat.finditer(text))
    if len(found) != 1:
        die("function missing or ambiguous: " + name)
    i = text.find("{", found[0].start(), found[0].end())
    depth, mode, esc = 0, None, False
    while i < len(text):
        ch, nxt = text[i], text[i + 1:i + 2]
        if mode == "line":
            mode = None if ch == "\n" else mode
        elif mode == "block":
            if ch == "*" and nxt == "/":
                mode, i = None, i + 1
        elif mode in ('"', "'"):
            if esc:
                esc = False
            elif ch == "\\":
                esc = True
            elif ch == mode:
                mode = None
        elif ch == "/" and nxt == "/":
            mode = "line"
        elif ch == "/" and nxt == "*":
            mode = "block"
        elif ch in ('"', "'"):
            mode = ch
        elif ch == "{":
            depth += 1
        elif ch == "}":
            depth -= 1
            if depth == 0:
                return i
        i += 1
    die("unbalanced braces in " + name)


def main():
    if len(sys.argv) != 2:
        die("usage: inject_p2.py <temporary lab2 copy>")
    root = os.path.abspath(sys.argv[1])
    makefile = os.path.join(root, "Makefile")
    pmm = os.path.join(root, "kern", "mm", "pmm.c")
    testdir = os.path.join(root, "kern", "test")

    mk = read(makefile)
    if "kern/test" in mk:
        die("Makefile already mentions kern/test; copy is not clean")
    os.makedirs(testdir, exist_ok=True)
    for name in FILES:
        shutil.copyfile(os.path.join(SRC, name), os.path.join(testdir, name))

    for var in ("KINCLUDE", "KSRCDIR"):
        m = re.search(r"(?m)^" + var + r"[ \t]+\+=.*(?:\n[ \t]+[^\n]*)*", mk)
        if m is None:
            die("Makefile variable not found: " + var)
        mk = mk[:m.end()] + "\n" + var + "\t+= kern/test" + mk[m.end():]
    write(makefile, mk)

    text = read(pmm)
    inc = list(re.finditer(r"(?m)^[ \t]*#include[ \t]*[<\"]pmm\.h[>\"][ \t]*$", text))
    if len(inc) != 1:
        die("#include <pmm.h> missing or ambiguous in pmm.c")
    text = text[:inc[0].end()] + "\n#include <test_p2_blackbox.h>" + text[inc[0].end():]
    end = function_end(text, "pmm_init")
    text = text[:end] + "    test_p2_blackbox();\n" + text[end:]
    write(pmm, text)
    print("p2 black-box injection complete: " + root)


if __name__ == "__main__":
    main()
