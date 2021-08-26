#pragma once

namespace el::ranges {

// From: https://en.cppreference.com/w/cpp/algorithm/ranges/for_each
template <class I, class F>
using for_each_result = std::ranges::in_fun_result<I, F>;

struct for_each_fn {
  template <std::input_iterator I,
            std::sentinel_for<I> S,
            class Proj = std::identity,
            std::indirectly_unary_invocable<std::projected<I, Proj>> Fun>
  constexpr auto operator()(I first, S last, Fun f, Proj proj = {}) const
      -> for_each_result<I, Fun> {
    for (; first != last; ++first) {
      std::invoke(f, std::invoke(proj, *first));
    }
    return {std::move(first), std::move(f)};
  }

  template <std::ranges::input_range R,
            class Proj = std::identity,
            std::indirectly_unary_invocable<
                std::projected<std::ranges::iterator_t<R>, Proj>> Fun>
  constexpr auto operator()(R&& r, Fun f, Proj proj = {}) const
      -> for_each_result<std::ranges::borrowed_iterator_t<R>, Fun> {
    return (*this)(std::ranges::begin(r), std::ranges::end(r), std::move(f),
                   std::ref(proj));
  }
};
inline constexpr for_each_fn for_each;

// From: https://en.cppreference.com/w/cpp/algorithm/ranges/all_any_none_of
struct all_of_fn {
  template <std::input_iterator I,
            std::sentinel_for<I> S,
            class Proj = std::identity,
            std::indirect_unary_predicate<std::projected<I, Proj>> Pred>
  constexpr auto operator()(I first, S last, Pred pred, Proj proj = {}) const
      -> bool {
    return std::ranges::find_if_not(first, last, std::ref(pred),
                                    std::ref(proj)) == last;
  }

  template <std::ranges::input_range R,
            class Proj = std::identity,
            std::indirect_unary_predicate<
                std::projected<std::ranges::iterator_t<R>, Proj>> Pred>
  constexpr auto operator()(R&& r, Pred pred, Proj proj = {}) const -> bool {
    return operator()(std::ranges::begin(r), std::ranges::end(r),
                      std::ref(pred), std::ref(proj));
  }
};
inline constexpr all_of_fn all_of;

struct any_of_fn {
  template <std::input_iterator I,
            std::sentinel_for<I> S,
            class Proj = std::identity,
            std::indirect_unary_predicate<std::projected<I, Proj>> Pred>
  constexpr auto operator()(I first, S last, Pred pred, Proj proj = {}) const
      -> bool {
    return std::ranges::find_if(first, last, std::ref(pred), std::ref(proj)) !=
           last;
  }

  template <std::ranges::input_range R,
            class Proj = std::identity,
            std::indirect_unary_predicate<
                std::projected<std::ranges::iterator_t<R>, Proj>> Pred>
  constexpr auto operator()(R&& r, Pred pred, Proj proj = {}) const -> bool {
    return operator()(std::ranges::begin(r), std::ranges::end(r),
                      std::ref(pred), std::ref(proj));
  }
};
inline constexpr any_of_fn any_of;

// From: https://en.cppreference.com/w/cpp/algorithm/ranges/find
struct find_if_fn {
  template <std::input_iterator I,
            std::sentinel_for<I> S,
            class Proj = std::identity,
            std::indirect_unary_predicate<std::projected<I, Proj>> Pred>
  constexpr auto operator()(I first, S last, Pred pred, Proj proj = {}) const
      -> I {
    for (; first != last; ++first) {
      if (std::invoke(pred, std::invoke(proj, *first))) {
        return first;
      }
    }
    return first;
  }

  template <std::ranges::input_range R,
            class Proj = std::identity,
            std::indirect_unary_predicate<
                std::projected<std::ranges::iterator_t<R>, Proj>> Pred>
  constexpr auto operator()(R&& r, Pred pred, Proj proj = {}) const
      -> std::ranges::borrowed_iterator_t<R> {
    return (*this)(std::ranges::begin(r), std::ranges::end(r), std::ref(pred),
                   std::ref(proj));
  }
};
inline constexpr find_if_fn find_if;

}  // namespace el::ranges
