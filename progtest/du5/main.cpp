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
#include <utility>
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

bool divisible(int number, int divisor) {
    return number % divisor == 0;
}

class time_c {
    int year;
    int month;
    int day;
    int hour;
    int minute;

    static bool leap_year(const int _year) {
        return (divisible(_year, 4) && !divisible(_year, 100)) || (divisible(_year, 400));
    }

    static int get_days(const int month) {
        int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        return days_in_month[month - 1];
    }

    static int get_days(const int month, const int year) {
        if (month == 2 && leap_year(year)) {
            return 29;
        }
        return get_days(month);
    }

    auto validate() const {
        return validate(*this);
    }

    static bool validate(const time_c &time) {
        if (time.month < 1 || time.month > 12)
            return false;
        if (time.day > get_days(time.month, time.year) || time.day < 1)
            return false;
        if (time.hour < 0 || time.hour > 23)
            return false;
        if (time.minute < 0 || time.minute > 59)
            return false;

        return true;
    }

public:
    time_c() = default;

    time_c(int year, int month, int day, int hour, int minute)
        : year(year), month(month), day(day), hour(hour), minute(minute) {
        if (!validate())
            throw std::runtime_error("date: parsing error");
    }

    friend std::istream & operator>>(std::istream & is, time_c & time) {
        int y, m, d, h, min;
        char sep1, sep2, sep3, sep4;

        if (is >> y >> sep1 >> m >> sep2 >> d >> std::noskipws >> sep3 >> std::skipws >> h >> sep4 >> min) {
            if (sep1 == '-' && sep2 == '-' && sep3 == ' ' && sep4 == ':') {
                time_c temp;
                temp.year = y; temp.month = m; temp.day = d; temp.hour = h; temp.minute = min;
                if (temp.validate()) {
                    time = temp;
                    return is;
                }
            }
        }
        is.setstate(std::ios::failbit);
        return is;
    }

    friend std::strong_ordering operator<=>(const time_c & lhs, const time_c & rhs) = default;
    friend bool operator==(const time_c & lhs, const time_c & rhs) = default;

    time_c operator+(int minutes) const {
        int y = year, mo = month, d = day, h = hour, mi = minute + minutes;

        h  += mi / 60;  mi %= 60;
        d  += h  / 24;  h  %= 24;

        while (d > get_days(mo, y)) {
            d -= get_days(mo, y);
            if (++mo > 12) { mo = 1; y++; }
        }

        return time_c(y, mo, d, h, mi);
    }

    time_c operator-(int minutes_to_sub) const {
        int y = year, mo = month, d = day, h = hour, mi = minute - minutes_to_sub;

        while (mi < 0) { mi += 60; h--; }
        while (h  < 0) { h  += 24; d--; }
        while (d  < 1) {
            if (--mo < 1) { mo = 12; y--; }
            d += get_days(mo, y);
        }

        return time_c(y, mo, d, h, mi);
    }
};

struct CAuditFilter {
    std::string zone;
    std::optional<time_c> not_before;
    std::optional<time_c> not_after;

    CAuditFilter(std::string zone) : zone(std::move(zone)) {}

    CAuditFilter notBefore(int year, int month, int day, int hour, int minute) const {
        CAuditFilter filter(*this);
        filter.not_before = time_c(year, month, day, hour, minute);
        return filter;
    }

    CAuditFilter notAfter(int year, int month, int day, int hour, int minute) const {
        CAuditFilter filter(*this);
        filter.not_after = time_c(year, month, day, hour, minute);
        return filter;
    }
};

class path_graph {
    std::unordered_map<std::string, std::vector<std::pair<std::string, int>>> paths;

    void insert(const std::string & from, const std::string & to, int minutes) {
        paths[from].emplace_back(to, minutes);
    }

public:
    void add(const std::string & from, const std::string & to, int minutes) {
        insert(from, to, minutes);
        insert(to, from, minutes);
    }

    int shortest_time(const std::string & from, const std::string & to) const {
        if (from == to) return 0;

        // Dijkstra's algorithm requires a priority queue for weighted edges
        std::priority_queue<std::pair<int, std::string>,
                            std::vector<std::pair<int, std::string>>,
                            std::greater<>> pq;
        std::unordered_map<std::string, int> distances;

        pq.emplace(0, from);
        distances[from] = 0;

        while (!pq.empty()) {
            auto [dist, current] = pq.top();
            pq.pop();

            if (dist > distances[current]) continue;
            if (current == to) return dist;

            auto it = paths.find(current);
            if (it == paths.end()) continue;

            for (const auto & [neighbor, minutes] : it->second) {
                int new_dist = dist + minutes;
                auto d_it = distances.find(neighbor);

                if (d_it == distances.end() || new_dist < d_it->second) {
                    distances[neighbor] = new_dist;
                    pq.emplace(new_dist, neighbor);
                }
            }
        }
        return -1;
    }
};

struct Event {
    time_c time;
    std::string zone;

    // Operátor potřebný pro chronologické seřazení událostí
    bool operator<(const Event & other) const {
        return time < other.time;
    }
};

class Person {
    std::string name;
    std::vector<Event> events;

public:
    Person(std::string name_) : name(std::move(name_)) {}

    void add_event(std::string zone, const time_c & time) {
        events.push_back({time, std::move(zone)});
    }

    bool could_visit(const std::string & target_zone,
                     const std::optional<time_c> & not_before,
                     const std::optional<time_c> & not_after,
                     const path_graph & graph) const {

        // Vytvoříme kopii událostí a bezpečně je chronologicky seřadíme
        std::vector<Event> sorted_events = events;
        std::sort(sorted_events.begin(), sorted_events.end());

        // Procházíme události po dvojicích (Vstup -> Výstup)
        for (size_t i = 0; i < sorted_events.size(); i += 2) {
            const Event & entry = sorted_events[i];
            bool has_exit = (i + 1 < sorted_events.size());

            int travel_in = graph.shortest_time(entry.zone, target_zone);
            if (travel_in < 0) continue;

            time_c earliest = entry.time + travel_in;

            if (has_exit) {
                const Event & exit = sorted_events[i + 1];
                int travel_out = graph.shortest_time(target_zone, exit.zone);
                if (travel_out < 0) continue;

                time_c latest = exit.time - travel_out;

                // Pokud earliest > latest, fyzicky nelze stihnout cestu tam a zpět.
                // (Ponecháno >, protože earliest == latest znamená korektní návštěvu na 0 minut - tzv. jen otočka)
                if (earliest > latest) continue;

                // Kontrola překryvu intervalu pobytu s hledaným časem
                if (not_after.has_value()  && earliest > *not_after)  continue;
                if (not_before.has_value() && latest   < *not_before) continue;
            } else {
                // Osoba vešla, ale log o výstupu neexistuje (stále je na základně)
                if (not_after.has_value() && earliest > *not_after) continue;
            }

            return true;
        }
        return false;
    }
};

class CVisitorLog {
    std::map<std::string, Person> people;
    std::shared_ptr<path_graph> path_graph_i;

    void insert_log(std::istream & is, const std::string & zone, size_t lines_cnt) {
        std::string line;
        std::getline(is, line); // Konzumace zbytku řádku za TEXT hlavičkou

        for (size_t i = 0; i < lines_cnt; i++) {
            if (!std::getline(is, line)) break;
            if (line.empty()) continue;

            std::istringstream iss(line);
            time_c time_tmp;

            if (!(iss >> time_tmp)) {
                throw std::runtime_error("parsing error");
            }

            std::string name;
            std::getline(iss >> std::ws, name);

            while (!name.empty() && std::isspace(static_cast<unsigned char>(name.back()))) {
                name.pop_back();
            }

            if (name.empty()) {
                throw std::runtime_error("parsing error");
            }

            // Místo řešení "kde zrovna osoba je" jen tupě přidáme log event
            auto result = people.find(name);
            if (result == people.end()) {
                auto [it, inserted] = people.emplace(name, Person(name));
                it->second.add_event(zone, time_tmp);
            } else {
                result->second.add_event(zone, time_tmp);
            }
        }
    }

public:
    CVisitorLog(const std::shared_ptr<path_graph> & path_graph_i_) : path_graph_i(path_graph_i_) {}

    std::set<std::string> search(const CAuditFilter & filter) const {
        std::set<std::string> names;
        for (const auto & [name, person] : people)
            if (person.could_visit(filter.zone, filter.not_before, filter.not_after, *path_graph_i))
                names.emplace(name);
        return names;
    }

    void insert(std::istream & is) {
        std::string mode;
        std::string zone;

        while (is >> mode >> zone) {
            if (mode == "TEXT") {
                int lines_cnt;
                if (is >> lines_cnt) {
                    insert_log(is, zone, lines_cnt);
                }
            }
        }
    }
};

class CMilBase {
    std::shared_ptr<path_graph> path = std::make_shared<path_graph>();

    void load_paths(std::ifstream &is) {
        std::string fst, sec, line;
        int minutes;

        while (std::getline(is, line)) {
            if (line.empty()) break;
            std::istringstream iss(line);
            if (iss >> fst >> sec >> minutes) {
                path->add(fst, sec, minutes);
            }
        }
    }

public:
    CMilBase() = default;

    void readBase(const std::string &baseFilename) {
        std::ifstream input(baseFilename);
        if (!input)
            throw std::runtime_error("error");

        load_paths(input);
        input.close();
    }

    CVisitorLog processLog(const std::string &logFilename) {
        std::ifstream input(logFilename);
        if (!input)
            throw std::runtime_error("didnt found the file");

        CVisitorLog log(path);
        log.insert(input);
        input.close();
        return log;
    }
};

#ifndef __PROGTEST__
#if 1
void basicTests(const CVisitorLog &log) {
    assert(log . search ( CAuditFilter ( "headquarters" ) )
        == ( std::set<std::string> { "Alice Cooper", "George Peterson", "Henry Montgomery", "Jane Bush", "John Smith",
            "Tim Cook", "Robert Smith" } ));
    assert(log . search ( CAuditFilter ( "flyingSaucerHangar" ) )
        == ( std::set<std::string> { "Alice Cooper", "Henry Montgomery", "Jane Bush", "John Smith", "Robert Smith" } ));
    assert(log . search ( CAuditFilter ( "flyingSaucerHangar" ) . notAfter ( 2026, 3, 10, 8, 0 ) )
        == ( std::set<std::string> { "Henry Montgomery", "Robert Smith" } ));
    assert(log . search ( CAuditFilter ( "flyingSaucerHangar" ) . notBefore ( 2026, 3, 11, 12, 0 ) )
        == ( std::set<std::string> { "Henry Montgomery", "Jane Bush", "John Smith" } ));
    assert(
        log . search ( CAuditFilter ( "flyingSaucerHangar" ) . notBefore ( 2026, 3, 10, 9, 0 ) . notAfter ( 2026, 3, 10,
            13, 0 ) )
        == ( std::set<std::string> { "Alice Cooper", "Henry Montgomery", "Jane Bush", "John Smith" } ));
    assert(
        log . search ( CAuditFilter ( "flyingSaucerHangar" ) . notBefore ( 2026, 3, 10, 9, 5 ) . notAfter ( 2026, 3, 10,
            9, 5 ) )
        == ( std::set<std::string> { "Henry Montgomery" } ));
    assert(
        log . search ( CAuditFilter ( "flyingSaucerHangar" ) . notBefore ( 2026, 3, 10, 9, 6 ) . notAfter ( 2026, 3, 10,
            9, 6 ) )
        == ( std::set<std::string> { "Henry Montgomery", "John Smith" } ));
    assert(
        log . search ( CAuditFilter ( "flyingSaucerHangar" ) . notBefore ( 2026, 3, 10, 9, 24 ) . notAfter ( 2026, 3, 10
            , 9, 24 ) )
        == ( std::set<std::string> { "Alice Cooper", "Henry Montgomery", "John Smith" } ));
    assert(
        log . search ( CAuditFilter ( "flyingSaucerHangar" ) . notBefore ( 2026, 3, 10, 9, 25 ) . notAfter ( 2026, 3, 10
            , 9, 25 ) )
        == ( std::set<std::string> { "Alice Cooper", "Henry Montgomery" } ));
    assert(
        log . search ( CAuditFilter ( "flyingSaucerHangar" ) . notBefore ( 2024, 2, 1, 0, 0 ) . notAfter ( 2024, 3, 31,
            0, 0 ) )
        == ( std::set<std::string> { "Robert Smith", "Henry Montgomery" } ));
    assert(
        log . search ( CAuditFilter ( "flyingSaucerHangar" ) . notBefore ( 2025, 2, 1, 0, 0 ) . notAfter ( 2025, 3, 31,
            0, 0 ) )
        == ( std::set<std::string> { "Henry Montgomery" } ));
    assert(log . search ( CAuditFilter ( "privateParking" ) )
        == ( std::set<std::string> { "<classified>" } ));
}
#endif

int main() {
    class CMilBase b;
    b.readBase("test/base.txt");

    CVisitorLog log = b.processLog("test/in1.log");


    for (const char *fn: std::initializer_list<const char *>{"test/in1.log", "test/in2.log", "test/in3.log"}) {
        basicTests(b.processLog(fn));
        std::cout << "OK - test: " << fn << std::endl;
    }
    return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
