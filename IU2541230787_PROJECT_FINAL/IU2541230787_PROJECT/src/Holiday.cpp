#include "elm/Holiday.h"
#include <ostream>
#include "elm/errors.h"
#include "elm/io.h"

namespace elm {

Holiday::Holiday(const std::string& id, const Date& d, const std::string& name) : Record(id), date_(d) { setName(name); }
void Holiday::setName(const std::string& n) {
    std::string t = io::trim(n);
    if (t.empty() || t.size() > 40) throw ValidationError("holiday name must be 1-40 characters");
    name_ = t;
}
void Holiday::display(std::ostream& os) const {
    os << "  Holiday ID  : " << id() << "\n  Date        : " << date_.str() << " (" << date_.weekdayName()
       << ")\n  Name        : " << name_ << "\n";
}
std::string Holiday::row() const { return io::pad(id(), 6) + io::pad(date_.str(), 12) + io::pad(date_.weekdayName(), 6) + name_; }
std::string Holiday::serialize() const { return io::escape(id()) + "\t" + date_.str() + "\t" + io::escape(name_); }
bool Holiday::matches(const std::string& t) const {
    return Record::matches(t) || io::containsCI(name_, t) || io::containsCI(date_.str(), t);
}
Holiday* Holiday::fromFields(const std::vector<std::string>& f) {
    if (f.size() != 3) throw ParseError("Holiday line needs 3 fields");
    try { return new Holiday(io::unescape(f[0]), Date::parse(f[1]), io::unescape(f[2])); }
    catch (const ValidationError& e) { throw ParseError(e.what()); }
}

}  // namespace elm
