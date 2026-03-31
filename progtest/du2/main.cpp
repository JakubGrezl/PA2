#ifndef __PROGTEST__
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <string>
#include <sstream>
#include <stdexcept>
#include <compare>
#include "date.h"
#endif __PROGTEST__
#include <pstl/glue_execution_defs.h>

//=================================================================================================
// date_format manipulator - a dummy implementation. Keep this code unless you implement your
// own working manipulator.
std::ios_base & (*date_format(const char *fmt))(std::ios_base &x) {
    return [](std::ios_base &ios) -> std::ios_base & { return ios; };
}

//=================================================================================================

int validate_input_number(std::string &number) {
    int value;

    try {
        value = std::stoi(number);
    } catch (std::exception &discard) {
        return false;
    }

    if (value > 9 && number.length() != 2) {
        return false;
    }

    if (value <= 9 && number.length() > 1) {
        return false;
    }

    return value;
}

// how many days are in each month of year, non leap year
namespace utils {
    bool dividible(int factor, int denominator) {
        return factor % denominator == 0;
    }

    std::size_t diff(int lhs, int rhs) {
        if (lhs > rhs)
            return lhs - rhs;
        return rhs - lhs;
    }
}

static constexpr int min_year = 2000;
static constexpr int max_year = 2030;

void DateTuple::copy(const DateTuple *origin) {
    day = origin->day;
    month = origin->month;
    year = origin->year;
};

DateTuple::DateTuple(const unsigned _day, const unsigned _month, const unsigned _year) {
    day = _day;
    month = _month;
    year = _year;
};

DateTuple &DateTuple::operator=(const DateTuple *rhs) {
    copy(rhs);
    delete rhs;
    return *this;
}

DateTuple CDate::convert(int days) {
    unsigned day, month = 1, year = min_year;

    while (days >= get_days(month, year)) {
        days -= get_days(month, year);
        month++;

        if (month > 12) {
            month = 1;
            year++;
        }
    }

    day = days + 1;

    return {day, month, year};
}

int CDate::convert(const int day, int month, const int year) {
    int tmp = (year - min_year) * 365;

    for (int tmp_year = year - 1; tmp_year >= min_year; tmp_year--) {
        tmp += leap_year(tmp_year) ? 1 : 0;
    }

    tmp += (day - 1);

    // on first, lower the month and then itterate
    for (--month; month >= 1; --month) {
        tmp += get_days(month, year);
    }
    return tmp;
}

bool CDate::validate_date(const int day, const int month, const int year) {
    if (month < 1 || month > 12)
        return false;

    if (day > get_days(month, year) || day < 1)
        return false;

    if (year < min_year || year > max_year)
        return false;

    return true;
}

int CDate::get_days() const {
    return day;
}

int CDate::get_days(const int month) {
    if (month > 12)
        throw std::out_of_range("out of the range of month!");

    const int month_dictionary[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    return month_dictionary[month - 1];
}

int CDate::get_days(const int month, const int year) {
    if (month == 2 && leap_year(year)) {
        return 29;
    }

    return get_days(month);
}

bool CDate::leap_year(const int _year) {
    return (utils::dividible(_year, 4) && !utils::dividible(_year, 100)) || (
               utils::dividible(_year, 400));
}

int CDate::cmp_date(const CDate &rhs) const {
    if (day < rhs.day)
        return -1;
    if (day > rhs.day)
        return 1;

    // same dates
    return 0;
}

std::size_t CDate::diff(const CDate &date) const {
    return utils::diff(day, date.get_days());
};

CDate::CDate(const int year, const int month, const int day) {
    if (!CDate::validate_date(day, month, year))
        throw "InvalidDateException";

    // maximalni a minimalni hodnoty jsou nezaporne (zarizuje validace nahore)
    this->day = convert(day, month, year);
}

// moves date by n days to future
CDate &CDate::operator+(int _day) {
    day += _day;
    return *this;
}

CDate &CDate::operator -(int _day) {
    day -= _day;
    return *this;
}

std::size_t CDate::operator -(const CDate &rhs) const {
    return diff(rhs);
}

CDate &CDate::operator++() {
    day += 1;
    return *this;
}

CDate CDate::operator++(int) {
    auto tmp = *this;
    operator++();
    return tmp;
}

CDate &CDate::operator--() {
    day -= 1;
    return *this;
}

CDate CDate::operator--(int) {
    auto tmp = *this;
    operator--();
    return tmp;
}

bool CDate::operator==(const CDate &rhs) const {
    return cmp_date(rhs) == 0;
}

bool CDate::operator!=(const CDate &rhs) const {
    return cmp_date(rhs) != 0;
}

bool CDate::operator<=(const CDate &rhs) const {
    return cmp_date(rhs) <= 0;
}

bool CDate::operator>=(const CDate &rhs) const {
    return cmp_date(rhs) >= 0;
}

bool CDate::operator<(const CDate &rhs) const {
    return cmp_date(rhs) < 0;
}

bool CDate::operator>(const CDate &rhs) const {
    return cmp_date(rhs) > 0;
}

std::ostream &operator<<(std::ostream &oss, const CDate &date) {
    const DateTuple result = CDate::convert(date.get_days());

    std::ostringstream tmp;

    tmp << result.year << '-'
            << std::setfill('0') << std::setw(2) << result.month << '-'
            << std::setfill('0') << std::setw(2) << result.day;

    return oss << tmp.str();
}

int validate_numbers(const std::string &number) {
    for (auto iterator: number) {
        if (!std::isdigit(iterator))
            throw std::invalid_argument("Invalid number!");
    }

    return std::stoi(number);
}

std::istream &operator>>(std::istream &iss, CDate &date) {
    DateTuple date_tuple(0, 0, 0);

    std::string tmp;
    if (!(iss >> tmp)) {
        iss.setstate(std::istream::failbit);
        return iss;
    };

    if (tmp[4] != '-' || tmp[7] != '-') {
        iss.setstate(std::istream::failbit);
        return iss;
    }

    try {
        date_tuple.year = validate_numbers(tmp.substr(0, 4));
        date_tuple.month = validate_numbers(tmp.substr(5, 2));
        date_tuple.day = validate_numbers(tmp.substr(8, 2));
    } catch (std::exception &discard) {
        iss.setstate(std::istream::failbit);
        return iss;
    }

    if (!CDate::validate_date(date_tuple.day, date_tuple.month, date_tuple.year)) {
        iss.setstate(std::istream::failbit);
        return iss;
    }

    date.day = CDate::convert(date_tuple.day, date_tuple.month, date_tuple.year);
    return iss;
}

#ifndef __PROGTEST__
int main() {
    std::ostringstream oss;
    std::istringstream iss;

    CDate a(2000, 1, 2);
    CDate b(2010, 2, 3);
    CDate c(2004, 2, 10);
    oss.str("");
    oss << a;

    assert(oss . str () == "2000-01-02");
    oss.str("");
    oss << b;
    assert(oss . str () == "2010-02-03");
    oss.str("");
    oss << c;
    assert(oss . str () == "2004-02-10");
    a = a + 1500;
    oss.str("");
    oss << a;
    assert(oss . str () == "2004-02-10");
    b = b - 2000;
    oss.str("");
    oss << b;
    assert(oss . str () == "2004-08-13");
    assert(b - a == 185);
    assert(( b == a ) == false);
    assert(( b != a ) == true);
    assert(( b <= a ) == false);
    assert(( b < a ) == false);
    assert(( b >= a ) == true);
    assert(( b > a ) == true);
    assert(( c == a ) == true);
    assert(( c != a ) == false);
    assert(( c <= a ) == true);
    assert(( c < a ) == false);
    assert(( c >= a ) == true);
    assert(( c > a ) == false);
    a = ++c;
    oss.str("");
    oss << a << " " << c;
    assert(oss . str () == "2004-02-11 2004-02-11");
    a = --c;
    oss.str("");
    oss << a << " " << c;
    assert(oss . str () == "2004-02-10 2004-02-10");
    a = c++;
    oss.str("");
    oss << a << " " << c;
    assert(oss . str () == "2004-02-10 2004-02-11");
    a = c--;
    oss.str("");
    oss << a << " " << c;
    assert(oss . str () == "2004-02-11 2004-02-10");
    iss.clear();
    iss.str("2015-09-03");
    assert(( iss >> a ));
    oss.str("");
    oss << a;
    assert(oss . str () == "2015-09-03");
    a = a + 70;
    oss.str("");
    oss << a;
    assert(oss . str () == "2015-11-12");
    oss.str("");
    oss << std::setw(20) << a;
    assert(oss . str () == "          2015-11-12");

    CDate d(2000, 1, 1);
    try {
        CDate e(2000, 32, 1);
        assert("No exception thrown!" == nullptr);
    } catch (...) {
    }
    iss.clear();
    iss.str("2000-12-33");
    assert(! ( iss >> d ));
    oss.str("");
    oss << d;
    assert(oss . str () == "2000-01-01");
    iss.clear();
    iss.str("2000-11-31");
    assert(! ( iss >> d ));
    oss.str("");
    oss << d;
    assert(oss . str () == "2000-01-01");
    iss.clear();
    iss.str("2000-02-29");
    assert(( iss >> d ));
    oss.str("");
    oss << d;
    assert(oss . str () == "2000-02-29");
    iss.clear();
    iss.str("2001-02-29");
    assert(! ( iss >> d ));
    oss.str("");
    oss << d;
    assert(oss . str () == "2000-02-29");

    return EXIT_SUCCESS;
}

#endif __PROGTEST__
