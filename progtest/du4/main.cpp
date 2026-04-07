#ifndef __PROGTEST__
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <cctype>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <set>
#include <list>
#include <map>
#include <vector>
#include <queue>
#include <string>
#include <algorithm>
#include <memory>
#include <functional>
#include <stdexcept>
#include <compare>
#include <iterator>
#endif /* __PROGTEST */

#ifndef __PROGTEST__
class CTimeStamp {
public:
  CTimeStamp(int year,
             int month,
             int day,
             int hour,
             int minute,
             int sec);

  std::strong_ordering operator <=
  >
  (
  const CTimeStamp &x
  )
  const;

  bool operator ==(const CTimeStamp &x) const;

  friend std::ostream &operator <<(std::ostream &os,
                                   const CTimeStamp &x);
};

//=================================================================================================
class CMailBody {
public:
  CMailBody(int size,
            const char data[]);

  // copy cons/op=/destructor is correctly implemented in the testing environment
  friend std::ostream &operator <<(std::ostream &os,
                                   const CMailBody &x) {
    return os << "mail body: " << x.m_Size << " B";
  }

private:
  int m_Size;
  char *m_Data;
};

//=================================================================================================
class CAttach {
public:
  CAttach(int x)
    : m_X(x) {
  }

  void addRef() {
    m_RefCnt++;
  }

  void release() {
    if (!--m_RefCnt)
      delete this;
  }

private:
  int m_X;
  int m_RefCnt = 1;

  CAttach(const CAttach &x);

  CAttach &operator =(const CAttach &x);

  ~CAttach() = default;

  friend std::ostream &operator <<(std::ostream &os,
                                   const CAttach &x) {
    return os << "attachment: " << x.m_X << " B";
  }
};

//=================================================================================================
#endif /* __PROGTEST__, DO NOT remove */


class CMail {
public:
  CMail(const CTimeStamp &timeStamp,
        const std::string &from,
        const CMailBody &body,
        CAttach *attach);

  const std::string &from() const;

  const CMailBody &body() const;

  const CTimeStamp &timeStamp() const;

  CAttach *attachment() const;

  friend std::ostream &operator <<(std::ostream &os,
                                   const CMail &x);

private:
  // todo
};

//=================================================================================================
class CMailBox {
public:
  CMailBox();

  bool delivery(const CMail &mail);

  bool newFolder(const std::string &folderName);

  bool moveMail(const std::string &fromFolder,
                const std::string &toFolder);

  std::list<CMail> listMail(const std::string &folderName,
                            const CTimeStamp &from,
                            const CTimeStamp &to) const;

  std::set<std::string> listAddr(const CTimeStamp &from,
                                 const CTimeStamp &to) const;

private:
  // todo
};

//=================================================================================================
#ifndef __PROGTEST__
static std::string showMail(const std::list<CMail> &l) {
  std::ostringstream oss;
  for (const auto &x: l)
    oss << x << std::endl;
  return oss.str();
}

static std::string showUsers(const std::set<std::string> &s) {
  std::ostringstream oss;
  for (const auto &x: s)
    oss << x << std::endl;
  return oss.str();
}

void test() {
  CAttach *att;
  std::ostringstream oss;

  att = new CAttach(100);
  CMail testMail(CTimeStamp(2026, 1, 2, 12, 5, 0), "test@domain.cz", CMailBody(10, "test, test"), att);
  att->release();
  assert(testMail . timeStamp () == CTimeStamp ( 2026, 1, 2, 12, 5, 0 ));
  assert(testMail . from () == "test@domain.cz");
  att = testMail.attachment();
  oss << *att;
  att->release();
  assert(oss . str () == "attachment: 100 B");
  assert(showMail ( { testMail } ) == "2026-01-02 12:05:00 test@domain.cz mail body: 10 B + attachment: 100 B\n");

  CMailBox m0;
  assert(
    m0 . delivery ( CMail ( CTimeStamp ( 2024, 3, 31, 15, 24, 13 ), "user1@fit.cvut.cz", CMailBody ( 14,
      "mail content 1" ), nullptr ) ));
  assert(
    m0 . delivery ( CMail ( CTimeStamp ( 2024, 3, 31, 15, 26, 23 ), "user2@fit.cvut.cz", CMailBody ( 22,
      "some different content" ), nullptr ) ));
  att = new CAttach(200);
  assert(
    m0 . delivery ( CMail ( CTimeStamp ( 2024, 3, 31, 11, 23, 43 ), "boss1@fit.cvut.cz", CMailBody ( 14,
      "urgent message" ), att ) ));
  assert(
    m0 . delivery ( CMail ( CTimeStamp ( 2024, 3, 31, 18, 52, 27 ), "user1@fit.cvut.cz", CMailBody ( 14,
      "mail content 2" ), att ) ));
  att->release();
  att = new CAttach(97);
  assert(
    m0 . delivery ( CMail ( CTimeStamp ( 2024, 3, 31, 16, 12, 48 ), "boss1@fit.cvut.cz", CMailBody ( 24,
      "even more urgent message" ), att ) ));
  att->release();
  assert(showMail ( m0 . listMail ( "inbox",
      CTimeStamp ( 2000, 1, 1, 0, 0, 0 ),
      CTimeStamp ( 2050, 12, 31, 23, 59, 59 ) ) ) ==
    "2024-03-31 11:23:43 boss1@fit.cvut.cz mail body: 14 B + attachment: 200 B\n"
    "2024-03-31 15:24:13 user1@fit.cvut.cz mail body: 14 B\n"
    "2024-03-31 15:26:23 user2@fit.cvut.cz mail body: 22 B\n"
    "2024-03-31 16:12:48 boss1@fit.cvut.cz mail body: 24 B + attachment: 97 B\n"
    "2024-03-31 18:52:27 user1@fit.cvut.cz mail body: 14 B + attachment: 200 B\n");
  assert(showMail ( m0 . listMail ( "inbox",
      CTimeStamp ( 2024, 3, 31, 15, 26, 23 ),
      CTimeStamp ( 2024, 3, 31, 16, 12, 48 ) ) ) == "2024-03-31 15:26:23 user2@fit.cvut.cz mail body: 22 B\n"
    "2024-03-31 16:12:48 boss1@fit.cvut.cz mail body: 24 B + attachment: 97 B\n");
  assert(showUsers ( m0 . listAddr ( CTimeStamp ( 2000, 1, 1, 0, 0, 0 ),
      CTimeStamp ( 2050, 12, 31, 23, 59, 59 ) ) ) == "boss1@fit.cvut.cz\n"
    "user1@fit.cvut.cz\n"
    "user2@fit.cvut.cz\n");
  assert(showUsers ( m0 . listAddr ( CTimeStamp ( 2024, 3, 31, 15, 26, 23 ),
      CTimeStamp ( 2024, 3, 31, 16, 12, 48 ) ) ) == "boss1@fit.cvut.cz\n"
    "user2@fit.cvut.cz\n");

  CMailBox m1;
  assert(m1 . newFolder ( "work" ));
  assert(m1 . newFolder ( "spam" ));
  assert(!m1 . newFolder ( "spam" ));
  assert(
    m1 . delivery ( CMail ( CTimeStamp ( 2024, 3, 31, 15, 24, 13 ), "user1@fit.cvut.cz", CMailBody ( 14,
      "mail content 1" ), nullptr ) ));
  att = new CAttach(500);
  assert(
    m1 . delivery ( CMail ( CTimeStamp ( 2024, 3, 31, 15, 26, 23 ), "user2@fit.cvut.cz", CMailBody ( 22,
      "some different content" ), att ) ));
  att->release();
  assert(
    m1 . delivery ( CMail ( CTimeStamp ( 2024, 3, 31, 11, 23, 43 ), "boss1@fit.cvut.cz", CMailBody ( 14,
      "urgent message" ), nullptr ) ));
  att = new CAttach(468);
  assert(
    m1 . delivery ( CMail ( CTimeStamp ( 2024, 3, 31, 18, 52, 27 ), "user1@fit.cvut.cz", CMailBody ( 14,
      "mail content 2" ), att ) ));
  att->release();
  assert(
    m1 . delivery ( CMail ( CTimeStamp ( 2024, 3, 31, 16, 12, 48 ), "boss1@fit.cvut.cz", CMailBody ( 24,
      "even more urgent message" ), nullptr ) ));
  assert(showMail ( m1 . listMail ( "inbox",
      CTimeStamp ( 2000, 1, 1, 0, 0, 0 ),
      CTimeStamp ( 2050, 12, 31, 23, 59, 59 ) ) ) == "2024-03-31 11:23:43 boss1@fit.cvut.cz mail body: 14 B\n"
    "2024-03-31 15:24:13 user1@fit.cvut.cz mail body: 14 B\n"
    "2024-03-31 15:26:23 user2@fit.cvut.cz mail body: 22 B + attachment: 500 B\n"
    "2024-03-31 16:12:48 boss1@fit.cvut.cz mail body: 24 B\n"
    "2024-03-31 18:52:27 user1@fit.cvut.cz mail body: 14 B + attachment: 468 B\n");
  assert(showMail ( m1 . listMail ( "work",
    CTimeStamp ( 2000, 1, 1, 0, 0, 0 ),
    CTimeStamp ( 2050, 12, 31, 23, 59, 59 ) ) ) == "");
  assert(m1 . moveMail ( "inbox", "work" ));
  assert(showMail ( m1 . listMail ( "inbox",
    CTimeStamp ( 2000, 1, 1, 0, 0, 0 ),
    CTimeStamp ( 2050, 12, 31, 23, 59, 59 ) ) ) == "");
  assert(showMail ( m1 . listMail ( "work",
      CTimeStamp ( 2000, 1, 1, 0, 0, 0 ),
      CTimeStamp ( 2050, 12, 31, 23, 59, 59 ) ) ) == "2024-03-31 11:23:43 boss1@fit.cvut.cz mail body: 14 B\n"
    "2024-03-31 15:24:13 user1@fit.cvut.cz mail body: 14 B\n"
    "2024-03-31 15:26:23 user2@fit.cvut.cz mail body: 22 B + attachment: 500 B\n"
    "2024-03-31 16:12:48 boss1@fit.cvut.cz mail body: 24 B\n"
    "2024-03-31 18:52:27 user1@fit.cvut.cz mail body: 14 B + attachment: 468 B\n");
  assert(
    m1 . delivery ( CMail ( CTimeStamp ( 2024, 3, 31, 19, 24, 13 ), "user2@fit.cvut.cz", CMailBody ( 14,
      "mail content 4" ), nullptr ) ));
  att = new CAttach(234);
  assert(
    m1 . delivery ( CMail ( CTimeStamp ( 2024, 3, 31, 13, 26, 23 ), "user3@fit.cvut.cz", CMailBody ( 9, "complains" ),
      att ) ));
  att->release();
  assert(showMail ( m1 . listMail ( "inbox",
      CTimeStamp ( 2000, 1, 1, 0, 0, 0 ),
      CTimeStamp ( 2050, 12, 31, 23, 59, 59 ) ) ) ==
    "2024-03-31 13:26:23 user3@fit.cvut.cz mail body: 9 B + attachment: 234 B\n"
    "2024-03-31 19:24:13 user2@fit.cvut.cz mail body: 14 B\n");
  assert(showMail ( m1 . listMail ( "work",
      CTimeStamp ( 2000, 1, 1, 0, 0, 0 ),
      CTimeStamp ( 2050, 12, 31, 23, 59, 59 ) ) ) == "2024-03-31 11:23:43 boss1@fit.cvut.cz mail body: 14 B\n"
    "2024-03-31 15:24:13 user1@fit.cvut.cz mail body: 14 B\n"
    "2024-03-31 15:26:23 user2@fit.cvut.cz mail body: 22 B + attachment: 500 B\n"
    "2024-03-31 16:12:48 boss1@fit.cvut.cz mail body: 24 B\n"
    "2024-03-31 18:52:27 user1@fit.cvut.cz mail body: 14 B + attachment: 468 B\n");
  assert(m1 . moveMail ( "inbox", "work" ));
  assert(showMail ( m1 . listMail ( "inbox",
    CTimeStamp ( 2000, 1, 1, 0, 0, 0 ),
    CTimeStamp ( 2050, 12, 31, 23, 59, 59 ) ) ) == "");
  assert(showMail ( m1 . listMail ( "work",
      CTimeStamp ( 2000, 1, 1, 0, 0, 0 ),
      CTimeStamp ( 2050, 12, 31, 23, 59, 59 ) ) ) == "2024-03-31 11:23:43 boss1@fit.cvut.cz mail body: 14 B\n"
    "2024-03-31 13:26:23 user3@fit.cvut.cz mail body: 9 B + attachment: 234 B\n"
    "2024-03-31 15:24:13 user1@fit.cvut.cz mail body: 14 B\n"
    "2024-03-31 15:26:23 user2@fit.cvut.cz mail body: 22 B + attachment: 500 B\n"
    "2024-03-31 16:12:48 boss1@fit.cvut.cz mail body: 24 B\n"
    "2024-03-31 18:52:27 user1@fit.cvut.cz mail body: 14 B + attachment: 468 B\n"
    "2024-03-31 19:24:13 user2@fit.cvut.cz mail body: 14 B\n");
}

int main() {
  test();
  return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
