#include "visuals.hpp"
#include <../client/external/imgui/imgui.h>

namespace visuals {
	static world_to_screen_fn s_w2s;
	static get_player_count_fn s_count;
	static get_player_fn s_get;

	void set_world_to_screen(world_to_screen_fn fn) { s_w2s = fn; }
	void set_get_player_count(get_player_count_fn fn) { s_count = fn; }
	void set_get_player(get_player_fn fn) { s_get = fn; }

	static inline ImU32 team_color(int team) {
		// adjust team ids/colors to match your game
		if (team == 2) return IM_COL32(0, 150, 255, 255); // friendly
		if (team == 3) return IM_COL32(255, 60, 60, 255);  // enemy
		return IM_COL32(255, 255, 255, 200);
	}

	void render() {
		// require callbacks to be set
		if (!s_w2s || !s_count || !s_get) return;

		ImDrawList* dl = ImGui::GetForegroundDrawList();
		const std::size_t total = s_count();

		for (std::size_t i = 0; i < total; ++i) {
			player_view pv{};
			if (!s_get(i, pv)) continue;
			if (!pv.alive) continue;

			ImVec2 screen;
			if (!s_w2s(pv.origin, screen)) continue;

			// Box / scale heuristics - tweak to match your game's projection and player height
			const float box_w = 40.0f;
			const float box_h = 90.0f;
			ImVec2 tl(screen.x - box_w * 0.5f, screen.y - box_h);
			ImVec2 br(tl.x + box_w, tl.y + box_h);

			const ImU32 col = team_color(pv.team);

			// Box
			dl->AddRect(tl, br, col, 0.0f, 0, 1.5f);

			// Health bar (left)
			const float health_pct = (pv.health > 0) ? (pv.health / 100.0f) : 0.0f;
			ImVec2 hb_tl(tl.x - 6.0f, tl.y);
			ImVec2 hb_br(hb_tl.x + 4.0f, br.y);
			dl->AddRectFilled(ImVec2(hb_tl.x, br.y - (br.y - tl.y) * health_pct), hb_br, IM_COL32(0, 220, 0, 220));
			dl->AddRect(hb_tl, hb_br, IM_COL32(0, 0, 0, 160), 0.0f, 0, 1.0f);

			// Name
			if (pv.name) {
				const ImU32 text_col = IM_COL32(255, 255, 255, 230);
				dl->AddText(ImVec2(tl.x, tl.y - 12.0f), text_col, pv.name);
			}
		}
	}
}