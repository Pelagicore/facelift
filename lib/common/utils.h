/*
 *   Copyright (C) 2017 Pelagicore AG
 *   SPDX-License-Identifier: LGPL-2.1
 *   This file is subject to the terms of the LGPL 2.1 license.
 *   Please see the LICENSE file for details.
 */

#pragma once

#include "assert.h"
#include "stddef.h"
#include <tuple>

#include <QList>

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

// user invokes this
template<typename Model, typename F, typename Tuple>
void call_method(Model *obj, F f, Tuple &&t)
{
    typedef typename std::decay<Tuple>::type ttype;
    detail::call_impl<Model, F, Tuple, 0 == std::tuple_size<ttype>::value, std::tuple_size<ttype>::value>::call_method(
        obj, f, std::forward<Tuple>(t));
}


template<typename Type>
inline const QList<Type> &validValues()
{
}
