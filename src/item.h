#pragma once

// #include "command.h"
#include "json.h"
#include <string>

struct Item {
  std::string name;
  int price;
  // Command effect;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Item, name, price
                                 // , effect
  );
};
