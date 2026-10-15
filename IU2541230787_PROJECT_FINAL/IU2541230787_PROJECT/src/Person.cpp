#include "elm/Person.h"
#include <iomanip>
#include <ostream>
#include "elm/errors.h"
#include "elm/io.h"

namespace elm {

static std::string checkedText(const std::string& v, const char* field, size_t maxLen) {
    std::string t = io::trim(v);
    if (t.empty()) throw ValidationError(std::string(field) + " cannot be empty");
    if (t.size() > maxLen) throw ValidationError(std::string(field) + " is too long (max " + std::to_string(maxLen) + ")");
    return t;
}

// ---------------- Person ----------------
Person::Person(const std::string& id, const std::string& name, const std::string& dept) : Record(id) {
    setName(name);
    setDepartment(dept);
}
void Person::setName(const std::string& n) {
    if (!io::validName(io::trim(n))) throw ValidationError("name must be 2-40 letters (spaces . ' - allowed)");
    name_ = io::titleCase(io::trim(n));
}
void Person::setDepartment(const std::string& d) { department_ = io::titleCase(checkedText(d, "department", 25)); }
bool Person::matches(const std::string& t) const {
    return Record::matches(t) || io::containsCI(name_, t) || io::containsCI(department_, t);
}

// ---------------- Employee ----------------
Employee::Employee(const std::string& id, const std::string& name, const std::string& dept,
                   const std::string& designation, const std::string& email, const Date& joined)
    : Person(id, name, dept), joined_(joined) {
    setDesignation(designation);
    setEmail(email);
}
void Employee::setDesignation(const std::string& d) { designation_ = checkedText(d, "designation", 30); }
void Employee::setEmail(const std::string& e) {
    std::string m = io::lower(io::trim(e));
    if (!io::validEmail(m)) throw ValidationError("'" + e + "' is not a valid e-mail address");
    email_ = m;
}
void Employee::display(std::ostream& os) const {
    os << "  Employee ID : " << id() << "\n  Name        : " << name() << "\n  Department  : " << department()
       << "\n  Designation : " << designation_ << "\n  E-mail      : " << email_
       << "\n  Joined      : " << joined_.str() << "\n";
}
std::string Employee::row() const {
    return io::pad(id(), 6) + io::pad(name(), 20) + io::pad(department(), 14) + io::pad(designation_, 20) + joined_.str();
}
std::string Employee::serialize() const {
    return io::escape(id()) + "\t" + io::escape(name()) + "\t" + io::escape(department()) + "\t" +
           io::escape(designation_) + "\t" + io::escape(email_) + "\t" + joined_.str();
}
Employee* Employee::fromFields(const std::vector<std::string>& f) {
    if (f.size() != 6) throw ParseError("Employee line needs 6 fields, got " + std::to_string(f.size()));
    try {
        return new Employee(io::unescape(f[0]), io::unescape(f[1]), io::unescape(f[2]), io::unescape(f[3]),
                            io::unescape(f[4]), Date::parse(f[5]));
    } catch (const ValidationError& e) { throw ParseError(e.what()); }
}

// ---------------- Manager ----------------
Manager::Manager(const std::string& id, const std::string& name, const std::string& dept,
                 const std::string& title, bool hr) : Person(id, name, dept), hr_(hr) {
    setTitle(title);
}
void Manager::setTitle(const std::string& t) { title_ = checkedText(t, "title", 30); }
bool Manager::authorises(const Employee& e) const { return hr_ || department() == e.department(); }
void Manager::display(std::ostream& os) const {
    os << "  Manager ID  : " << id() << "\n  Name        : " << name() << "\n  Department  : " << department()
       << "\n  Title       : " << title_ << "\n  Role        : " << role()
       << (hr_ ? "  (may approve every department)" : "") << "\n";
}
std::string Manager::row() const {
    return io::pad(id(), 6) + io::pad(name(), 20) + io::pad(department(), 14) + io::pad(title_, 20) + role();
}
std::string Manager::serialize() const {
    return io::escape(id()) + "\t" + io::escape(name()) + "\t" + io::escape(department()) + "\t" +
           io::escape(title_) + "\t" + (hr_ ? "1" : "0");
}
Manager* Manager::fromFields(const std::vector<std::string>& f) {
    if (f.size() != 5) throw ParseError("Manager line needs 5 fields, got " + std::to_string(f.size()));
    try {
        return new Manager(io::unescape(f[0]), io::unescape(f[1]), io::unescape(f[2]), io::unescape(f[3]), f[4] == "1");
    } catch (const ValidationError& e) { throw ParseError(e.what()); }
}

}  // namespace elm
