#pragma once

#include "json.h"

class Terrain {
public:
  Terrain(char display = 0);

  operator char() const
  {
    return _display;
  }

private:
  char _display;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Terrain, _display);
};

namespace terrains {

class Dirt : public Terrain {};

class Mountain : public Terrain {};

class Forest : public Terrain {};

class Water : public Terrain {};

} // namespace terrains
