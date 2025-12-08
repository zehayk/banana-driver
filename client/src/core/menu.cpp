#include "menu.hpp"

#include "globals.hpp"
#include "hooks.hpp"
#include "interfaces.hpp"

#include <../client/external/imgui/imgui.h>
#include <../client/external/imgui/imgui_impl_dx11.h>
#include <../client/external/imgui/imgui_impl_win32.h>

#include <stdexcept>

static WNDPROC original_wndproc = nullptr;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

LRESULT __stdcall hook_wndproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	if (msg == WM_KEYDOWN && wparam == VK_INSERT) {
		globals::menu_open = !globals::menu_open;

		hooks::original_set_relative_mouse_mode(
			interfaces::input_system, 
			globals::menu_open ? false : globals::relative_mouse_mode);
	}

	if (globals::menu_open) {
		ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam);
		return true;
	} else {
		return CallWindowProcA(original_wndproc, hwnd, msg, wparam, lparam);
	}
}

namespace menu {
	void create() {
		if (!interfaces::d3d11_device || !interfaces::d3d11_device_context || !interfaces::hwnd) {
			throw std::runtime_error("interfaces not initialized");
		}

		original_wndproc = reinterpret_cast<WNDPROC>(SetWindowLongPtrA(
										interfaces::hwnd, GWLP_WNDPROC, 
										reinterpret_cast<LONG_PTR>(hook_wndproc)));


		// IMGUI MENU RAAAAAAHHHHH
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

		ImGui::StyleColorsDark();

		if (!ImGui_ImplWin32_Init(interfaces::hwnd)) {
			throw std::runtime_error("failed to initialize imgui win32 impl");
		}
		if (!ImGui_ImplDX11_Init(interfaces::d3d11_device, interfaces::d3d11_device_context)) {
			throw std::runtime_error("failed to initialize imgui dx11 impl");
		}


	}

	void destroy() {
		// destroy imgui
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();

		//destroy the rest
		if (original_wndproc) {
			SetWindowLongPtrA(interfaces::hwnd, GWLP_WNDPROC, 
								reinterpret_cast<LONG_PTR>(original_wndproc));
			original_wndproc = nullptr;
		}
	}

	void render() {
		if (!globals::menu_open) {
			return;
		}

		ImGui::Begin("cheat menu");
		ImGui::Text("shawarma goatted");
		ImGui::End();
	}
}


