#include "features_manager.hpp"
#include "bhop.hpp"
#include "esp.hpp"

namespace features_manager {
	std::unique_ptr<std::thread> bhop_thread_ptr = nullptr;
	std::unique_ptr<std::thread> glow_thread_ptr = nullptr;

	void run() {
		while (true) {
			if (bhop_thread_ptr == nullptr || !bhop_thread_ptr->joinable()) {
				//features::bhop_thread_ptr = std::make_unique<std::thread>(bhop::run_bhop);
			}
			if (glow_thread_ptr == nullptr || !glow_thread_ptr->joinable()) {
				glow_thread_ptr = std::make_unique<std::thread>(esp::run_esp);
			}
		}
	}

	void start() {
		features_manager_t_ptr = std::make_unique<std::thread>(run);
	}
}