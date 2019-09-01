#pragma once
#include <tuple>
#include <type_traits>
#include <utility>

#define TREF

// refs: https://woboq.com/blog/verdigris-implementation-tricks.html
namespace tref {

using namespace std;

template <int N> struct id : public id<N - 1> {
  static constexpr int value = N;
  static constexpr id<N - 1> prev() { return {}; }
};

template <typename T> struct tag;

template <> struct id<0> { static constexpr int value = 0; };

template <typename T, typename F> constexpr auto tuple_for(T &&t, F &&f) {
  return apply([&f](auto &... x) { return (f(x) && ...); }, forward<T>(t));
};

template <typename T, class = void_t<>> struct IsReflected : false_type {};

template <typename T>
struct IsReflected<T, void_t<decltype(&T::w_states)>> : true_type {};

#define _RefReturn(R)                                                          \
  ->decltype(R) { return R; }

#define _RefNextId(preState) tref::id<std::tuple_size_v<decltype(preState)> + 1>

//////////////////////////////////////////////////////////////////////////

#define _ref_last_state() w_state(tref::id<255>{})

#define _ref_fields_common(T)                                                  \
  static constexpr std::tuple<> w_state(tref::id<0>) { return {}; }

#define _ref_fields_root(T)                                                    \
public:                                                                        \
  static constexpr auto w_states() { return _ref_last_state(); }

#define _ref_fields()                                                          \
public:                                                                        \
  static constexpr auto w_states() {                                           \
    return std::tuple_cat(_ref_last_state(), super::w_states());               \
  }

#define ref_field_meta(S, ...)                                                 \
  static constexpr auto w_state(_RefNextId(_ref_last_state()) n) _RefReturn(   \
      std::tuple_cat(w_state(n.prev()), std::make_tuple(std::make_tuple(       \
                                            #S, &self::S, __VA_ARGS__))))

#define ref_field(S) ref_field_meta(S, nullptr)

//////////////////////////////////////////////////////////////////////////

#define RefRoot(T, ...)                                                        \
  _RefCommon(T);                                                               \
  _RefSubClassRoot();                                                          \
  _RefFieldsRoot(__VA_ARGS__)

#define RefType(T, ...)                                                        \
  using super = _parent;                                                       \
  _RefCommon(T);                                                               \
  _RefSubClassRoot();                                                          \
  _RefSubClass();                                                              \
  _RefFields(__VA_ARGS__)

#define _RefCommon(T)                                                          \
  using self = T;                                                              \
                                                                               \
protected:                                                                     \
  using _parent = T;                                                           \
                                                                               \
public:                                                                        \
  static constexpr auto __name = #T;

//////////////////////////////////////////////////////////////////////////
/// fields

#define _RefFieldsRoot(...)                                                    \
public:                                                                        \
  static constexpr auto w_states() { return std::make_tuple(__VA_ARGS__); }

#define _RefFields(...)                                                        \
public:                                                                        \
  static constexpr auto w_states() {                                           \
    return std::tuple_cat(super::w_states(), std::make_tuple(__VA_ARGS__));    \
  }

#define RefFieldMeta(F, ...) std::make_tuple(#F, &self::F, __VA_ARGS__)
#define RefField(F) RefFieldMeta(F, nullptr)

template <typename T> constexpr auto fieldsOf() { return T::w_states(); };

  //////////////////////////////////////////////////////////////////////////
  /// subclass

#define _RefSubClassRoot()                                                     \
  friend constexpr std::tuple<> subclass(tref::tag<self> *, tref::id<0>) {     \
    return {};                                                                 \
  }

#define _RefSubClass()                                                         \
  friend constexpr auto subclass(                                              \
      tref::tag<super> *,                                                      \
      _RefNextId(subclass((tref::tag<super> *)0, tref::id<255>{})) n)          \
      _RefReturn(std::tuple_cat(subclass((tref::tag<super> *)0, n.prev()),     \
                                std::make_tuple((self *)0)))

template <typename T> constexpr auto subclassOf() {
  return subclass((tag<T> *)0, id<255>{});
}

template <class T, typename F> bool eachSubClass(F &&f, int level = 0) {
  auto &tp = subclassOf<T>();
  return tuple_for(tp, [&](auto *c) {
    using C = remove_pointer_t<decltype(c)>;
    return f(c, level) && eachSubClass<C>(f, level + 1);
  });
}

} // namespace tref
