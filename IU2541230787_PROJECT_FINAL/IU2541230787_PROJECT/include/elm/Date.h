// Date.h - small calendar date class (no <ctime> arithmetic, fully deterministic).
#pragma once
#include <string>

namespace elm {

class Date {
    int y_, m_, d_;
public:
    Date();                                  // default: 2000-01-01
    Date(int y, int m, int d);               // throws ValidationError if impossible
    static bool isLeap(int y);
    static int  daysInMonth(int y, int m);
    static bool tryParse(const std::string& s, Date& out);   // "YYYY-MM-DD"
    static Date parse(const std::string& s);                 // throws ValidationError
    static Date today();                     // system date, or env ELM_TODAY for tests
    static Date fromSerial(long days);       // days since 1970-01-01

    long serial() const;
    int  weekday() const;                    // 0 = Sunday ... 6 = Saturday
    bool isWeekend() const { int w = weekday(); return w == 0 || w == 6; }
    int  year() const { return y_; }
    int  month() const { return m_; }
    int  day() const { return d_; }
    Date addDays(long n) const { return fromSerial(serial() + n); }
    std::string str() const;                 // "YYYY-MM-DD"
    std::string weekdayName() const;         // "Mon" ...

    long operator-(const Date& o) const { return serial() - o.serial(); }
    bool operator==(const Date& o) const { return serial() == o.serial(); }
    bool operator!=(const Date& o) const { return !(*this == o); }
    bool operator<(const Date& o) const { return serial() < o.serial(); }
    bool operator<=(const Date& o) const { return serial() <= o.serial(); }
    bool operator>(const Date& o) const { return serial() > o.serial(); }
    bool operator>=(const Date& o) const { return serial() >= o.serial(); }
};

}  // namespace elm
