#include <Windows.h>
#include "esp.hpp"

#include "..\core\driver_communication.hpp"

#include "..\globals.hpp"

using namespace driver_communication;

namespace esp {
    void esp_worker() {
        HANDLE driver = get_driver_handle();
        DWORD pid = get_target_pid();

        if (driver == nullptr || driver == INVALID_HANDLE_VALUE || pid == 0) {
            return;
        }


        std::uintptr_t client = 0;  // used to be const
        while (true) {
            while (!globals::glow_enabled || !process_connected_to_cs2() || client == 0) {
                client = 0; // if we lose connection reset client
                client = get_module_base(pid, L"client.dll");
                //Sleep(500);
            }

            const auto local_player_pawn = driver::read_memory<std::uintptr_t>(
                driver, client + dwLocalPlayerPawn);

            auto localTeam = driver::read_memory<int>(driver, local_player_pawn + loc_offsets::iTeamNum);
			auto entityList = driver::read_memory<uintptr_t>(driver, client + loc_offsets::entityList);
            if (!entityList)
                return;

            int glowNum = 0;

            for (int i = 1; i < 64; i++)
            {
                //uintptr_t list_entry1 = *(uintptr_t*)(entityList + (8 * (i & 0x7FFF) >> 9) + 16);
                uintptr_t list_entry1 = driver::read_memory<uintptr_t>(driver, entityList + (8 * (i & 0x7FFF) >> 9) + 16);
                if (!list_entry1)
                    continue;

                //uintptr_t playerController = *(uintptr_t*)(list_entry1 + 120 * (i & 0x1FF));
                uintptr_t playerController = driver::read_memory<uintptr_t>(driver, list_entry1 + 112 * (i & 0x1FF));
                if (!playerController)
                    continue;

                //uintptr_t  playerPawn = *(uintptr_t*)(playerController + loc_offsets::hPlayerPawn); ///uint32_t
                uint32_t  playerPawn = driver::read_memory<uintptr_t>(driver, playerController + loc_offsets::hPlayerPawn);
				if (!playerPawn)  // cant find this one - returns 0, then on the next iteration, it gives BSOD
                    continue;

                //uintptr_t list_entry2 = *(uintptr_t*)(entityList + 0x8 * ((playerPawn & 0x7FFF) >> 9) + 16);
                uintptr_t list_entry2 = driver::read_memory<uintptr_t>(driver, entityList + 0x8 * ((playerPawn & 0x7FFF) >> 9) + 16);
                if (!list_entry2)
                    continue;

                //uintptr_t pCSPlayerPawnPtr = *(uintptr_t*)(list_entry2 + 120 * (playerPawn & 0x1FF));
                uintptr_t pCSPlayerPawnPtr = driver::read_memory<uintptr_t>(driver, list_entry2 + 112 * (playerPawn & 0x1FF));
                if (!pCSPlayerPawnPtr)
                    continue;  // this is 0 watafak

                //int team = *(int*)(pCSPlayerPawnPtr + loc_offsets::iTeamNum);
                int team = driver::read_memory<uintptr_t>(driver, pCSPlayerPawnPtr + loc_offsets::iTeamNum);
                if (team == localTeam)
                    continue;

                //int health = *(int*)(pCSPlayerPawnPtr + loc_offsets::iHealth);
                int health = driver::read_memory<uintptr_t>(driver, pCSPlayerPawnPtr + loc_offsets::iHealth);
                if (health <= 0)
                    continue;


                // Put glow in it's own file later
                /* ------------------------------------------------------GLOW------------------------------------------------------ */
                float glowColor[4] = { config::glow_red, config::glow_green, config::glow_blue, config::glow_alpha };
                DWORD colorArgb = ((DWORD)(glowColor[3] * 255) << 24) |  // A
                    ((DWORD)(glowColor[2] * 255) << 16) |			    // B
                    ((DWORD)(glowColor[1] * 255) << 8) |			   // G
                    ((DWORD)(glowColor[0] * 255));                    // R
                uint32_t glowType;
                if (config::glow_type < 0) {
                    glowType = 0;
                }
                else if (config::glow_type > 3) {
                    glowType = 3;
                }
                else {
                    glowType = static_cast<uint32_t>(config::glow_type);
                }

                driver::write_memory(driver, pCSPlayerPawnPtr + loc_offsets::glow + loc_offsets::glowing, true);
                // Glow Types:
                // 0 = never | 1 = when being used | 2 = when being looked at | 3 = constantly
                driver::write_memory(driver, pCSPlayerPawnPtr + loc_offsets::glow + loc_offsets::glowType, glowType);
                driver::write_memory(driver, pCSPlayerPawnPtr + loc_offsets::glow + loc_offsets::glowColorOverride, colorArgb);
                //driver::write_memory(driver, pCSPlayerPawnPtr + loc_offsets::glow + loc_offsets::glowColorOverride, 255);           // Red
				//driver::write_memory(driver, pCSPlayerPawnPtr + loc_offsets::glow + loc_offsets::glowColorOverride + 0x4, 255);     // Green
				//driver::write_memory(driver, pCSPlayerPawnPtr + loc_offsets::glow + loc_offsets::glowColorOverride + 0x8, 255);     // Blue
				//driver::write_memory(driver, pCSPlayerPawnPtr + loc_offsets::glow + loc_offsets::glowColorOverride + 0xc, 255);     // Alpha
                /* ---------------------------------------------------------------------------------------------------------------- */
                
                
                glowNum++;
            }
			esp::number_of_glowing_entities = glowNum;
            //set_num_glow_entities(shitter);
        }
    }

    void run_esp() {
        while (true) {
            if (globals::glow_enabled) {
                esp_worker();
            }
		}
    }
}
