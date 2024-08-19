#pragma once

#include <random>

namespace little_sb::random {

auto uniform(auto const lower_bound,
             auto const upper_bound) -> decltype(lower_bound + upper_bound)
{
  static std::random_device rd;
  static std::mt19937_64 gen(rd());

  std::uniform_int_distribution<> distrib{lower_bound, upper_bound};
  return distrib(gen);
}

} // namespace little_sb::random
