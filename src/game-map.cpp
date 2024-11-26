#include "game-map.h"

Game_map::Game_map(std::size_t height, std::size_t width)
    : _game_map(height, std::vector<Basic_terrain>(width))
{
}
auto Game_map::to_char_matrix() const -> std::vector<std::vector<char>>
{
  std::vector<std::vector<char>> result;
  result.reserve(_game_map.size());
  for (auto const &line : _game_map) {
    result.emplace_back();
    result.back().reserve(result.back().size());
    for (auto const &ele : line) {
      result.back().push_back(static_cast<char>(ele));
    }
  }
  return result;
}
void Game_map::modify(std::size_t row, std::size_t col, Basic_terrain terrain)
{
  _game_map[row][col] = terrain;
}
auto Game_map::height() const -> std::size_t
{
  return _game_map.size();
};
auto Game_map::width() const -> std::size_t
{
  return _game_map[0].size();
}
