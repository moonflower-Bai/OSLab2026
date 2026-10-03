#!/usr/bin/env python3
"""Replay repository authorization adaptations on native OpenSpec prompts."""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path
from typing import Dict, List, Tuple


ROOT = Path(__file__).resolve().parents[1]
WORKFLOWS = {
    "propose": "openspec-propose",
    "apply": "openspec-apply-change",
    "update": "openspec-update-change",
    "explore": "openspec-explore",
    "archive": "openspec-archive-change",
    "sync": "openspec-sync-specs",
}
SKILL_ROOTS = (".agents", ".claude", ".cursor", ".opencode", ".hermes")
POLICY_BLOCK = """<!-- repository-authorization:start -->
**Repository authorization:** Read the root `AGENTS.md` before using this workflow. Its routing and scope authorization rules control the confirmation steps below: honor explicit implementation approval and quick/direct edit requests, reuse applicable authority, and ask only for unapproved requirement increments. If routing selects direct maintenance, perform that edit and validation without running the planning steps below. A proposal-only or discussion request remains planning-only. This workflow does not grant archive or commit authority.
**Repository branches:** Follow the root AGENTS.md experiment dev/stable/release and non-experiment branch conventions, including their pull request targets. Do not substitute a tool-specific branch prefix or infer authority to create or push submission branches.
<!-- repository-authorization:end -->
"""
POLICY_PATTERN = re.compile(
    r"<!-- repository-authorization:start -->.*?<!-- repository-authorization:end -->\n?",
    re.DOTALL,
)

# Replace only known authorization paragraphs; retain native commands and checks.
# Each required replacement must exist afterward so unknown templates fail closed.
REPLACEMENTS: Dict[str, List[Tuple[str, str]]] = {
    "propose": [
        (
            r"^\*\*Planning boundary\*\*: .+$",
            "**Planning boundary**: A proposal-only request authorizes planning only. Apply the root AGENTS.md authorization rules first: when the user has explicitly approved implementation or requested a quick/direct edit of this scope, do not discard that authority or require a new message. Reuse an existing change when appropriate; finish any necessary records and continue with the authorized work. Without implementation authority, complete the planning artifacts and wait for approval.",
        ),
        (
            r"^When the user is ready to implement, they must start the apply workflow explicitly\.$",
            "Implementation needs applicable authority, not a mandatory second invocation; reuse an explicit approval already present in the request.",
        ),
        (
            r"^- The request that invoked this workflow authorizes planning only\..+$",
            "- Preserve a proposal-only request's planning boundary. Do not erase explicit implementation approval or a quick/direct edit request; follow AGENTS.md and continue authorized work without a redundant approval round.",
        ),
        (
            r'^- Prompt: "The artifacts are ready for review\..+$',
            "- Next step: continue implementation when already authorized; otherwise present the artifacts and request only the missing implementation approval.",
        ),
    ],
    "apply": [
        (r"^   \*\*Pause if:\*\*$", "   **Clarify or pause only when needed:**"),
        (
            r"^   - Task is unclear → ask for clarification$",
            "   - A material requirement is unclear and blocks progress → ask for that decision; resolve routine implementation details autonomously.",
        ),
        (
            r"^   - Implementation reveals a design issue → suggest updating artifacts$",
            "   - A design issue changes an unapproved requirement → confirm the increment; otherwise update the existing artifacts and continue.",
        ),
        (
            r"^   - Error or blocker encountered → report and wait for guidance$",
            "   - A blocker requires user input → report it and request the missing input; repair in-scope errors and retry validation without new approval.",
        ),
        (
            r"^   - User interrupts$",
            "   - The user explicitly stops the work or revokes authority → stop that scope; incorporate ordinary steering and continue authorized work.",
        ),
        (
            r"^   - A task needs work beyond what the spec and tasks describe, .+$",
            "   - A task requires an unapproved requirement or scope increment, or would weaken the agreed acceptance criteria → explain and confirm that increment; necessary local implementation details remain within the existing authority.",
        ),
        (
            r"^- If task is ambiguous, pause and ask before implementing$",
            "- Ask only about material unresolved requirements; do not stop for routine implementation choices covered by existing authority.",
        ),
        (
            r"^- If implementation reveals issues, pause and suggest artifact updates$",
            "- Resolve in-scope implementation issues and keep artifacts coherent; confirm only unapproved requirement or scope changes.",
        ),
        (
            r"^- Pause on errors, blockers, or unclear requirements - don't guess$",
            "- Continue repairs and validation within the authorized scope; pause dependent work only for an unresolved user decision, missing authority, or an explicit stop.",
        ),
        (
            r"^- When a task needs work beyond what the spec describes, .+$",
            "- Confirm only unapproved requirement or scope increments; never silently narrow, defer, or weaken agreed behavior to make implementation fit.",
        ),
    ],
    "update": [
        (
            r"^5\. \*\*Confirm and apply, one artifact at a time\*\*$",
            "5. **Apply revisions within the authorized scope**",
        ),
        (
            r"^   - Show each proposed revision and why - including the requested edit drafted in step 4\. Write only after the user confirms\.$",
            "   - Explain the revisions and why. The user's explicit revision request or applicable maintenance authority already covers in-scope edits; write the coherent revisions without approval for each artifact. Ask only about unapproved requirement increments.",
        ),
        (
            r"^6\. \*\*Point to the next step \(guidance only - NEVER act on it\)\*\*$",
            "6. **Continue only with applicable implementation authority**",
        ),
        (
            r"^   - Change already implemented \(tasks checked off / already applied\) -> .+$",
            "   - If the revised plan needs implementation, continue with the native apply workflow when explicitly approved or directly requested; otherwise request the missing authority. Completed checkboxes alone never establish authority.",
        ),
        (
            r"^- Planning artifacts only - NEVER edit implementation code\..+$",
            "- This action edits existing planning artifacts. If the request is planning-only, do not implement; if implementation is explicitly approved or a quick/direct edit is requested, continue via the authorized maintenance or native apply path without requiring a new user message.",
        ),
        (
            r"^- Confirm every edit with the user before writing\.$",
            "- Reuse the user's revision or maintenance authority for in-scope edits; confirm only new unapproved requirements, not each file or artifact.",
        ),
    ],
    "explore": [
        (
            r"^\*\*IMPORTANT: Explore mode is for thinking, not implementing\.\*\* .+$",
            "**IMPORTANT: Exploration alone does not authorize implementation.** Read and investigate without confirmation. An explicit capture request authorizes the named artifacts and necessary prerequisites. If the user explicitly approves implementation or requests a quick/direct edit, follow AGENTS.md and transition to the authorized maintenance or native apply path without another confirmation round. Discussion answers and silence are not write authority; ask only for genuinely unapproved scope.",
        ),
        (
            r"^- \*\*Keep a conversational record\*\* - .+$",
            "- **Keep a conversational record** - Distinguish confirmed requirements, proposed defaults, and unresolved questions. Discussion and silence do not grant write authority; reuse explicit capture, implementation, or quick/direct edit authority already given under AGENTS.md.",
        ),
        (
            r"^If the user asks you to capture the exploration as a new change, that request is the confirmation required above\..+$",
            "An explicit capture request already authorizes the named change, its requested artifacts, and necessary prerequisites. Do not ask again for that scope; confirm only unapproved requirement increments. An implementation or quick/direct edit request follows the root authorization rules instead of forcing another planning-only turn. For a capture request, proceed as follows:",
        ),
        (
            r"^Capture the artifact\(s\) the user requested without asking them to invoke another workflow command\..+$",
            "Capture the requested artifacts without another invocation or approval for their necessary prerequisites. If only capture was requested, report its status and stop before implementation. If implementation is explicitly approved or a quick/direct edit is requested, continue through the authorized path described in AGENTS.md without demanding another user message.",
        ),
        (
            r"^- \*\*Don't implement\*\* - .+$",
            "- **Respect the request's scope** - A discussion-only or capture-only request does not authorize implementation. An explicit implementation approval or quick/direct edit request does: leave the exploration stance and execute the authorized maintenance or native apply workflow without a redundant approval.",
        ),
        (
            r"^- \*\*Don't auto-capture\*\* - .+$",
            "- **Don't infer write authority from discussion** - Offer capture when none was requested. Reuse explicit capture or implementation authority already given; ask only for genuinely unapproved scope, not a mandatory separate confirmation message.",
        ),
    ],
    "archive": [
        (
            r"^   \*\*Prompt options:\*\*$",
            "   **Reuse archive authority:** A valid archive request includes normal spec sync under AGENTS.md. If no capability is sync-blocked, honor an explicit sync preference or sync needed deltas and archive directly; when already synced, archive directly. Prompt only for unresolved conflicts, skipping required sync, or unapproved exceptions. For those decisions, offer:",
        ),
        (
            r"^   Route on the answer:$",
            "   Route on the applicable existing choice, or the answer to a genuinely missing decision:",
        ),
        (
            r"^- Don't block archive on warnings - just inform and confirm$",
            "- Reuse applicable archive authority; incomplete-work exceptions still need explicit authority, while routine successful archive and sync do not need repeated confirmation.",
        ),
        (
            r"^- If delta specs exist, always run the sync assessment and show the combined summary before prompting$",
            "- If delta specs exist, assess and summarize their sync state; prompt only when a material decision is not already authorized.",
        ),
        (
            r"^- Existing CLI checks, resolved paths, prompts, and command contracts are unchanged$",
            "- Preserve CLI checks, resolved paths, and command contracts; interpret confirmation prompts using existing scope authority from AGENTS.md.",
        ),
    ],
    "sync": [],
}
FORBIDDEN = (
    "Any implementation or apply instruction in that request does not carry forward",
    "Confirm every edit with the user before writing",
    "Write only after the user confirms",
    "Error or blocker encountered → report and wait for guidance",
    "guidance only - NEVER act on it",
    "wait for the user's confirmation in a separate message",
    "wait for explicit confirmation in a separate user message",
)


def workflow_paths(root: Path = ROOT) -> List[Tuple[Path, str]]:
    result = []
    for directory in SKILL_ROOTS:
        for workflow, skill in WORKFLOWS.items():
            result.append((root / directory / "skills" / skill / "SKILL.md", workflow))
    for workflow in WORKFLOWS:
        result.append((root / ".claude/commands/opsx" / f"{workflow}.md", workflow))
        for directory in (".cursor", ".opencode"):
            result.append((root / directory / "commands" / f"opsx-{workflow}.md", workflow))
    return result


def adapt_prompt(text: str, workflow: str) -> str:
    header = re.match(r"\A---\n.*?\n---\n", text, re.DOTALL)
    if header is None:
        raise ValueError("缺少有效的 YAML frontmatter")
    if POLICY_PATTERN.search(text):
        text = POLICY_PATTERN.sub(lambda match: POLICY_BLOCK, text)
    else:
        offset = header.end()
        text = text[:offset] + "\n" + POLICY_BLOCK + text[offset:]
    if text.count("<!-- repository-authorization:start -->") != 1:
        raise ValueError("仓库授权引导重复或不完整，需人工审阅")
    for pattern, replacement in REPLACEMENTS[workflow]:
        text = re.sub(pattern, lambda match: replacement, text, flags=re.MULTILINE)
        if replacement not in text:
            raise ValueError(f"未知模板，需审阅适配条款：{replacement[:65]}")
    if workflow == "explore":
        text = text.replace(
            "If the condition applies, or the prerequisite is not conditional, treat it as a normal prerequisite and ask before expanding the capture. Do not create an unrequested prerequisite unless the user approves.",
            "If the prerequisite is necessary for the authorized capture, create it without another approval; ask only when it introduces an unapproved requirement or scope increment.",
        ).replace(
            "If a requested artifact is blocked by a prerequisite the user did not ask to capture and cannot be conditionally skipped, explain that dependency and ask before expanding the capture.",
            "If a prerequisite is necessary for the authorized capture, complete it without another approval; explain and confirm only unapproved requirement or scope increments.",
        )
    for old in FORBIDDEN:
        if old in text:
            raise ValueError(f"仍有旧的重复审批条款：{old}")
    return text


def check_workflows(root: Path = ROOT) -> List[str]:
    errors = []
    for path, workflow in workflow_paths(root):
        label = path.relative_to(root).as_posix()
        try:
            original = path.read_text(encoding="utf-8")
            if adapt_prompt(original, workflow) != original:
                errors.append(f"{label}: 提示词未同步，请运行 scripts/sync_ai_workflows.py --write")
        except (OSError, ValueError) as exc:
            errors.append(f"{label}: {exc}")
    return errors


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group(required=True)
    mode.add_argument("--check", action="store_true", help="只读检查全部平台提示词")
    mode.add_argument("--write", action="store_true", help="恢复本仓库的授权适配")
    args = parser.parse_args()
    if args.check:
        errors = check_workflows()
        if errors:
            print("\n".join(errors), file=sys.stderr)
            return 1
        print(f"AI 提示词检查通过：{len(workflow_paths())} 份。")
        return 0

    # Validate every template before writing, so unknown versions cannot cause
    # a partial adaptation. Only files in the fixed native entry inventory change.
    updates = []
    try:
        for path, workflow in workflow_paths():
            original = path.read_text(encoding="utf-8")
            revised = adapt_prompt(original, workflow)
            if revised != original:
                updates.append((path, revised))
    except (OSError, ValueError) as exc:
        print(f"提示词同步未执行：{path.relative_to(ROOT)}: {exc}", file=sys.stderr)
        return 1
    for path, revised in updates:
        try:
            path.write_text(revised, encoding="utf-8")
        except OSError as exc:
            print(f"提示词写入失败：{path.relative_to(ROOT)}: {exc}", file=sys.stderr)
            return 1
    print(f"AI 提示词同步完成：修改 {len(updates)} / {len(workflow_paths())} 份。")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
