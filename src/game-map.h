#pragma once

#include "json.h"
#include "terrain.h"
#include <vector>

class Game_map {
public:
  Game_map() = default; // Provided to construct json. Make them happy.
  Game_map(std::size_t height, std::size_t width);

  [[nodiscard]] auto to_char_matrix() const -> std::vector<std::vector<char>>;

  [[nodiscard]] auto height() const -> std::size_t;
  [[nodiscard]] auto width() const -> std::size_t;

  void modify(std::size_t row, std::size_t col, Basic_terrain terrain);

  void update(auto const &players)
  {
    // Clears and fills in the map.
    _game_map.assign(_game_map.size(),
                     std::vector<Basic_terrain>(_game_map[0].size()));
    // `player`: std::unique_ptr<Player>
    for (auto const &player : players) {
      auto const &dir{player->position().dir};
      modify(dir.x, dir.y, Basic_terrain{'P'});
    }
  }

private:
  std::vector<std::vector<Basic_terrain>> _game_map;
  std::size_t _height;
  std::size_t _width;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(Game_map, _game_map);
};
