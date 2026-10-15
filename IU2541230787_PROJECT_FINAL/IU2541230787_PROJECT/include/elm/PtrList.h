// PtrList.h - owning, growable array of T* (dynamic memory in one reusable place).
// Capacity doubles when full (new T*[]), every element is deleted in the destructor,
// copying is forbidden so two lists can never delete the same object (Rule of Three/Five).
#pragma once
#include <string>
#include <utility>
#include "errors.h"

namespace elm {

template <class T>
class PtrList {
    T** data_;
    int size_;
    int cap_;

    void grow() {
        int ncap = cap_ * 2;
        T** nd = new T*[static_cast<size_t>(ncap)];
        for (int i = 0; i < size_; ++i) nd[i] = data_[i];
        delete[] data_;
        data_ = nd;
        cap_ = ncap;
    }
public:
    explicit PtrList(int cap = 4) : data_(new T*[static_cast<size_t>(cap > 0 ? cap : 4)]), size_(0), cap_(cap > 0 ? cap : 4) {}
    ~PtrList() { clear(); delete[] data_; }
    PtrList(const PtrList&) = delete;
    PtrList& operator=(const PtrList&) = delete;
    PtrList(PtrList&& o) noexcept : data_(o.data_), size_(o.size_), cap_(o.cap_) {
        o.data_ = nullptr; o.size_ = 0; o.cap_ = 0;
    }
    PtrList& operator=(PtrList&& o) noexcept {
        if (this != &o) {
            clear(); delete[] data_;
            data_ = o.data_; size_ = o.size_; cap_ = o.cap_;
            o.data_ = nullptr; o.size_ = 0; o.cap_ = 0;
        }
        return *this;
    }

    void add(T* p) {                         // takes ownership of p
        if (!data_) { data_ = new T*[4]; cap_ = 4; }
        if (size_ == cap_) grow();
        data_[size_++] = p;
    }
    T* operator[](int i) const {
        if (i < 0 || i >= size_) throw ElmError("index out of range: " + std::to_string(i));
        return data_[i];
    }
    int size() const { return size_; }
    int capacity() const { return cap_; }
    bool empty() const { return size_ == 0; }

    bool removeAt(int i) {                   // deletes the object, closes the gap
        if (i < 0 || i >= size_) return false;
        delete data_[i];
        for (int k = i; k + 1 < size_; ++k) data_[k] = data_[k + 1];
        --size_;
        return true;
    }
    void clear() {
        for (int i = 0; i < size_; ++i) delete data_[i];
        size_ = 0;
    }
};

}  // namespace elm
