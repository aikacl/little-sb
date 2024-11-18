#pragma once

#include "json.h"
#include "terrain.h"
#include <vector>

class Game_map {
public:
  Game_map() = default; // Provided to construct json. Make them happy.
  Game_map(std::size_t height, std::size_t width);

private:
  std::vector<std::vector<Terrain>> _game_map;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Game_map, _game_map);
};
