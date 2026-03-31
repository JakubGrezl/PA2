#ifndef DATE_H_INCLUDED
#define DATE_H_INCLUDED

#include <iostream>
#include <stddef.h>

struct DateTuple {
public:
  unsigned day;
  unsigned month;
  unsigned year;

  void copy(const DateTuple * origin);

  DateTuple(const unsigned _day, const unsigned _month, const unsigned _year);

  /*!
  copies right to the left and destroys right
  @return reference to left
  @param rhs pointer to right
  @warning rhs will be destroyed
  */
  DateTuple & operator= (const DateTuple * rhs);
};

class CDate {
private:
  /**
   * @param days
   * @return returns construct object to DateTuple
   */
  static DateTuple convert(int days);

  /**
   * converts date to days range from year_min
   * @return number of days
   */
  static int convert(const int day, int month, const int year);
  static bool validate_date(const int day, const int month, const int year);

  int day;
public:

  /*!
  get days in month
  @return returns number of days in month
 */
  static int get_days(const int month, const int year);

  /*!
   get days in month
   @return returns number of days in month
   @warning this overload doesn't implement leap year
  */
  static int get_days(const int month);

    /*!
  get days from object
  @return returns days saved in CDate class
 */
  int get_days() const;

  static bool leap_year(const int _year);

  int cmp_date(const CDate & rhs) const;

  std::size_t diff(const CDate & date) const;

  CDate(const int year, const int month, const int day);

  // moves date by n days to future
  CDate & operator + (int _day);

  CDate & operator - (int _day);

  std::size_t operator - (const CDate &rhs) const;

  CDate & operator++ ();

  CDate operator++ (int);
  CDate & operator-- ();

  CDate operator-- (int);

  bool operator== (const CDate &rhs) const;

  bool operator!= (const CDate &rhs) const;

  bool operator<= (const CDate &rhs) const ;

  bool operator>= (const CDate &rhs) const;

  bool operator< (const CDate &rhs) const;

  bool operator> (const CDate &rhs) const;

  friend std::ostream & operator<< (std::ostream & oss, const CDate & date);
  friend std::istream & operator>> (std::istream & iss, CDate & date);
};

#endif