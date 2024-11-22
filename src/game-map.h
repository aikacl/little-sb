#pragma once

#include "json.h"
#include "terrain.h"
#include <vector>

class Game_map {
public:
  Game_map() = default; // Provided to construct json. Make them happy.
  Game_map(std::size_t height, std::size_t width);

  [[nodiscard]] auto to_char_matrix() const -> std::vector<std::vector<char>>;
  void modify(std::size_t row, std::size_t col, Basic_terrain terrain);

private:
  std::vector<std::vector<Basic_terrain>> _game_map;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Game_map, _game_map);
};
