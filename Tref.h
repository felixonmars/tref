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

/***************************************************

Tref: A *T*iny compile time *ref*lection system.

Features:
- Only utilize C++17 language features, no external preprocessor needed.
- Reflect at compile time with minimal runtime overhead.
- Normal class reflection.
- Class template reflection.
- Reflect element with additional meta data.
- Enum class reflection, support user defined value and meta for each item.
- Reflect external types of third-party code.
- Reflect class level and instance level variables and functions.
- Reflect nested member types.
- Factory pattern: introspect all sub-classes from one base class.

TODO:
- Reflect overloaded functions.
- Reflect function details, e.g. arguments and return type.
- Specify a new name for reflected element.

*/

#pragma once
#include <array>
#include <string_view>
#include <tuple>
#include <type_traits>

#define _TrefHasTref
#define _TrefVersion 0x010000

#ifdef _MSC_VER
#define __TrefCxxVersion _MSVC_LANG
#else
#define __TrefCxxVersion __cplusplus
#endif

#if __TrefCxxVersion < 201700L
#error "Need a c++17 compiler"
#endif

namespace tref {
namespace imp {

using namespace std;

//////////////////////////////////////////////////////////////////////////
// Common facility

template <typename T>
struct Type {
  using type = T;
};

template <size_t L, size_t... R>
constexpr auto tail(index_sequence<L, R...>) {
  return index_sequence<R...>();
};

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
  using ret_t = R;
};

template <typename R, typename C, typename... A>
struct func_trait<R (C::*)(A...)> {
  using args_t = tuple<A...>;
  using ret_t = R;
};

//  common macros

#define _TrefReturn(...) \
  ->decltype(__VA_ARGS__) { return __VA_ARGS__; }

#define _TrefMsvcExpand(...) __VA_ARGS__
#define _TrefDelay(X, ...) _TrefMsvcExpand(X(__VA_ARGS__))
#define _TrefDelay2(X, ...) _TrefMsvcExpand(X(__VA_ARGS__))
#define _TrefFirst(...) _TrefMsvcExpand(_TrefFirst2(__VA_ARGS__))
#define _TrefFirst2(A, ...) A
#define _TrefSecond(...) _TrefMsvcExpand(_TrefSecond2(__VA_ARGS__))
#define _TrefSecond2(A, B, ...) B
#define _TrefTail(A, ...) __VA_ARGS__
#define _TrefStringify(...) _TrefStringify2(__VA_ARGS__)
#define _TrefStringify2(...) #__VA_ARGS__

#define _TrefRemoveParen(A) \
  _TrefDelay(_TrefRemoveParen2, _TrefRemoveParenHelper A)
#define _TrefRemoveParen2(...) \
  _TrefDelay2(_TrefTail, _TrefRemoveParenHelper##__VA_ARGS__)
#define _TrefRemoveParenHelper(...) _, __VA_ARGS__
#define _TrefRemoveParenHelper_TrefRemoveParenHelper _,

#define _TrefFirstRemoveParen(X) _TrefFirst(_TrefRemoveParen(X))
#define _TrefSecondRemoveParen(X) _TrefSecond(_TrefRemoveParen(X))

// macro version of map

#define _TrefMap(f, arg1, ...)                                         \
  _TrefMsvcExpand(_TrefDelay(_TrefChooseMap, _TrefCount(__VA_ARGS__))( \
      f, arg1, __VA_ARGS__))

#define _TrefChooseMap(N) _TrefMap##N

#define _TrefMap1(m, a, x) m(a, x)
#define _TrefMap2(m, a, x, ...) \
  m(a, x) _TrefMsvcExpand(_TrefMap1(m, a, __VA_ARGS__))
#define _TrefMap3(m, a, x, ...) \
  m(a, x) _TrefMsvcExpand(_TrefMap2(m, a, __VA_ARGS__))
#define _TrefMap4(m, a, x, ...) \
  m(a, x) _TrefMsvcExpand(_TrefMap3(m, a, __VA_ARGS__))
#define _TrefMap5(m, a, x, ...) \
  m(a, x) _TrefMsvcExpand(_TrefMap4(m, a, __VA_ARGS__))
#define _TrefMap6(m, a, x, ...) \
  m(a, x) _TrefMsvcExpand(_TrefMap5(m, a, __VA_ARGS__))
#define _TrefMap7(m, a, x, ...) \
  m(a, x) _TrefMsvcExpand(_TrefMap6(m, a, __VA_ARGS__))
#define _TrefMap8(m, a, x, ...) \
  m(a, x) _TrefMsvcExpand(_TrefMap7(m, a, __VA_ARGS__))
#define _TrefMap9(m, a, x, ...) \
  m(a, x) _TrefMsvcExpand(_TrefMap8(m, a, __VA_ARGS__))
#define _TrefMap10(m, a, x, ...) \
  m(a, x) _TrefMsvcExpand(_TrefMap9(m, a, __VA_ARGS__))
#define _TrefMap11(m, a, x, ...) \
  m(a, x) _TrefMsvcExpand(_TrefMap10(m, a, __VA_ARGS__))
#define _TrefMap12(m, a, x, ...) \
  m(a, x) _TrefMsvcExpand(_TrefMap11(m, a, __VA_ARGS__))
#define _TrefMap13(m, a, x, ...) \
  m(a, x) _TrefMsvcExpand(_TrefMap12(m, a, __VA_ARGS__))
#define _TrefMap14(m, a, x, ...) \
  m(a, x) _TrefMsvcExpand(_TrefMap13(m, a, __VA_ARGS__))
#define _TrefMap15(m, a, x, ...) \
  m(a, x) _TrefMsvcExpand(_TrefMap14(m, a, __VA_ARGS__))
#define _TrefMap16(m, a, x, ...) \
  m(a, x) _TrefMsvcExpand(_TrefMap15(m, a, __VA_ARGS__))
#define _TrefMap17(m, a, x, ...) \
  m(a, x) _TrefMsvcExpand(_TrefMap16(m, a, __VA_ARGS__))
#define _TrefMap18(m, a, x, ...) \
  m(a, x) _TrefMsvcExpand(_TrefMap17(m, a, __VA_ARGS__))
#define _TrefMap19(m, a, x, ...) \
  m(a, x) _TrefMsvcExpand(_TrefMap18(m, a, __VA_ARGS__))
#define _TrefMap20(m, a, x, ...) \
  m(a, x) _TrefMsvcExpand(_TrefMap19(m, a, __VA_ARGS__))

#define _TrefEvaluateCount(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, \
                           _13, _14, _15, _16, _17, _18, _19, _20, N, ...)    \
  N

#define _TrefCount(...)                                                       \
  _TrefMsvcExpand(_TrefEvaluateCount(__VA_ARGS__, 20, 19, 18, 17, 16, 15, 14, \
                                     13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2,  \
                                     1))

//////////////////////////////////////////////////////////////////////////
// Core facility

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
#define _TrefStateCnt(C, Tag)                                                \
  std::tuple_element_t<0,                                                    \
                       decltype(_tref_state((_TrefRemoveParen(C)**)0, Tag{}, \
                                            tref::imp::Id<>{}))>::value

#define _TrefStatePush(C, Tag, ...)                                            \
  friend constexpr auto _tref_state(                                           \
      _TrefRemoveParen(C)**, Tag, tref::imp::Id<_TrefStateCnt(C, Tag) + 1> id) \
      _TrefReturn(std::tuple(id, __VA_ARGS__))

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
  constexpr auto cnt = _TrefStateCnt(C, Tag);
  if constexpr (cnt > 0) {
    return state_fold<C, Tag>(tail(make_index_sequence<cnt + 1>{}), f);
  } else
    return true;
};

//////////////////////////////////////////////////////////////////////////

struct DummyBase;

struct MemberTag {};
struct MemberTypeTag {};
struct SubclassTag {};

void _tref_class_info(void*);

// Use function to delay the evaluation.
template <typename T>
constexpr auto is_reflected() {
  return !std::is_same_v<decltype(_tref_class_info((T**)0)), void>;
};

// Use function to delay the evaluation.
template <typename T>
constexpr auto class_info() {
  return _tref_class_info((T**)0);
};

// Use macro to delay the evaluation.
#define _TrefBaseClass(T) \
  typename decltype(tref::class_info<_TrefRemoveParen(T)>())::base_t

// Use function to delay the evaluation.
template <typename T>
constexpr auto has_base_class() {
  if constexpr (is_reflected<T>()) {
    return !std::is_same_v<_TrefBaseClass(T), DummyBase>;
  }
  return false;
};

// Meta for Member

template <typename T, typename Meta>
struct MemberInfo {
  using enclosing_class_t = imp::enclosing_class_t<T>;
  using member_t = imp::member_t<T>;

  static constexpr auto is_member_v = !is_same_v<enclosing_class_t, void>;

  int index;
  string_view name;

  // Possible values:
  // 1. Address of member variables & functions
  // 2. Type<T> for member type T: use decltype(value)::type to retrive it.
  T value;
  Meta meta;

  constexpr MemberInfo(int idx, string_view n, T a, Meta m)
      : index{idx}, name{n}, value{a}, meta{m} {}
};

// Meta for class

template <typename T, typename Base, typename Meta>
struct ClassInfo {
  using class_t = T;
  using base_t = Base;

  string_view name;
  size_t size;
  Type<Base> base;
  Meta meta;

  constexpr ClassInfo(T*, string_view n, size_t sz, Type<Base> b, Meta&& m)
      : name{n}, size{sz}, base{b}, meta{move(m)} {
    if constexpr (!is_same_v<Base, DummyBase>) {
      static_assert(is_base_of_v<Base, class_t>, "invalid base class");
    }
  }

  template <typename Tag, typename F>
  constexpr bool each_r(F&& f, int level = 0) const {
    auto next = each_state<T, Tag>([&](auto& info) { return f(info, level); });
    if (next)
      if constexpr (!is_same_v<Base, DummyBase>)
        return class_info<Base>().each_r<Tag>(f, level + 1);
    return next;
  }

  // Iterate through the members recursively.
  //
  // @param f: [](MemberInfo info, int level) -> bool, return false to stop the
  // iterating.
  template <typename F>
  constexpr bool each_member(F&& f) const {
    return each_r<MemberTag>(f);
  }

  constexpr int get_member_index(string_view name) {
    int idx = invalid_index;
    each_member([&](auto info, int) {
      if (info.name == name) {
        idx = info.index;
        return false;
      }
      return true;
    });
    return idx;
  }

  template <int index>
  constexpr auto get_member() {
    return get<1>(get_state<T, MemberTag, index>());
  }

  // Iterate through the subclasses recursively.
  //
  // NOTE:
  // if use it at compiling time, please make sure to call it behind all
  // the declarations of the subclasses, otherwise the subclasses may be
  // incomplete.
  //
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

#define _TrefClassMeta(T, Base, meta)                                 \
  friend constexpr auto _tref_class_info(_TrefRemoveParen(T)**) {     \
    return tref::imp::ClassInfo{                                      \
        (_TrefRemoveParen(T)*)0, _TrefStringify(_TrefRemoveParen(T)), \
        sizeof T, tref::imp::Type<_TrefRemoveParen(Base)>{}, meta};   \
  }

//////////////////////////////////////////////////////////////////////////

template <typename T, class = void_t<>>
struct get_parent {
  using type = DummyBase;
};

template <typename T>
struct get_parent<T, void_t<decltype((typename T::__parent_t*)0)>> {
  using type = typename T::__parent_t;
};

#define _TrefTypeCommon(T, Base, meta) \
 private:                              \
  using self_t = _TrefRemoveParen(T);  \
                                       \
 public:                               \
  using __parent_t = self_t;           \
  _TrefClassMeta(T, Base, meta);

#define _TrefPushMember(Tag, T, val, meta)                                 \
  _TrefStatePush(                                                          \
      self_t, Tag,                                                         \
      tref::imp::MemberInfo{id.value, _TrefStringify(_TrefRemoveParen(T)), \
                            val, meta})

#define _TrefPushSubclass(base) \
  _TrefStatePush(base, tref::imp::SubclassTag, tref::imp::Type<self_t>{})

//////////////////////////
// Reflect the type and register it into a hierarchy tree.
// NOTE: the entire tree should be in same namespace.
//////////////////////////

#define _TrefRootType(T) _TrefRootTypeWithMeta(T, nullptr)
#define _TrefRootTypeWithMeta(T, meta) \
  _TrefTypeCommon(T, tref::imp::DummyBase, meta)

#define _TrefSubType(T) _TrefSubTypeWithMeta(T, nullptr)
#define _TrefSubTypeWithMeta(T, meta) \
  _TrefTypeWithMeta(T, meta);         \
  _TrefPushSubclass(__base_t);

// Just reflect the type.

#define _TrefType(T) _TrefTypeWithMeta(T, nullptr)
#define _TrefTypeWithMeta(T, meta)                                            \
 private:                                                                     \
  using __base_t = typename tref::imp::get_parent<_TrefRemoveParen(T)>::type; \
  _TrefTypeCommon(T, __base_t, meta);

// reflect member variable & function
//
// TODO: support function overloading.
//
#define _TrefMember(t) _TrefMemberWithMeta(t, nullptr)
#define _TrefMemberWithMeta(t, meta) \
  _TrefPushMember(tref::imp::MemberTag, t, &self_t::_TrefRemoveParen(t), meta)

// reflect member type
#define _TrefMemberType(T) _TrefMemberTypeWithMeta(T, nullptr)
#define _TrefMemberTypeWithMeta(T, meta)       \
  _TrefPushMember(tref::imp::MemberTypeTag, T, \
                  tref::imp::Type<_TrefRemoveParen(T)>{}, meta)

//////////////////////////
// Reflect external types
// NOTE: not support template.
//////////////////////////

// Reflect the external type and register it into a hierarchy tree.
// NOTE: the entire tree should be in same namespace.

#define _TrefExternalRootType(T) _TrefExternalRootTypeWithMeta(T, nullptr)
#define _TrefExternalRootTypeWithMeta(T, meta) \
  _TrefTypeCommon(T, tref::imp::DummyBase, meta)

#define _TrefExternalSubType(T, Base) \
  _TrefExternalSubTypeWithMeta(T, Base, nullptr)
#define _TrefExternalSubTypeWithMeta(T, Base, meta) \
  _TrefTypeCommon(T, Base, meta);                   \
  _TrefPushSubclass(Base);

// Just reflect the external type.
#define _TrefExternalType(T, Base) _TrefTypeCommon(T, Base)

//////////////////////////////////////////////////////////////////////////
// enum reflection

struct EnumValueConvertor {
  template <typename T>
  constexpr EnumValueConvertor(T v) : value((size_t)v) {
    static_assert(sizeof(T) <= sizeof(value));
  }

  template <typename U>
  constexpr EnumValueConvertor operator=(U) {
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
  T value;
  Meta meta;
};

template <typename T, size_t N, typename ItemMeta>
using EnumItems = array<EnumItem<T, ItemMeta>, N>;

template <typename T, size_t N, typename Meta, typename ItemMeta>
struct EnumInfo {
  using enum_t = T;

  string_view name;
  size_t size;
  EnumItems<T, N, ItemMeta> items;
  Meta meta;

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
};

template <typename T, int N, typename Meta, typename ItemMeta>
constexpr auto makeEnumInfo(string_view name,
                            size_t size,
                            const EnumItems<T, N, ItemMeta>& items,
                            Meta meta) {
  return EnumInfo<T, N, Meta, ItemMeta>{name, size, items, meta};
}

void* _tref_enum_info(void*);

template <typename T, typename = enable_if_t<is_enum_v<T>, bool>>
constexpr auto enum_info() {
  return _tref_enum_info((T**)0);
};

// Use it out of class.
#define _TrefEnumGlobal(T, ...) _TrefEnumGlobalWithMeta(T, nullptr, __VA_ARGS__)
#define _TrefEnumGlobalWithMeta(T, meta, ...) \
  enum class T { __VA_ARGS__ };               \
  _TrefEnumImpWithMeta(T, meta, __VA_ARGS__)

// Use it inside of class.
#define _TrefEnum(T, ...) _TrefEnumWithMeta(T, 0, __VA_ARGS__)
#define _TrefEnumWithMeta(T, meta, ...) \
  enum class T { __VA_ARGS__ };         \
  friend _TrefEnumImpWithMeta(T, meta, __VA_ARGS__)

// Reflect enum items of already defined enum.
#define _TrefEnumImp(T, ...) _TrefEnumImpWithMeta(T, nullptr, __VA_ARGS__)
#define _TrefEnumImpWithMeta(T, meta, ...)                                   \
  constexpr auto _tref_enum_info(_TrefRemoveParen(T)**) {                    \
    return tref::imp::EnumInfo<_TrefRemoveParen(T), _TrefCount(__VA_ARGS__), \
                               decltype(meta), nullptr_t>{                   \
        _TrefStringify(_TrefRemoveParen(T)),                                 \
        sizeof(_TrefRemoveParen(T)),                                         \
        {_TrefEnumStringize(T, __VA_ARGS__)},                                \
        std::move(meta)};                                                    \
  }

#define _TrefEnumStringize(P, ...) \
  _TrefMsvcExpand(_TrefMap(_TrefEnumStringizeSingle, P, __VA_ARGS__))

#define _TrefEnumStringizeSingle(P, E)                                         \
  tref::imp::EnumItem<_TrefRemoveParen(P), nullptr_t>{                         \
      tref::imp::enum_trim_name(#E),                                           \
      (tref::imp::EnumValueConvertor)_TrefRemoveParen(P)::_TrefRemoveParen(E), \
      nullptr},

  //////////////////////////////////////////////////////////////////////////
  // version 2 support meta for enum items.

#define _TrefEnum2(T, ...) _TrefEnumWithMeta2(T, 0, __VA_ARGS__)
#define _TrefEnumWithMeta2(T, meta, ...) \
  _TrefEnumDefineEnum2(T, __VA_ARGS__);  \
  friend _TrefEnumImpWithMeta2(T, meta, __VA_ARGS__)

#define _TrefEnumGlobal2(T, ...) \
  _TrefEnumGlobalWithMeta2(T, nullptr, __VA_ARGS__)

#define _TrefEnumGlobalWithMeta2(T, meta, ...) \
  _TrefEnumDefineEnum2(T, __VA_ARGS__);        \
  _TrefEnumImpWithMeta2(T, meta, __VA_ARGS__)

#define _TrefFirstArgAsEnumItemDef(P, E) _TrefFirstRemoveParen(E),
#define _TrefEnumDefineEnum2(T, ...)                                      \
  enum class T {                                                          \
    _TrefMsvcExpand(_TrefMap(_TrefFirstArgAsEnumItemDef, T, __VA_ARGS__)) \
  }

#define _TrefEnumImp2(T, ...) _TrefEnumImpWithMeta2(T, 0, __VA_ARGS__)
#define _TrefEnumImpWithMeta2(T, meta, ...)                                \
  constexpr auto _tref_enum_info(_TrefRemoveParen(T)**) {                  \
    return tref::imp::makeEnumInfo<_TrefRemoveParen(T),                    \
                                   _TrefCount(__VA_ARGS__)>(               \
        _TrefStringify(_TrefRemoveParen(T)), sizeof(_TrefRemoveParen(T)),  \
        std::array{_TrefEnumStringize2(T, __VA_ARGS__)}, std::move(meta)); \
  }

#define _TrefEnumStringize2(P, ...) \
  _TrefMsvcExpand(_TrefMap(_TrefEnumStringizeSingle2, P, __VA_ARGS__))

#define _TrefEnumStringizeSingle2(P, E)                                       \
  tref::imp::EnumItem<_TrefRemoveParen(P),                                    \
                      decltype(_TrefRemoveParen(_TrefSecondRemoveParen(E)))>{ \
      tref::imp::enum_trim_name(_TrefStringify(_TrefFirstRemoveParen(E))),    \
      (tref::imp::EnumValueConvertor)_TrefRemoveParen(P)::_TrefRemoveParen(   \
          _TrefFirstRemoveParen(E)),                                          \
      _TrefRemoveParen(_TrefSecondRemoveParen(E))},

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

  constexpr Flags() {}
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

// public APIs

using imp::class_info;
using imp::ClassInfo;
using imp::enclosing_class_t;
using imp::has_base_class;
using imp::is_reflected;
using imp::member_t;
using imp::MemberInfo;

#define TrefHasTref _TrefHasTref
#define TrefVersion _TrefVersion

#define TrefBaseClass _TrefBaseClass
#define TrefRootType _TrefRootType
#define TrefRootTypeWithMeta _TrefRootTypeWithMeta
#define TrefSubType _TrefSubType
#define TrefSubTypeWithMeta _TrefSubTypeWithMeta
#define TrefType _TrefType
#define TrefTypeWithMeta _TrefTypeWithMeta

#define TrefMember _TrefMember
#define TrefMemberWithMeta _TrefMemberWithMeta
#define TrefMemberType _TrefMemberType
#define TrefMemberTypeWithMeta _TrefMemberTypeWithMeta

#define TrefExternalRootType _TrefExternalRootType
#define TrefExternalRootTypeWithMeta _TrefExternalRootTypeWithMeta
#define TrefExternalSubType _TrefExternalSubType
#define TrefExternalSubTypeWithMeta _TrefExternalSubTypeWithMeta
#define TrefExternalType _TrefExternalType

/// enum

using imp::enum_info;
using imp::enum_to_string;
using imp::Flags;
using imp::string_to_enum;

#define TrefEnumGlobal _TrefEnumGlobal
#define TrefEnumGlobal2 _TrefEnumGlobal2
#define TrefEnumGlobalWithMeta _TrefEnumGlobalWithMeta
#define TrefEnumGlobalWithMeta2 _TrefEnumGlobalWithMeta2
#define TrefEnum _TrefEnum
#define TrefEnum2 _TrefEnum2
#define TrefEnumWithMeta _TrefEnumWithMeta
#define TrefEnumWithMeta2 _TrefEnumWithMeta2
#define TrefEnumImp _TrefEnumImp
#define TrefEnumImp2 _TrefEnumImp2
#define TrefEnumImpWithMeta _TrefEnumImpWithMeta
#define TrefEnumImpWithMeta2 _TrefEnumImpWithMeta2

}  // namespace tref
