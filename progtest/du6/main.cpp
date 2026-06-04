#ifndef __PROGTEST__
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <cctype>
#include <cmath>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <memory>
#include <functional>
#include <stdexcept>
#include <compare>
#include "ipaddress.h"
using namespace std::literals;
#endif /* __PROGTEST__ */

class CRec {
protected:
  std::string name_;

  /*!
   * @param os printing without name!!!
   */
  virtual void print(std::ostream &os) const = 0;
public:
  virtual ~CRec() = default;

  explicit CRec(const std::string & name): name_(name) {};

  const std::string & name() const {
    return name_;
  }

  // abstract method, every should implement by its own
  virtual const std::string type() const = 0;

  virtual std::unique_ptr<CRec> clone() const = 0;

  /*!
   * call this on every cmp_eq implementation
  */
  virtual bool cmp_eq(const CRec & rhs) const {
    return name_ == rhs.name_;
  }

  friend std::ostream & operator<< ( std::ostream & os, const CRec & rec ) {
    os << rec.name_ << ' ' << rec.type() <<  ' ';
    rec.print(os);
    return os;
  }

  // this will call == on every seperate cmp_eq implementation
  friend bool operator==(const CRec &lhs, const CRec &rhs) {
    return lhs.cmp_eq(rhs);
  };
};

class CRecA : public CRec
{
  CIPv4 record_;

  public:
    CRecA(const std::string &name, const CIPv4 record) : CRec(name), record_(record) {};

    const std::string type() const override {
      return "A";
    }

    void print(std::ostream &os) const override {
      os << record_;
    };

    std::unique_ptr<CRec> clone() const override {
      return std::make_unique<CRecA>(*this);
    }

  bool cmp_eq(const CRec &rhs) const override {
      if (!CRec::cmp_eq(rhs)) return false;

      const auto *r = dynamic_cast<const CRecA*>(&rhs);

      // nullptr on err
      if (!r) return false;

      return record_ == r->record_;
    }
};

class CRecAAAA: public CRec
{
  CIPv6 record_;

public:
  CRecAAAA(const std::string &name, const CIPv6 record) : CRec(name), record_(record) {}

  void print(std::ostream &os) const override {
    os << record_;
  };

  std::unique_ptr<CRec> clone() const override {
    return std::make_unique<CRecAAAA>(*this);
  }

  const std::string type() const override {
    return "AAAA";
  }

  bool cmp_eq(const CRec &rhs) const override {
    if (!CRec::cmp_eq(rhs)) return false;

    const auto *r = dynamic_cast<const CRecAAAA*>(&rhs);

    // nullptr on err
    if (!r) return false;

    return record_ == r->record_;
  }
};

class CRecMX: public CRec
{
  std::string record_;
  int priority_;

  public:
    CRecMX(const std::string &name, const std::string &record, int priority) : CRec(name), record_(record), priority_(priority) {}

    const std::string type() const override {
      return "MX";
    }

   void print(std::ostream &os) const override {
      os << priority_ << ' ' << record_;
    };

    std::unique_ptr<CRec> clone() const override {
      return std::make_unique<CRecMX>(*this);
    }

  bool cmp_eq(const CRec &rhs) const override {
      if (!CRec::cmp_eq(rhs)) return false;

      const auto *r = dynamic_cast<const CRecMX*>(&rhs);

      // nullptr on err
      if (!r) return false;

      if (priority_ != r->priority_)
        return false;

      return record_ == r->record_;
    }
};

class search_result {
  std::vector<std::shared_ptr<CRec>> results_;

public:
  search_result() = default;

  void add(const std::shared_ptr<CRec> & rec) {
    results_.push_back(rec);
  }

  size_t size() const {
    return results_.size();
  }

  const CRec& operator[](size_t idx) const {
    if (idx >= results_.size())
      throw std::out_of_range(std::to_string(idx));
    return *results_[idx];
  }

  friend std::ostream& operator<<(std::ostream &os, const search_result &sr) {
    for (const auto & rec : sr.results_)
      os << *rec << "\n";
    return os;
  }
};

class CZone
{
  std::string name_;

  std::list<std::shared_ptr<CRec>> records_;
  std::unordered_map<std::string, std::vector<std::list<std::shared_ptr<CRec>>::iterator>> index_;

public:
  CZone(const std::string & name): name_(name) {};

  CZone(const CZone& other) : name_(other.name_) {
    for (const auto& rec : other.records_) {
      records_.push_back(rec);
      index_[rec->name()].push_back(std::prev(records_.end()));
    }
  }

  CZone& operator=(const CZone& other) {
    if (this == &other) return *this;

    name_ = other.name_;
    records_.clear();
    index_.clear();

    for (const auto& rec : other.records_) {
      records_.push_back(rec);
      index_[rec->name()].push_back(std::prev(records_.end()));
    }
    return *this;
  }

  bool add(const CRec & item) {
    auto& vec = index_[item.name()];

    for (auto it : vec) {
      if ((*it)->cmp_eq(item)) return false;
    }

    records_.push_back(item.clone());
    vec.push_back(std::prev(records_.end()));
    return true;
  }

  bool del(const CRec & item) {
    auto it_map = index_.find(item.name());
    if (it_map == index_.end()) return false;

    auto& vec = it_map->second;

    for (auto vit = vec.begin(); vit != vec.end(); ++vit) {
      if ((**vit)->cmp_eq(item)) {
        records_.erase(*vit);
        vec.erase(vit);
        return true;
      }
    }
    return false;
  }

  search_result search(const std::string &name) const {
    search_result result;
    auto it_map = index_.find(name);


    if (it_map != index_.end()) {
      for (auto it : it_map->second) {
        result.add(*it);
      }
    }

    return result;
  }

  friend std::ostream& operator<<(std::ostream &os, const CZone &zone) {
    os << zone.name_ << "\n";

    size_t i = 0;
    size_t size = zone.records_.size();

    for (auto it = zone.records_.begin(); it != zone.records_.end(); ++it, ++i) {
      bool last = (i == size - 1);
      os << (last ? " \\- " : " +- ");
      os << **it;
      os << "\n";
    }

    return os;
  }
};

#ifndef __PROGTEST__
int main ()
{
  std::ostringstream oss;

  CZone z0 ( "fit.cvut.cz" );
  assert ( z0 . add ( CRecA ( "progtest", CIPv4 ( "147.32.232.142" ) ) ) == true );
  assert ( z0 . add ( CRecAAAA ( "progtest", CIPv6 ( "2001:718:2:2902:0:1:2:3" ) ) ) == true );
  assert ( z0 . add ( CRecA ( "courses", CIPv4 ( "147.32.232.158" ) ) ) == true );
  assert ( z0 . add ( CRecA ( "courses", CIPv4 ( "147.32.232.160" ) ) ) == true );
  assert ( z0 . add ( CRecA ( "courses", CIPv4 ( "147.32.232.159" ) ) ) == true );
  assert ( z0 . add ( CRecAAAA ( "progtest", CIPv6 ( "2001:718:2:2902:1:2:3:4" ) ) ) == true );
  assert ( z0 . add ( CRecMX ( "courses", "relay.fit.cvut.cz.", 0 ) ) == true );
  assert ( z0 . add ( CRecMX ( "courses", "relay2.fit.cvut.cz.", 10 ) ) == true );
  oss . str ( "" );
  oss << z0;
  assert ( oss . str () ==
    "fit.cvut.cz\n"
    " +- progtest A 147.32.232.142\n"
    " +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    " +- courses A 147.32.232.158\n"
    " +- courses A 147.32.232.160\n"
    " +- courses A 147.32.232.159\n"
    " +- progtest AAAA 2001:718:2:2902:1:2:3:4\n"
    " +- courses MX 0 relay.fit.cvut.cz.\n"
    " \\- courses MX 10 relay2.fit.cvut.cz.\n" );
  assert ( z0 . search ( "progtest" ) . size () == 3 );
  oss . str ( "" );
  oss << z0 . search ( "progtest" );
  assert ( oss . str () ==
    "progtest A 147.32.232.142\n"
    "progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    "progtest AAAA 2001:718:2:2902:1:2:3:4\n" );
  assert ( z0 . del ( CRecA ( "courses", CIPv4 ( "147.32.232.160" ) ) ) == true );
  assert ( z0 . add ( CRecA ( "courses", CIPv4 ( "147.32.232.122" ) ) ) == true );
  oss . str ( "" );
  oss << z0;
  assert ( oss . str () ==
    "fit.cvut.cz\n"
    " +- progtest A 147.32.232.142\n"
    " +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    " +- courses A 147.32.232.158\n"
    " +- courses A 147.32.232.159\n"
    " +- progtest AAAA 2001:718:2:2902:1:2:3:4\n"
    " +- courses MX 0 relay.fit.cvut.cz.\n"
    " +- courses MX 10 relay2.fit.cvut.cz.\n"
    " \\- courses A 147.32.232.122\n" );
  assert ( z0 . search ( "courses" ) . size () == 5 );
  oss . str ( "" );
  oss << z0 . search ( "courses" );
  assert ( oss . str () ==
    "courses A 147.32.232.158\n"
    "courses A 147.32.232.159\n"
    "courses MX 0 relay.fit.cvut.cz.\n"
    "courses MX 10 relay2.fit.cvut.cz.\n"
    "courses A 147.32.232.122\n" );
  oss . str ( "" );
  oss << z0 . search ( "courses" ) [ 0 ];
  assert ( oss . str () == "courses A 147.32.232.158" );
  assert ( z0 . search ( "courses" ) [ 0 ] . name () == "courses" );
  assert ( z0 . search ( "courses" ) [ 0 ] . type () == "A" );
  oss . str ( "" );
  oss << z0 . search ( "courses" ) [ 1 ];
  assert ( oss . str () == "courses A 147.32.232.159" );
  assert ( z0 . search ( "courses" ) [ 1 ] . name () == "courses" );
  assert ( z0 . search ( "courses" ) [ 1 ] . type () == "A" );
  oss . str ( "" );
  oss << z0 . search ( "courses" ) [ 2 ];
  assert ( oss . str () == "courses MX 0 relay.fit.cvut.cz." );
  assert ( z0 . search ( "courses" ) [ 2 ] . name () == "courses" );
  assert ( z0 . search ( "courses" ) [ 2 ] . type () == "MX" );
  try
  {
    oss . str ( "" );
    oss << z0 . search ( "courses" ) [ 10 ];
    assert ( "No exception thrown!" == nullptr );
  }
  catch ( const std::out_of_range & e )
  {
  }
  catch ( ... )
  {
    assert ( "Invalid exception thrown!" == nullptr );
  }
  dynamic_cast<const CRecAAAA &> ( z0 . search ( "progtest" ) [ 1 ] );
  CZone z1 ( "fit2.cvut.cz" );
  z1 . add ( z0 . search ( "progtest" ) [ 2 ] );
  z1 . add ( z0 . search ( "progtest" ) [ 0 ] );
  z1 . add ( z0 . search ( "progtest" ) [ 1 ] );
  z1 . add ( z0 . search ( "courses" ) [ 2 ] );
  oss . str ( "" );
  oss << z1;
  assert ( oss . str () ==
    "fit2.cvut.cz\n"
    " +- progtest AAAA 2001:718:2:2902:1:2:3:4\n"
    " +- progtest A 147.32.232.142\n"
    " +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    " \\- courses MX 0 relay.fit.cvut.cz.\n" );
  dynamic_cast<const CRecA &> ( z1 . search ( "progtest" ) [ 1 ] );

  CZone z2 ( "fit.cvut.cz" );
  assert ( z2 . add ( CRecA ( "progtest", CIPv4 ( "147.32.232.142" ) ) ) == true );
  assert ( z2 . add ( CRecAAAA ( "progtest", CIPv6 ( "2001:718:2:2902:0:1:2:3" ) ) ) == true );
  assert ( z2 . add ( CRecA ( "progtest", CIPv4 ( "147.32.232.144" ) ) ) == true );
  assert ( z2 . add ( CRecMX ( "progtest", "relay.fit.cvut.cz.", 10 ) ) == true );
  assert ( z2 . add ( CRecA ( "progtest", CIPv4 ( "147.32.232.142" ) ) ) == false );
  assert ( z2 . del ( CRecA ( "progtest", CIPv4 ( "147.32.232.140" ) ) ) == false );
  assert ( z2 . del ( CRecA ( "progtest", CIPv4 ( "147.32.232.142" ) ) ) == true );
  assert ( z2 . del ( CRecA ( "progtest", CIPv4 ( "147.32.232.142" ) ) ) == false );
  assert ( z2 . add ( CRecMX ( "progtest", "relay.fit.cvut.cz.", 20 ) ) == true );
  assert ( z2 . add ( CRecMX ( "progtest", "relay.fit.cvut.cz.", 10 ) ) == false );
  oss . str ( "" );
  oss << z2;
  assert ( oss . str () ==
    "fit.cvut.cz\n"
    " +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    " +- progtest A 147.32.232.144\n"
    " +- progtest MX 10 relay.fit.cvut.cz.\n"
    " \\- progtest MX 20 relay.fit.cvut.cz.\n" );
  assert ( z2 . search ( "progtest" ) . size () == 4 );
  oss . str ( "" );
  oss << z2 . search ( "progtest" );
  assert ( oss . str () ==
    "progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    "progtest A 147.32.232.144\n"
    "progtest MX 10 relay.fit.cvut.cz.\n"
    "progtest MX 20 relay.fit.cvut.cz.\n" );
  assert ( z2 . search ( "courses" ) . size () == 0 );
  oss . str ( "" );
  oss << z2 . search ( "courses" );
  assert ( oss . str () == "" );
  try
  {
    dynamic_cast<const CRecMX &> ( z2 . search ( "progtest" ) [ 0 ] );
    assert ( "Invalid type" == nullptr );
  }
  catch ( const std::bad_cast & e )
  {
  }

  CZone z4 ( "fit.cvut.cz" );
  assert ( z4 . add ( CRecA ( "progtest", CIPv4 ( "147.32.232.142" ) ) ) == true );
  assert ( z4 . add ( CRecA ( "courses", CIPv4 ( "147.32.232.158" ) ) ) == true );
  assert ( z4 . add ( CRecA ( "courses", CIPv4 ( "147.32.232.160" ) ) ) == true );
  assert ( z4 . add ( CRecA ( "courses", CIPv4 ( "147.32.232.159" ) ) ) == true );
  CZone z5 ( z4 );
  assert ( z4 . add ( CRecAAAA ( "progtest", CIPv6 ( "2001:718:2:2902:0:1:2:3" ) ) ) == true );
  assert ( z4 . add ( CRecAAAA ( "progtest", CIPv6 ( "2001:718:2:2902:1:2:3:4" ) ) ) == true );
  assert ( z5 . del ( CRecA ( "progtest", CIPv4 ( "147.32.232.142" ) ) ) == true );
  oss . str ( "" );
  oss << z4;
  assert ( oss . str () ==
    "fit.cvut.cz\n"
    " +- progtest A 147.32.232.142\n"
    " +- courses A 147.32.232.158\n"
    " +- courses A 147.32.232.160\n"
    " +- courses A 147.32.232.159\n"
    " +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    " \\- progtest AAAA 2001:718:2:2902:1:2:3:4\n" );
  oss . str ( "" );
  oss << z5;
  assert ( oss . str () ==
    "fit.cvut.cz\n"
    " +- courses A 147.32.232.158\n"
    " +- courses A 147.32.232.160\n"
    " \\- courses A 147.32.232.159\n" );
  z5 = z4;
  assert ( z4 . add ( CRecMX ( "courses", "relay.fit.cvut.cz.", 0 ) ) == true );
  assert ( z4 . add ( CRecMX ( "courses", "relay2.fit.cvut.cz.", 10 ) ) == true );
  oss . str ( "" );
  oss << z4;
  assert ( oss . str () ==
    "fit.cvut.cz\n"
    " +- progtest A 147.32.232.142\n"
    " +- courses A 147.32.232.158\n"
    " +- courses A 147.32.232.160\n"
    " +- courses A 147.32.232.159\n"
    " +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    " +- progtest AAAA 2001:718:2:2902:1:2:3:4\n"
    " +- courses MX 0 relay.fit.cvut.cz.\n"
    " \\- courses MX 10 relay2.fit.cvut.cz.\n" );
  oss . str ( "" );
  oss << z5;
  assert ( oss . str () ==
    "fit.cvut.cz\n"
    " +- progtest A 147.32.232.142\n"
    " +- courses A 147.32.232.158\n"
    " +- courses A 147.32.232.160\n"
    " +- courses A 147.32.232.159\n"
    " +- progtest AAAA 2001:718:2:2902:0:1:2:3\n"
    " \\- progtest AAAA 2001:718:2:2902:1:2:3:4\n" );


  return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
