#include <iostream>
#include <Windows.h>
#include <thread>
#include <chrono>

#include "..\cs2_dump\client_dll.hpp"
#include "..\cs2_dump\offsets.hpp"
#include "..\cs2_dump\buttons.hpp"

#include "..\core\driver_communication.hpp"

#include "..\globals.hpp"

#define SPACE_BAR 0x20

using namespace cs2_dumper;
using namespace cs2_dumper::schemas::client_dll;
using namespace cs2_dumper::offsets::client_dll;

using namespace driver_communication;


namespace bhop {
	void bhop_worker() {  // run in separate thread
		HANDLE driver = get_driver_handle();
		DWORD pid = get_target_pid();

		if (driver == nullptr || driver == INVALID_HANDLE_VALUE || pid == 0) {
			return;
		}
		
        //if (driver::attach_to_process(driver, pid) == true) {
        //if (process_connected_to_cs2() == true) {
            //std::cout << "Attachment successful.\n";

            //bhop cheat here
            //if (const std::uintptr_t client = get_module_base(pid, L"client.dll"); client != 0) {
            //std::cout << "Client found.\n";

        std::uintptr_t client = 0;
        while (true) {

            while (!globals::bhop_enabled || !process_connected_to_cs2() || client == 0) {
				client = 0; // if we lose connection reset client
                client = get_module_base(pid, L"client.dll");
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }

			//MessageBox(nullptr, L"Bhop activated", L"Info", MB_OK);
            // idk where the magic numbers come from. Youtube guy didn't explain

            const auto local_player_pawn = driver::read_memory<std::uintptr_t>(driver, client + dwLocalPlayerPawn);

            if (local_player_pawn == 0)
                continue;

            /* ------------------------------------------------------BHOP------------------------------------------------------ */
            const uint32_t STANDING = 65665;
            const uint32_t CROUCHING = 65667;

            uint32_t PLUS_JUMP = 65537;
            uint32_t MINUS_JUMP = 256;

            uint32_t plusJumpDelay = 10;
            uint32_t minusJumpDelay = 10;

            std::uintptr_t ForceJump = client + buttons::jump;
            const auto flags = driver::read_memory<std::uint32_t>(driver, local_player_pawn + C_BaseEntity::m_fFlags);

            

            if (GetAsyncKeyState(SPACE_BAR) & 0x8000)
            {
                if (flags == STANDING || flags == CROUCHING)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(plusJumpDelay));
                    driver::write_memory(driver, ForceJump + buttons::jump, PLUS_JUMP);
                }
                else
                {
                    driver::write_memory(driver, ForceJump + buttons::jump, MINUS_JUMP);
                }
            }

            //const bool on_the_ground = flags & (1 << 0);
            //const bool space_pressed = GetAsyncKeyState(VK_SPACE);
            //const auto force_jump = driver::read_memory<DWORD>(driver, local_player_pawn + buttons::jump);

            //if (space_pressed && on_the_ground) {
            //    //Sleep(10);
            //    std::this_thread::sleep_for(std::chrono::milliseconds(10));
            //    driver::write_memory(driver, client + buttons::jump, 65537);
            //}
            //else if (space_pressed && !on_the_ground) {
            //    driver::write_memory(driver, client + buttons::jump, 256);
            //}
            //else if (!space_pressed && force_jump == 65537) {
            //    driver::write_memory(driver, client + buttons::jump, 256);
            //}

            /* ---------------------------------------------------------------------------------------------------------------- */

        }
	}

    void run_bhop() {
        // create new bhop worker thread, and monitor (join it), if it exits, start a new one

        while (true) {
            bhop_worker();
        }
    }
}