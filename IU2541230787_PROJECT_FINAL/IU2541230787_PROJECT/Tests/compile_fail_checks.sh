#!/bin/bash
# Proves encapsulation / abstraction / ownership rules are enforced BY THE COMPILER:
# every snippet below must FAIL to compile.   Run from the project folder:  bash Tests/compile_fail_checks.sh
cd "$(dirname "$0")/.." || exit 1
mkdir -p Tests/_work
pass=0; total=0
check() {   # $1 = description, $2 = code
  total=$((total+1)); printf '#include "elm/LeaveSystem.h"\nusing namespace elm;\nint main(){ %s }\n' "$2" > Tests/_work/cf.cpp
  if out=$(g++ -std=c++17 -Iinclude -fsyntax-only Tests/_work/cf.cpp 2>&1); then echo "[FAIL] $1  (it compiled!)"
  else pass=$((pass+1)); echo "[PASS] $1"; echo "       compiler says: $(echo "$out" | grep -m1 -E 'error' | sed 's/^[^:]*:[0-9]*:[0-9]*: //')"; fi
}
check "Record is abstract - cannot be instantiated"            'Record r; (void)r;'
check "Person is abstract - cannot be instantiated"            'Person p; (void)p;'
check "LeavePolicy is abstract - cannot be instantiated"       'LeavePolicy p; (void)p;'
check "Private member is not reachable (Employee name_)"       'Employee e; e.name_ = "x";'
check "Record id is read-only from outside (no setter)"        'Employee e; e.setId("E1");'
check "PtrList cannot be copied (would double-delete)"         'PtrList<Employee> a; PtrList<Employee> b(a); (void)b;'
check "LeaveSystem cannot be copied"                           'LeaveSystem a("x"); LeaveSystem b(a); (void)b;'
check "Const reference cannot be modified (Employee&)"         'const Employee e; e.setName("Abc");'
check "Status enum class does not convert to int implicitly"   'int s = Status::Pending; (void)s;'
echo; echo "$pass/$total compile-fail proofs behaved as required"; [ "$pass" = "$total" ]
