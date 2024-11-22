#pragma once

#include "json.h"

class Basic_terrain {
public:
  Basic_terrain(char display = '.');

  operator char() const
  {
    return _display;
  }

private:
  char _display;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Basic_terrain, _display);
};

namespace terrains {

class Dirt : public Basic_terrain {};

class Mountain : public Basic_terrain {};

class Forest : public Basic_terrain {};

class Water : public Basic_terrain {};

} // namespace terrains
