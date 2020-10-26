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

#include <list>
#include <unordered_map>
#include <tuple>

#include <QTextStream>
#include <QList>

template<typename ElementType>
inline QTextStream &operator<<(QTextStream &outStream, const QList<ElementType> &f)
{
    M_UNUSED(outStream, f);
    //    outStream << toString(f);
    return outStream;
}

namespace facelift {

template<typename StreamType>
struct StreamReadFunction
{
    StreamReadFunction(StreamType &msg) :
        m_msg(msg)
    {
    }

    StreamType &m_msg;

    template<typename T>
    void operator()(T &t)
    {
        m_msg >> t;
    }
};

template<typename StreamType>
struct StreamWriteFunction
{
    StreamWriteFunction(StreamType &msg) :
        m_msg(msg)
    {
    }

    StreamType &m_msg;

    template<typename T>
    void operator()(T &&t)
    {
        m_msg << t;
    }
};


template<size_t I = 0, typename Func, typename ... Ts>
typename std::enable_if<I == sizeof ... (Ts)>::type
for_each_in_tuple(std::tuple<Ts ...> &, Func)
{
}

template<size_t I = 0, typename Func, typename ... Ts>
typename std::enable_if < I<sizeof ... (Ts)>::type
for_each_in_tuple(std::tuple<Ts ...> &tpl, Func func)
{
    func(std::get<I>(tpl));
    for_each_in_tuple<I + 1>(tpl, func);
}


template<size_t I = 0, typename Func, typename ... Ts>
typename std::enable_if<I == sizeof ... (Ts)>::type
for_each_in_tuple_const(const std::tuple<Ts ...> &, Func)
{
}

template<size_t I = 0, typename Func, typename ... Ts>
typename std::enable_if < I<sizeof ... (Ts)>::type
for_each_in_tuple_const(const std::tuple<Ts ...> &tpl, Func func)
{
    func(std::get<I>(tpl));
    for_each_in_tuple_const<I + 1>(tpl, func);
}


namespace detail {
template<typename Model, typename F, typename Tuple, bool Done, int Total, int ... N>
struct call_impl
{
    static void call_method(Model *obj, F f, Tuple &&t)
    {
        call_impl<Model, F, Tuple, Total == 1 + sizeof ... (N), Total, N ..., sizeof ... (N)>::call_method(obj, f,
                std::forward<Tuple>(t));
    }
};

template<typename Model, typename F, typename Tuple, int Total, int ... N>
struct call_impl<Model, F, Tuple, true, Total, N ...>
{
    static void call_method(Model *obj, F f, Tuple &&t)
    {
        (obj->*f)(std::get<N>(std::forward<Tuple>(t)) ...);
    }
};
}

template<typename Model, typename F, typename Tuple>
void call_method(Model *obj, F f, Tuple &&t)
{
    typedef typename std::decay<Tuple>::type ttype;
    detail::call_impl<Model, F, Tuple, 0 == std::tuple_size<ttype>::value, std::tuple_size<ttype>::value>::call_method(
        obj, f, std::forward<Tuple>(t));
}

inline void generateToString(QTextStream &message)
{
    Q_UNUSED(message);
}


template<typename FirstParameterTypes, typename ... ParameterTypes>
void generateToString(QTextStream &message, const FirstParameterTypes &firstParameter, const ParameterTypes & ... parameters)
{
    message << firstParameter << ", ";
    generateToString(message, parameters ...);
}


template<typename Key, typename Value>
class MostRecentlyUsedCache
{
public:
    MostRecentlyUsedCache(unsigned int size) : m_size(size)
    {
    }

    bool exists(const Key &key) const
    {
        return (m_map.find(key) != m_map.end());
    }

    Value get(const Key &key)
    {
        auto it = m_map.find(key);
        m_list.splice(m_list.begin(), m_list, it->second);  // move to front to "log" recent usage
        return it->second->second;
    }

    void insert(const Key &key, const Value &val)
    {
        auto it = m_map.find(key);
        if (it != m_map.end()) {
            m_list.erase(it->second);
            m_map.erase(it);
        }

        m_list.push_front(std::make_pair(key, val));
        m_map.insert({key, m_list.begin()});

        while (m_map.size() > m_size) {
            auto last = m_list.end();
            last--;
            m_map.erase(last->first);
            m_list.pop_back();
        }
    }

    void remove(const Key &key)
    {
        auto it = m_map.find(key);
        m_list.erase(it->second);
        m_map.erase(key);
    }

    void clear()
    {
        m_map.clear();
        m_list.clear();
    }

private:
    unsigned int m_size;
    std::list<std::pair<Key, Value> > m_list;  // first item is most recently used
    std::unordered_map<Key, decltype(m_list.begin())> m_map;
};

}
