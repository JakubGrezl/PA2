#include <iostream>
#include <vector>
#include <unordered_map>
#include <utility>
#include <cassert>
#include <memory>
#include <algorithm>
#include <stdexcept>
#include <cstdint>
#include <initializer_list>

using namespace std;

class CSequence {
public:
    CSequence() = default;

    void add(long int x) {
        // TODO
    }

    void add(const int * st, const int * end) {
        // TODO
    }

    bool findSum(long int x, std::pair<std::size_t, std::size_t> & first, std::pair<std::size_t, std::size_t> & last) const {
        // TODO
        return false;
    }

private:
    // TODO
};

int main() {
    CSequence seq;
    std::pair<std::size_t, std::size_t> f, l;

    std::initializer_list<int> t1 = {2, 3, 4, 1, 5};
    seq.add(t1.begin(), t1.end());

    assert(seq.findSum(5, f, l) == true);
    assert(f.first == 0 && f.second == 2);
    assert(l.first == 4 && l.second == 5);

    assert(seq.findSum(10, f, l) == true);
    assert(f.first == 0 && f.second == 4);
    assert(l.first == 2 && l.second == 5);

    assert(seq.findSum(15, f, l) == true);
    assert(f.first == 0 && f.second == 5);
    assert(l.first == 0 && l.second == 5);

    assert(seq.findSum(99, f, l) == false);

    std::initializer_list<int> t2 = {-2, 7, -5};
    seq.add(t2.begin(), t2.end());

    assert(seq.findSum(5, f, l) == true);
    assert(f.first == 0 && f.second == 2);
    assert(l.first == 5 && l.second == 7);

    assert(seq.findSum(0, f, l) == true);
    assert(f.first == 5 && f.second == 8);
    assert(l.first == 5 && l.second == 8);

    assert(seq.findSum(2, f, l) == true);
    assert(f.first == 0 && f.second == 1);
    assert(l.first == 6 && l.second == 8);

    seq.add(15);
    std::initializer_list<int> t3 = {-5, -10};
    seq.add(t3.begin(), t3.end());

    assert(seq.findSum(15, f, l) == true);
    assert(f.first == 0 && f.second == 5);
    assert(l.first == 8 && l.second == 9);

    assert(seq.findSum(0, f, l) == true);
    assert(f.first == 5 && f.second == 8);
    assert(l.first == 8 && l.second == 11);

    std::initializer_list<int> t4 = {0, 0};
    seq.add(t4.begin(), t4.end());

    assert(seq.findSum(0, f, l) == true);
    assert(f.first == 5 && f.second == 8);
    assert(l.first == 12 && l.second == 13);

    assert(seq.findSum(1000000, f, l) == false);
    assert(seq.findSum(-9999, f, l) == false);

    std::cout << "Vsechny asserty prosly v poradku!" << std::endl;
    return 0;
}