#!/usr/bin/env python3
"""重新計算 README.md 裡進度追蹤表的完成數字。

你只需要手動把某一關的 `- [ ]` 改成 `- [x]`（這一步沒辦法自動化——只有你知道
自己是不是真的通過驗收條件），存檔後跑這支腳本，它會自動：
  1. 掃過每個章節標題底下的 checklist，數出「幾個打勾 / 總共幾關」
  2. 把章節標題那行的 `— a/b` 更新成正確數字，全部完成時加上 ✅
  3. 重新算出檔案最下面「總進度：X / Y」那一行（排除最終 Boss）

用法：
    python scripts/update_progress.py
    python scripts/update_progress.py path/to/README.md   # 指定其他路徑
"""

import re
import sys
from pathlib import Path

HEADER_RE = re.compile(r"^(### .+?)\s*—\s*\d+/\d+(?:\s*✅)?\s*$")
ITEM_RE = re.compile(r"^- \[( |x|X)\] ")
TOTAL_RE = re.compile(r"^\*\*總進度：\d+ / \d+(.*)\*\*$")


def update_progress(path: Path) -> None:
    lines = path.read_text(encoding="utf-8").splitlines()

    boss_section = False
    grand_done = 0
    grand_total = 0

    i = 0
    while i < len(lines):
        m = HEADER_RE.match(lines[i])
        if m:
            title = m.group(1)
            boss_section = "最終 Boss" in title

            done = 0
            total = 0
            j = i + 1
            while j < len(lines) and not HEADER_RE.match(lines[j]) and not lines[j].startswith("---"):
                item = ITEM_RE.match(lines[j])
                if item:
                    total += 1
                    if item.group(1).lower() == "x":
                        done += 1
                j += 1

            mark = " ✅" if total > 0 and done == total else ""
            lines[i] = f"{title.rstrip()} — {done}/{total}{mark}"

            if not boss_section:
                grand_done += done
                grand_total += total

            i = j
            continue
        i += 1

    for idx, line in enumerate(lines):
        tm = TOTAL_RE.match(line)
        if tm:
            suffix = tm.group(1)
            lines[idx] = f"**總進度：{grand_done} / {grand_total}{suffix}**"
            break

    path.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"更新完成：{grand_done} / {grand_total}（不含最終 Boss）")


if __name__ == "__main__":
    target = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(__file__).resolve().parent.parent / "README.md"
    update_progress(target)