#pragma once

#include "player-fwd.h"

namespace item {

// Applied command pattern.
class Effect {
public:
  virtual ~Effect() = default;
  virtual void perform(Player &p) = 0;
  virtual void deperform(Player &p) = 0;
};

class EnhancementEffect : public Effect {};

class MultiplyEnhancementEffect : public EnhancementEffect {
public:
  ~MultiplyEnhancementEffect() override = default;

  void perform(Player &p) override;
  void deperform(Player &p) override;
};

} // namespace item
