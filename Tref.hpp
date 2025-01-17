﻿// Tref: A *T*iny compile time *ref*lection system.

/***********************************************************************
Copyright 2019-2020 crazybie<soniced@sina.com>

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#ifndef TREF_H
#define TREF_H
#pragma once

#include <array>
#include <string_view>
#include <tuple>
#include <type_traits>

#define ZTrefHasTref
#define ZTrefVersion 0x010000

#ifdef _MSC_VER
#define ZTrefCxxVersion _MSVC_LANG
#else
#define ZTrefCxxVersion __cplusplus
#endif

#if ZTrefCxxVersion < 201700L
#error "Need a c++17 compiler"
#endif

namespace tref {
namespace imp {

using namespace std;

//////////////////////////////////////////////////////////////////////////
//
// Common facility
//
//////////////////////////////////////////////////////////////////////////

template <typename T>
struct Type {
  using type = T;
};

template <size_t L, size_t... R>
constexpr auto tail(index_sequence<L, R...>) {
  return index_sequence<R...>();
}

// member pointer trait

template <class C>
struct member_pointer_trait {
  using enclosing_class_t = void;
  using member_t = void;
};

template <class T, class C>
struct member_pointer_trait<T C::*> {
  using member_t = T;
  using enclosing_class_t = C;
};

template <class T>
using member_t = typename member_pointer_trait<T>::member_t;

template <class T>
using enclosing_class_t = typename member_pointer_trait<T>::enclosing_class_t;

// function trait

template <typename T>
struct func_trait : func_trait<decltype(&T::operator())> {};

template <typename R, typename C, typename... A>
struct func_trait<R (C::*)(A...) const> {
  using args_t = tuple<A...>;
  static constexpr auto args_count = sizeof...(A);
  using ret_t = R;
};

template <typename R, typename C, typename... A>
struct func_trait<R (C::*)(A...)> {
  using args_t = tuple<A...>;
  static constexpr auto args_count = sizeof...(A);
  using ret_t = R;
};

// function overloading helper

template <typename... Args>
struct Overload {
  template <typename R, typename T>
  constexpr auto operator()(R (T::*ptr)(Args...)) const {
    return ptr;
  }
  template <typename R, typename T>
  constexpr auto operator()(R (T::*ptr)(Args...) const) const {
    return ptr;
  }
  template <typename R>
  constexpr auto operator()(R (*ptr)(Args...)) const {
    return ptr;
  }
};

template <typename... Args>
constexpr Overload<Args...> overload_v{};

template <typename... Meta>
struct Metas : Meta... {
  constexpr explicit Metas(Meta... m) : Meta(m)... {}
};

//  common macros

#define ZTrefReturn(...) \
  ->decltype(__VA_ARGS__) { return __VA_ARGS__; }

#define ZTrefMsvcExpand(...) __VA_ARGS__
#define ZTrefDelay(X, ...) ZTrefMsvcExpand(X(__VA_ARGS__))
#define ZTrefDelay2(X, ...) ZTrefMsvcExpand(X(__VA_ARGS__))
#define ZTrefFirst(...) ZTrefMsvcExpand(ZTrefFirst2(__VA_ARGS__))
#define ZTrefFirst2(A, ...) A
#define ZTrefSecond(...) ZTrefMsvcExpand(ZTrefSecond2(__VA_ARGS__))
#define ZTrefSecond2(A, B, ...) B
#define ZTrefTail(A, ...) __VA_ARGS__
#define ZTrefStringify(...) ZTrefStringify2(__VA_ARGS__)
#define ZTrefStringify2(...) #__VA_ARGS__

#define ZTrefRemoveParen(A) \
  ZTrefDelay(ZTrefRemoveParen2, ZTrefRemoveParenHelper A)
#define ZTrefRemoveParen2(...) \
  ZTrefDelay2(ZTrefTail, ZTrefRemoveParenHelper##__VA_ARGS__)
#define ZTrefRemoveParenHelper(...) _, __VA_ARGS__
#define ZTrefRemoveParenHelperZTrefRemoveParenHelper _,

#define ZTrefFirstRemoveParen(X) ZTrefFirst(ZTrefRemoveParen(X))
#define ZTrefSecondRemoveParen(X) ZTrefSecond(ZTrefRemoveParen(X))

// macro version of map

#define ZTrefMap(f, arg1, ...)                                         \
  ZTrefMsvcExpand(ZTrefDelay(ZTrefChooseMap, ZTrefCount(__VA_ARGS__))( \
      f, arg1, __VA_ARGS__))

#define ZTrefChooseMap(N) ZTrefMap##N

#define ZTrefMap1(m, a, x) m(a, x)
#define ZTrefMap2(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap1(m, a, __VA_ARGS__))
#define ZTrefMap3(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap2(m, a, __VA_ARGS__))
#define ZTrefMap4(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap3(m, a, __VA_ARGS__))
#define ZTrefMap5(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap4(m, a, __VA_ARGS__))
#define ZTrefMap6(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap5(m, a, __VA_ARGS__))
#define ZTrefMap7(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap6(m, a, __VA_ARGS__))
#define ZTrefMap8(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap7(m, a, __VA_ARGS__))
#define ZTrefMap9(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap8(m, a, __VA_ARGS__))
#define ZTrefMap10(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap9(m, a, __VA_ARGS__))
#define ZTrefMap11(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap10(m, a, __VA_ARGS__))
#define ZTrefMap12(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap11(m, a, __VA_ARGS__))
#define ZTrefMap13(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap12(m, a, __VA_ARGS__))
#define ZTrefMap14(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap13(m, a, __VA_ARGS__))
#define ZTrefMap15(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap14(m, a, __VA_ARGS__))
#define ZTrefMap16(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap15(m, a, __VA_ARGS__))
#define ZTrefMap17(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap16(m, a, __VA_ARGS__))
#define ZTrefMap18(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap17(m, a, __VA_ARGS__))
#define ZTrefMap19(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap18(m, a, __VA_ARGS__))
#define ZTrefMap20(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap19(m, a, __VA_ARGS__))
#define ZTrefMap21(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap20(m, a, __VA_ARGS__))
#define ZTrefMap22(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap21(m, a, __VA_ARGS__))
#define ZTrefMap23(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap22(m, a, __VA_ARGS__))
#define ZTrefMap24(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap23(m, a, __VA_ARGS__))
#define ZTrefMap25(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap24(m, a, __VA_ARGS__))
#define ZTrefMap26(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap25(m, a, __VA_ARGS__))
#define ZTrefMap27(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap26(m, a, __VA_ARGS__))
#define ZTrefMap28(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap27(m, a, __VA_ARGS__))
#define ZTrefMap29(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap28(m, a, __VA_ARGS__))
#define ZTrefMap30(m, a, x, ...) \
  m(a, x) ZTrefMsvcExpand(ZTrefMap29(m, a, __VA_ARGS__))

#define ZTrefEvaluateCount(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, \
                           _13, _14, _15, _16, _17, _18, _19, _20, _21, _22,  \
                           _23, _24, _25, _26, _27, _28, _29, _30, N, ...)    \
  N

#define ZTrefCount(...)                                                        \
  ZTrefMsvcExpand(ZTrefEvaluateCount(                                          \
      __VA_ARGS__, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, \
      15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1))

//////////////////////////////////////////////////////////////////////////
//
// Core facility
//
//////////////////////////////////////////////////////////////////////////

template <int N = 255>
struct Id : Id<N - 1> {
  enum { value = N };
};

template <>
struct Id<0> {
  enum { value = 0 };
};

constexpr auto invalid_index = 0;

template <typename C, typename Tag>
tuple<Id<invalid_index>> _tref_state(C**, Tag, Id<0> id);

// use macro to delay the evaluation
#define ZTrefStateCnt(C, Tag)                                                \
  std::tuple_element_t<0,                                                    \
                       decltype(_tref_state((ZTrefRemoveParen(C)**)0, Tag(), \
                                            tref::imp::Id<>{}))>::value

#define ZTrefStatePush(C, Tag, ...)                                       \
  constexpr auto _tref_state(ZTrefRemoveParen(C)**, Tag,                  \
                             tref::imp::Id<ZTrefStateCnt(C, Tag) + 1> id) \
      ZTrefReturn(std::tuple(id, __VA_ARGS__))

template <class C, class Tag, int idx>
constexpr auto get_state() {
  return _tref_state((C**)0, Tag{}, Id<idx>{});
}

template <class C, class Tag, class F, size_t... Is>
constexpr bool state_fold(index_sequence<Is...>, F&& f) {
  return (f(get<1>(get_state<C, Tag, Is>())) && ...);
}

template <typename C, typename Tag, typename F>
constexpr bool each_state(F f) {
  constexpr auto cnt = ZTrefStateCnt(C, Tag);
  if constexpr (cnt > 0) {
    return state_fold<C, Tag>(tail(make_index_sequence<cnt + 1>{}), f);
  } else {
    return true;
  }
}

//////////////////////////////////////////////////////////////////////////
//
// class reflection
//
//////////////////////////////////////////////////////////////////////////

struct DummyBase;

struct FieldTag {};
struct MemberTypeTag {};
struct SubclassTag {};

void _tref_class_info(void*);

template <typename T>
constexpr auto is_reflected_v =
    !std::is_same_v<decltype(_tref_class_info((T**)0)), void>;

// Use function to delay the evaluation. (for non-conformance mode of MSVC)
template <typename T>
constexpr auto class_info() {
  return _tref_class_info((T**)0);
}

// Use macro to delay the evaluation. (for non-conformance mode of MSVC)
#define ZTrefBaseOf(T) \
  typename decltype(tref::imp::class_info<ZTrefRemoveParen(T)>())::base_t

template <typename T>
constexpr auto has_base_class_v =
    is_reflected_v<T> && !std::is_same_v<ZTrefBaseOf(T), DummyBase>;

// Meta for Member

template <typename T, typename Meta>
struct FieldInfo {
  using enclosing_class_t = imp::enclosing_class_t<T>;
  using member_t = imp::member_t<T>;

  static constexpr auto is_member_v = !is_same_v<enclosing_class_t, void>;

  int         index;
  string_view name;

  // Possible values:
  // 1. Address of member variables & functions
  // 2. Type<T> for member type T: use decltype(value)::type to retrive it.
  T    value;
  Meta meta;

  constexpr FieldInfo(int idx, string_view n, T a, Meta m)
      : index{idx}, name{n}, value{a}, meta{m} {}
};

// Meta for class

template <typename T, typename Base, typename Meta>
struct ClassInfo {
  using class_t = T;
  using base_t = Base;

  string_view name;
  size_t      size;
  Type<Base>  base;
  Meta        meta;

  constexpr ClassInfo(T*, string_view n, size_t sz, Type<Base> b, Meta&& m)
      : name{n}, size{sz}, base{b}, meta{move(m)} {
    if constexpr (!is_same_v<Base, DummyBase>) {
      static_assert(is_base_of_v<Base, class_t>, "invalid base class");
    }
  }

  template <typename Tag, typename F>
  constexpr bool each_r(F&& f, int level = 0) const {
    auto next =
        each_state<T, Tag>([&](const auto& info) { return f(info, level); });
    if (next)
      if constexpr (!is_same_v<Base, DummyBase>)
        return class_info<Base>().template each_r<Tag>(f, level + 1);
    return next;
  }

  // Iterate through the members recursively.
  // @param f: [](MemberInfo info, int level) -> bool, return false to stop the
  // iterating.
  template <typename F>
  constexpr bool each_field(F&& f) const {
    return each_r<FieldTag>(f);
  }

  constexpr auto get_field_index(string_view field_name) const {
    int idx = invalid_index;
    each_field([&](auto info, int) {
      if (info.name == field_name) {
        idx = info.index;
        return false;
      }
      return true;
    });
    return idx;
  }

  template <size_t index>
  constexpr auto get_field() const {
    return get<1>(get_state<T, FieldTag, index>());
  }

  // Iterate through the subclasses recursively.
  // @param F: [](ClassInfo info, int level) -> bool, return false to stop the
  // iterating.
  template <typename F>
  constexpr bool each_subclass(F&& f, int level = 0) const {
    return each_state<T, SubclassTag>([&](auto info) {
      using S = typename decltype(info)::type;
      return f(class_info<S>(), level) &&
             class_info<S>().each_subclass(f, level + 1);
    });
  }

  // Iterate through the member types.
  // @param F: [](MemberInfo info) -> bool, return false to stop the
  // iterating.
  template <typename F>
  constexpr bool each_member_type(F&& f) const {
    return each_r<MemberTypeTag>(f);
  }
};

#define ZTrefClassMetaImp(T, Base, meta)                              \
  constexpr auto _tref_class_info(ZTrefRemoveParen(T)**) {            \
    return tref::imp::ClassInfo{                                      \
        (ZTrefRemoveParen(T)*)0, ZTrefStringify(ZTrefRemoveParen(T)), \
        sizeof(ZTrefRemoveParen(T)),                                  \
        tref::imp::Type<ZTrefRemoveParen(Base)>{}, meta};             \
  }

#define ZTrefClassMeta(T, Base, meta) friend ZTrefClassMetaImp(T, Base, meta)

#define ZTrefPushFieldImp(T, Tag, name, val, meta) \
  ZTrefStatePush(T, Tag, tref::imp::FieldInfo{id.value, name, val, meta})

//////////////////////////////////////////////////////////////////////////
//
// macros for class reflection
//
//////////////////////////////////////////////////////////////////////////

// sub type

template <typename T, class = void_t<>>
struct get_parent {
  using type = DummyBase;
};

template <typename T>
struct get_parent<T, void_t<typename T::__parent_t>> {
  using type = typename T::__parent_t;
};

#define ZTrefSubType(T) \
  ZTrefSubTypeImp(T, (typename ZTrefRemoveParen(T)::__base_t))

// NOTE: can't put into class:
// 1. template overloads to increase the id will not work on clang.
// 1. friend function unrelated to the current class may be removed by clang.
#define ZTrefSubTypeImp(T, Base)                                                       \
  ZTrefStatePush(Base, tref::imp::SubclassTag, tref::imp::Type<ZTrefRemoveParen(T)>{}) \
      ZTrefAllowSemicolon(ZTrefRemoveParen(T))

// fix lint issue: `TrefSubType(T);` : empty statement.
#define ZTrefAllowSemicolon(...) using __ = std::void_t<__VA_ARGS__>

//

#define ZTrefTypeCommon(T, Base, meta) \
 private:                              \
  using self_t = ZTrefRemoveParen(T);  \
                                       \
 public:                               \
  using __parent_t = self_t;           \
  ZTrefClassMeta(T, Base, meta);

// Just reflect the type.

#define ZTrefType(T) ZTrefTypeWithMeta(T, nullptr)
#define ZTrefTypeWithMeta(T, meta)                                            \
 public:                                                                      \
  using __base_t = typename tref::imp::get_parent<ZTrefRemoveParen(T)>::type; \
  ZTrefTypeCommon(T, __base_t, meta);

// reflect member variable & function

#define ZTrefField1(t) ZTrefFieldWithMeta2(t, nullptr)
#define ZTrefFieldWithMeta2(t, meta) ZTrefFieldWithMeta2Imp(self_t, t, meta)
#define ZTrefFieldWithMeta2Imp(T, t, meta)               \
  ZTrefPushFieldImp(T, tref::imp::FieldTag,              \
                    ZTrefStringify(ZTrefRemoveParen(t)), \
                    &T::ZTrefRemoveParen(t), meta)

// provide arguments for overloaded function
#define ZTrefField2(t, sig) ZTrefFieldWithMeta3(t, sig, nullptr)
#define ZTrefFieldWithMeta3(t, sig, meta) \
  ZTrefFieldWithMeta3Imp(self_t, t, sig, meta)

#define ZTrefFieldWithMeta3Imp(T, t, sig, meta)                              \
  ZTrefPushFieldImp(                                                         \
      T, tref::imp::FieldTag, ZTrefStringify(ZTrefRemoveParen(t)),           \
      tref::imp::overload_v<ZTrefRemoveParen(sig)>(&T::ZTrefRemoveParen(t)), \
      meta)

// auto select from ZTrefField1 or ZTrefField2 by argument count
#define ZTrefFieldImp(...) \
  ZTrefMsvcExpand(         \
      ZTrefDelay(ZTrefChooseField, ZTrefCount(__VA_ARGS__))(__VA_ARGS__))
#define ZTrefChooseField(N) ZTrefField##N

#define ZTrefField(...) friend ZTrefFieldImp(__VA_ARGS__)

// auto select from ZTrefFieldWithMeta2 or ZTrefFieldWithMeta3 by argument
// count
#define ZTrefFieldWithMetaImp(...)                     \
  ZTrefMsvcExpand(ZTrefDelay(ZTrefChooseFieldWithMeta, \
                             ZTrefCount(__VA_ARGS__))(__VA_ARGS__))
#define ZTrefChooseFieldWithMeta(N) ZTrefFieldWithMeta##N

#define ZTrefFieldWithMeta(...) friend ZTrefFieldWithMetaImp(__VA_ARGS__)

// reflect member type
#define ZTrefMemberTypeImp(T) ZTrefMemberTypeWithMetaImp(T, nullptr)
#define ZTrefMemberTypeWithMetaImp(T, meta)              \
  ZTrefPushFieldImp(self_t, tref::imp::MemberTypeTag,    \
                    ZTrefStringify(ZTrefRemoveParen(T)), \
                    tref::imp::Type<ZTrefRemoveParen(T)>{}, meta)

#define ZTrefMemberType(T) friend ZTrefMemberTypeImp(T)
#define ZTrefMemberTypeWithMeta(T, meta) \
  friend ZTrefMemberTypeWithMetaImp(T, meta)

//////////////////////////////////////////////////////////////////////////
//
// enum reflection
//
//////////////////////////////////////////////////////////////////////////

struct EnumValueConvertor {
  template <typename T>
  constexpr explicit EnumValueConvertor(T v) : value((size_t)v) {
    static_assert(sizeof(T) <= sizeof(value));
  }

  template <typename U>
  constexpr EnumValueConvertor& operator=(U) {
    return *this;
  }

  template <typename U>
  constexpr operator U() {
    static_assert(is_enum_v<U>);
    return (U)value;
  }

  size_t value = 0;
};

template <typename T, typename Meta>
struct EnumItem {
  string_view name;
  T           value;
  Meta        meta;
};

template <typename T, size_t N, typename ItemMeta>
using EnumItems = array<EnumItem<T, ItemMeta>, N>;

template <typename T, size_t N, typename Meta, typename ItemMeta>
struct EnumInfo {
  using enum_t = T;

  string_view               name;
  size_t                    size = 0;
  EnumItems<T, N, ItemMeta> items;
  Meta                      meta;

  // @param f: [](string_view name, enum_t val)-> bool, return false to stop
  // the iterating.
  //
  // NOTE: the name.data() is not the name of the item, please use
  // string(name.data(), name.size()).
  template <typename F>
  constexpr auto each_item(F&& f) const {
    for (auto& e : items) {
      if (!f(e))
        return false;
    }
    return true;
  }

  static constexpr auto npos = -1;

  constexpr int index_of_value(T v) {
    auto i = 0;
    for (auto& e : items) {
      if (e.value == v) {
        return i;
      }
      i++;
    }
    return npos;
  }

  constexpr int index_of_name(string_view n) {
    auto i = 0;
    for (auto& e : items) {
      if (e.name == n) {
        return i;
      }
      i++;
    }
    return npos;
  }
};

template <typename T, int N, typename Meta, typename ItemMeta>
constexpr auto makeEnumInfo(string_view                      name,
                            size_t                           size,
                            const EnumItems<T, N, ItemMeta>& items,
                            Meta                             meta) {
  return EnumInfo<T, N, Meta, ItemMeta>{name, size, items, meta};
}

void* _tref_enum_info(void*);

template <typename T, typename = enable_if_t<is_enum_v<T>, bool>>
constexpr auto enum_info() {
  return _tref_enum_info((T**)0);
}

// Use it out of class.
#define ZTrefEnum(T, ...) ZTrefEnumWithMeta(T, nullptr, __VA_ARGS__)
#define ZTrefEnumWithMeta(T, meta, ...) \
  enum class T { __VA_ARGS__ };         \
  ZTrefEnumImpWithMeta(T, meta, __VA_ARGS__)

// Use it inside of class.
#define ZTrefMemberEnum(T, ...) ZTrefMemberEnumWithMeta(T, 0, __VA_ARGS__)
#define ZTrefMemberEnumWithMeta(T, meta, ...) \
  enum class T { __VA_ARGS__ };               \
  friend ZTrefEnumImpWithMeta(T, meta, __VA_ARGS__)

// Reflect enum items of already defined enum.
#define ZTrefEnumImp(T, ...) ZTrefEnumImpWithMeta(T, nullptr, __VA_ARGS__)
#define ZTrefEnumImpWithMeta(T, meta, ...)                                   \
  constexpr auto _tref_enum_info(ZTrefRemoveParen(T)**) {                    \
    return tref::imp::EnumInfo<ZTrefRemoveParen(T), ZTrefCount(__VA_ARGS__), \
                               decltype(meta), nullptr_t>{                   \
        ZTrefStringify(ZTrefRemoveParen(T)),                                 \
        sizeof(ZTrefRemoveParen(T)),                                         \
        {ZTrefEnumStringize(T, __VA_ARGS__)},                                \
        std::move(meta)};                                                    \
  }

#define ZTrefEnumStringize(P, ...) \
  ZTrefMsvcExpand(ZTrefMap(ZTrefEnumStringizeSingle, P, __VA_ARGS__))

// (EnumValueConvertor)EnumType::EnumItem = EnumItemValue,
#define ZTrefEnumStringizeSingle(P, E)                                         \
  tref::imp::EnumItem<ZTrefRemoveParen(P), nullptr_t>{                         \
      tref::imp::enum_trim_name(#E),                                           \
      (tref::imp::EnumValueConvertor)ZTrefRemoveParen(P)::ZTrefRemoveParen(E), \
      nullptr},

//////////////////////////////////////////////////////////////////////////
// ex version support meta for enum items.

#define ZTrefMemberEnumEx(T, ...) ZTrefMemberEnumWithMetaEx(T, 0, __VA_ARGS__)
#define ZTrefMemberEnumWithMetaEx(T, meta, ...) \
  ZTrefEnumDefineEnum2(T, __VA_ARGS__);         \
  friend ZTrefEnumImpWithMetaEx(T, meta, __VA_ARGS__)

#define ZTrefEnumEx(T, ...) ZTrefEnumWithMetaEx(T, nullptr, __VA_ARGS__)

#define ZTrefEnumWithMetaEx(T, meta, ...) \
  ZTrefEnumDefineEnum2(T, __VA_ARGS__);   \
  ZTrefEnumImpWithMetaEx(T, meta, __VA_ARGS__)

#define ZTrefFirstArgAsEnumItemDef(P, E) ZTrefFirstRemoveParen(E),
#define ZTrefEnumDefineEnum2(T, ...)                                      \
  enum class T {                                                          \
    ZTrefMsvcExpand(ZTrefMap(ZTrefFirstArgAsEnumItemDef, T, __VA_ARGS__)) \
  }

#define ZTrefEnumImpEx(T, ...) ZTrefEnumImpWithMetaEx(T, 0, __VA_ARGS__)
#define ZTrefEnumImpWithMetaEx(T, meta, ...)                               \
  constexpr auto _tref_enum_info(ZTrefRemoveParen(T)**) {                  \
    return tref::imp::makeEnumInfo<ZTrefRemoveParen(T),                    \
                                   ZTrefCount(__VA_ARGS__)>(               \
        ZTrefStringify(ZTrefRemoveParen(T)), sizeof(ZTrefRemoveParen(T)),  \
        std::array{ZTrefEnumStringize2(T, __VA_ARGS__)}, std::move(meta)); \
  }

#define ZTrefEnumStringize2(P, ...) \
  ZTrefMsvcExpand(ZTrefMap(ZTrefEnumStringizeSingle2, P, __VA_ARGS__))

#define ZTrefEnumStringizeSingle2(P, E)                                       \
  tref::imp::EnumItem<ZTrefRemoveParen(P),                                    \
                      decltype(ZTrefRemoveParen(ZTrefSecondRemoveParen(E)))>{ \
      tref::imp::enum_trim_name(ZTrefStringify(ZTrefFirstRemoveParen(E))),    \
      (tref::imp::EnumValueConvertor)ZTrefRemoveParen(P)::ZTrefRemoveParen(   \
          ZTrefFirstRemoveParen(E)),                                          \
      ZTrefRemoveParen(ZTrefSecondRemoveParen(E))},

/////////////////////////////////////

constexpr string_view enum_trim_name(string_view s) {
  auto p = s.find_first_of('=');
  if (p != string_view::npos)
    p = s.rfind(' ', p);
  return s.substr(0, p);
}

template <typename T>
constexpr auto enum_to_string(T v) {
  static_assert(is_enum_v<T>);
  for (auto& e : enum_info<T>().items) {
    if (e.value == v) {
      return e.name;
    }
  }
  return string_view{};
}

template <typename T>
constexpr auto string_to_enum(string_view s, T default_) {
  static_assert(is_enum_v<T>);
  for (auto& e : enum_info<T>().items) {
    if (s == e.name) {
      return e.value;
    }
  }
  return default_;
}

template <typename T, typename Storage = unsigned int>
struct Flags {
  static_assert(std::is_enum_v<T>);
  Storage value = 0;

  constexpr Flags() = default;
  constexpr void clear() { value = 0; }
  constexpr bool hasFlag(T e) {
    assert(e < sizeof(value) * 8);
    return (value & (1 << static_cast<Storage>(e))) != 0;
  }
  constexpr void setFlag(T e) {
    assert(e < sizeof(value) * 8);
    value |= (1 << static_cast<Storage>(e));
  }
  constexpr void clearFlag(T e) {
    assert(e < sizeof(value) * 8);
    value &= ~(1 << static_cast<Storage>(e));
  }
};

}  // namespace imp

//////////////////////////////////////////////////////////////////////////
//
// public APIs
//
//////////////////////////////////////////////////////////////////////////

#define TrefHasTref ZTrefHasTref
#define TrefVersion ZTrefVersion

using imp::class_info;
using imp::ClassInfo;
using imp::enclosing_class_t;
using imp::FieldInfo;
using imp::func_trait;
using imp::has_base_class_v;
using imp::is_reflected_v;
using imp::member_t;
using imp::Metas;
using imp::overload_v;

#define TrefType ZTrefType
#define TrefTypeWithMeta ZTrefTypeWithMeta
#define TrefSubType ZTrefSubType
#define TrefBaseOf ZTrefBaseOf

#define TrefField ZTrefField
#define TrefFieldWithMeta ZTrefFieldWithMeta
#define TrefMemberType ZTrefMemberType
#define TrefMemberTypeWithMeta ZTrefMemberTypeWithMeta

//////////////////////////
// Reflect external types
// NOTE: not support template.
//////////////////////////
#define TrefNoBase tref::imp::DummyBase
#define TrefExternalTypeWithMeta ZTrefClassMetaImp
#define TrefExternalFieldWithMeta ZTrefFieldWithMeta2Imp
#define TrefExternalOverloadedFieldWithMeta ZTrefFieldWithMeta3Imp
#define TrefExternalSubType ZTrefSubTypeImp

/// enum

using imp::enum_info;
using imp::enum_to_string;
using imp::Flags;
using imp::string_to_enum;

// ex version support meta for enum items.

#define TrefEnum ZTrefEnum
#define TrefEnumEx ZTrefEnumEx
#define TrefEnumWithMeta ZTrefEnumWithMeta
#define TrefEnumWithMetaEx ZTrefEnumWithMetaEx
#define TrefMemberEnum ZTrefMemberEnum
#define TrefMemberEnumEx ZTrefMemberEnumEx
#define TrefMemberEnumWithMeta ZTrefMemberEnumWithMeta
#define TrefMemberEnumWithMetaEx ZTrefMemberEnumWithMetaEx
#define TrefExternalEnum ZTrefEnumImp
#define TrefExternalEnumEx ZTrefEnumImpEx
#define TrefExternalEnumWithMeta ZTrefEnumImpWithMeta
#define TrefExternalEnumWithMetaEx ZTrefEnumImpWithMetaEx

}  // namespace tref
#endif