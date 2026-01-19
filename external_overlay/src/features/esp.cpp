#include <Windows.h>
#include "esp.hpp"

#include "..\core\driver_communication.hpp"

#include "..\globals.hpp"

#include "classes\bone.hpp"
#include "classes\render.h"

#include <stdexcept>
#include <locale>
#include <codecvt>

using namespace driver_communication;

namespace esp {
    void esp_worker() {
        HANDLE driver = get_driver_handle();
        DWORD pid = get_target_pid();

        if (driver == nullptr || driver == INVALID_HANDLE_VALUE || pid == 0) {
            return;
        }


        std::uintptr_t client = 0;  // used to be const
        static Player players[64];
        while (true) {
            while ((!globals::show_skeleton_esp && !globals::show_head_tracker && !globals::glow_enabled) || !process_connected_to_cs2() || client == 0) {
                client = 0; // if we lose connection reset client
                client = get_module_base(pid, L"client.dll");
                //Sleep(500);
            }

            const auto local_player_pawn = driver::read_memory<std::uintptr_t>(driver, client + loc_offsets::dwLocalPlayerPawn);

            localTeam = driver::read_memory<int>(driver, local_player_pawn + loc_offsets::iTeamNum);
			auto entityList = driver::read_memory<uintptr_t>(driver, client + loc_offsets::entityList);
            if (!entityList)
                return;
            Vector3 localOrigin = driver::read_memory<Vector3>(driver, local_player_pawn + loc_offsets::vOldOrigin);

            int glowNum = 0;

            for (int i = 1; i < 64; i++)
            {
                /*if (players_list_ptrs[i] == nullptr) {
                    Player player;
                    players_list_ptrs[i] = &player;
                }
				Player& player = *players_list_ptrs[i];*/

                Player& player = players[i];       // reference to persistent storage
				
                //uintptr_t pCSPlayerPawnPtr;
                //try {
                std::string outdated_offsets_msg = "Offsets outdated (size of one entity-list entry is wrong)";
                uintptr_t list_entry1 = driver::read_memory<uintptr_t>(driver, entityList + (8 * (i & 0x7FFF) >> 9) + 16);
                if (!list_entry1) {
                    //players_list_ptrs[i] = nullptr;
                    continue;  // throw std::runtime_error(outdated_offsets_msg);
                }
                uintptr_t playerController = driver::read_memory<uintptr_t>(driver, list_entry1 + 112 * (i & 0x1FF));
                if (!playerController) {
                    //players_list_ptrs[i] = nullptr;
                    continue;  // throw std::runtime_error(outdated_offsets_msg);
                }
                uint32_t  playerPawn = driver::read_memory<uintptr_t>(driver, playerController + loc_offsets::hPlayerPawn);
                if (!playerPawn) {  // cant find this one - returns 0, then on the next iteration, it gives BSOD
                    //players_list_ptrs[i] = nullptr;
                    continue;  // throw std::runtime_error(outdated_offsets_msg);
                }
                uintptr_t list_entry2 = driver::read_memory<uintptr_t>(driver, entityList + 0x8 * ((playerPawn & 0x7FFF) >> 9) + 16);
                if (!list_entry2) {
                    //players_list_ptrs[i] = nullptr;
                    continue;  // throw std::runtime_error(outdated_offsets_msg);
                }
                player.pCSPlayerPawnPtr = driver::read_memory<uintptr_t>(driver, list_entry2 + 112 * (playerPawn & 0x1FF));
                if (!player.pCSPlayerPawnPtr) {
                    //players_list_ptrs[i] = nullptr;
                    continue;  // throw std::runtime_error(outdated_offsets_msg);
                }
                /*} catch (const std::runtime_error& e) {
                    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
                    MessageBoxW(NULL, converter.from_bytes(e.what()).c_str(), L"ESP Error", MB_OK | MB_ICONERROR);
                    return;
				}*/
                player.team = driver::read_memory<uintptr_t>(driver, player.pCSPlayerPawnPtr + loc_offsets::iTeamNum);
                if (player.team == localTeam) {
                    players_list_ptrs[i] = nullptr;
                    continue;
                }
                player.health = driver::read_memory<uintptr_t>(driver, player.pCSPlayerPawnPtr + loc_offsets::iHealth);
                if (player.health <= 0 || player.health > 100) {
                    players_list_ptrs[i] = nullptr;
                    continue;
                }
                player.armor = driver::read_memory<uintptr_t>(driver, player.pCSPlayerPawnPtr + loc_offsets::armorValue);


                // Put glow in it's own file later
                /* ------------------------------------------------------GLOW------------------------------------------------------ */
                //{
                //    float glowColor[4] = { glow_config::glow_red, glow_config::glow_green, glow_config::glow_blue, glow_config::glow_alpha };
                //    DWORD colorArgb = ((DWORD)(glowColor[3] * 255) << 24) |  // A
                //        ((DWORD)(glowColor[2] * 255) << 16) |			    // B
                //        ((DWORD)(glowColor[1] * 255) << 8) |			   // G
                //        ((DWORD)(glowColor[0] * 255));                    // R
                //    uint32_t glowType;
                //    if (glow_config::glow_type < 0) {
                //        glowType = 0;
                //    }
                //    else if (glow_config::glow_type > 3) {
                //        glowType = 3;
                //    }
                //    glowType = static_cast<uint32_t>(glow_config::glow_type);

                //    driver::write_memory(driver, player.pCSPlayerPawnPtr + loc_offsets::glow + loc_offsets::glowing, true);
                //    // Glow Types:
                //    // 0 = never | 1 = when being used | 2 = when being looked at | 3 = constantly
                //    driver::write_memory(driver, player.pCSPlayerPawnPtr + loc_offsets::glow + loc_offsets::glowType, glowType);
                //    driver::write_memory(driver, player.pCSPlayerPawnPtr + loc_offsets::glow + loc_offsets::glowColorOverride, colorArgb);
                //    glowNum++;
                //}
                /* ---------------------------------------------------------------------------------------------------------------- */
                




                // ESP ------------------------------------------------------------------------------------------------------------ */
                // Read entity controller from the player pawn
                uintptr_t handle = driver::read_memory<std::uintptr_t>(driver, player.pCSPlayerPawnPtr + loc_offsets::hController);
                int index = handle & 0x7FFF;
                int segment = index >> 9;
                int entry = index & 0x1FF;

                uintptr_t controllerListSegment = driver::read_memory<std::uintptr_t>(driver, entityList + 0x8 * segment + 0x10);
                uintptr_t controller = driver::read_memory<uintptr_t>(driver, controllerListSegment + 112 * entry);

                if (!controller) {
                    players_list_ptrs[i] = nullptr;
                    continue;
                }

                // Read player name from the controller
                char buffer[256] = {};
                driver::read_memory_char(driver, controller + loc_offsets::iszPlayerName, &buffer);
                buffer[sizeof(buffer) - 1] = '\0';
                std::string player_name = buffer;

				view_matrix_t viewMatrix = driver::read_memory<view_matrix_t>(driver, client + loc_offsets::dwViewMatrix);
                player.gameSceneNode = driver::read_memory<uintptr_t>(driver, player.pCSPlayerPawnPtr + loc_offsets::pGameSceneNode);
                player.origin = driver::read_memory<Vector3>(driver, player.pCSPlayerPawnPtr + loc_offsets::vOldOrigin);
                player.head = { player.origin.x, player.origin.y, player.origin.z + 75.f };

                if (player.origin.x == localOrigin.x && player.origin.y == localOrigin.y && player.origin.z == localOrigin.z) {
                    players_list_ptrs[i] = nullptr;
                    continue;
                }

                //if (config::render_distance != -1 && (localOrigin - player.origin).length2d() > config::render_distance) continue;
                if (player.origin.x == 0 && player.origin.y == 0) {
                    players_list_ptrs[i] = nullptr;
                    continue;
                }

                // Bone array offset updated from 0x1F0 to 0x210 (m_modelState + 128)
                if (globals::show_skeleton_esp) {
                    player.gameSceneNode = driver::read_memory<uintptr_t>(driver, player.pCSPlayerPawnPtr + loc_offsets::pGameSceneNode);
                    player.boneArray = driver::read_memory<uintptr_t>(driver, player.gameSceneNode + 0x210);

                    // first read all the skeleton nodes
					int number_of_bones = sizeof(bone_to_game_idx) / sizeof(bone_to_game_idx[0]);
                    for (int i = 0; i < number_of_bones; ++i) {
                        bone boneName = static_cast<bone>(i);
                        int boneIndex = bone_to_game_idx[boneName];
                        uintptr_t boneAddress = player.boneArray + boneIndex * 32;
                        Vector3 bonePos = driver::read_memory<Vector3>(driver, boneAddress);
						player.bonePositions[boneName] = bonePos.WorldToScreen(viewMatrix);
                    }
                }

                // read only head
                if (globals::show_head_tracker && !globals::show_skeleton_esp) {
                    player.gameSceneNode = driver::read_memory<uintptr_t>(driver, player.pCSPlayerPawnPtr + loc_offsets::pGameSceneNode);
                    player.boneArray = driver::read_memory<uintptr_t>(driver, player.gameSceneNode + 0x210);
                    
                    uintptr_t boneAddress = player.boneArray + head * 32;
                    Vector3 bonePos = driver::read_memory<Vector3>(driver, boneAddress);
					player.bonePositions[head] = bonePos.WorldToScreen(viewMatrix);
                }

                // TODO: later
                if (globals::show_extra_flags) {
                    /*
                    * Reading values for extra flags is now separated from the other reads
                    * This removes unnecessary memory reads, improving performance when not showing extra flags
                    */
                    // player.is_defusing = process->read<bool>(player.pCSPlayerPawn + updater::offsets::m_bIsDefusing);
                    // 
                    // playerMoneyServices = process->read<uintptr_t>(player.entity + updater::offsets::m_pInGameMoneyServices);
                    // player.money = process->read<int32_t>(playerMoneyServices + updater::offsets::m_iAccount);
                    // 
                    // player.flashAlpha = process->read<float>(player.pCSPlayerPawn + updater::offsets::m_flFlashOverlayAlpha);
                    // 
                    // clippingWeapon = process->read<std::uint64_t>(player.pCSPlayerPawn + updater::offsets::m_pClippingWeapon);
                    // std::uint64_t firstLevel = process->read<std::uint64_t>(clippingWeapon + 0x10); // First offset
                    // weaponData = process->read<std::uint64_t>(firstLevel + 0x20); // Final offset
                    // /*weaponData = process->read<std::uint64_t>(clippingWeapon + 0x10);
                    // weaponData = process->read<std::uint64_t>(weaponData + updater::offsets::m_szName);*/
                    // char buffer[MAX_PATH];
                    // process->read_raw(weaponData, buffer, sizeof(buffer));
                    // std::string weaponName = std::string(buffer);
                    // if (weaponName.compare(0, 7, "weapon_") == 0)
                    //     player.weapon = weaponName.substr(7, weaponName.length()); // Remove weapon_ prefix
                    // else
                    //     player.weapon = "Invalid Weapon Name";
                }


                // pass player ptr to players list
				//players_list_ptrs[i] = &player;

                // mark as valid for rendering
                players_list_ptrs[i] = &players[i];
            }
			esp::number_of_glowing_entities = glowNum;
            //set_num_glow_entities(shitter);
        }
    }

    void draw_esp(bool *drew) {
        if (globals::show_skeleton_esp == false && globals::show_head_tracker == false) {
            return;
        }

        for (int i = 1; i < 64; ++i)
        {
            esp::Player* player = players_list_ptrs[i];
            if (player == nullptr)
                continue;

            *drew = true;

            for (const BoneConnection& bc : boneConnection)
            {
                Render::DrawLine(
                    (*player).bonePositions[bc.bomeFrom].x,
                    (*player).bonePositions[bc.bomeFrom].y,
                    (*player).bonePositions[bc.boneTo].x,
                    (*player).bonePositions[bc.boneTo].y,
                    ((*player).team == localTeam) ? globals::esp_config::esp_skeleton_color_team : globals::esp_config::esp_skeleton_color_enemy,
                    2
                );
            }
        }
	}

    void run_esp() {
        while (true) {
            if (globals::glow_enabled || globals::show_skeleton_esp || globals::show_head_tracker) {
                esp_worker();
            }
		}
    }
}
