/*
 *   Copyright (C) 2017 Pelagicore AG
 *   SPDX-License-Identifier: LGPL-2.1
 *   This file is subject to the terms of the LGPL 2.1 license.
 *   Please see the LICENSE file for details.
 */

#pragma once

#include "assert.h"


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
