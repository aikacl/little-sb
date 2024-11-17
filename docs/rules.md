# Rules

This files defines how will the game run.

## Battle

After both 10 round of hits, each player can choose to escape.

## Battle field: 2D

The game map is a rectangle with borders. Players will initially be placed to
different positions, and not too close.

### Player
Player has following properties:
- Movement volecity
- Visual range

### Terrains
In each cell, there may be different **terrains**. And they make different effects.
- Basic. No any effect.
- Dirt. Lower speed upon it.
- Mountain. Upon it, your visual range will be greater; otherwise, your sight
  through it will be blocked.
- Forest. Inside it, you are invisible from outside.
- Water. Cannot move upon it.

### Weather
In different period of time, there are weathers, with different effect on
players.
- Sunny. No effects.
- Raining. All players' volecity should decrease with a factor.
- Foggy. All players' visual range will decrease with a factor.
