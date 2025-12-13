#pragma once
#include <thread>

namespace features_manager {
	inline std::unique_ptr<std::thread> features_manager_t_ptr = nullptr;

	void start();
	
	//void initialize_features();
	//void toggle_feature(const char* feature_name, bool enabled);

}