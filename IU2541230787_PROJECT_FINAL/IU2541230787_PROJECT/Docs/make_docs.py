#!/usr/bin/env python3
"""Builds the filled Project Report and Submission Template (.docx). Test rows come from Tests/results.json."""
import json, os
from docx import Document
from docx.shared import Pt, Inches, RGBColor
from docx.enum.text import WD_ALIGN_PARAGRAPH

R = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
res = {r["id"]: r for r in json.load(open(f"{R}/Tests/results.json"))}
NAME, ENR, DATE = "Krish Singh", "IU2541230787", "15 October 2026"
PICK = ["T01", "T02", "T07", "T09", "T12", "T14", "T16", "T19", "T20", "T21", "T24", "T25", "T27", "T28", "T40"]   # 15 required tests

def base():
    d = Document(); s = d.styles["Normal"]; s.font.name = "Calibri"; s.font.size = Pt(11)
    for sec in d.sections: sec.left_margin = sec.right_margin = Inches(0.9); sec.top_margin = sec.bottom_margin = Inches(0.8)
    return d
def title(d, t, sub):
    p = d.add_paragraph(); r = p.add_run(t); r.bold = True; r.font.size = Pt(16)
    p = d.add_paragraph(); r = p.add_run(sub); r.bold = True; r.font.size = Pt(14)
def table(d, head, rows, widths=None, size=9):
    t = d.add_table(rows=1, cols=len(head)); t.style = "Table Grid"
    for i, h in enumerate(head):
        c = t.rows[0].cells[i]; c.text = ""; r = c.paragraphs[0].add_run(h); r.bold = True; r.font.size = Pt(size)
    for row in rows:
        cells = t.add_row().cells
        for i, v in enumerate(row):
            cells[i].text = ""; cells[i].paragraphs[0].add_run(str(v)).font.size = Pt(size)
    if widths:
        for row in t.rows:
            for i, w in enumerate(widths): row.cells[i].width = Inches(w)
    d.add_paragraph(); return t
def para(d, text): return d.add_paragraph(text)
def bullets(d, items):
    for i in items: d.add_paragraph(i, style="List Bullet")
def img(d, name, cap, w=6.7):
    d.add_picture(f"{R}/UML/{name}", width=Inches(w)); d.paragraphs[-1].alignment = WD_ALIGN_PARAGRAPH.CENTER
    p = d.add_paragraph(cap); p.alignment = WD_ALIGN_PARAGRAPH.CENTER; p.runs[0].italic = True

CLASSES = [  # class, data members, member functions, responsibility
 ("Record (abstract)", "id_", "id(), kind(), display(), row(), serialize(), matches()", "Common root of every stored entity; forces each entity to know how to show and save itself."),
 ("Person (abstract)", "name_, department_", "setName(), setDepartment(), role(), matches()", "Shared, validated identity of Employee and Manager."),
 ("Employee", "designation_, email_, joined_", "Employee(), Employee(...), setEmail(), display(), serialize(), fromFields()", "A staff member who can apply for leave."),
 ("Manager", "title_, hr_", "Manager(), authorises(Employee), role(), display()", "Approves/rejects leave; HR managers may decide for every department."),
 ("LeaveRequest", "employeeId_, type_, from_, to_, applied_, reason_, days_, status_", "LeaveRequest(), calculateDays(), countWorkingDays(), overlaps(), covers(), setStatus(), display()", "One leave application and its life-cycle (Pending, Approved, Rejected, Cancelled)."),
 ("Approval", "requestId_, managerId_, decision_, comment_, decidedOn_", "Approval(), approve(), reject(), display()", "A manager's recorded decision (audit trail) on one request."),
 ("Holiday", "date_, name_", "Holiday(), setDate(), setName(), display()", "Public holiday - not counted as a leave day."),
 ("LeavePolicy + 5 subclasses", "(none; rules are virtual functions)", "code(), yearlyQuota(), maxConsecutive(), minNoticeDays(), maxBackdateDays(), paid(), check()", "AnnualLeave, SickLeave, CasualLeave, ParentalLeave, UnpaidLeave each define their own limits (polymorphism)."),
 ("LeaveSystem", "fm_, audit_, 5 x PtrList, policies_ (LeavePolicy**), today_, dirty_", "addX(), searchX(), updateX(), deleteX(), applyLeave(), decide(), cancelLeave(), balance(), reports, save(), load()", "Controller: owns all data and enforces every business rule; never reads the keyboard."),
 ("ConsoleUI", "sys_ (LeaveSystem&)", "run(), addMenu() ... exitMenu(), applyForLeave(), reviewQueue()", "Menus and prompts only; catches exceptions and prints friendly messages."),
 ("FileManager", "dir_", "save<T>(), readLines(), backup(), writeLines(), checksum()", "Atomic file save/load with header, record count and checksum."),
 ("PtrList<T>", "data_ (T**), size_, cap_", "add(), operator[], removeAt(), clear(), ~PtrList()", "Owning dynamic array that doubles when full and deletes its objects."),
 ("AuditLog / Date", "path_ / y_, m_, d_", "log(), tail() / weekday(), addDays(), parse(), today()", "Change history file; calendar arithmetic written from scratch."),
]

def report():
    d = base()
    title(d, f"{ENR} - CPP74 - PROJECT REPORT", "Employee Leave Management")
    for l in [f"Enrollment No.: {ENR}", f"Student Name: {NAME}", "Semester: 3", "Subject: C++ Guided Project"]: para(d, l)
    d.add_heading("1. Introduction", 1)
    para(d, "Employee Leave Management is a console-based, object-oriented C++17 application that replaces paper or e-mail leave requests. Employees apply for leave, managers approve or reject, and the system automatically counts working days, checks leave balances and enforces company rules. All data is stored in files and restored when the program is started again.")
    d.add_heading("2. Problem Statement", 1)
    para(d, "Manual leave handling leads to miscounted days (weekends and holidays), double-booked dates, overdrawn leave balances, too many people absent from one team, and no record of who approved what. The project provides a single program that validates every request against clear rules and keeps a permanent, auditable record.")
    d.add_heading("3. Objectives", 1)
    bullets(d, ["Model employees, managers, leave requests, approvals and holidays as cooperating classes.",
        "Demonstrate classes, encapsulation, constructors/destructors, inheritance, runtime polymorphism, dynamic memory, file handling, validation and exception handling.",
        "Enforce realistic rules: yearly quotas, notice periods, back-dating, overlap detection, department cover limit.",
        "Provide Add / Display / Search / Update / Delete, the leave process, reports and Save / Load through the required 9-option menu.",
        "Prove correctness with automated tests (41 end-to-end, 84 unit checks, 9 compile-fail proofs) and memory-sanitizer runs."])
    d.add_heading("4. Functional Requirements", 1)
    bullets(d, ["FR1 Add employees, managers, public holidays and leave requests with full input validation (name, e-mail, real calendar dates).",
        "FR2 Display each record type as a table, and 'Everything' through one polymorphic loop.",
        "FR3 Search employees/managers/holidays/requests by text, and requests overlapping a date range.",
        "FR4 Update employees, managers, holidays and Pending requests (days and balance recalculated).",
        "FR5 Delete employees, managers and holidays, protected by referential rules (no deletion with active leave or recorded decisions).",
        "FR6 Leave process: apply, approve/reject by an authorised manager, cancel, check balance.",
        "FR7 Reports: employee balance, department summary, who is on leave on a date, monthly calendar, overall summary, CSV export, audit log.",
        "FR8 Save, reload, backup, load demo data; automatic load at start and save prompt at exit.",
        "FR9 Business rules: Annual 18 days/yr (3 days notice), Sick 10 (back-date 7), Casual 7 (max 3 days), Parental 30 (14 days notice), Unpaid unlimited (7 days notice); no overlap; at most half of a department away per day; rejection needs a reason."])
    d.add_heading("5. System / Project Flow", 1)
    para(d, "USER -> MAIN MENU (ConsoleUI) -> LeaveSystem (rules) -> Entity classes (Employee, Manager, LeaveRequest, Approval, Holiday, LeavePolicy) -> FileManager -> DATA FILES (Data/*.dat, audit.log)")
    para(d, "Start: load existing records -> user performs operations (kept in memory, each change written to the audit log) -> Save (atomic write of five .dat files) -> Exit. main.cpp only creates the LeaveSystem and ConsoleUI objects and calls run().")
    d.add_heading("6. Class Descriptions", 1)
    table(d, ["Class", "Data Members", "Member Functions", "Responsibility"], CLASSES, [1.2, 1.7, 2.0, 2.0], 8)
    d.add_heading("7. OOP Concepts Used", 1)
    bullets(d, ["Classes & objects: 13 classes (5 entities, 6 policies, controller, UI, file manager, container, date, audit log).",
        "Encapsulation: all data members are private (Record::id_ has no setter); changes go through validating setters. Compile-fail test proves private access is rejected.",
        "Constructors: default and parameterized constructors on all entities; Date validates in its constructor.",
        "Destructors: virtual ~Record() and ~LeavePolicy(); ~PtrList() and ~LeaveSystem() release dynamic memory.",
        "Inheritance: Record -> Person -> Employee/Manager; Record -> LeaveRequest/Approval/Holiday; LeavePolicy -> 5 leave types (multi-level).",
        "Runtime polymorphism: Record* calls display()/row()/serialize(); LeavePolicy* calls quota/notice/limit functions that differ per leave type.",
        "Dynamic memory: PtrList<T> (new T*[], doubling, delete[]), LeavePolicy** array, objects created with new and owned by lists; unique_ptr guards against leaks when a rule throws. LeakSanitizer reports zero leaks.",
        "Exception handling: ElmError hierarchy (Validation, Rule, NotFound, File, Parse); failed operations leave data unchanged.",
        "Templates and STL: PtrList<T>, FileManager::save<T>, std::vector, std::map, std::sort, std::filesystem."])
    d.add_heading("8. File Handling and Data Persistence", 1)
    para(d, "Five record files (employees.dat, managers.dat, requests.dat, approvals.dat, holidays.dat) plus audit.log live in Data/. Each .dat starts with a header '#ELM1 <kind> <count> <checksum>' followed by one tab-separated line per record (tabs/newlines escaped). Save writes <file>.tmp and renames it, so a crash never leaves a half-written file. Load parses every line; a corrupt line is skipped with a warning, a wrong header raises FileError, a missing file means first run, and record-count/checksum mismatches and dangling references are reported. ID counters are rebuilt from the loaded data. Test T40 closes and reopens the program and confirms the data is still present.")
    d.add_heading("9. UML Diagrams", 1)
    img(d, "Use_Case_Diagram.png", "Figure 1 - Use Case Diagram (3 actors, 14 use cases)")
    img(d, "Class_Diagram.png", "Figure 2 - Class Diagram")
    img(d, "Sequence_Diagram_1.png", "Figure 3 - Sequence Diagram 1: apply for leave and approve")
    img(d, "Sequence_Diagram_2.png", "Figure 4 - Sequence Diagram 2: cancel a request and save")
    d.add_heading("10. Implementation Details", 1)
    bullets(d, ["Source: 13 .cpp and 17 headers in src/ and include/elm/, about 2,300 lines including tests; compiled with -std=c++17 -Wall -Wextra -Wpedantic -Wold-style-cast -Wshadow -Werror (zero warnings).",
        "Working days: LeaveRequest::calculateDays walks the date range, skipping Saturday/Sunday and holidays; the result is stored so later holiday edits cannot change approved history.",
        "Date class: own implementation of civil-calendar day numbers; leap years, weekday and date arithmetic are unit-tested.",
        "Balance is derived, not stored: sum of Approved (used) and Pending days per type per year, so cancelling a request automatically restores the balance.",
        "Cover rule: on approval, every working day of the request is checked; no more than half of the employee's department (minimum 1) may be on approved leave.",
        "Safety: all multi-step changes validate first and modify data last; decide() builds the Approval before touching the request status.",
        "Testing tools: Tests/run_tests.py (stdin-driven end-to-end), Tests/unit_tests.cpp (ASan + UBSan), Tests/compile_fail_checks.sh."])
    d.add_heading("11. Testing and Results", 1)
    para(d, "The 15 required test cases are listed below (suite ID in brackets). The complete suite of 41 end-to-end tests is in Tests/results.json with a screenshot per test in Tests/Screenshots/; in addition 84 unit checks (Tests/unit_tests.cpp) and 9 compile-fail proofs were run.")
    rows = [[f"T{i+1:02d}", f"{res[k]['operation']} [{k}] - {res[k]['input']}", res[k]["expected"], "As expected" if res[k]["status"] == "PASS" else "Mismatch", res[k]["status"]] for i, k in enumerate(PICK)]
    table(d, ["Test ID", "Input / Operation", "Expected", "Actual", "Status"], rows, [0.5, 2.6, 2.2, 0.9, 0.6], 8)
    n = sum(r["status"] == "PASS" for r in res.values())
    para(d, f"Summary: {n}/{len(res)} end-to-end tests PASS; 84/84 unit checks PASS (no memory errors or leaks); 9/9 compile-fail proofs PASS.")
    d.add_heading("12. Sample Screens / Output", 1)
    for k, c in [("T19", "Applying for leave"), ("T27", "Department cover rule blocking an approval"), ("T32", "Monthly leave calendar"), ("T40", "Data still present after restart")]:
        d.add_picture(f"{R}/Tests/Screenshots/{k}_PASS.png", width=Inches(6.2)); d.paragraphs[-1].alignment = WD_ALIGN_PARAGRAPH.CENTER
        p = d.add_paragraph(f"{c} ({k})"); p.alignment = WD_ALIGN_PARAGRAPH.CENTER; p.runs[0].italic = True
    d.add_heading("13. Limitations", 1)
    bullets(d, ["Console interface only; no login/password - the manager enters their own ID when deciding.", "Half-day leave and leave that crosses a year end are not supported (split into two requests).", "Weekends are fixed as Saturday/Sunday; no per-employee shift calendars.", "Text files are used instead of a database, so there is no multi-user concurrency."])
    d.add_heading("14. Future Scope", 1)
    bullets(d, ["Login with roles and password hashing.", "Half-day leave, carry-forward of unused balance, pro-rata quota for new joiners.", "E-mail notification to managers and employees.", "Replace files by SQLite and add a GUI or web front end."])
    d.add_heading("15. Conclusion", 1)
    para(d, "The project delivers a complete leave-management workflow with clean separation between data classes, business rules, user interface and storage. It demonstrates every required C++ concept in a working program, goes beyond the blueprint with realistic HR rules, reports, audit logging and robust file handling, and is backed by automated tests and sanitizer runs.")
    d.save(f"{R}/Docs/{ENR}_CPP74_Project_Report.docx")

def submission():
    d = base()
    title(d, f"{ENR} - CPP74 - SUBMISSION TEMPLATE", "")
    for l in ["Project Name: Employee Leave Management", f"Enrollment No.: {ENR}", f"Student Name: {NAME}", f"Date of Submission: {DATE}"]: para(d, l)
    d.add_heading("1. Project Summary", 1)
    p = para(d, ""); p.add_run("Problem Statement: ").bold = True
    p.add_run("Manual leave handling causes miscounted days, double bookings, overdrawn balances and no audit trail. A console program is needed to apply, approve and track leave with automatic rule checking and permanent storage.")
    p = para(d, ""); p.add_run("Objectives: ").bold = True
    p.add_run("Apply OOP (inheritance, polymorphism, dynamic memory, file handling, exceptions) to build a reliable leave system with the required 9-option menu, UML documentation and a tested implementation.")
    p = para(d, ""); p.add_run("Key Features: ").bold = True
    bullets(d, ["Employees, managers, holidays, leave requests and approvals with full validation.", "Five leave types as polymorphic policies (quota, notice, back-dating, max days).", "Working-day counting that skips weekends and holidays; overlap and balance checks.", "Manager authorisation by department and a department cover rule.", "Reports: balance, department summary, who is on leave, monthly calendar, summary, CSV export, audit log.", "Atomic file saving with checksum, corrupt-line recovery, backup and demo data.", "41 end-to-end tests, 84 unit checks with ASan/UBSan, 9 compile-fail proofs."])
    d.add_heading("2. Class Summary", 1)
    rows = [("Record / Person", "Abstract roots: identity, validation, virtual display/serialize", "id_, name_, department_", "display(), serialize(), role(), matches()"),
            ("Employee", "Staff member who applies for leave", "designation_, email_, joined_", "Employee(), setEmail(), display()"),
            ("Manager", "Decides leave; HR may decide for all departments", "title_, hr_", "authorises(), role()"),
            ("LeaveRequest", "One leave application and its status", "employeeId_, type_, from_, to_, days_, status_", "calculateDays(), overlaps(), setStatus()"),
            ("Approval", "Manager's decision record", "requestId_, managerId_, decision_, comment_", "approve(), reject()"),
            ("LeaveSystem", "Controller: rules, data ownership, reports", "5 PtrList members, policies_, fm_, audit_", "applyLeave(), decide(), cancelLeave(), balance(), report functions, save(), load()"),
            ("FileManager", "Atomic save/load with checksum", "dir_", "save<T>(), readLines(), backup()"),
            ("LeavePolicy (+5)", "Leave-type rules via virtual functions", "-", "yearlyQuota(), maxConsecutive(), minNoticeDays(), check()"),
            ("PtrList<T>, Holiday, ConsoleUI, AuditLog, Date", "Dynamic array; public holiday; menus; change log; calendar", "data_/size_/cap_; date_/name_; sys_; path_; y_/m_/d_", "add(), removeAt(); run(); log(); addDays()")]
    table(d, ["Class", "Purpose / Responsibility", "Key Data Members", "Key Member Functions"], rows, [1.3, 2.0, 1.9, 1.9], 8)
    d.add_heading("3. Project Flow", 1)
    para(d, "USER -> ConsoleUI (main menu) -> LeaveSystem (rules) -> Employee / Manager / LeaveRequest / Approval / Holiday objects -> FileManager -> FILE (Data/*.dat)")
    d.add_heading("4. UML Deliverables", 1)
    table(d, ["Diagram", "File Name", "Completed"], [("Use Case Diagram", "UML/Use_Case_Diagram.png", "☑"), ("Class Diagram", "UML/Class_Diagram.png", "☑"), ("Sequence Diagram 1", "UML/Sequence_Diagram_1.png", "☑"), ("Sequence Diagram 2", "UML/Sequence_Diagram_2.png", "☑")], [2, 3.2, 1])
    d.add_heading("5. C++ Concepts Demonstrated", 1)
    table(d, ["Concept", "Where Used in My Project", "Explanation"], [
        ("Encapsulation", "All classes; Person::name_, Record::id_", "Data is private; only validating setters change it. No setter exists for id."),
        ("Constructor", "Employee, Manager, LeaveRequest, Approval, Holiday, Date", "Default and parameterized constructors; constructors validate input and throw on bad data."),
        ("Destructor", "Record, LeavePolicy (virtual); PtrList, LeaveSystem", "Virtual destructors make delete through base pointers safe; PtrList/LeaveSystem free their dynamic memory."),
        ("Inheritance", "Record -> Person -> Employee/Manager; Record -> LeaveRequest/Approval/Holiday; LeavePolicy -> 5 types", "Common behaviour in base classes, specialised in derived classes."),
        ("Runtime Polymorphism", "Display > Everything (Record*); LeavePolicy::check()", "One loop over Record* prints five different kinds; each leave type returns its own limits."),
        ("Dynamic Memory", "PtrList<T>::grow(), LeaveSystem::policies_", "new T*[2*cap] and delete[]; objects created with new and deleted by the owning list; ASan reports no leaks."),
        ("File Handling", "FileManager, LeaveSystem::save()/load()", "Tab-separated .dat files with header+checksum, atomic rename, load at start, corrupt lines skipped."),
        ("Exception Handling", "errors.h, ConsoleUI::guarded()", "ValidationError, RuleError, NotFoundError, FileError, ParseError caught at the menu; data stays consistent.")], [1.3, 2.4, 3.4], 8)
    d.add_heading("6. Testing", 1)
    rows = [[f"T{i+1:02d}", res[k]["operation"], res[k]["input"], res[k]["expected"], "As expected" if res[k]["status"] == "PASS" else "Mismatch", "Pass" if res[k]["status"] == "PASS" else "Fail"] for i, k in enumerate(PICK)]
    table(d, ["Test ID", "Operation", "Input", "Expected Output", "Actual Output", "Pass/Fail"], rows, [0.5, 1.5, 1.7, 1.7, 0.8, 0.5], 7)
    d.add_heading("7. Deployment", 1)
    for l in ["Compiler / IDE Used: g++ 13.3 (C++17), any text editor / VS Code", "Compilation Command: g++ -std=c++17 -Iinclude -o elm src/*.cpp   (or: make)", "Executable Name: elm  (elm.exe on Windows via BUILD_WINDOWS.bat)", "Data File(s): Data/employees.dat, managers.dat, requests.dat, approvals.dat, holidays.dat, audit.log"]: para(d, l)
    d.add_heading("8. Student Declaration", 1)
    para(d, "I understand the submitted project and can explain its classes, data members, member functions, file handling and UML diagrams.")
    para(d, "Student Signature: ____________________")
    d.save(f"{R}/Docs/{ENR}_CPP74_Submission_Template.docx")

report(); submission(); print("docs written")
