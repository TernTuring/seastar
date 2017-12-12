/*
 * This file is open source software, licensed to you under the terms
 * of the Apache License, Version 2.0 (the "License").  See the NOTICE file
 * distributed with this work for additional information regarding copyright
 * ownership.  You may not use this file except in compliance with the License.
 *
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
/*
 * Copyright (C) 2017 ScyllaDB.
 */

#pragma once

#include <boost/variant.hpp>
#include <boost/version.hpp>

#if (BOOST_VERSION < 105800)

#error "Boost version >= 1.58 is required for using variant visitation helpers."
#error "Earlier versions lack support for return value deduction and move-only return values"

#endif

namespace seastar {

/// \cond internal
namespace internal {

#if __cplusplus >= 201703L // C++17

template<typename... Args> struct variant_visitor : Args... { using Args::operator()...; };
template<typename... Args> variant_visitor(Args&&...) -> variant_visitor<Args...>;

#else

template <typename... Args>
struct variant_visitor;

template <typename FuncObj, typename... Args>
struct variant_visitor<FuncObj, Args...> : FuncObj, variant_visitor<Args...>
{
    variant_visitor(FuncObj&& func_obj, Args&&... args)
        : FuncObj(std::move(func_obj))
        , variant_visitor<Args...>(std::move(args)...) {}

    using FuncObj::operator();
    using variant_visitor<Args...>::operator();
};

template <typename FuncObj>
struct variant_visitor<FuncObj> : FuncObj
{
    variant_visitor(FuncObj&& func_obj) : FuncObj(std::forward<FuncObj>(func_obj)) {}

    using FuncObj::operator();
};

#endif

template <typename... Args>
auto make_visitor(Args&&... args)
{
    return variant_visitor<Args...>(std::forward<Args>(args)...);
}

}
/// \endcond

/// \addtogroup utilities
/// @{

/// Applies a static visitor comprised of supplied lambdas to a variant.
/// Note that the lambdas should cover all the types that the variant can possibly hold.
///
/// Returns the common type of return types of all lambdas.
///
/// \tparam Variant the type of a variant
/// \tparam Args types of lambda objects
/// \param variant the variant object
/// \param args lambda objects each accepting one or some types stored in the variant as input
/// \return
template <typename Variant, typename... Args>
inline auto visit(Variant&& variant, Args&&... args)
{
    static_assert(sizeof...(Args) > 0, "At least one lambda must be provided for visitation");
    return boost::apply_visitor(
        internal::make_visitor(std::forward<Args>(args)...),
        std::forward<Variant>(variant));
};


/// @}

}
