#pragma once

#include "features/classes/render.h"

namespace globals {
	inline int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	inline int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	// internal
	inline bool menu_open = true;
	inline bool relative_mouse_mode = false;  // might not need anymore

	// user settings
	inline bool aimbot_enabled = false;
	inline bool esp_enabled = false;
	inline bool glow_enabled = true;
	inline bool bhop_enabled = false;

	inline bool show_skeleton_esp = true;
	inline bool show_head_tracker = false;
	inline bool show_extra_flags = false;

	namespace esp_config {
		inline RGB esp_box_color_team = { 75, 175, 75 };
		inline RGB esp_box_color_enemy = { 225, 75, 75 };
		inline RGB esp_skeleton_color_team = { 75, 175, 75 };
		inline RGB esp_skeleton_color_enemy = { 225, 75, 75 };
		inline RGB esp_name_color = { 250, 250, 250 };
		inline RGB esp_distance_color = { 75, 75, 175 };
	}

	// gonna have more stuff later on
}