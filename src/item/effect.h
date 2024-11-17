#pragma once

#include "player-fwd.h"

namespace item {

// Applied command pattern.
class Effect {
public:
  virtual ~Effect() = default;
  virtual void perform(player::Player &p) = 0;
  virtual void deperform(player::Player &p) = 0;
};

class EnhancementEffect : public Effect {};

class MultiplyEnhancementEffect : public EnhancementEffect {
public:
  ~MultiplyEnhancementEffect() override = default;

  void perform(player::Player &p) override;
  void deperform(player::Player &p) override;
};

} // namespace item
