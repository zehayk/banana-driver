#pragma once

#include <functional>
#include <cstddef>
#include <../client/external/imgui/imgui.h>

namespace visuals {
	// Generic player view used by visuals subsystem. Populate this from your existing player list.
	struct player_view {
		float origin[3];     // world position (x,y,z) - whatever your world coord layout is
		int health = 100;
		int team = 0;
		bool alive = false;
		const char* name = nullptr;
	};

	// Callbacks you must provide so visuals can query players and project to screen.
	using world_to_screen_fn = std::function<bool(const float origin[3], ImVec2& out)>;
	using get_player_count_fn = std::function<std::size_t()>;
	using get_player_fn = std::function<bool(std::size_t index, player_view& out)>;

	// Set callbacks (call once during initialization)
	void set_world_to_screen(world_to_screen_fn fn);
	void set_get_player_count(get_player_count_fn fn);
	void set_get_player(get_player_fn fn);

	// Called each frame from the present hook. Draws boxes, names and health bars.
	void render();
}