#ifndef __PROGTEST__
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <iostream>
#endif /* __PROGTEST__ */

template <class T>
class shared_ptr {
private:
    std::size_t * _cnt_ref = nullptr;
    T * _ptr = nullptr;
public:
    shared_ptr() = default;

    explicit shared_ptr(T * __ptr) : _cnt_ref(new std::size_t(1)), _ptr(__ptr) {}

    shared_ptr(const shared_ptr & source) : _cnt_ref(source._cnt_ref), _ptr(source._ptr) {
        if (_cnt_ref) {
            (*_cnt_ref)++;
        }
    }

    /*!
     * moving constructor, cleans up source
     */
    shared_ptr(shared_ptr && source) noexcept : _cnt_ref(source._cnt_ref), _ptr(source._ptr) {
        source._cnt_ref = nullptr;
        source._ptr = nullptr;
    }

    shared_ptr & operator=(const shared_ptr & source) {
        if (this == &source) return *this;
        free();
        _ptr = source._ptr;
        _cnt_ref = source._cnt_ref;
        if (_cnt_ref) (*_cnt_ref)++;
        return *this;
    }

    shared_ptr & operator=(shared_ptr && source) noexcept {
        if (this == &source) return *this;
        free();
        _ptr = source._ptr;
        _cnt_ref = source._cnt_ref;

        source._ptr = nullptr;
        source._cnt_ref = nullptr;
        return *this;
    }

    ~shared_ptr() {
        free();
    }

    void free() {
        if (_cnt_ref != nullptr) {
            --(*_cnt_ref);
            if ((*_cnt_ref) == 0) {
                delete [] _ptr;
                delete _cnt_ref;
            }
            _cnt_ref = nullptr;
            _ptr = nullptr;
        }
    }

    bool is_valid() const {
        return _ptr != nullptr;
    }

    // validates if it is same ptr
    bool is_valid(const shared_ptr & ptr) const {
        return _ptr == ptr._ptr;
    }

    std::size_t cnt_ref() const {
        return _cnt_ref ? *_cnt_ref : 0;
    }

    T & operator[](std::size_t idx) {
        return at(idx);
    }

    const T & operator[](std::size_t idx) const {
        return at(idx);
    }

    T & operator->() {
        return ref();
    }

    T & operator*() {
        return ref();
    }

    T & ref() const {
        if (_ptr == nullptr)
            throw std::runtime_error("non exists");
        return *_ptr;
    }

    T & at(std::size_t idx) {
        return _ptr[idx];
    }

    const T & at(std::size_t idx) const {
        return _ptr[idx];
    }
};

class CFile {
public:
    CFile() = default;

    // create copy -> deep copy version history to keep instances isolated
    CFile(const CFile & from) : data(from.data), capacity(from.capacity), position(from.position) {
        if (from.prev_version) {
            prev_version = new CFile(*from.prev_version);
        }
    }

    // moves content and cleans up source
    CFile(CFile && from) noexcept
        : data(std::move(from.data)), capacity(from.capacity), position(from.position), prev_version(from.prev_version) {
        from.capacity = 0;
        from.position = 0;
        from.prev_version = nullptr;
    }

    ~CFile() {
        delete prev_version;
    };

    CFile & operator=(CFile && from) noexcept {
        if (this == &from) return *this;
        delete prev_version;

        data = std::move(from.data);
        capacity = from.capacity;
        position = from.position;
        prev_version = from.prev_version;

        from.capacity = 0;
        from.position = 0;
        from.prev_version = nullptr;
        return *this;
    }

    CFile & operator=(const CFile & from) {
        if (this == &from) return *this;
        delete prev_version;

        data = from.data;
        capacity = from.capacity;
        position = from.position;
        if (from.prev_version) {
            prev_version = new CFile(*from.prev_version);
        } else {
            prev_version = nullptr;
        }

        return *this;
    }

    bool seek(size_t offset);
    size_t read(uint8_t dst[], size_t bytes);
    size_t write(const uint8_t src[], size_t bytes);
    void truncate();
    size_t fileSize() const;
    void addVersion();
    bool undoVersion();

private:
    shared_ptr<uint8_t> data;
    std::size_t capacity = 0;
    std::size_t position = 0;
    CFile * prev_version = nullptr;

    /*!
     * allocates array to bytes and copies entire content from old arr if existed
     * updates capacity!!!
    */
    void alloc(std::size_t bytes) {
        if (bytes < capacity) {
            throw std::out_of_range("bytes must be more then capacity");
        }

        const auto tmp = new uint8_t[bytes];

        if (data.is_valid()) {
            for (size_t i = 0; i < capacity; i++) {
                tmp[i] = data[i];
            }
        }

        capacity = bytes;
        data = shared_ptr<uint8_t>(tmp);
    }
};

std::size_t CFile::write(const uint8_t src[], size_t bytes) {
    // data isn't even allocated yet || data is shared across more versions || capacity isn't enough
    if (!data.is_valid() || data.cnt_ref() > 1 || position + bytes > capacity) {
        std::size_t new_capacity = capacity;

        if (new_capacity < position + bytes) {
            new_capacity = position + bytes;
        }

        alloc(new_capacity);
    }

    for (std::size_t i = 0; i < bytes; i++) {
        data[position++] = src[i];
    }
    return bytes;
}

std::size_t CFile::read(uint8_t dst[], size_t bytes) {
    if (!data.is_valid()) return 0;
    std::size_t i = 0;

    for (; i < bytes && position < capacity; i++) {
        dst[i] = data[position++];
    }
    return i;
}

bool CFile::seek(size_t offset) {
    if (offset > capacity)
        return false;
    position = offset;
    return true;
}

size_t CFile::fileSize() const {
    return capacity;
}

void CFile::truncate() {
    capacity = position;
}

void CFile::addVersion() {
    CFile * new_prev = new CFile();
    new_prev->data = data;
    new_prev->capacity = capacity;
    new_prev->position = position;
    new_prev->prev_version = prev_version;

    prev_version = new_prev;
}

bool CFile::undoVersion() {
    if (prev_version == nullptr)
        return false;

    CFile * old_prev = prev_version;
    data = old_prev->data;
    capacity = old_prev->capacity;
    position = old_prev->position;
    prev_version = old_prev->prev_version;

    old_prev->prev_version = nullptr;
    delete old_prev;

    return true;
}

#ifndef __PROGTEST__
bool writeTest(CFile &x,
               const std::initializer_list<uint8_t> &data,
               size_t wrLen) {
    return x.write(data.begin(), data.size()) == wrLen;
}

bool readTest(CFile &x,
              const std::initializer_list<uint8_t> &data,
              size_t rdLen) {
    uint8_t tmp[100];
    uint32_t idx = 0;

    if (x.read(tmp, rdLen) != data.size())
        return false;
    for (auto v: data)
        if (tmp[idx++] != v)
            return false;
    return true;
}

int main() {
    CFile f0;

    assert(writeTest ( f0, {10, 20, 30}, 3 ));
    assert(f0 . fileSize () == 3);
    assert(writeTest ( f0, {60, 70, 80}, 3 ));
    assert(f0 . fileSize () == 6);
    assert(f0 . seek ( 2 ));
    assert(writeTest ( f0, {5, 4}, 2 ));
    assert(f0 . fileSize () == 6);
    assert(f0 . seek ( 1 ));
    assert(readTest ( f0, {20, 5, 4, 70, 80}, 7 ));
    assert(f0 . seek ( 3 ));
    f0.addVersion();
    assert(f0 . seek ( 6 ));
    assert(writeTest ( f0, {100, 101, 102, 103}, 4 ));
    f0.addVersion();
    assert(f0 . seek ( 5 ));
    CFile f1(f0);
    f0.truncate();
    assert(f0 . seek ( 0 ));
    assert(readTest ( f0, {10, 20, 5, 4, 70}, 20 ));
    assert(f0 . undoVersion ());
    assert(f0 . seek ( 0 ));
    assert(readTest ( f0, {10, 20, 5, 4, 70, 80, 100, 101, 102, 103}, 20 ));
    assert(f0 . undoVersion ());
    assert(f0 . seek ( 0 ));
    assert(readTest ( f0, {10, 20, 5, 4, 70, 80}, 20 ));
    assert(!f0 . seek ( 100 ));
    assert(writeTest ( f1, {200, 210, 220}, 3 ));
    assert(f1 . seek ( 0 ));
    assert(readTest ( f1, {10, 20, 5, 4, 70, 200, 210, 220, 102, 103}, 20 ));
    assert(f1 . undoVersion ());
    assert(f1 . undoVersion ());
    assert(readTest ( f1, {4, 70, 80}, 20 ));
    assert(!f1 . undoVersion ());
    return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
