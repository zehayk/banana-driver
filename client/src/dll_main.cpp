#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <stdexcept>

#include "core/interfaces.hpp"
#include "core/hooks.hpp"
#include "core/menu.hpp"

DWORD WINAPI cheat_thread(LPVOID instance) {
	try {
		// this order is important
		// must create menu before hooks are called
		interfaces::create();
		menu::create();
		hooks::create();
	}
	catch (const std::exception& e) {
		hooks::destroy();
		menu::destroy();  // again destroy order important
		interfaces::destroy();

		MessageBoxA(nullptr, e.what(), "Error", MB_ICONERROR);
		FreeLibraryAndExitThread(reinterpret_cast<HMODULE>(instance), 0);
	}

	while (!(GetAsyncKeyState(VK_END) & 1)) {
		Sleep(100);
	}

	hooks::destroy();
	menu::destroy();
	interfaces::destroy();

	//MessageBox(nullptr, L"Cheat is exiting...", L"Info", MB_ICONINFORMATION);

	FreeLibraryAndExitThread(reinterpret_cast<HMODULE>(instance), 0);

	return 0;
}

DWORD APIENTRY DllMain(HINSTANCE instance, DWORD reason, LPVOID) {
	if (reason == DLL_PROCESS_ATTACH) {
		DisableThreadLibraryCalls(instance);

		HANDLE thread = CreateThread(nullptr, 0, cheat_thread, instance, 0, nullptr);
		if (thread != INVALID_HANDLE_VALUE) {
			CloseHandle(thread);
			return TRUE;
		}
	}

	return FALSE;
}