# Commands

This file defines part of commands, with their usage.

## Full list

- Event **move**(glm::vec2 *direction*);
	- Parameters:
		- direction Specifies the direction of player's movement.
	- Returns:
		- Event{"ok"}
		- Event{"error", "Cannot move"}
