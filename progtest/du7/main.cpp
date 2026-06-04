#ifndef __PROGTEST__
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <memory>
#include <optional>
#include <iterator>
#include <functional>
#include <stdexcept>
#include <type_traits>
#include <cassert>

class CRange {
public:
  CRange(size_t from,
         size_t to)
    : m_From(from),
      m_To(to) {
    if (from > to)
      throw std::invalid_argument("invalid range");
  }

  size_t m_From;
  size_t m_To;
};

class CRangeRev {
public:
  CRangeRev(size_t from,
            size_t to)
    : m_From(from),
      m_To(to) {
    if (from < to)
      throw std::invalid_argument("invalid range");
  }

  size_t m_From;
  size_t m_To;
};
#endif /* __PROGTEST__ */

class CTensorI {
  virtual CTensorI
};

template<typename T_, size_t DIM_>
class CTensor {
public:
  template <typename... Args>
  CTensor(const Args&... _args) {
    std::vector<T_> args;
    (args.emplace_back(_args), ...);
  };


  // constructor -- create + set all elements
  // constructor -- from initializer_list
  // destructor (opt)
  // slice ()
  // operator ()
  // operator <<
private:
  // dimension and value saved
  void add(T_ value, size_t current_dimension) {
    if (current_dimension >= DIM_)
      throw std::invalid_argument("invalid dimension");

    arr.push_back(std::make_pair(current_dimension, value));
  }

  std::vector<std::pair<size_t, T_>> arr;
};

template<typename T_>
class CTensorView {
public:
  // operator ()
  // operator <<
private:
  // todo
};


#ifndef __PROGTEST__
template<typename T_>
std::string toString(const T_ &x) {
  std::ostringstream oss;
  oss << x;
  return oss.str();
}

int main() {
  CTensor<int, 2> m1(0, 3, 5);
  CTensor<int, 2> m2{{1, 2, 3}, {4, 5, 6}};
  CTensor<std::string, 2> m4
  {
    {"test", "progtest"},
    {"PA1", "PA2"}
  };

  assert(toString ( m1 ) == R"({
  {0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0}
})");

  assert(toString ( m2 ) == R"({
  {1, 2, 3},
  {4, 5, 6}
})");

  assert(toString ( m4 ) == R"({
  {test, progtest},
  {PA1, PA2}
})");

  assert(toString ( m2 . slice ( CRange ( 0, 1 ), CRange ( 1, 2 ) ) ) == R"({
  {2, 3},
  {5, 6}
})");
  assert(toString ( m2 . slice ( 0, CRange ( 1, 2 ) ) ) == "{2, 3}");
  assert(toString ( m2 . slice ( 1 ) ) == "{4, 5, 6}");
  assert(toString ( m2 . slice ( CRange ( 0, 1 ), 2 ) ) == "{3, 6}");

  auto v2 = m2.slice(CRange(0, 1), CRange(1, 2));
  assert(v2 ( 0, 0 ) == 2);
  v2(1, 1) = 666;
  assert(v2 ( 1, 1 ) == 666);

  assert(toString ( v2 ) == R"({
  {2, 3},
  {5, 666}
})");

  assert(toString ( m2 ) == R"({
  {1, 2, 3},
  {4, 5, 666}
})");

  assert(toString ( m2 . slice ( 1, CRange ( 1, 2 ) ) ) == "{5, 666}");
  assert(toString ( m2 . slice ( 1, CRangeRev ( 2, 1 ) ) ) == "{666, 5}");
  assert(toString ( m2 . slice ( 1, CRange ( 2, 2 ) ) ) == "{666}");
  assert(toString ( m2 . slice ( CRange ( 1, 1 ), 2 ) ) == "{666}");
  assert(toString ( m2 . slice ( CRange ( 1, 1 ), CRange ( 2, 2 ) ) ) == R"###({
  {666}
})###");

  assert(toString ( m4 . slice ( CRangeRev ( 1, 0 ), CRangeRev ( 1, 0 ) ) ) == R"({
  {PA2, PA1},
  {progtest, test}
})");

  try {
    m2(3, 4)++;
    assert("missing an exception" == nullptr);
  } catch (const std::exception &e) {
  }

  try {
    v2(2, 0)--;
    assert("missing an exception" == nullptr);
  } catch (const std::exception &e) {
  }

  try {
    CTensor<int, 2> m5
    {
      {3, 5, 8},
      {2, 7}
    };
    assert("missing an exception" == nullptr);
  } catch (const std::exception &e) {
  }

  try {
    m1.slice(CRange(0, 4), CRange(0, 2));
    assert("missing an exception" == nullptr);
  } catch (const std::exception &e) {
  }

  return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
