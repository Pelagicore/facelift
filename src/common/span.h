/**********************************************************************
 **
 ** Copyright (C) 2018 Luxoft Sweden AB
 **
 ** This file is part of the FaceLift project
 **
 ** Permission is hereby granted, free of charge, to any person
 ** obtaining a copy of this software and associated documentation files
 ** (the "Software"), to deal in the Software without restriction,
 ** including without limitation the rights to use, copy, modify, merge,
 ** publish, distribute, sublicense, and/or sell copies of the Software,
 ** and to permit persons to whom the Software is furnished to do so,
 ** subject to the following conditions:
 **
 ** The above copyright notice and this permission notice shall be
 ** included in all copies or substantial portions of the Software.
 **
 ** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 ** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 ** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 ** NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 ** BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 ** ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 ** CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 ** SOFTWARE.
 **
 ** SPDX-License-Identifier: MIT
 **
 **********************************************************************/

#pragma once

#include <initializer_list>

namespace facelift
{

// The class template span describes an object that can refer to a contiguous sequence of objects with the first element of the sequence at position zero
// See https://en.cppreference.com/w/cpp/container/span
template<typename ElementType>
class span
{
public:
    typedef size_t size_type;
    typedef const ElementType* iterator;
    typedef const ElementType* const_iterator;

public:

    span()
    {
    }

    span(const_iterator __a, size_type __l) :
        _M_array(__a), _M_len(__l)
    {
    }

    template<size_t size>
    span(const std::array<ElementType, size>& array) :
        _M_array(array.begin()), _M_len(size)
    {
    }

    constexpr size_type size() const noexcept
    {
        return _M_len;
    }

    constexpr const_iterator begin() const noexcept
    {
        return _M_array;
    }

    constexpr const_iterator end() const noexcept
    {
        return begin() + size();
    }

    void clear()
    {
        _M_array = nullptr;
        _M_len = 0;
    }

private:
    iterator _M_array = nullptr;
    size_type _M_len = 0;

};

}
