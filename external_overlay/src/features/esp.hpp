#pragma once

#include <map>
#include <string>
#include <array>

#include "vector.hpp"

#include "../cs2_dump/client_dll.hpp"
#include "../cs2_dump/offsets.hpp"

using namespace cs2_dumper;
using namespace cs2_dumper::schemas::client_dll;
//using namespace cs2_dumper::offsets::client_dll;

namespace esp {
	struct Player {
		bool skip = false;

		// pointers
		uintptr_t pCSPlayerPawnPtr;

		// info
		int team;
		int health;
		int armor;
		std::string name;
		std::string weapon;
		uintptr_t spottedState;

		// pos
		uintptr_t gameSceneNode;
		Vector3 origin;
		Vector3 head;
		uintptr_t boneArray;
		std::array<Vector3, 17> bonePositions = {};
	};

	// pointer list for both threads to access
	inline Player *players_list_ptrs[64] = {};
	inline int localTeam = -1;

	namespace glow_config {
		inline float glow_red = 1.0f;
		inline float glow_green = 0.0f;
		inline float glow_blue = 1.0f;
		inline float glow_alpha = 1.0f;

		// Glow Types (according to UC, shit don't even work):
		// 0 = never | 1 = when being used | 2 = when being looked at | 3 = constantly
		inline int glow_type = 0;
	}

	namespace loc_offsets {
		constexpr auto entityList  = cs2_dumper::offsets::client_dll::dwEntityList;
		constexpr auto pGameSceneNode = C_BaseEntity::m_pGameSceneNode;

		// player ptrs
		constexpr auto dwLocalPlayerPawn = cs2_dumper::offsets::client_dll::dwLocalPlayerPawn;
		constexpr auto dwViewMatrix = cs2_dumper::offsets::client_dll::dwViewMatrix;
		constexpr auto hPlayerPawn = CCSPlayerController::m_hPlayerPawn;
		constexpr auto hController = C_BasePlayerPawn::m_hController;
		constexpr auto vOldOrigin = C_BasePlayerPawn::m_vOldOrigin;
		
		// player info
		constexpr auto iTeamNum = C_BaseEntity::m_iTeamNum;
		constexpr auto iHealth = C_BaseEntity::m_iHealth;
		constexpr auto armorValue = C_CSPlayerPawn::m_ArmorValue;
		constexpr auto iszPlayerName = CBasePlayerController::m_iszPlayerName;

		// glow
		constexpr auto glow = C_BaseModelEntity::m_Glow;
		constexpr auto glowObjectManager = cs2_dumper::offsets::client_dll::dwGlowManager;
		constexpr auto glowing = CGlowProperty::m_bGlowing;
		constexpr auto glowColor = C_DynamicProp::m_glowColor;
		constexpr auto glowColorOverride = CGlowProperty::m_glowColorOverride;
		constexpr auto glowType = CGlowProperty::m_iGlowType;
	};

	/*class CBones {
	public:
		std::map<std::string, Vector3> bonePositions;
	};*/

	inline int number_of_glowing_entities = 0;

	/*int get_num_glow_entities() {
		return number_of_glowing_entities;
	}
	void set_num_glow_entities(int num) {
		number_of_glowing_entities = num;
	}*/

	void draw_esp(bool *drew);

	void run_esp(); // must run in a thread

	
}