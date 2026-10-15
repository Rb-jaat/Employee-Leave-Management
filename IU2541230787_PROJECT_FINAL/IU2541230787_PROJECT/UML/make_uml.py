#!/usr/bin/env python3
"""Draws the 4 UML diagrams as PNG files (class + use case with Graphviz, sequence with matplotlib).
Run:  python3 UML/make_uml.py"""
import os, subprocess
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle

HERE = os.path.dirname(os.path.abspath(__file__))
OWNER = "Employee Leave Management - Krish Singh - IU2541230787"

def dot_png(src, name):
    open(os.path.join(HERE, name + ".dot"), "w").write(src)
    subprocess.run(["dot", "-Tpng", "-Gdpi=110", os.path.join(HERE, name + ".dot"), "-o", os.path.join(HERE, name + ".png")], check=True)

# ------------------------------------------------------------------ class diagram
def esc(t): return t.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")
def cls(name, attrs, methods, head="#2f5d8a", stereo="", italic=False):
    n = f"<I>{esc(name)}</I>" if italic else f"<B>{esc(name)}</B>"
    st = f'<FONT POINT-SIZE="9">{stereo}</FONT><BR/>' if stereo else ""
    a = "".join(f'<TR><TD ALIGN="LEFT" BORDER="0"><FONT POINT-SIZE="10">{esc(x)}</FONT></TD></TR>' for x in attrs) or '<TR><TD BORDER="0"> </TD></TR>'
    m = "".join(f'<TR><TD ALIGN="LEFT" BORDER="0"><FONT POINT-SIZE="10">{esc(x)}</FONT></TD></TR>' for x in methods) or '<TR><TD BORDER="0"> </TD></TR>'
    key = name.replace("<", "_").replace(">", "").replace("*", "P")
    return (f'{key} [label=<<TABLE BORDER="1" CELLBORDER="0" CELLSPACING="0" CELLPADDING="3" BGCOLOR="white" COLOR="#444444">'
            f'<TR><TD BGCOLOR="{head}"><FONT COLOR="white">{st}{n}</FONT></TD></TR>'
            f'<TR><TD><TABLE BORDER="0" CELLSPACING="0" CELLPADDING="1">{a}</TABLE></TD></TR><HR/>'
            f'<TR><TD><TABLE BORDER="0" CELLSPACING="0" CELLPADDING="1">{m}</TABLE></TD></TR></TABLE>>];')

def class_diagram():
    C = ['digraph G { rankdir=BT; nodesep=0.3; ranksep=0.8; bgcolor="white"; labelloc=t; fontsize=18; fontname="Helvetica";'
         f' label="Class Diagram - {OWNER}"; node [shape=plain, fontname="Helvetica"]; edge [fontname="Helvetica", fontsize=9];']
    C += [
     cls("Record", ["- id_ : string"], ["+ id() : string", "+ kind() : string {abstract}", "+ display(os) : void {abstract}", "+ row() : string {abstract}",
          "+ serialize() : string {abstract}", "+ matches(text) : bool", "+ ~Record() {virtual}"], "#6a3fb5", "«abstract»", True),
     cls("Person", ["- name_ : string", "- department_ : string"], ["+ name() / department()", "+ setName(n) / setDepartment(d)", "+ role() : string {abstract}", "+ matches(text) : bool"], "#6a3fb5", "«abstract»", True),
     cls("Employee", ["- designation_ : string", "- email_ : string", "- joined_ : Date"], ["+ Employee()", "+ Employee(id,name,dept,desig,email,joined)", "+ setDesignation() / setEmail()",
          "+ display() / row() / serialize()", "+ fromFields(f) : Employee* {static}"]),
     cls("Manager", ["- title_ : string", "- hr_ : bool"], ["+ Manager()", "+ Manager(id,name,dept,title,hr)", "+ authorises(Employee) : bool", "+ role() : string", "+ display() / row() / serialize()"]),
     cls("LeaveRequest", ["- employeeId_ : string", "- type_ : string (policy code)", "- from_, to_, applied_ : Date", "- reason_ : string", "- days_ : int", "- status_ : Status"],
         ["+ LeaveRequest()", "+ LeaveRequest(id,emp,type,from,to,reason,applied)", "+ calculateDays(holidays) : int", "+ countWorkingDays(from,to,h) {static}", "+ overlaps(a,b) : bool", "+ covers(d) : bool",
          "+ setStatus(s) / setDates() / setReason()", "+ display() / row() / serialize()"]),
     cls("Approval", ["- requestId_ : string", "- managerId_ : string", "- decision_ : Decision", "- comment_ : string", "- decidedOn_ : Date"],
         ["+ Approval(id,requestId,managerId)", "+ approve(comment, on) : void", "+ reject(comment, on) : void", "+ display() / row() / serialize()"]),
     cls("Holiday", ["- date_ : Date", "- name_ : string"], ["+ Holiday(id,date,name)", "+ setDate() / setName()", "+ display() / row() / serialize()"]),
     cls("LeavePolicy", [], ["+ code() / name() : string {abstract}", "+ yearlyQuota() : int {abstract}", "+ maxConsecutive() : int {abstract}", "+ minNoticeDays() : int", "+ maxBackdateDays() : int", "+ paid() : bool",
          "+ check(from,to,days,today) : void", "+ describe() : string"], "#b5651d", "«abstract»", True),
     cls("AnnualLeave", [], ["quota 18, notice 3d"], "#b5651d"), cls("SickLeave", [], ["quota 10, back-date 7d"], "#b5651d"),
     cls("CasualLeave", [], ["quota 7, max 3 days"], "#b5651d"), cls("ParentalLeave", [], ["quota 30, notice 14d"], "#b5651d"),
     cls("UnpaidLeave", [], ["unlimited, unpaid"], "#b5651d"),
     cls("PtrList<T>", ["- data_ : T**", "- size_ : int", "- cap_ : int"], ["+ add(T*) : void", "+ operator[](i) : T*", "+ removeAt(i) : bool", "+ size() / capacity() : int", "+ ~PtrList()  (delete[])", "+ PtrList(const PtrList&) = delete"], "#2e7d32", "«template»"),
     cls("FileManager", ["- dir_ : string"], ["+ save<T>(file, list, kind) : int", "+ readLines(file, kind, ok, warnings)", "+ backup(stamp) : string", "- writeLines() / checksum()"], "#00796b"),
     cls("AuditLog", ["- path_ : string"], ["+ log(date, action, detail)", "+ tail(n) : vector<string>"], "#00796b"),
     cls("LeaveSystem", ["- fm_ : FileManager", "- audit_ : AuditLog", "- employees_ : PtrList<Employee>", "- managers_ : PtrList<Manager>", "- requests_ : PtrList<LeaveRequest>",
          "- approvals_ : PtrList<Approval>", "- holidays_ : PtrList<Holiday>", "- policies_ : LeavePolicy**", "- today_ : Date"],
         ["+ addEmployee() / addManager() / addHoliday()", "+ searchEmployees() / searchRequests() ...", "+ updateEmployee() / updateRequest() ...", "+ deleteEmployee() / deleteManager() ...",
          "+ applyLeave(emp,type,from,to,reason) : LeaveRequest&", "+ decide(req,mgr,approve,comment) : Approval&", "+ cancelLeave(req) : void", "+ balance(emp,type,year) : Balance",
          "+ balanceReport() / departmentReport() / monthCalendar()", "+ save() / load() / backup() / loadDemoData()"], "#c62828"),
     cls("ConsoleUI", ["- sys_ : LeaveSystem&"], ["+ run() : int", "- addMenu() / displayMenu() / searchMenu()", "- updateMenu() / deleteMenu() / processMenu()", "- reportMenu() / fileMenu() / exitMenu()"], "#455a64"),
     cls("Date", ["- y_, m_, d_ : int"], ["+ Date(y,m,d)", "+ today() / parse() / fromSerial() {static}", "+ weekday() / isWeekend()", "+ addDays(n) : Date", "+ operator- / < / ==", "+ str() : string"], "#455a64"),
    ]
    inh = 'edge [arrowhead=empty, color="#333333"];'
    C.append(inh)
    for a, b in [("Person", "Record"), ("Employee", "Person"), ("Manager", "Person"), ("LeaveRequest", "Record"), ("Approval", "Record"), ("Holiday", "Record"),
                 ("AnnualLeave", "LeavePolicy"), ("SickLeave", "LeavePolicy"), ("CasualLeave", "LeavePolicy"), ("ParentalLeave", "LeavePolicy"), ("UnpaidLeave", "LeavePolicy")]:
        C.append(f"{a} -> {b};")
    C.append('edge [arrowhead=none, arrowtail=diamond, dir=back, color="#c62828", fontcolor="#c62828"];')
    for tgt, lab in [("PtrList_T", '"5 lists  1..*"'), ("LeavePolicy", '"policies_[5]  5"'), ("FileManager", '"1"'), ("AuditLog", '"1"')]:
        C.append(f'{tgt} -> LeaveSystem [label={lab}];')
    C.append('edge [arrowhead=vee, arrowtail=none, dir=forward, color="#1565c0", fontcolor="#1565c0", style=solid];')
    C.append('LeaveSystem -> ConsoleUI [dir=back, label="uses (reference)"];')
    C.append('edge [arrowhead=vee, color="#1565c0", fontcolor="#1565c0", style=solid];')
    C.append('LeaveRequest -> Employee [label="applied by\\n*  ->  1", constraint=false];')
    C.append('Approval -> LeaveRequest [label="decides\\n1  ->  1", constraint=false];')
    C.append('Approval -> Manager [label="decided by\\n* -> 1", constraint=false];')
    C.append('edge [style=dashed, arrowhead=vee, color="#777777", fontcolor="#777777"];')
    C.append('Employee -> Date [label="joined_"]; LeaveRequest -> Date [label="from_/to_"]; Holiday -> Date; LeaveRequest -> LeavePolicy [label="type_ code (lookup)"]; FileManager -> Record [label="serialize()"];')
    C.append("}")
    dot_png("\n".join(C), "Class_Diagram")

# ------------------------------------------------------------------ use case diagram
def use_case():
    S = ['digraph U { rankdir=LR; bgcolor="white"; fontname="Helvetica"; labelloc=t; fontsize=18; nodesep=0.28; ranksep=1.6;'
         f' label="Use Case Diagram - {OWNER}"; node [fontname="Helvetica", fontsize=11];']
    for a, c in [("Employee", "#2f5d8a"), ("Manager", "#2e7d32"), ("HRAdmin", "#c62828")]:
        lab = {"Employee": "Employee", "HRAdmin": "HR Administrator", "Manager": "Manager"}[a]
        S.append(f'{a} [shape=box, style="rounded,filled", fillcolor="#f4f6fa", color="{c}", penwidth=2, label=<<FONT POINT-SIZE="22">&#9786;</FONT><BR/><B>{lab}</B><BR/><FONT POINT-SIZE="9">«actor»</FONT>>];')
    S.append('subgraph cluster_sys { label="Employee Leave Management System"; style="rounded"; color="#555555"; fontsize=14; node [shape=ellipse, style=filled, fillcolor="#fff8e1", color="#b58900"];')
    uc = {"UC1": "Apply for leave", "UC2": "Check leave balance", "UC3": "Cancel leave request", "UC4": "Search / view requests",
          "UC5": "Approve or reject request", "UC6": "View who is on leave /\\nmonthly calendar", "UC7": "Manage employees\\n(add/update/delete)", "UC8": "Manage managers",
          "UC9": "Manage public holidays", "UC10": "Generate reports /\\nexport CSV", "UC11": "Save / load / backup data", "UC12": "View audit log",
          "UC13": "Validate leave rules\\n(notice, balance, overlap)", "UC14": "Check department\\ncover rule"}
    for k, v in uc.items(): S.append(f'{k} [label="{v}"];')
    S.append("}")
    S.append('edge [arrowhead=none, color="#444444"];')
    for a, ks in {"Employee": ["UC1", "UC2", "UC3", "UC4"], "Manager": ["UC5", "UC6", "UC4", "UC10"], "HRAdmin": ["UC7", "UC8", "UC9", "UC10", "UC11", "UC12", "UC6"]}.items():
        for k in ks: S.append(f"{a} -> {k};")
    S.append('edge [style=dashed, arrowhead=vee, fontsize=10];')
    S.append('UC1 -> UC13 [label="«include»"]; UC5 -> UC14 [label="«include»"]; UC3 -> UC2 [label="«extend»", dir=back];')
    S.append("}")
    dot_png("\n".join(S), "Use_Case_Diagram")

# ------------------------------------------------------------------ sequence diagrams
def sequence(fname, title, lifelines, msgs, notes=()):
    n = len(lifelines); W = max(15, n * 2.5); H = 1.15 + 0.62 * len(msgs)
    fig, ax = plt.subplots(figsize=(W, H)); ax.set_xlim(0, n); ax.set_ylim(0, len(msgs) + 3); ax.axis("off")
    ax.invert_yaxis()
    ax.text(n / 2, 0.15, f"{title}   -   {OWNER}", ha="center", va="center", fontsize=13, fontweight="bold")
    X = {l: i + 0.5 for i, l in enumerate(lifelines)}
    top, bot = 1.0, len(msgs) + 2.4
    for l, x in X.items():
        ax.add_patch(Rectangle((x - 0.42, top - 0.35), 0.84, 0.6, fc="#e8eef7", ec="#2f5d8a", lw=1.4))
        actor = l.startswith("a:")
        ax.text(x, top - 0.05, l[2:] if actor else l, ha="center", va="center", fontsize=10, fontweight="bold")
        ax.plot([x, x], [top + 0.25, bot], ls=(0, (5, 4)), color="#8a8a8a", lw=1)
    y = top + 0.8
    for m in msgs:
        src, dst, label, kind = m
        xs, xd = X[src], X[dst]
        if kind == "self":
            ax.add_patch(Rectangle((xs - 0.05, y - 0.08), 0.1, 0.5, fc="#fdf3d0", ec="#444", lw=0.8))
            ax.annotate("", xy=(xs + 0.05, y + 0.32), xytext=(xs + 0.05, y), arrowprops=dict(arrowstyle="-|>", color="#222", connectionstyle="arc3,rad=-1.2"))
            ax.text(xs + 0.28, y + 0.12, label, fontsize=9, va="center", ha="left")
        elif kind == "note":
            ax.add_patch(Rectangle((min(xs, xd) - 0.4, y - 0.2), abs(xd - xs) + 0.8, 0.42, fc="#fff4e5", ec="#c77700", lw=1))
            ax.text((xs + xd) / 2, y + 0.01, label, fontsize=9, ha="center", va="center", style="italic")
        else:
            ret = kind == "return"
            ax.annotate("", xy=(xd, y), xytext=(xs, y), arrowprops=dict(arrowstyle="->" if ret else "-|>", color="#1b5e20" if ret else "#222", ls="--" if ret else "-", lw=1.3))
            ax.text((xs + xd) / 2, y - 0.1, label, fontsize=9, ha="center", va="bottom", color="#1b5e20" if ret else "#111")
        y += 1
    fig.tight_layout(); fig.savefig(os.path.join(HERE, fname), dpi=110); plt.close(fig)

def seq1():
    L = ["a:Employee", "a:Manager", "ConsoleUI", "LeaveSystem", "LeavePolicy", "LeaveRequest", "Approval"]
    M = [("a:Employee", "ConsoleUI", "6 > 1  Apply (id, type, dates, reason)", "call"),
         ("ConsoleUI", "LeaveSystem", "applyLeave(empId, type, from, to, reason)", "call"),
         ("LeaveSystem", "LeaveSystem", "employee(id), policy(type)  [NotFoundError if absent]", "self"),
         ("LeaveSystem", "LeaveRequest", "new LeaveRequest(...)  +  calculateDays(holidays)", "call"),
         ("LeaveSystem", "LeavePolicy", "check(from, to, days, today)   {virtual: notice / max / back-date}", "call"),
         ("LeavePolicy", "LeaveSystem", "OK  (or RuleError)", "return"),
         ("LeaveSystem", "LeaveSystem", "checkNoOverlap(), checkBalance(), requests_.add(), touch()", "self"),
         ("LeaveSystem", "ConsoleUI", "LeaveRequest& (status = Pending)", "return"),
         ("ConsoleUI", "a:Employee", "\"R0011 submitted: N working day(s)\"", "return"),
         ("a:Manager", "ConsoleUI", "6 > 2  Review queue, choose request, Approve", "call"),
         ("ConsoleUI", "LeaveSystem", "decide(reqId, mgrId, true, comment)", "call"),
         ("LeaveSystem", "LeaveSystem", "manager.authorises(employee);  department cover rule on each working day", "self"),
         ("LeaveSystem", "Approval", "new Approval(id, reqId, mgrId)  ->  approve(comment, today)", "call"),
         ("LeaveSystem", "LeaveRequest", "setStatus(Approved)", "call"),
         ("LeaveSystem", "ConsoleUI", "Approval&  (balance 'used' increases)", "return"),
         ("ConsoleUI", "a:Manager", "\"R0011 Approved (approval A0007)\"", "return")]
    sequence("Sequence_Diagram_1.png", "Sequence Diagram 1 - Main transaction: apply for leave, then approve", L, M)

def seq2():
    L = ["a:Employee", "ConsoleUI", "LeaveSystem", "LeaveRequest", "AuditLog", "FileManager", "a:DataFiles"]
    M = [("a:Employee", "ConsoleUI", "6 > 3  Cancel request R0011, confirm y", "call"),
         ("ConsoleUI", "LeaveSystem", "cancelLeave(\"R0011\")", "call"),
         ("LeaveSystem", "LeaveRequest", "request(id)  ->  status(), from()", "call"),
         ("LeaveSystem", "LeaveSystem", "approved & already started?  or not Pending/Approved?  -> throw RuleError", "self"),
         ("LeaveSystem", "LeaveRequest", "setStatus(Cancelled)   (balance is recomputed from statuses)", "call"),
         ("LeaveSystem", "AuditLog", "touch(\"CANCEL\", id)  ->  log(date, action, detail)", "call"),
         ("LeaveSystem", "ConsoleUI", "void  (or ElmError caught by ConsoleUI::guarded)", "return"),
         ("ConsoleUI", "a:Employee", "\"Cancelled. Balance restored.\"", "return"),
         ("a:Employee", "ConsoleUI", "8 > 1  Save now", "call"),
         ("ConsoleUI", "LeaveSystem", "save()", "call"),
         ("LeaveSystem", "FileManager", "save(\"requests.dat\", requests_, \"LeaveRequest\")   (x5 files)", "call"),
         ("FileManager", "LeaveRequest", "serialize()   for each record (virtual call via T*)", "call"),
         ("FileManager", "a:DataFiles", "write requests.dat.tmp (header + checksum + lines), then rename()", "call"),
         ("FileManager", "LeaveSystem", "records written", "return"),
         ("LeaveSystem", "ConsoleUI", "dirty_ = false", "return"),
         ("ConsoleUI", "a:Employee", "\"Saved N record(s).\"", "return")]
    sequence("Sequence_Diagram_2.png", "Sequence Diagram 2 - Cancel a leave request and save to file", L, M)

class_diagram(); use_case(); seq1(); seq2(); print("UML diagrams written to", HERE)
