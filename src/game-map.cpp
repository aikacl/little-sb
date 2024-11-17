#include "game-map.h"

Game_map::Game_map(std::size_t height, std::size_t width)
    : _game_map(height, std::vector<Terrain>(width))
{
}
