#pragma once

#include "effect.h"
#include "json.h"
#include <memory>
#include <string>

namespace item {

struct Item_info {
  std::string name;
  int price;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Item_info, name, price);
};

struct Item {
  Item_info info;
  std::shared_ptr<Effect> effect;
};

} // namespace item
