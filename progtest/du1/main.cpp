#ifndef __PROGTEST__
#include <algorithm>
#include <cassert>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <list>
#include <memory>
#include <string>
#include <vector>
#endif /* __PROGTEST__ */

namespace convert {
    std::string to_lower(std::string copy) {
        for (auto &item : copy) {
            item = std::tolower(item);
        }

        return copy;
    }

}

class Company {
public:
    std::string name;
    std::string name_low;
    std::string addr;
    std::string addr_low;
    std::string taxID;
    int invoices_sum = 0;

    // konstruktor automaticky generuje nazvy name_low pro lepsi search
    Company(const std::string &name, const std::string &addr, const std::string &taxID) {
        this->name = name;
        this->name_low = convert::to_lower(name);
        this->addr = addr;
        this->addr_low = convert::to_lower(addr);
        this->taxID = taxID;
    };
};

// tenhle napad jsem ukradl z priority_queue abych se priznal
template <class T, class binary_function = std::less<>>
class HeapClass {
private:
    std::vector<T> heap;
public:

    HeapClass() = default;

    void push(const T item) {
        heap.push_back(item);
        std::push_heap(heap.begin(), heap.end(), binary_function());
    }

    void pop() {
        std::pop_heap(heap.begin(), heap.end(), binary_function());
        heap.pop_back();
    }

    // kdybych umel dedit tak ty fce zdedim :(, ne

    auto size() {
        return heap.size();
    }

    auto empty() {
        return heap.empty();
    }

    auto top() {
        return heap.front();
    }
};

/*
 * @template T datatype of median
 */
template <class T>
class MedianArray {
    private:
        HeapClass<T, std::greater<>> high_heap;
        HeapClass<T> low_heap;

        T median = 0;

        // switching heap heads to rebalance
        void rebalance_heap() {
            if (low_heap.size() > high_heap.size() + 1) {
                high_heap.push(low_heap.top());
                low_heap.pop();
            }
            else if (high_heap.size() > low_heap.size() + 1) {
                low_heap.push(high_heap.top());
                high_heap.pop();
            }
        }
    public:
        MedianArray() = default;

        void insert(const T value) {
            if (low_heap.empty() || value <= low_heap.top()) {
                low_heap.push(value);
            } else {
                high_heap.push(value);
            }

            rebalance_heap();

            if (high_heap.size() >= low_heap.size()) {
                median = high_heap.top();
            } else {
                median = low_heap.top();
            }
        }

        T get_median() const {
            return median;
        }
};

class SyncArrayCompany {
private:
    auto lower_bound_id(const std::string &taxID)const  {
        return std::lower_bound(main_arr.begin(), main_arr.end(), taxID,
                                [](const std::shared_ptr<Company>& company, const std::string &taxID) -> bool {
                                    return taxID < company->taxID;
                                });
    }

    /* this also takes addresses to the eqq*/
    auto lower_bound_name(const std::string &name)  const{
        return std::lower_bound(name_desc_view.begin(), name_desc_view.end(), convert::to_lower(name),
                                [](const std::shared_ptr<Company>& company, const std::string &name) -> bool {
                                    return name < company->name_low;
                                });
    }

    auto lower_bound_name(std::string name, std::string addr)  const {
        return std::lower_bound(name_desc_view.begin(), name_desc_view.end(), name,
                                [addr](const std::shared_ptr<Company>& company, const std::string name) -> bool {
                                    if (name == company->name_low)
                                        return addr < company->addr_low;

                                    return name < company->name_low;
                                });
    }


public:
    enum FIND_BY {
        TAXID,
        NAME
    };

    // sorted by id;
    // we will need to free each pointer on end of life
    std::vector<std::shared_ptr<Company>> main_arr;
    std::vector<std::shared_ptr<Company>> name_desc_view;
    MedianArray<unsigned int> median_arr;

    SyncArrayCompany() = default;

    /**
     * @return exactly one defined by id / name-addr pair
    */
    auto find_uniq(const std::string &taxID) const  {
        auto found = lower_bound_id(taxID);

        if (!iterator_validator(found))
            return main_arr.end();


        if (found->get()->taxID == taxID) {
            return found;
        }
        return main_arr.end();
    }

     std::vector<std::shared_ptr<Company>>::const_iterator find_uniq(std::string name, std::string addr) const {
        name = convert::to_lower(name);
        addr = convert::to_lower(addr);

        auto found = lower_bound_name(name, addr);

        if (!iterator_validator(found)) {
            return name_desc_view.end();
        }

        if (found->get()->addr_low == addr && found->get()->name_low == name ) {
            return found;
        }

        return name_desc_view.end();
    }

    bool iterator_validator(const std::vector<std::shared_ptr<Company>>::const_iterator& iterator) const {
        if (iterator == main_arr.end()) {
            return false;
        }

        if (iterator == name_desc_view.end()) {
            return false;
        }

        return true;
    }

    bool remove(const std::string &name, const std::string &addr) {
        auto found = find_uniq(name, addr);
        if (!iterator_validator(found)) {
            return false;
        }
        auto found2 = find_uniq(found->get()->taxID);
        if (!iterator_validator(found2)) {
            return false;
        }
        name_desc_view.erase(found);
        main_arr.erase(found2);
        return true;
    }

    bool remove(const std::string &taxID) {
            auto found2 = find_uniq(taxID);
            if (!iterator_validator(found2)) {
                return false;
            }
            auto found = find_uniq(found2->get()->name, found2->get()->addr);
            if (!iterator_validator(found)) {
                return false;
            }

            name_desc_view.erase(found);
            main_arr.erase(found2);
            return true;
    }

    bool add(const std::string &name, const std::string &addr, const std::string &taxID) {
        auto found_id = lower_bound_id(taxID);

        auto inserted = main_arr.insert(found_id, std::make_shared<Company>(name, addr, taxID));

        auto found_name = lower_bound_name(inserted->get()->name_low, inserted->get()->addr_low);

        // constructs shared pointer with same ownership
        name_desc_view.emplace(found_name, *inserted);
        return true;
    }
};


class CVATRegister {
public:
    bool newCompany(const std::string &name, const std::string &addr,
                    const std::string &taxID);

    bool cancelCompany(const std::string &name, const std::string &addr);

    bool cancelCompany(const std::string &taxID);

    bool invoice(const std::string &taxID, unsigned int amount);

    bool invoice(const std::string &name, const std::string &addr,
                 unsigned int amount);

    bool auditCompany(const std::string &name, const std::string &addr,
                      unsigned int &sumIncome) const;

    bool auditCompany(const std::string &taxID, unsigned int &sumIncome) const;

    bool firstCompany(std::string &name, std::string &addr) const;

    bool nextCompany(std::string &name, std::string &addr) const;

    unsigned int medianInvoice() const;

    CVATRegister() = default;

    ~CVATRegister() = default;

    // id sorted
    SyncArrayCompany arr;

    /*returns if item can be added into array (to prevent duplicates)*/
    bool validate(const std::string &name, const std::string &addr) const {
        auto found = arr.find_uniq(name, addr);

        return !arr.iterator_validator(found);
    }

    bool validate(const std::string &taxID) const {
        auto found = arr.find_uniq(taxID);

        return !arr.iterator_validator(found);
    }
};

bool CVATRegister::newCompany(const std::string &name, const std::string &addr, const std::string &taxID) {
    if (validate(name, addr) && validate(taxID)) {
        arr.add(name, addr, taxID);
        return true;
    };

    return false;
}

bool CVATRegister::cancelCompany(const std::string &name, const std::string &addr) {
    return arr.remove(name, addr);
}

bool CVATRegister::cancelCompany(const std::string &taxID) {
    return arr.remove(taxID);
}

bool CVATRegister::auditCompany(const std::string &taxID, unsigned int &sumIncome) const {
    auto found = arr.find_uniq(taxID);

    if (!arr.iterator_validator(found)) {
        return false;
    }

    sumIncome = (*found)->invoices_sum;
    return true;
}

bool CVATRegister::auditCompany(const std::string &name, const std::string &addr, unsigned int &sumIncome) const {
    auto found = arr.find_uniq(name, addr);

    if (!arr.iterator_validator(found)) {
        return false;
    }

    sumIncome = (*found)->invoices_sum;
    return true;
}

bool CVATRegister::invoice(const std::string &name, const std::string &addr, unsigned int amount) {
    auto found = arr.find_uniq(name, addr);

    if (!arr.iterator_validator(found)) {
        return false;
    }

    found->get()->invoices_sum += amount;
    arr.median_arr.insert(amount);

    return true;
}

bool CVATRegister::invoice(const std::string &taxID, unsigned int amount) {
    auto found = arr.find_uniq(taxID);

    if (!arr.iterator_validator(found)) {
        return false;
    }

    found->get()->invoices_sum += amount;
    arr.median_arr.insert(amount);
    return true;
}

unsigned int CVATRegister::medianInvoice() const {
    return arr.median_arr.get_median();
}

bool CVATRegister::firstCompany(std::string &name, std::string &addr) const {
    if (arr.name_desc_view.size() < 1) {
        return false;
    }
    auto found = *arr.name_desc_view.rbegin();


    name = found->name;
    addr = found->addr;
    return true;
}

bool CVATRegister::nextCompany(std::string &name, std::string &addr) const {
    auto found = arr.find_uniq(name, addr);

    if (found == arr.name_desc_view.end() || found == arr.name_desc_view.begin()) {
        return false;
    }

    --found;

    name = found->get()->name;
    addr = found->get()->addr;

    return true;
}

#ifndef __PROGTEST__
int               main           ()
{
  std::string name, addr;
  unsigned int sumIncome;

  CVATRegister b1;
  assert ( b1 . newCompany ( "ACME", "Thakurova", "666/666" ) );
  assert ( b1 . newCompany ( "ACME", "Kolejni", "666/666/666" ) );
  assert ( b1 . newCompany ( "Dummy", "Thakurova", "123456" ) );
  assert ( b1 . invoice ( "666/666", 2000 ) );
  assert ( b1 . medianInvoice () == 2000 );
  assert ( b1 . invoice ( "666/666/666", 3000 ) );
  assert ( b1 . medianInvoice () == 3000 );
  assert ( b1 . invoice ( "123456", 4000 ) );
  assert ( b1 . medianInvoice () == 3000 );
  assert ( b1 . invoice ( "aCmE", "Kolejni", 5000 ) );
  assert ( b1 . medianInvoice () == 4000 );
  assert ( b1 . auditCompany ( "ACME", "Kolejni", sumIncome ) && sumIncome == 8000 );
  assert ( b1 . auditCompany ( "123456", sumIncome ) && sumIncome == 4000 );
  assert ( b1 . firstCompany ( name, addr ) && name == "ACME" && addr == "Kolejni" );
  assert ( b1 . nextCompany ( name, addr ) && name == "ACME" && addr == "Thakurova" );
  assert ( b1 . nextCompany ( name, addr ) && name == "Dummy" && addr == "Thakurova" );
  assert ( ! b1 . nextCompany ( name, addr ) );
  assert ( b1 . cancelCompany ( "ACME", "KoLeJnI" ) );
  assert ( b1 . medianInvoice () == 4000 );
  assert ( b1 . cancelCompany ( "666/666" ) );
  assert ( b1 . medianInvoice () == 4000 );
  assert ( b1 . invoice ( "123456", 100 ) );
  assert ( b1 . medianInvoice () == 3000 );
  assert ( b1 . invoice ( "123456", 300 ) );

  assert ( b1 . medianInvoice () == 3000 );
  assert ( b1 . invoice ( "123456", 200 ) );
  assert ( b1 . medianInvoice () == 2000 );
  assert ( b1 . invoice ( "123456", 230 ) );
  assert ( b1 . medianInvoice () == 2000 );
  assert ( b1 . invoice ( "123456", 830 ) );
  assert ( b1 . medianInvoice () == 830 );
  assert ( b1 . invoice ( "123456", 1830 ) );
  assert ( b1 . medianInvoice () == 1830 );
  assert ( b1 . invoice ( "123456", 2830 ) );
  assert ( b1 . medianInvoice () == 1830 );
  assert ( b1 . invoice ( "123456", 2830 ) );
  assert ( b1 . medianInvoice () == 2000 );
  assert ( b1 . invoice ( "123456", 3200 ) );
  assert ( b1 . medianInvoice () == 2000 );
  assert ( b1 . firstCompany ( name, addr ) && name == "Dummy" && addr == "Thakurova" );
  assert ( ! b1 . nextCompany ( name, addr ) );
  assert ( b1 . cancelCompany ( "123456" ) );
  assert ( ! b1 . firstCompany ( name, addr ) );

  CVATRegister b2;
  assert ( b2 . newCompany ( "ACME", "Kolejni", "abcdef" ) );
  assert ( b2 . newCompany ( "Dummy", "Kolejni", "123456" ) );
  assert ( ! b2 . newCompany ( "AcMe", "kOlEjNi", "1234" ) );
  assert ( b2 . newCompany ( "Dummy", "Thakurova", "ABCDEF" ) );
  assert ( b2 . medianInvoice () == 0 );
  assert ( b2 . invoice ( "ABCDEF", 1000 ) );
  assert ( b2 . medianInvoice () == 1000 );
  assert ( b2 . invoice ( "abcdef", 2000 ) );
  assert ( b2 . medianInvoice () == 2000 );
  assert ( b2 . invoice ( "aCMe", "kOlEjNi", 3000 ) );
  assert ( b2 . medianInvoice () == 2000 );
  assert ( ! b2 . invoice ( "1234567", 100 ) );
  assert ( ! b2 . invoice ( "ACE", "Kolejni", 100 ) );
  assert ( ! b2 . invoice ( "ACME", "Thakurova", 100 ) );
  assert ( ! b2 . auditCompany ( "1234567", sumIncome ) );
  assert ( ! b2 . auditCompany ( "ACE", "Kolejni", sumIncome ) );
  assert ( ! b2 . auditCompany ( "ACME", "Thakurova", sumIncome ) );
  assert ( ! b2 . cancelCompany ( "1234567" ) );
  assert ( ! b2 . cancelCompany ( "ACE", "Kolejni" ) );
  assert ( ! b2 . cancelCompany ( "ACME", "Thakurova" ) );
  assert ( b2 . cancelCompany ( "abcdef" ) );
  assert ( b2 . medianInvoice () == 2000 );
  assert ( ! b2 . cancelCompany ( "abcdef" ) );
  assert ( b2 . newCompany ( "ACME", "Kolejni", "abcdef" ) );
  assert ( b2 . cancelCompany ( "ACME", "Kolejni" ) );
  assert ( ! b2 . cancelCompany ( "ACME", "Kolejni" ) );


  return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */

