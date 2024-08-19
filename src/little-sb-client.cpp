#include "application.h"

auto main() -> int
{
  try {
    Application app{"localhost", 1438};
  }
  catch (std::exception &e) {
    std::println("Exception: {}", e.what());
  }

  return 0;
}
