/* TRef: A tiny compile time reflection system.
Author: soniced@sina.com

Refs:
https://woboq.com/blog/verdigris-implementation-tricks.html
https://www.codeproject.com/Articles/1002895/Clean-Reflective-Enums-Cplusplus-Enum-to-String-wi

*/

#pragma once
#include <string_view>
#include <tuple>
#include <type_traits>

#define TREF

namespace tref {
namespace imp {

using namespace std;

//////////////////////////////////////////////////////////////////////////
/// Common

constexpr auto tuple_for = [](auto&& t, auto&& f) {
  return apply([&f](auto&... x) { (..., f(x)); }, t);
};

#define _TrefReturn(T) \
  ->decltype(T) { return T; }

template <size_t L, size_t... R>
constexpr auto tail(index_sequence<L, R...>) {
  return index_sequence<R...>();
};

template <typename T>
constexpr auto is_pointer_to_pointer_v =
    is_pointer_v<T>&& is_pointer_v<remove_pointer_t<T>>;

//////////////////////////////////////////////////////////////////////////
/// macro version of map

#define _TrefMap(macro, arg, ...)                                    \
  _TrefIdentify(_TrefApply(_TrefChooseMap, _TrefCount(__VA_ARGS__))( \
      macro, arg, __VA_ARGS__))

#define _TrefChooseMap(N) _TrefMap##N
#define _TrefApply(macro, ...) _TrefIdentify(macro(__VA_ARGS__))
#define _TrefIdentify(x) x

#define _TrefMap1(m, a, x) m(a, x)
#define _TrefMap2(m, a, x, ...) \
  m(a, x) _TrefIdentify(_TrefMap1(m, a, __VA_ARGS__))
#define _TrefMap3(m, a, x, ...) \
  m(a, x) _TrefIdentify(_TrefMap2(m, a, __VA_ARGS__))
#define _TrefMap4(m, a, x, ...) \
  m(a, x) _TrefIdentify(_TrefMap3(m, a, __VA_ARGS__))
#define _TrefMap5(m, a, x, ...) \
  m(a, x) _TrefIdentify(_TrefMap4(m, a, __VA_ARGS__))
#define _TrefMap6(m, a, x, ...) \
  m(a, x) _TrefIdentify(_TrefMap5(m, a, __VA_ARGS__))
#define _TrefMap7(m, a, x, ...) \
  m(a, x) _TrefIdentify(_TrefMap6(m, a, __VA_ARGS__))
#define _TrefMap8(m, a, x, ...) \
  m(a, x) _TrefIdentify(_TrefMap7(m, a, __VA_ARGS__))
#define _TrefMap9(m, a, x, ...) \
  m(a, x) _TrefIdentify(_TrefMap8(m, a, __VA_ARGS__))
#define _TrefMap10(m, a, x, ...) \
  m(a, x) _TrefIdentify(_TrefMap9(m, a, __VA_ARGS__))
#define _TrefMap11(m, a, x, ...) \
  m(a, x) _TrefIdentify(_TrefMap10(m, a, __VA_ARGS__))
#define _TrefMap12(m, a, x, ...) \
  m(a, x) _TrefIdentify(_TrefMap11(m, a, __VA_ARGS__))
#define _TrefMap13(m, a, x, ...) \
  m(a, x) _TrefIdentify(_TrefMap12(m, a, __VA_ARGS__))
#define _TrefMap14(m, a, x, ...) \
  m(a, x) _TrefIdentify(_TrefMap13(m, a, __VA_ARGS__))
#define _TrefMap15(m, a, x, ...) \
  m(a, x) _TrefIdentify(_TrefMap14(m, a, __VA_ARGS__))
#define _TrefMap16(m, a, x, ...) \
  m(a, x) _TrefIdentify(_TrefMap15(m, a, __VA_ARGS__))
#define _TrefMap17(m, a, x, ...) \
  m(a, x) _TrefIdentify(_TrefMap16(m, a, __VA_ARGS__))
#define _TrefMap18(m, a, x, ...) \
  m(a, x) _TrefIdentify(_TrefMap17(m, a, __VA_ARGS__))
#define _TrefMap19(m, a, x, ...) \
  m(a, x) _TrefIdentify(_TrefMap18(m, a, __VA_ARGS__))
#define _TrefMap20(m, a, x, ...) \
  m(a, x) _TrefIdentify(_TrefMap19(m, a, __VA_ARGS__))

#define _TrefEvaluateCount(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, \
                           _13, _14, _15, _16, _17, _18, _19, _20, N, ...)    \
  N

#define _TrefCount(...)                                                     \
  _TrefIdentify(_TrefEvaluateCount(__VA_ARGS__, 20, 19, 18, 17, 16, 15, 14, \
                                   13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1))

//////////////////////////////////////////////////////////////////////////

template <int N = 255>
struct Id : Id<N - 1> {
  enum { value = N };
};

template <>
struct Id<0> {
  enum { value = 0 };
};

template <typename C, typename T>
tuple<Id<0>> _tref_elem(C**, T, Id<0> id);

#define _TrefElemCnt(C, T)                                 \
  std::tuple_element_t<0, decltype(_tref_elem((C**)0, T{}, \
                                              tref::imp::Id<>{}))>::value

#define _TrefPush(C, T, ...)                                                 \
  friend constexpr auto _tref_elem(C**, T,                                   \
                                   tref::imp::Id<_TrefElemCnt(C, T) + 1> id) \
      _TrefReturn(std::tuple(id, __VA_ARGS__))

template <class C, class T, class F, size_t... Is>
constexpr bool fold(index_sequence<Is...>, F&& f) {
  return (f(_tref_elem((C**)0, T{}, Id<Is>{})) && ...);
}

template <typename C, typename T, typename F>
constexpr bool each(F f) {
  constexpr auto cnt = _TrefElemCnt(C, T);
  if constexpr (cnt > 0) {
    return fold<C, T>(tail(make_index_sequence<cnt + 1>{}), f);
  } else
    return true;
};

//////////////////////////////////////////////////////////////////////////

struct DummyBase {};
void _tref_class_meta(void*);

template <typename T>
constexpr auto is_reflected_v =
    !std::is_same_v<decltype(_tref_class_meta((T**)0)), void>;

template <typename T>
constexpr auto class_meta_v = _tref_class_meta((T**)0);

#define _TrefClassMeta(T, Base)                 \
  friend constexpr auto _tref_class_meta(T**) { \
    return std::tuple(#T, sizeof(T), (Base*)0); \
  }

template <typename T>
constexpr auto has_base_v =
    is_reflected_v<T> &&
    !std::is_same_v<tuple_element_t<2, decltype(_tref_class_meta((T**)0))>,
                    DummyBase*>;

template <typename T>
using base_class_t =
    remove_pointer_t<tuple_element_t<2, decltype(_tref_class_meta((T**)0))>>;

//////////////////////////////////////////////////////////////////////////

struct MemberTag {};
struct SubclassTag {};

template <typename T, class = void_t<>>
struct get_parent {
  using type = DummyBase;
};

template <typename T>
struct get_parent<T, void_t<decltype((typename T::__parent*)0)>> {
  using type = typename T::__parent;
};

template <typename T>
using auto_base_t = typename get_parent<T>::type;

#define TrefTypeRoot(T) _TrefTypeCommon(T, tref::imp::DummyBase);
//#define TrefTypeRoot(T) TrefType(T)

#define TrefType(T)                         \
 private:                                   \
  using base_t = _parent;     \
  _TrefTypeCommon(T, base_t);               \
  _TrefRegisterSubclass(base_t);

#define TrefTypeExternalRoot(T) _TrefTypeCommon(T, tref::imp::DummyBase)
#define TrefTypeExternal(T, Base) \
  _TrefTypeCommon(T, Base);       \
  _TrefRegisterSubclass(Base);

#define _TrefRegisterSubclass(base) \
  _TrefPush(base, tref::imp::SubclassTag, (self_t*)0)

#define _TrefTypeCommon(T, Base) \
 private:                        \
  using self_t = T;              \
                                 \
 protected:                      \
  using _parent = T;             \
 public:                         \
  _TrefClassMeta(T, Base);

#define TrefMemberWithMeta(F, ...) _TrefMember(F, &self_t::F, __VA_ARGS__)
#define TrefMember(F) TrefMemberWithMeta(F, 0)
#define TrefMemberTypeWithMeta(F, ...) _TrefMember(F, (F**)0, __VA_ARGS__)
#define TrefMemberType(F) TrefMemberTypeWithMeta(F, 0)

#define _TrefMember(F, val, ...) \
  _TrefPush(self_t, tref::imp::MemberTag, #F, val, __VA_ARGS__)

/// recursive
/// @param F: [](const char* name, auto addr, auto meta, int level)
template <class C, typename F>
constexpr bool each_member(F&& f, int level = 0) {
  auto next = each<C, MemberTag>([&](auto info) {
    return f(get<1>(info), get<2>(info), get<3>(info), level);
  });
  if (next)
    if constexpr (has_base_v<C>)
      return each_member<base_class_t<C>>(f, level + 1);
  return true;
}

/// recursive
/// @param F: [](C* ptr, tuple<const char* name, int size>, int level)
template <class C, typename F>
constexpr bool each_subclass(F&& f, int level = 0) {
  return each<C, SubclassTag>([&](auto info) {
    using S = remove_pointer_t<tuple_element_t<1, decltype(info)>>;
    return f(get<1>(info), class_meta_v<S>, level) &&
           each_subclass<S>(f, level + 1);
  });
}

//////////////////////////////////////////////////////////////////////////
/// enum reflection

struct IgnoreAssignment {
  template <typename T>
  constexpr IgnoreAssignment(T v) : value((int)v) {}
  constexpr IgnoreAssignment operator=(int) { return *this; }
  int value;
};

void _tref_enum_items(void*);

#define TrefEnumImp(T, ...)                                               \
  constexpr auto _tref_enum_items(T*) {                                   \
    return std::tuple{_TrefIdentify(_TrefEnumStringize(T, __VA_ARGS__))}; \
  }

#define _TrefEnumStringizeSingle(P, E)          \
  std::tuple{(tref::imp::IgnoreAssignment)P::E, \
             tref::imp::_tref_trim_value(#E)},

#define _TrefEnumStringize(P, ...) \
  _TrefIdentify(_TrefMap(_TrefEnumStringizeSingle, P, __VA_ARGS__))

#define TrefEnumGlobal(T, ...)  \
  enum class T { __VA_ARGS__ }; \
  TrefEnumImp(T, __VA_ARGS__)

#define TrefEnum(T, ...)        \
  enum class T { __VA_ARGS__ }; \
  friend TrefEnumImp(T, __VA_ARGS__)

constexpr string_view _tref_trim_value(string_view s) {
  auto p = s.find_first_of(' ');
  if (p == string_view::npos)
    p = s.find_first_of('=');
  return s.substr(0, p);
}

template <typename T>
constexpr auto enum_to_string(T v) {
  constexpr auto& items = _tref_enum_items((T*)0);
  string_view ret = "";
  tuple_for(items, [&](auto e) {
    if (get<0>(e).value == (int)v) {
      ret = get<1>(e);
    }
  });
  return ret;
}

template <typename T>
constexpr auto string_to_enum(string_view s, T defVal) {
  constexpr auto& items = _tref_enum_items((T*)0);
  auto ret = defVal;
  tuple_for(items, [&](auto n) {
    if (s == get<1>(n)) {
      ret = (T)get<0>(n).value;
    }
  });
  return ret;
}

// @param F: [](T value, string_view name)
template <typename T, typename F>
constexpr auto enum_each(F f) {
  static_assert(is_enum_v<T>);
  constexpr auto& items = _tref_enum_items((T*)0);
  tuple_for(items, [&](auto n) { f((T)get<0>(n).value, get<1>(n)); });
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

using imp::each_member;
using imp::each_subclass;

using imp::base_class_t;
using imp::class_meta_v;
using imp::has_base_v;
using imp::is_pointer_to_pointer_v;
using imp::is_reflected_v;

using imp::enum_each;
using imp::enum_to_string;
using imp::Flags;
using imp::string_to_enum;

}  // namespace tref
