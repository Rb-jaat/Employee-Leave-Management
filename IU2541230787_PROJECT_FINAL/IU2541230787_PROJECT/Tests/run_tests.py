#!/usr/bin/env python3
"""End-to-end tests: drives ./elm through stdin exactly like a user, checks the console output,
writes Tests/results.json and a terminal-style screenshot per test in Tests/Screenshots/.
Run from the project folder:  python3 Tests/run_tests.py      (needs: Pillow)"""
import json, os, re, shutil, subprocess, sys
from PIL import Image, ImageDraw, ImageFont

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
WORK = os.path.join(ROOT, "Tests", "_work")
SHOTS = os.path.join(ROOT, "Tests", "Screenshots")
EXE = os.path.join(ROOT, "elm")
TODAY = "2026-10-15"
FONT = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf"

def run(lines, data, env_extra=None):
    env = dict(os.environ, ELM_TODAY=TODAY)
    env.update(env_extra or {})
    p = subprocess.run([EXE, "--data", data], input="\n".join(lines) + "\n", capture_output=True, text=True, env=env, timeout=30, cwd=WORK)
    return p.stdout + p.stderr, p.returncode

def fresh(name, demo=False):
    d = os.path.join(WORK, name)
    shutil.rmtree(d, ignore_errors=True)
    if demo: shutil.copytree(os.path.join(WORK, "_demo"), d)
    return d

def build_demo():
    d = os.path.join(WORK, "_demo"); shutil.rmtree(d, ignore_errors=True)
    out, _ = run(["8", "4", "y", "8", "1", "9"], d)
    assert "Saved" in out, out

# (id, operation, input summary, data-mode, lines, expected substrings, forbidden substrings)
T = []
def t(id_, op, inp, mode, lines, expect, forbid=()): T.append((id_, op, inp, mode, lines, expect, tuple(forbid)))

EXIT = ["9", "n"]
t("T01", "Add employee (valid)", "Krish Singh / Engineering / Developer / krish@acme.com / 2025-08-01", "empty",
  ["1", "1", "krish singh", "Engineering", "Developer", "krish@acme.com", "2025-08-01"] + EXIT, ["Added employee E001"])
t("T02", "Add employee: invalid e-mail", "e-mail 'bad' then valid", "empty",
  ["1", "1", "Test User", "HR", "Exec", "bad", "t@acme.com", "2025-01-01"] + EXIT, ["Enter a valid e-mail", "Added employee E001"])
t("T03", "Add employee: invalid name", "name 'R2D2' then valid", "empty",
  ["1", "1", "R2D2", "Test User", "HR", "Exec", "t@acme.com", "2025-01-01"] + EXIT, ["Name must be 2-40", "Added employee E001"])
t("T04", "Add employee: impossible date", "joining date 2026-02-30", "empty",
  ["1", "1", "Test User", "HR", "Exec", "t@acme.com", "2026-02-30", "2025-01-01"] + EXIT, ["Not a real calendar date", "Added employee E001"])
t("T05", "Add manager + holiday", "manager Priya Nair (HR), holiday 2026-12-25", "empty",
  ["1", "2", "Priya Nair", "HR", "HR Director", "y", "1", "3", "2026-12-25", "Christmas Day"] + EXIT, ["Added manager M001", "Added holiday H001"])
t("T06", "Add duplicate holiday date", "2026-12-25 twice", "empty",
  ["1", "3", "2026-12-25", "Christmas", "1", "3", "2026-12-25", "Xmas"] + EXIT, ["Added holiday H001", "already a holiday"])
t("T07", "Display all employees", "Display > Employees", "demo", ["2", "1"] + EXIT, ["Aarav Sharma", "Neha Gupta", "8 record(s)"])
t("T08", "Display everything (polymorphic)", "Display > Everything", "demo", ["2", "6"] + EXIT,
  ["[Employee]", "[Manager]", "[LeaveRequest]", "[Approval]", "[Holiday]"])
t("T09", "Search employee by department", "text 'sales'", "demo", ["3", "1", "sales"] + EXIT, ["Vikram Rao", "Sana Khan", "Arjun Verma", "3 record(s)"], ["Aarav"])
t("T10", "Search with no match", "text 'zzzz'", "demo", ["3", "1", "zzzz"] + EXIT, ["(no records)"])
t("T11", "Search requests overlapping a date range", "2026-10-26 .. 2026-10-30", "demo", ["3", "4", "2026-10-26", "2026-10-30"] + EXIT, ["R0001", "R0003"], ["R0008"])
t("T12", "Update employee", "E001: new designation 'Senior Engineer', others kept", "demo",
  ["4", "1", "E001", "", "", "Senior Engineer", "", "2", "1"] + EXIT, ["Updated E001", "Senior Engineer"])
t("T13", "Update employee: duplicate e-mail", "E001 e-mail -> diya@acme.com", "demo",
  ["4", "1", "E001", "", "", "", "diya@acme.com"] + EXIT, ["already belongs to E002"])
t("T14", "Update pending leave request", "R0005 -> 2026-11-09..2026-11-10", "demo",
  ["4", "3", "R0005", "2026-11-09", "2026-11-10", ""] + EXIT, ["Updated R0005 - now 2 working day(s)"])
t("T15", "Update an approved request (refused)", "R0001 is Approved", "demo",
  ["4", "3", "R0001", "2026-11-09", "2026-11-10", ""] + EXIT, ["only a Pending request can be edited"])
t("T16", "Delete employee with pending leave", "E003 has pending/approved leave", "demo", ["5", "1", "E003", "y"] + EXIT, ["still has"], ["Deleted."])
t("T17", "Delete employee (no active leave)", "add E009, then delete E009", "demo",
  ["1", "1", "Temp Person", "HR", "Exec", "temp@acme.com", "2025-01-01", "5", "1", "E009", "y"] + EXIT, ["Added employee E009", "Deleted. 0 history"])
t("T18", "Delete holiday", "H001 (New Year's Day)", "demo", ["5", "3", "H001", "y", "2", "5"] + EXIT, ["Deleted.", "9 record(s)"])
t("T19", "Apply for leave (valid)", "E001 AL 2026-11-16..2026-11-17 'Visit home'", "demo",
  ["6", "1", "E001", "AL", "2026-11-16", "2026-11-17", "Visit home"] + EXIT, ["submitted: 2 working day(s)", "Pending"])
t("T20", "Apply for leave: overlap", "E001 AL 2026-10-30 (already on R0001)", "demo",
  ["6", "1", "E001", "AL", "2026-10-30", "2026-10-30", "x"] + EXIT, ["overlap with Approved request R0001"])
t("T21", "Apply: insufficient balance", "11 days, then 5 more with only 4 left", "demo",
  ["6", "1", "E003", "AL", "2026-12-01", "2026-12-15", "Long trip", "6", "1", "E003", "AL", "2026-12-16", "2026-12-31", "More"] + EXIT,
  ["submitted: 11 working day(s)", "insufficient Annual Leave balance"])
t("T22", "Apply: notice period rule", "AL starting tomorrow (3 days notice needed)", "demo",
  ["6", "1", "E003", "AL", "2026-10-16", "2026-10-16", "x"] + EXIT, ["needs at least 3 day(s) notice"])
t("T23", "Apply: weekend only", "2026-10-17..2026-10-18 (Sat-Sun)", "demo",
  ["6", "1", "E003", "SL", "2026-10-17", "2026-10-18", "x"] + EXIT, ["no working day"])
t("T24", "Manager approves request", "R0003 by M001", "demo",
  ["6", "2", "R0003", "M001", "1", "Fine"] + EXIT, ["R0003 Approved"])
t("T25", "Manager rejects (reason mandatory)", "R0006 by M001; empty reason first", "demo",
  ["6", "2", "R0006", "M001", "2", "", "Team release week"] + EXIT, ["cannot be empty", "R0006 Rejected"])
t("T26", "Wrong-department manager", "Engineering head decides a Sales request", "demo",
  ["6", "2", "R0005", "M001", "1", ""] + EXIT, ["cannot decide for the Sales department"])
t("T27", "Department cover rule", "3rd Engineering approval on same day blocked", "demo",
  ["6", "1", "E002", "AL", "2026-10-29", "2026-10-30", "A", "6", "1", "E003", "AL", "2026-10-29", "2026-10-30", "B",
   "6", "2", "R0011", "M001", "1", "", "6", "2", "R0012", "M001", "1", ""] + EXIT, ["R0011 Approved", "cover rule"])
t("T28", "Cancel a pending request", "R0005", "demo", ["6", "3", "R0005", "y", "2", "3"] + EXIT, ["Cancelled. Balance restored", "Cancelled"])
t("T29", "Cancel an already rejected request", "R0004 is Rejected", "demo", ["6", "3", "R0004", "y"] + EXIT, ["cannot be cancelled"])
t("T30", "Check leave balance", "E001", "demo", ["6", "4", "E001"] + EXIT, ["Leave balance for Aarav Sharma", "AL Annual Leave"])
t("T31", "Report: department summary", "Reports > 2", "demo", ["7", "2"] + EXIT, ["Department summary", "Engineering", "Sales"])
t("T32", "Report: monthly calendar", "October 2026", "demo", ["7", "4", "2026", "10"] + EXIT, ["Leave calendar - October 2026", "Aarav Sharma"])
t("T33", "Report: who is on leave", "2026-10-30", "demo", ["7", "3", "2026-10-30"] + EXIT, ["Aarav Sharma", "Total on approved leave: 1"])
t("T34", "Report: overall summary", "Reports > 5", "demo", ["7", "5"] + EXIT, ["Approval rate", "Most leave taken"])
t("T35", "Export CSV", "Reports > 6 -> report.csv", "demo", ["7", "6", "report.csv"] + EXIT, ["Exported to report.csv"])
t("T36", "Audit log", "after adding an employee", "demo",
  ["1", "1", "Temp Person", "HR", "Exec", "temp@acme.com", "2025-01-01", "8", "1", "7", "7"] + EXIT, ["ADD_EMPLOYEE", "E009 Temp Person"])
t("T37", "Invalid menu input", "'abc' and '42'", "demo", ["abc", "42"] + EXIT, ["Enter a whole number between 1 and 9"])
t("T38", "Exit asks to save unsaved work", "add record then Exit > y", "empty",
  ["1", "1", "Temp Person", "HR", "Exec", "temp@acme.com", "2025-01-01", "9", "y"], ["Save before exit", "Saved.", "Goodbye"])
t("T39", "Input closed (Ctrl-D) auto-saves", "stream ends after an edit", "empty",
  ["1", "1", "Temp Person", "HR", "Exec", "temp@acme.com", "2025-01-01"], ["Input closed", "auto-saved"])

def png(path, title, status, text, inputs):
    text = re.sub(r"\n={78}\n  EMPLOYEE LEAVE.*?\n  5\. Delete Record\n", "\n  [ main menu ]\n", text, flags=re.S)
    lines = [l.rstrip() for l in text.splitlines()]
    lines = [l for l in lines if l.strip() != "" or True]
    if len(lines) > 56: lines = lines[:6] + ["  ... (output shortened) ..."] + lines[-48:]
    f = ImageFont.truetype(FONT, 15); w = 100 * 9 + 24
    lh = 19; h = 44 + lh * (len(lines) + 2) + 14
    im = Image.new("RGB", (w, h), (24, 26, 33)); d = ImageDraw.Draw(im)
    d.rectangle([0, 0, w, 30], fill=(48, 52, 64))
    for i, c in enumerate([(255, 95, 86), (255, 189, 46), (39, 201, 63)]): d.ellipse([12 + i * 22, 9, 24 + i * 22, 21], fill=c)
    d.text((90, 7), title, font=f, fill=(210, 214, 225))
    d.text((w - 100, 7), status, font=f, fill=(90, 220, 120) if status == "PASS" else (255, 90, 90))
    y = 40
    for l in lines:
        col = (230, 230, 235)
        if "!" in l[:6]: col = (255, 170, 90)
        elif re.search(r"(Added|Saved|Updated|Deleted|Approved|Rejected|Cancelled|submitted|Exported)", l): col = (120, 230, 140)
        elif l.startswith("==") or "[ main menu ]" in l: col = (120, 170, 255)
        d.text((12, y), l[:110], font=f, fill=col); y += lh
    im.save(path)

def main():
    os.makedirs(WORK, exist_ok=True); shutil.rmtree(SHOTS, ignore_errors=True); os.makedirs(SHOTS)
    if not os.path.exists(EXE): sys.exit("build first: make")
    build_demo()
    results = []
    for (id_, op, inp, mode, lines, expect, forbid) in T:
        d = fresh(id_, demo=(mode == "demo"))
        out, rc = run(lines, d)
        miss = [e for e in expect if e not in out]
        bad = [b for b in forbid if b in out]
        ok = not miss and not bad and rc == 0
        if id_ == "T35": ok = ok and os.path.exists(os.path.join(WORK, "report.csv")) and open(os.path.join(WORK, "report.csv")).readline().startswith("RequestID,")
        if id_ == "T38" or id_ == "T39":                      # data must really be on disk afterwards
            out2, _ = run(["2", "1"] + EXIT, d); ok = ok and "Temp Person" in out2; out += "\n--- program restarted: Display > Employees ---\n" + out2
        results.append(dict(id=id_, operation=op, input=inp, expected=" ; ".join(expect), actual=("matched: " + " ; ".join(expect)) if ok else f"MISSING {miss} UNEXPECTED {bad}",
                            status="PASS" if ok else "FAIL"))
        png(os.path.join(SHOTS, f"{id_}_{'PASS' if ok else 'FAIL'}.png"), f"{id_}  {op}", "PASS" if ok else "FAIL", out, lines)
        print(f"{id_} {'PASS' if ok else 'FAIL'}  {op}")
        if not ok: print("   ", miss, bad, rc); print(out[-1500:])
    # persistence across two separate program runs
    d = fresh("T40"); run(["1", "1", "Krish Singh", "Engineering", "Developer", "krish@acme.com", "2025-08-01", "8", "1", "9"], d)
    out, _ = run(["2", "1"] + EXIT, d)
    ok = "Krish Singh" in out and "1 record(s)" in out
    results.append(dict(id="T40", operation="Persistence across restarts", input="run 1: add + save, quit; run 2: display", expected="Krish Singh ; 1 record(s)", actual="matched" if ok else "FAIL", status="PASS" if ok else "FAIL"))
    png(os.path.join(SHOTS, f"T40_{'PASS' if ok else 'FAIL'}.png"), "T40  Persistence across restarts (second run)", "PASS" if ok else "FAIL", out, [])
    print("T40", "PASS" if ok else "FAIL")
    # corrupt file warning
    d = fresh("T41", demo=True)
    with open(os.path.join(d, "employees.dat"), "a") as f: f.write("junk line\n")
    out, _ = run(EXIT, d); ok = "warning(s)" in out and "skipped" in out
    results.append(dict(id="T41", operation="Corrupt data file", input="junk line appended to employees.dat", expected="warning(s) ; skipped", actual="matched" if ok else "FAIL", status="PASS" if ok else "FAIL"))
    png(os.path.join(SHOTS, f"T41_{'PASS' if ok else 'FAIL'}.png"), "T41  Corrupt data file is reported, not fatal", "PASS" if ok else "FAIL", out, [])
    print("T41", "PASS" if ok else "FAIL")
    json.dump(results, open(os.path.join(ROOT, "Tests", "results.json"), "w"), indent=1)
    n = sum(r["status"] == "PASS" for r in results)
    print(f"\n{n}/{len(results)} end-to-end tests passed"); sys.exit(0 if n == len(results) else 1)

main()
