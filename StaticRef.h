#pragma once
#include <tuple>

#define TREF

/// refs: https://woboq.com/blog/verdigris-implementation-tricks.html
namespace tref {

using namespace std;

//////////////////////////////////////////////////////////////////////////
/// core

template<int N>
struct Id : Id<N - 1>
{
  static constexpr Id<N - 1> prev() { return {}; }
};

template<>
struct Id<0>
{};

using MaxId = Id<255>;

#define _RefReturn(T)                                                          \
  ->decltype(T) { return T; }

#define _RefInit(F, T)                                                         \
  friend constexpr std::tuple<> F(T**, tref::Id<0>) { return {}; }

#define _RefNextId(F, T)                                                       \
  tref::Id<std::tuple_size_v<decltype(F((T**)0, tref::MaxId{}))> + 1>

#define _RefPush(F, T, ...)                                                    \
  friend constexpr auto F(T**, _RefNextId(F, T) n) _RefReturn(                 \
    std::tuple_cat(F((T**)0, n.prev()), std::make_tuple(__VA_ARGS__)))

//////////////////////////////////////////////////////////////////////////

#define ReflectedTypeRoot(T)                                                   \
  _RefCommon(T);                                                               \
  _RefInit(subclass, self);                                                    \
  _RefInit(fields, self);

#define ReflectedType(T)                                                       \
  using super = _parent;                                                       \
  _RefCommon(T);                                                               \
  _RefInit(subclass, self);                                                    \
  _RefPush(subclass, super, (self*)0);                                         \
  _RefInit(fields, self);

#define _RefCommon(T)                                                          \
  using self = T;                                                              \
  struct RefTag;                                                               \
                                                                               \
protected:                                                                     \
  using _parent = T;                                                           \
                                                                               \
public:                                                                        \
  static constexpr auto __meta = std::make_tuple(#T, __FILE__, __LINE__);

#define ReflectedMeta(F, ...)                                                  \
  _RefPush(fields, self, std::make_tuple(#F, &self::F, __VA_ARGS__))

#define Reflected(F) ReflectedMeta(F, nullptr)

//////////////////////////////////////////////////////////////////////////

#define _RefDefChecker(name, expr)                                             \
  template<typename T, class = void_t<>>                                       \
  struct name : false_type                                                     \
  {};                                                                          \
                                                                               \
  template<typename T>                                                         \
  struct name<T, void_t<decltype((expr*)0)>> : true_type                       \
  {};

_RefDefChecker(IsReflected, typename T::RefTag);
_RefDefChecker(HasSuper, typename T::super);

/// break the iteration if f(x) returns false.
template<typename T, typename F>
constexpr bool
tupleFor(T&& t, F&& f)
{
  return apply([&f](auto&... x) { return (f(x) && ...); }, forward<T>(t));
};

template<typename T>
constexpr auto
fieldsOf()
{
  return fields((T**)0, MaxId{});
}

/// recursive
template<class T, typename F>
constexpr bool
eachField(F&& f, int level = 0)
{
  auto next = tupleFor(fieldsOf<T>(), [&](auto& c) {
    auto [name, ptr, meta] = c;
    return f(name, ptr, meta, level);
  });
  if (next)
    if constexpr (HasSuper<T>::value)
      return eachField<typename T::super>(f, level + 1);
  return true;
}

template<typename T>
constexpr auto
subclassesOf()
{
  return subclass((T**)0, MaxId{});
}

/// recursive
template<class T, typename F>
constexpr bool
eachSubclass(F&& f, int level = 0)
{
  auto& tp = subclassesOf<T>();
  return tupleFor(tp, [&](auto* c) {
    using C = remove_pointer_t<decltype(c)>;
    return f(c, level) && eachSubclass<C>(f, level + 1);
  });
}

} // namespace tref
