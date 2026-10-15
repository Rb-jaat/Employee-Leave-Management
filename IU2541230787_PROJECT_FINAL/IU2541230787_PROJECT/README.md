# Employee Leave Management System (IU2541230787 - CPP74)
Author: Krish Singh | Enrollment No.: IU2541230787 | Semester 3 | C++17 console OOP project

Employees apply for leave, managers approve or reject, and the system enforces real HR rules: working-day counting (weekends and public holidays excluded), per-type yearly quotas, notice periods, back-dating limits, overlap detection and a department cover rule. Everything is saved to `Data/` and reloaded on start.

## Build and run
    make                      # strict build (-Wall -Wextra -Wpedantic -Werror) -> ./elm
    ./elm                     # run from this folder so Data/ is found
    ./elm --data MyFolder     # use another data folder
Without make: `g++ -std=c++17 -Iinclude -o elm src/*.cpp`. Windows: `BUILD_WINDOWS.bat` (MinGW g++). Set `ELM_TODAY=2026-10-15` to fix "today" for repeatable demos.

## Menu (blueprint)
1 Add (employee / manager / holiday / leave request) - 2 Display (incl. "Everything", polymorphic) - 3 Search - 4 Update - 5 Delete - 6 Leave Process (apply / approve-reject / cancel / balance) - 7 Reports (balance, department, who-is-on-leave, month calendar, summary, CSV export, audit log) - 8 Save / Load (save, reload, backup, demo data, clear) - 9 Exit. First run: choose 8 > 4 to load demo data, then 8 > 1 to save.

## Business rules
Annual 18/yr (3 days notice) - Sick 10/yr (back-date 7 days) - Casual 7/yr (max 3 days) - Parental 30/yr (14 days notice) - Unpaid unlimited (7 days notice). One request must stay inside one calendar year; no overlap with own active requests; only Pending requests can be edited; a manager may decide only for their department (HR for all); at most half of a department can be on approved leave on one day; a rejection needs a reason; started leave cannot be cancelled; staff with active leave and managers with decisions cannot be deleted.

## Design in one paragraph
`Record` (abstract) -> `Person` (abstract) -> `Employee`, `Manager`; `Record` -> `LeaveRequest`, `Approval`, `Holiday`. `LeavePolicy` (abstract) has five concrete leave types held in a `LeavePolicy**` array. `PtrList<T>` is an owning dynamic array (new[]/delete[], doubling, copy disabled). `LeaveSystem` is the controller (no keyboard code), `ConsoleUI` the menus, `FileManager` saves tab-separated files atomically with header and checksum, `AuditLog` records every change, `Date` is a calendar class written from scratch.

## Verify
    make test                        # 41 end-to-end tests through stdin -> Tests/results.json + Tests/Screenshots/
    make unit                        # 84 white-box checks under AddressSanitizer + UBSan (memory-leak free)
    bash Tests/compile_fail_checks.sh  # 9 proofs the compiler enforces encapsulation / abstraction / ownership
    python3 UML/make_uml.py          # redraw the 4 UML diagrams

## Folders
`include/elm/` headers | `src/` code | `Data/` records | `Tests/` tests + screenshots | `UML/` diagrams | `Docs/` report + submission template | `VIVA_NOTES.md`
