#!/usr/bin/env python3
"""Validate repository governance invariants without rewriting any file."""

from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
from pathlib import Path
from typing import Iterable, List, Optional, Sequence, Set


ROOT = Path(__file__).resolve().parents[1]
PROMPT_PATH = re.compile(
    r"^openspec/changes/(?:(?:archive/[^/]+)|(?:[^/]+))/prompt\.md$"
)
TEXT_SUFFIXES = {
    ".c",
    ".h",
    ".s",
    ".S",
    ".md",
    ".json",
    ".yaml",
    ".yml",
    ".py",
    ".txt",
    ".mk",
}
ALWAYS_CHECK_EOL = {
    ".gitattributes",
    ".gitignore",
    "AGENTS.md",
    "CLAUDE.md",
    "README.md",
    "lab-status.json",
    "lab1/AGENTS.md",
    "openspec/config.yaml",
    "prompts/README.md",
    "scripts/check_governance.py",
    ".github/workflows/governance.yml",
}

SENSITIVE_PATTERNS = [
    re.compile(r"(?:姓名|名字)\s*[:：=]\s*\S{2,}"),
    re.compile(r"学号\s*[:：=]\s*[A-Za-z0-9-]{4,}"),
    re.compile(r"(?<!\d)\d{17}[\dXx](?!\d)"),
    re.compile(r"(?i)\b(?:sk-[A-Za-z0-9_-]{8,}|gh[pousr]_[A-Za-z0-9]{8,}|AKIA[0-9A-Z]{12,})\b"),
    re.compile(r"(?i)(?:password|passwd|密码|token|api[ _-]?key)\s*[:：=]\s*\S+"),
    re.compile(r"-----BEGIN (?:RSA |EC |OPENSSH )?PRIVATE KEY-----"),
]


def run(args: Sequence[str]) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        list(args),
        cwd=ROOT,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        check=False,
    )


def git_paths(args: Sequence[str]) -> Set[str]:
    proc = subprocess.run(
        ["git", *args],
        cwd=ROOT,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=False,
    )
    if proc.returncode != 0:
        return set()
    return {
        item.decode("utf-8", errors="surrogateescape")
        for item in proc.stdout.split(b"\0")
        if item
    }


def line_number(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1


def status_violations(data: object) -> List[str]:
    errors: List[str] = []
    if not isinstance(data, dict):
        return ["lab-status.json 顶层必须是对象"]
    current = data.get("current")
    frozen = data.get("frozen")
    if not isinstance(current, str) or not re.fullmatch(r"lab\d+", current):
        errors.append("lab-status.json.current 必须是 labN")
    if not isinstance(frozen, list) or any(
        not isinstance(item, str) or not re.fullmatch(r"lab\d+", item)
        for item in frozen
    ):
        errors.append("lab-status.json.frozen 必须是 labN 字符串数组")
        return errors
    if len(frozen) != len(set(frozen)):
        errors.append("lab-status.json.frozen 不得包含重复章节")
    if current in frozen:
        errors.append("当前章节不得同时出现在 frozen 中")
    return errors


def rule_violations(agents: str, claude: str, cursor: str) -> List[str]:
    errors: List[str] = []
    if ".agents/skills/" in agents or ".agents/skills/" in cursor:
        errors.append("规则入口不得硬编码 .agents/skills/ 路径")
    for term in ("Propose", "Apply", "Archive", "Commit", "lab-status.json"):
        if term not in agents:
            errors.append(f"AGENTS.md 缺少阶段或状态声明：{term}")
    if claude != "@AGENTS.md\n":
        errors.append("CLAUDE.md 必须仅导入 @AGENTS.md")
    if "AGENTS.md" not in cursor or "/opsx-" not in cursor:
        errors.append("Cursor 入口必须指向根规则和原生 /opsx-* workflow")
    return errors


def prompt_change_name(path: str) -> Optional[str]:
    parts = path.split("/")
    if len(parts) == 4 and parts[:2] == ["openspec", "changes"]:
        return parts[2]
    if (
        len(parts) == 5
        and parts[:3] == ["openspec", "changes", "archive"]
    ):
        return re.sub(r"^\d{4}-\d{2}-\d{2}-", "", parts[3])
    return None


def prompt_violations(path: str, text: str) -> List[str]:
    errors: List[str] = []
    if not PROMPT_PATH.fullmatch(path):
        return [f"{path}: Prompt 证据路径不被允许"]
    change = prompt_change_name(path)
    required = [
        "# User Prompts",
        f"change: {change}",
        "reviewed_for_relevance: true",
        "reviewed_for_sensitive_data: true",
    ]
    for marker in required:
        if marker not in text:
            errors.append(f"{path}: 缺少元数据 {marker}")
    if not re.search(r"(?m)^## \d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2} [+-]\d{4}$", text):
        errors.append(f"{path}: 缺少带时区的 Prompt 时间")
    if "<redacted>" in text:
        errors.append(f"{path}: 包含禁止的脱敏占位符")
    for pattern in SENSITIVE_PATTERNS:
        for match in pattern.finditer(text):
            errors.append(
                f"{path}:{line_number(text, match.start())}: 匹配敏感信息模式"
            )
    return errors


def changed_paths(base: Optional[str]) -> Set[str]:
    if base and set(base) != {"0"}:
        exists = run(["git", "cat-file", "-e", f"{base}^{{commit}}"])
        if exists.returncode == 0:
            return git_paths(["diff", "--name-only", "-z", f"{base}...HEAD"])
    paths = git_paths(["diff", "--name-only", "-z"])
    paths |= git_paths(["diff", "--cached", "--name-only", "-z"])
    paths |= {
        path
        for path in git_paths(["ls-files", "--others", "--exclude-standard", "-z"])
        if not path.startswith("openspec/changes/archive/")
    }
    return paths


def candidate_paths() -> Set[str]:
    return git_paths(["ls-files", "--cached", "--others", "--exclude-standard", "-z"])


def is_text_candidate(path: str) -> bool:
    name = Path(path).name
    return path in ALWAYS_CHECK_EOL or name in {"Makefile", "AGENTS.md"} or Path(path).suffix in TEXT_SUFFIXES


def contains_carriage_return(data: bytes) -> bool:
    return b"\r" in data


def check_eol(paths: Iterable[str]) -> List[str]:
    errors: List[str] = []
    for path in sorted(set(paths) | ALWAYS_CHECK_EOL):
        target = ROOT / path
        if not target.is_file() or not is_text_candidate(path):
            continue
        data = target.read_bytes()
        if contains_carriage_return(data):
            errors.append(f"{path}: 文本必须使用 LF 行尾")
    return errors


def check_prompt_gate(paths: Iterable[str]) -> List[str]:
    errors: List[str] = []
    for path in sorted(paths):
        if path.startswith("prompts/") and path != "prompts/README.md":
            errors.append(f"{path}: 会话级 Prompt 归档不得进入提交")
            continue
        if Path(path).name != "prompt.md":
            continue
        if not PROMPT_PATH.fullmatch(path):
            errors.append(f"{path}: Prompt 证据路径不被允许")
            continue
        target = ROOT / path
        if target.is_file():
            errors.extend(prompt_violations(path, target.read_text(encoding="utf-8")))
    return errors


def check_scope(paths: Iterable[str], current: str, frozen: Sequence[str]) -> List[str]:
    errors: List[str] = []
    for path in sorted(paths):
        chapter = path.split("/", 1)[0]
        if not re.fullmatch(r"lab\d+", chapter):
            continue
        if chapter in frozen:
            errors.append(f"{path}: 冻结章节 {chapter} 不得修改")
        elif chapter != current:
            errors.append(f"{path}: 实验实现只能修改当前章节 {current}")
    return errors


def run_openspec_validation() -> List[str]:
    errors: List[str] = []
    for mode in ("--all", "--archived"):
        proc = run(["openspec", "validate", mode, "--strict"])
        if proc.returncode != 0:
            output = proc.stdout.strip()
            errors.append(f"openspec validate {mode} --strict 失败\n{output}")
    return errors


def self_test() -> List[str]:
    failures: List[str] = []

    def expect(label: str, condition: bool) -> None:
        if not condition:
            failures.append(f"自检失败：{label}")

    expect("非法状态被拒绝", bool(status_violations({"current": "one", "frozen": []})))
    expect(
        "冲突状态被拒绝",
        bool(status_violations({"current": "lab1", "frozen": ["lab1"]})),
    )
    expect(
        "私有 skill 路径被拒绝",
        bool(rule_violations(".agents/skills/x", "@AGENTS.md\n", "AGENTS.md /opsx-")),
    )
    expect(
        "冻结章节 diff 被拒绝",
        bool(check_scope(["lab2/kern/init.c"], "lab1", ["lab2"])),
    )
    expect(
        "非当前章节 diff 被拒绝",
        bool(check_scope(["lab3/kern/init.c"], "lab1", [])),
    )
    safe = (
        "# User Prompts\n\n"
        "change: demo\n"
        "reviewed_for_relevance: true\n"
        "reviewed_for_sensitive_data: true\n\n"
        "## 2026-09-22 10:00:00 +0800\n\n"
        "讨论姓名学号和 API Key 的保存策略，不包含任何实际值。\n"
    )
    expect(
        "安全 Prompt 通过",
        not prompt_violations("openspec/changes/demo/prompt.md", safe),
    )
    unsafe_samples = [
        "姓名：某某某",
        "学号：2026123456",
        "11010519491231002X",
        "api_key=secret-value",
        "sk-abcdefghijk",
        "<redacted>",
    ]
    for sample in unsafe_samples:
        expect(
            f"敏感样例被拒绝：{sample[:8]}",
            bool(
                prompt_violations(
                    "openspec/changes/demo/prompt.md", safe + "\n" + sample
                )
            ),
        )
    expect(
        "非法 Prompt 路径被拒绝",
        bool(prompt_violations("prompts/person/session.md", safe)),
    )
    expect(
        "CRLF 被拒绝",
        contains_carriage_return(b"line\r\n")
        and not contains_carriage_return(b"line\n"),
    )
    return failures


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--base", help="用于章节范围检查的 Git 基线提交")
    parser.add_argument(
        "--self-test",
        action="store_true",
        help="先运行内置安全与违规样例",
    )
    args = parser.parse_args()

    errors: List[str] = []
    if args.self_test:
        errors.extend(self_test())

    status_path = ROOT / "lab-status.json"
    try:
        status = json.loads(status_path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as exc:
        errors.append(f"lab-status.json 无法解析：{exc}")
        status = {}
    errors.extend(status_violations(status))

    current = status.get("current") if isinstance(status, dict) else None
    frozen = status.get("frozen") if isinstance(status, dict) else None
    if isinstance(current, str) and not (ROOT / current).is_dir():
        errors.append(f"当前章节目录不存在：{current}")
    if isinstance(frozen, list):
        for chapter in frozen:
            if isinstance(chapter, str) and not (ROOT / chapter).is_dir():
                errors.append(f"冻结章节目录不存在：{chapter}")

    try:
        agents = (ROOT / "AGENTS.md").read_text(encoding="utf-8")
        claude = (ROOT / "CLAUDE.md").read_text(encoding="utf-8")
        cursor = (ROOT / ".cursor/rules/openspec.mdc").read_text(encoding="utf-8")
        errors.extend(rule_violations(agents, claude, cursor))
    except OSError as exc:
        errors.append(f"规则入口无法读取：{exc}")

    changed = changed_paths(args.base)
    candidates = candidate_paths()
    if isinstance(current, str) and isinstance(frozen, list):
        errors.extend(check_scope(changed, current, frozen))
    errors.extend(check_eol(changed))
    errors.extend(check_prompt_gate(candidates))
    errors.extend(run_openspec_validation())

    if errors:
        print("治理检查失败：", file=sys.stderr)
        for error in errors:
            print(f"- {error}", file=sys.stderr)
        return 1
    print("治理检查通过；敏感信息模式扫描不能替代人工审阅。")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
