#pragma once
#include <tuple>
#include <type_traits>
#include <utility>

#define TREF

// refs: https://woboq.com/blog/verdigris-implementation-tricks.html

namespace tref {

using namespace std;

//////////////////////////////////////////////////////////////////////////
/// core

template <int N> struct id : public id<N - 1> {
  static constexpr int value = N;
  static constexpr id<N - 1> prev() { return {}; }
};

template <> struct id<0> { static constexpr int value = 0; };

#define _RefReturn(R)                                                          \
  ->decltype(R) { return R; }

#define _RefInit(F, T)                                                         \
  friend constexpr std::tuple<> F(T **, tref::id<0>) { return {}; }

#define _RefNextId(F, T)                                                       \
  tref::id<std::tuple_size_v<decltype(F((T **)0, tref::id<255>{}))> + 1>

#define _RefPush(F, T, ...)                                                    \
  friend constexpr auto F(T **, _RefNextId(F, T) n) _RefReturn(                \
      std::tuple_cat(F((T **)0, n.prev()), std::make_tuple(__VA_ARGS__)))

//////////////////////////////////////////////////////////////////////////
/// interface

#define RefRoot(T)                                                             \
  _RefCommon(T);                                                               \
  _RefInit(subclass, self);                                                    \
  _RefInit(fields, self);

#define RefType(T)                                                             \
  using super = _parent;                                                       \
  _RefCommon(T);                                                               \
  _RefInit(subclass, self);                                                    \
  _RefPush(subclass, super, (self *)0);                                        \
  _RefInit(fields, self);

#define _RefCommon(T)                                                          \
  using self = T;                                                              \
  struct RefTag;                                                               \
                                                                               \
protected:                                                                     \
  using _parent = T;                                                           \
                                                                               \
public:                                                                        \
  static constexpr auto __name = #T;

#define RefFieldMeta(F, ...)                                                   \
  _RefPush(fields, self, std::make_tuple(#F, &self::F, __VA_ARGS__))

#define RefField(F) RefFieldMeta(F, nullptr)

//////////////////////////////////////////////////////////////////////////
/// helpers

#define DefChecker(name, expr)                                                 \
  template <typename T, class = void_t<>> struct name : false_type {};         \
  template <typename T>                                                        \
  struct name<T, void_t<decltype((expr *)0)>> : true_type {};

DefChecker(IsReflected, typename T::RefTag);
DefChecker(HasSuper, typename T::super);

template <typename T, typename F> constexpr auto tuple_for(T &&t, F &&f) {
  return apply([&f](auto &... x) { return (f(x) && ...); }, forward<T>(t));
};

/// fields

template <typename T> constexpr auto fieldsOf() {
  return fields((T **)0, id<255>{});
}

template <class T, typename F> bool eachFields(F &&f, int level = 0) {
  auto &tp = fieldsOf<T>();
  auto next = tuple_for(tp, [&](auto &c) {
    auto [name, v, meta] = c;
    return f(name, v, meta, level);
  });
  if (next)
    if constexpr (HasSuper<T>::value)
      return eachFields<T::super>(f, level + 1);
  return true;
}

/// subclass

template <typename T> constexpr auto subclassOf() {
  return subclass((T **)0, id<255>{});
}

template <class T, typename F> bool eachSubClass(F &&f, int level = 0) {
  auto &tp = subclassOf<T>();
  return tuple_for(tp, [&](auto *c) {
    using C = remove_pointer_t<decltype(c)>;
    return f(c, level) && eachSubClass<C>(f, level + 1);
  });
}

} // namespace tref
