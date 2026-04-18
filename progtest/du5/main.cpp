#ifndef __PROGTEST__
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <cmath>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <list>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <stack>
#include <queue>
#include <deque>
#include <algorithm>
#include <memory>
#include <compare>
#include <stdexcept>
#include <optional>
using namespace std::literals;
#endif /* __PROGTEST__ */

class CAuditFilter
{
  public:
    // todo
};

class CVisitorLog
{
  public:
    std::set<std::string> search ( filter ) const
    {
    }
    // todo
};

class CMilBase
{
  public:
    CMilBase         ()
    {
    }
    void readBase    ( const std::string & baseFilename )
    {
    }

    CVisitorLog processLog  ( const std::string & logFilename )
    {
    }
  private:
    // todo
};

#ifndef __PROGTEST__
void basicTests ( const CVisitorLog & log )
{
  assert ( log . search ( CAuditFilter ( "headquarters" ) )
           == ( std::set<std::string> { "Alice Cooper", "George Peterson", "Henry Montgomery", "Jane Bush", "John Smith", "Tim Cook", "Robert Smith" } ) );
  assert ( log . search ( CAuditFilter ( "flyingSaucerHangar" ) )
           == ( std::set<std::string> { "Alice Cooper", "Henry Montgomery", "Jane Bush", "John Smith", "Robert Smith" } ) );
  assert ( log . search ( CAuditFilter ( "flyingSaucerHangar" ) . notAfter ( 2026, 3, 10, 8, 0 ) )
           == ( std::set<std::string> { "Henry Montgomery", "Robert Smith" } ) );
  assert ( log . search ( CAuditFilter ( "flyingSaucerHangar" ) . notBefore ( 2026, 3, 11, 12, 0 ) )
           == ( std::set<std::string> { "Henry Montgomery", "Jane Bush", "John Smith" } ) );
  assert ( log . search ( CAuditFilter ( "flyingSaucerHangar" ) . notBefore ( 2026, 3, 10, 9, 0 ) . notAfter ( 2026, 3, 10, 13, 0 ) )
           == ( std::set<std::string> { "Alice Cooper", "Henry Montgomery", "Jane Bush", "John Smith" } ) );
  assert ( log . search ( CAuditFilter ( "flyingSaucerHangar" ) . notBefore ( 2026, 3, 10, 9, 5 ) . notAfter ( 2026, 3, 10, 9, 5 ) )
           == ( std::set<std::string> { "Henry Montgomery" } ) );
  assert ( log . search ( CAuditFilter ( "flyingSaucerHangar" ) . notBefore ( 2026, 3, 10, 9, 6 ) . notAfter ( 2026, 3, 10, 9, 6 ) )
           == ( std::set<std::string> { "Henry Montgomery", "John Smith" } ) );
  assert ( log . search ( CAuditFilter ( "flyingSaucerHangar" ) . notBefore ( 2026, 3, 10, 9, 24 ) . notAfter ( 2026, 3, 10, 9, 24 ) )
           == ( std::set<std::string> { "Alice Cooper", "Henry Montgomery", "John Smith" } ) );
  assert ( log . search ( CAuditFilter ( "flyingSaucerHangar" ) . notBefore ( 2026, 3, 10, 9, 25 ) . notAfter ( 2026, 3, 10, 9, 25 ) )
           == ( std::set<std::string> { "Alice Cooper", "Henry Montgomery" } ) );
  assert ( log . search ( CAuditFilter ( "flyingSaucerHangar" ) . notBefore ( 2024, 2, 1, 0, 0 ) . notAfter ( 2024, 3, 31, 0, 0 ) )
           == ( std::set<std::string> { "Robert Smith", "Henry Montgomery" } ) );
  assert ( log . search ( CAuditFilter ( "flyingSaucerHangar" ) . notBefore ( 2025, 2, 1, 0, 0 ) . notAfter ( 2025, 3, 31, 0, 0 ) )
           == ( std::set<std::string> { "Henry Montgomery" } ) );
  assert ( log . search ( CAuditFilter ( "privateParking" )  )
           == ( std::set<std::string> { "<classified>" } ) );
}

int main ()
{
  class CMilBase b;
  b . readBase ( "base.txt" );

  for ( const char * fn : std::initializer_list<const char *> { "in1.log", "in2.log", "in3.log" } )
    basicTests ( b . processLog ( fn ) );
  return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */