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
#include <utility>
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
  CTimeStamp(int year, int month, int day, int hour, int minute, int sec): year(year), month(month), day(day), hour(hour), minute(minute), sec(sec) {};

  CTimeStamp(CTimeStamp && from) noexcept : year(from.year), month(from.month), day(from.day), hour(from.hour), minute(from.minute), sec(from.sec) {
    from.year = 0;
    from.month = 0;
    from.day = 0;
    from.hour = 0;
    from.minute = 0;
    from.sec = 0;
  };

  CTimeStamp(const CTimeStamp & from) = default;

  ~CTimeStamp() = default;

  // checks all values separatly
  std::strong_ordering operator<=>(const CTimeStamp &x) const = default;
  bool operator==(const CTimeStamp &x) const = default;

  friend std::ostream &operator <<(std::ostream &os, const CTimeStamp &x) {
    //           set fill once;
    return os << std::setfill('0')
      << std::setw(4) << x.year << '-'
      << std::setw(2) << x.month << '-'
      << std::setw(2) << x.day << ' '
      << std::setw(2) << x.hour << ':'
      << std::setw(2) << x.minute << ':'
      << std::setw(2) << x.sec
      << std::setfill(' ');
        // reseting set fill
  };

private:
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int sec;
};

//=================================================================================================
class CMailBody {
public:
  CMailBody(int size, const char data[]): m_Size(size), m_Data(new char[m_Size]) {
    // copy
    for (int i = 0; i < m_Size; i++) {
      m_Data[i] =  data[i];
    }
  };

  CMailBody(CMailBody && from) noexcept : m_Size(from.m_Size), m_Data(from.m_Data) {
    from.m_Size = 0;
    from.m_Data = nullptr;
  };

  CMailBody(const CMailBody & from): m_Size(from.m_Size), m_Data(new char[m_Size]) {
    for (int i = 0; i < m_Size; i++) {
      m_Data[i] = from.m_Data[i];
    }
  };

  ~CMailBody() {
    delete [] m_Data;
  };

  // copy cons/op=/destructor is correctly implemented in the testing environment
  friend std::ostream &operator <<(std::ostream &os, const CMailBody &x) {
    return os << "mail body: " << x.m_Size << " B";
  }

private:
  int m_Size;
  char *m_Data;
};

//=================================================================================================
class CAttach {
public:
  explicit CAttach(const int x): m_X(x) {};

  CAttach(CAttach && from) noexcept: m_X(from.m_X), m_RefCnt(from.m_RefCnt) {
    from.m_RefCnt = 0;
    from.m_X = 0;
  };

  void addRef() {
    m_RefCnt++;
  }

  void release() {
    if (!--m_RefCnt)
      delete this;
  }

  CAttach(const CAttach &from) = default;

  CAttach & operator= (const CAttach &from) = default;

  friend std::ostream &operator <<(std::ostream &os, const CAttach &x) {
    return os << "attachment: " << x.m_X << " B";
  }

private:
  int m_X;
  int m_RefCnt = 1;
};

//=================================================================================================
#endif /* __PROGTEST__, DO NOT remove */
class CMail {
public:
  CMail(const CTimeStamp &timeStamp, const std::string &from, const CMailBody &body, CAttach *attach)
    : time_stamp_(timeStamp), from_(from), body_(body), attachment_(attach) {
    if (attach != nullptr) attach->addRef();
  };

  CMail(CMail && from) noexcept: time_stamp_(std::move(from.time_stamp_)), from_(std::move(from.from_)), body_(std::move(from.body_)), attachment_(std::move(from.attachment_)) {};

  CMail(const CMail & from) : time_stamp_(from.time_stamp_), from_(from.from_), body_(from.body_), attachment_(from.attachment()) {};

  ~CMail() = default;

  const std::string & from() const {
    return from_;
  };

  const CMailBody & body() const {
    return body_;
  };

  const CTimeStamp &timeStamp() const {
    return time_stamp_;
  };

  CAttach * attachment() const {
    if (attachment_ != nullptr) {
      attachment_->addRef();
    }
    return attachment_;
  };

  friend std::ostream &operator <<(std::ostream &os, const CMail &x) {
    os << x.timeStamp() << ' ' << x.from() << ' ' << x.body();

    if (x.attachment() != nullptr) os << " + " << *(x.attachment_);

    return os;
  };

  std::weak_ordering operator <=>(const CMail &rhs) const {
    return this->timeStamp() <=> rhs.timeStamp();
  };
private:
  CTimeStamp time_stamp_;
  std::string from_;
  CMailBody body_;
  CAttach * attachment_ = nullptr;
};

//=================================================================================================
class CMailBox {
public:
  CMailBox() {
    addrs.emplace("inbox");
  };

  CMailBox(const CMailBox & from) = default;

  bool delivery(const CMail & mail) {
    return insert("inbox", mail);
  }

  bool newFolder(const std::string & folderName) {
    return addrs.emplace(folderName).second;
  };

  bool moveMail(const std::string &fromFolder, const std::string &toFolder) {
    auto range = folders.equal_range(fromFolder);

    if (fromFolder == toFolder)
      return true;

    if (range.first == folders.end())
      return false;

    while (true)  {
      // extracts content to node, moved;
      auto node = folders.extract(fromFolder);
      // check if found all
      if (node.empty())
        break;

      // change folder
      node.key() = toFolder;
      // move back to array
      folders.insert(std::move(node));
    }

    return true;
  };

  std::list<CMail> listMail(const std::string &folderName, const CTimeStamp &from, const CTimeStamp &to) const {
    auto [fst, snd] = folders.equal_range(folderName);

    if (fst == snd)
      return {};

    std::list<CMail> mails;

    for (auto it = fst; it != snd; ++it) {
      if (auto timestamp = it->second.timeStamp(); timestamp >= from && timestamp <= to) {
        mails.emplace_back(it->second);
      }
    }

    mails.sort();

    return mails;
  };

  std::set<std::string> listAddr(const CTimeStamp &from, const CTimeStamp &to) const {
    std::set<std::string> addrs_tmp;
    for (const auto & folder : folders) {
      if (auto timestamp = folder.second.timeStamp(); timestamp >= from && timestamp <= to) {
        addrs_tmp.emplace(folder.second.from());
      }
    }

    return addrs_tmp;
  };

  void printAddrs() {
    for (const auto &x: addrs) {
      std::cout << x << std::endl;
    }
  }

  void printMail() {
    for (const auto &x: folders) {
      std::cout << x.first << x.second << std::endl;
    }
  }

private:
  std::multimap<std::string, CMail> folders;
  std::set<std::string> addrs;

  /*!
   * constructs copy of CMail referenced value
   */
  bool insert(const std::string & folder_name, const CMail & mail) {
    if (!addrs.contains(folder_name))
      return false;
    folders.emplace(folder_name, mail);
    return true;
  }
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
  // endless loop
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
