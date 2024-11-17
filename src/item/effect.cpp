#include "effect.h"
#include "player.h"

void item::MultiplyEnhancementEffect::perform(player::Player &p)
{
  p._damage_amplification.insert();
}

void item::MultiplyEnhancementEffect::deperform(player::Player &p) {}
