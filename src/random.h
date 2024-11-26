#pragma once

#include <random>

namespace little_sb::random {

template <typename T> inline auto uniform(T lower_bound, T upper_bound) -> T
{
  static std::random_device rd;
  static std::mt19937_64 gen(rd());

  if constexpr (std::is_integral_v<T>) {
    std::uniform_int_distribution<> distrib{lower_bound, upper_bound};
    return distrib(gen);
  }
  else {
    std::uniform_real_distribution<> distrib{lower_bound, upper_bound};
    return distrib(gen);
  }
}

template <typename T> inline auto uniform(std::pair<T, T> const &bound) -> T
{
  return uniform(bound.first, bound.second);
}

// @param ratio The probability of which the funtion returns a true value.
inline auto probability(float ratio) -> bool
{
  constexpr auto max{std::numeric_limits<int>::max()};
  return static_cast<float>(uniform(0, max)) / static_cast<float>(max) <= ratio;
}

} // namespace little_sb::random
