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

class Company {
public:
    std::string name;
    std::string addr;
    std::string taxID;

    Company(const std::string &name, const std::string &addr, const std::string &taxID) {
        this->name = name;
        this->addr = addr;
        this->taxID = taxID;
    };
};

class Invoice {
public:
    Company &company;
    int amount;

    Invoice(Company &_company, const int _amount) : company(_company), amount(_amount) {
    }
};

class SyncArrayCompany {
private:

    inline auto lower_bound_id(const std::string &taxID) {
        return std::lower_bound(main_arr.begin(), main_arr.end(), taxID,
                                [](const std::shared_ptr<Company>& company, const std::string &taxID) -> bool {
                                    return taxID < company->taxID;
                                });
    }

    inline auto lower_bound_name(const std::string &name) {
        return std::lower_bound(name_desc_view.begin(), name_desc_view.end(), name,
                                [](const std::shared_ptr<Company>& company, const std::string &name) -> bool {
                                    return name < company->name;
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

    SyncArrayCompany() = default;

    bool add(const std::string &name, const std::string &addr, const std::string &taxID) {
        auto found_id = lower_bound_id(taxID);

        auto inserted = main_arr.insert(found_id, std::make_shared<Company>(name, addr, taxID));

        auto found_name = lower_bound_name(name);

        // constructs shared pointer with same ownership
        name_desc_view.emplace(found_name, *inserted);
        return true;
    }

    /*
     * ptr cause it can find nothing (nullptr)
     * find acually always returns the new ptr, which isn't then used again, so the vector resizing won't provoke seg fault.
     */
    ;

    // returns copy of shared_ptr (safe)
    inline std::shared_ptr<Company> find_id(const std::string &taxID) {
        const auto iterator = lower_bound_id(taxID);

        if (iterator != main_arr.end() && (*iterator)->taxID == taxID) {
            return *iterator;
        }
        return nullptr;
    }

    /**
     * @return iterator to lower bound of name view or end of view if none found
     */
    inline auto find_name(const std::string &name) {
        const auto iterator = lower_bound_name(name);

        if (iterator != name_desc_view.end() && (*iterator)->name == name) {
            return iterator;
        } else {
            return name_desc_view.end();
        }
    };
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

    void print(SyncArrayCompany::FIND_BY find_by) {
        if (find_by == SyncArrayCompany::FIND_BY::TAXID) {
            for (const auto& item: arr.main_arr) {
                std::cout <<
                        "name: " << item->name << "; addr: " << item->addr << "; id: " << item->taxID << std::endl;
            }
        } else {
            for (const auto& item: arr.name_desc_view) {
                std::cout <<
                        "name: " << item->name << "; addr: " << item->addr << "; id: " << item->taxID << std::endl;
            }
        }
    }

    /*returns if item can be added into array (to prevent duplicates)*/
    bool validate(const std::string &name, const std::string &addr) {
        auto found = arr.find_name(name);;

        if (found == arr.name_desc_view.end()) {
            return true;
        }

        // itterates trough array from found to end ptr
        for (auto it = found; it != arr.name_desc_view.end(); ++it) {
            auto item = it->get();


            if (item->name != name) {
                break;
            }
            if (item->addr == addr) {
                return false;
            }
        }


        return true;
    }

    bool validate(const std::string &taxID) {
        if (arr.find_id(taxID) != nullptr) {
            std::cout << "found faulty" << taxID << std::endl;
            return false;
        };

        return true;
    }
};

bool CVATRegister::newCompany(const std::string &name, const std::string &addr, const std::string &taxID) {
    if (validate(name, addr) && validate(taxID)) {
        arr.add(name, addr, taxID);
        return true;
    };

    return false;
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
