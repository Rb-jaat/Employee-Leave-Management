# Viva notes - Employee Leave Management (Krish Singh, IU2541230787)

**Responsibility of each class**
Record: abstract id + virtual display/row/serialize. Person: name+department with validation. Employee / Manager: people (Manager::authorises decides who may approve). LeaveRequest: one application, counts working days. Approval: a manager's decision. Holiday: non-working date. LeavePolicy (+5 subclasses): the rules of each leave type. LeaveSystem: owns all data, enforces every rule. ConsoleUI: menus only. FileManager: safe save/load. AuditLog: change history. PtrList<T>: owning dynamic array. Date: calendar maths.

**One relationship in the class diagram**: LeaveSystem *owns* five PtrLists (composition): when LeaveSystem is destroyed the lists delete every object. Approval -> LeaveRequest is an association (an approval refers to exactly one request by id).

**Inheritance + runtime polymorphism**: Display > Everything loops over `Record*` and calls `display()`; the right version for Employee/Manager/LeaveRequest/Approval/Holiday runs (ConsoleUI.cpp, case 6). Also `LeavePolicy::check()` calls virtual `maxConsecutive()`, `minNoticeDays()`, `maxBackdateDays()`, so each leave type behaves differently.

**Dynamic memory**: `PtrList::grow()` does `new T*[2*cap]`, copies, `delete[]` the old array; every element is deleted in `~PtrList`. `LeaveSystem` allocates `new LeavePolicy*[5]` and frees it in its destructor. Why needed: the number of employees/requests is unknown at compile time and elements are polymorphic. ASan/LeakSanitizer (`make unit`) reports no leaks.

**Constructors / destructors**: default + parameterized constructors on Employee, Manager, LeaveRequest, Approval, Holiday; virtual destructor in Record and LeavePolicy; PtrList has destructor, deleted copy, move.

**Exception handling**: ElmError hierarchy (ValidationError, RuleError, NotFoundError, FileError, ParseError). LeaveSystem throws, ConsoleUI::guarded catches and prints; nothing crashes on bad input.

**File handling**: start -> `load()`; edits in memory; `save()` writes each `.dat` as `.tmp` then renames (atomic). Header `#ELM1 kind count checksum`; corrupt lines are skipped with a warning; missing file = first run. Demo: add employee, Save, exit, run again, Display - still there (test T40).

**Why working days are stored in the request**: later holiday changes must not silently change an approved request's balance.
