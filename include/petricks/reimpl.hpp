#pragma once
#ifndef __PETRICKS_REIMPL__
#define __PETRICKS_REIMPL__

#include <cstdint>
#include <algorithm>

/**
 *  These types are written because this project is limited to C++11.
 *  So don't use them on purpose.
 */

namespace pe {

/**
 * A naive span implemented for simple usage.
 */
template <typename T>
class span {
    T* _data; size_t _size;
public:
    span(T* data, size_t size) : _data(data), _size(size) {}
    T* begin() const { return _data; }
    T* end() const { return _data + _size; }
    T& operator[](size_t idx) const { return _data[idx]; }
    T* data() const { return _data; }
    size_t size() const { return _size; } 
}; // class span

template <typename CharT>
class basic_string_view {
    const CharT* _data; size_t _size;
public:
    basic_string_view(const CharT* data, size_t size) : _data(data), _size(size) {}
    basic_string_view(const CharT* data) : _data(data), _size(0) { for (; _data[_size] != 0; ++_size) {} }
    const CharT* begin() const { return _data; }
    const CharT* end() const { return _data + _size; }
    const CharT& operator[](size_t idx) const { return _data[idx]; }
    const CharT* data() const { return _data; }
    size_t size() const { return _size; }
    size_t length() const { return _size; }

    basic_string_view<CharT> substr(size_t pos, size_t count = size_t(-1)) const {
        if (pos >= _size) { return {_data + _size, 0}; }
        return {_data + pos, std::min(count, _size - pos)};
    }

    bool operator==(const basic_string_view<CharT>& other) const {
        if (_size != other._size) { return false; }
        for (size_t i = 0; i < _size; ++i) {
            if ((*this)[i] != other[i]) { return false; }
        }
        return true;
    }
    bool operator==(const CharT* other) const {
        for (size_t i = 0; i < _size; ++i) {
            if (other[i] == 0 || (*this)[i] != other[i]) { return false; }
        }
        return true;
    }
}; // class basic_string_view

using string_view = basic_string_view<char>;
using wstring_view = basic_string_view<wchar_t>;


template <class T1, class T2>
class ebco_pair : protected std::remove_cv<T1>::type {
    T2 second_;
public:
    using first_type = T1;
    using second_type = T2;

    ebco_pair() {}

    template <typename PT1, typename PT2>
    ebco_pair(PT1&& x, PT2&& y):
        first_type(std::forward<PT1>(x)), second_(std::forward<PT2>(y)) {}

    first_type& first() { return *this; }
    const first_type& first() const { return *this; }

    second_type& second() { return second_; }
    const second_type& second() const { return second_; }
}; // class ebco_pair

} // namespace pe

#endif // __PETRICKS_REIMPL__
