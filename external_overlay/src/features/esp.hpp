#pragma once

#include <map>
#include <string>

#include "vector.hpp"

#include "../cs2_dump/client_dll.hpp"
#include "../cs2_dump/offsets.hpp"

using namespace cs2_dumper;
using namespace cs2_dumper::schemas::client_dll;
using namespace cs2_dumper::offsets::client_dll;

namespace esp {

	namespace config {
		inline float glow_red = 1.0f;
		inline float glow_green = 0.0f;
		inline float glow_blue = 1.0f;
		inline float glow_alpha = 1.0f;

		// Glow Types (according to UC, shit don't even work):
		// 0 = never | 1 = when being used | 2 = when being looked at | 3 = constantly
		inline int glow_type = 0;
	}

	namespace loc_offsets {
		constexpr auto localPlayer = dwLocalPlayerPawn;
		constexpr auto entityList  = dwEntityList;
		
		constexpr auto iTeamNum = C_BaseEntity::m_iTeamNum;
		constexpr auto iHealth = C_BaseEntity::m_iHealth;
		//constexpr auto glowIndex = 

		constexpr auto hPlayerPawn = CCSPlayerController::m_hPlayerPawn;

		constexpr auto glow = C_BaseModelEntity::m_Glow;
		constexpr auto glowObjectManager = dwGlowManager;
		constexpr auto glowing = CGlowProperty::m_bGlowing;
		constexpr auto glowColor = C_DynamicProp::m_glowColor;
		constexpr auto glowColorOverride = CGlowProperty::m_glowColorOverride;
		constexpr auto glowType = CGlowProperty::m_iGlowType;
	};

	class CBones {
	public:
		std::map<std::string, Vector3> bonePositions;
	};

	inline int number_of_glowing_entities = 0;

	/*int get_num_glow_entities() {
		return number_of_glowing_entities;
	}
	void set_num_glow_entities(int num) {
		number_of_glowing_entities = num;
	}*/

	void run_esp(); // must run in a thread

	
}