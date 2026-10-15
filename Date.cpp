#include "elm/Date.h"
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include "elm/errors.h"

namespace elm {

static const char* const kDays[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

bool Date::isLeap(int y) { return (y % 4 == 0 && y % 100 != 0) || y % 400 == 0; }

int Date::daysInMonth(int y, int m) {
    static const int dm[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (m < 1 || m > 12) return 0;
    return (m == 2 && isLeap(y)) ? 29 : dm[m - 1];
}

Date::Date() : y_(2000), m_(1), d_(1) {}

Date::Date(int y, int m, int d) : y_(y), m_(m), d_(d) {
    if (y < 1970 || y > 2100 || m < 1 || m > 12 || d < 1 || d > daysInMonth(y, m))
        throw ValidationError("invalid date " + std::to_string(y) + "-" + std::to_string(m) + "-" + std::to_string(d));
}

bool Date::tryParse(const std::string& s, Date& out) {
    if (s.size() != 10 || s[4] != '-' || s[7] != '-') return false;
    for (size_t i = 0; i < s.size(); ++i)
        if (i != 4 && i != 7 && (s[i] < '0' || s[i] > '9')) return false;
    int y = std::atoi(s.substr(0, 4).c_str());
    int m = std::atoi(s.substr(5, 2).c_str());
    int d = std::atoi(s.substr(8, 2).c_str());
    try { out = Date(y, m, d); } catch (const ValidationError&) { return false; }
    return true;
}

Date Date::parse(const std::string& s) {
    Date d;
    if (!tryParse(s, d)) throw ValidationError("'" + s + "' is not a valid date (use YYYY-MM-DD)");
    return d;
}

Date Date::today() {
    if (const char* env = std::getenv("ELM_TODAY")) {       // fixed date for repeatable tests
        Date d;
        if (tryParse(env, d)) return d;
    }
    std::time_t t = std::time(nullptr);
    std::tm* lt = std::localtime(&t);
    return Date(lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday);
}

// Howard Hinnant's civil-calendar algorithms (exact for the proleptic Gregorian calendar).
long Date::serial() const {
    long y = y_ - (m_ <= 2 ? 1 : 0);
    long era = (y >= 0 ? y : y - 399) / 400;
    long yoe = y - era * 400;
    long doy = (153 * (m_ + (m_ > 2 ? -3 : 9)) + 2) / 5 + d_ - 1;
    long doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + doe - 719468;
}

Date Date::fromSerial(long z) {
    z += 719468;
    long era = (z >= 0 ? z : z - 146096) / 146097;
    long doe = z - era * 146097;
    long yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    long y = yoe + era * 400;
    long doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    long mp = (5 * doy + 2) / 153;
    long d = doy - (153 * mp + 2) / 5 + 1;
    long m = mp < 10 ? mp + 3 : mp - 9;
    return Date(static_cast<int>(y + (m <= 2 ? 1 : 0)), static_cast<int>(m), static_cast<int>(d));
}

int Date::weekday() const { return static_cast<int>(((serial() % 7) + 7 + 4) % 7); }  // 1970-01-01 = Thu

std::string Date::str() const {
    char buf[16];
    std::snprintf(buf, sizeof buf, "%04d-%02d-%02d", y_, m_, d_);
    return buf;
}

std::string Date::weekdayName() const { return kDays[weekday()]; }

}  // namespace elm
