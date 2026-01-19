#include <Windows.h>
#include <dwmapi.h>
#include <d3d11.h>
#include <stdexcept>
#include <thread>

#include <format>
#include <iostream>

#include <sstream>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_dx11.h>
#include <imgui/imgui_impl_win32.h>

#include "core/driver_communication.hpp"

#include "features/features_manager.hpp"
#include "features/bhop.hpp"
#include "features/esp.hpp"

#include "globals.hpp"
#include <windowsx.h>


extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


struct feature_threads {
	HANDLE* bhop_thread{ nullptr };
};


bool ForegroundIsTarget(HWND targetWnd) {
	HWND fg = GetForegroundWindow();
	if (!fg) return false;
	// simple compare by HWND:
	if (fg == targetWnd) return true;

	// or compare by process id:
	DWORD fgPID = 0; GetWindowThreadProcessId(fg, &fgPID);
	DWORD targetPID = 0; GetWindowThreadProcessId(targetWnd, &targetPID);
	return fgPID != 0 && fgPID == targetPID;
}

LRESULT CALLBACK window_procedure(HWND window, UINT message, WPARAM w_param, LPARAM l_param) {
	//if (message == WM_KEYDOWN && w_param == VK_INSERT) {
	//	globals::menu_open = !globals::menu_open;  // toggle menu
	//}

	if (ImGui_ImplWin32_WndProcHandler(window, message, w_param, l_param)) {
		return 0L;
	}

	//if (message == WM_DESTROY) {
	if (message == WM_DESTROY || (message == WM_KEYDOWN && w_param == VK_END)) {
		PostQuitMessage(0);
		return 0L;
	}

	switch (message)
	{
		case WM_NCHITTEST:
		{
			const LONG bordderWidth = GetSystemMetrics(SM_CXSIZEFRAME);
			const LONG titleBarHeight = GetSystemMetrics(SM_CYCAPTION);
			POINT cursorPos = { GET_X_LPARAM(w_param), GET_Y_LPARAM(l_param) };
			RECT windowRect;
			GetWindowRect(window, &windowRect);

			if (cursorPos.y >= windowRect.top && cursorPos.y < windowRect.top + titleBarHeight)
				return HTCAPTION;
			break;
		}
	default:
		break;
	}

	return DefWindowProc(window, message, w_param, l_param);
}

INT APIENTRY WinMain(HINSTANCE instance, HINSTANCE, PSTR, INT cmd_show) {
	// initialize driver stuff
	driver_communication::initialize_driver_communication();

	// create window class and whatever
	WNDCLASSEXW wc{};
	wc.cbSize = sizeof(WNDCLASSEXW);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = window_procedure;
	wc.hInstance = instance;
	wc.lpszClassName = L"External Overlay Class";
	RegisterClassExW(&wc);


	HWND window = CreateWindowExW(WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT,
									wc.lpszClassName, L"External Overlay", WS_POPUP,
								    0, 0, globals::screenWidth, globals::screenHeight,
									nullptr, nullptr, wc.hInstance, nullptr);

	SetLayeredWindowAttributes(window, RGB(0, 0, 0), BYTE(255), LWA_ALPHA);

	{
		RECT client_area{};
		GetClientRect(window, &client_area);

		RECT window_area{};
		GetWindowRect(window, &window_area);

		POINT diff{};
		ClientToScreen(window, &diff);

		const MARGINS margins{
			client_area.left + (diff.x - window_area.left),
			client_area.top + (diff.y - window_area.top),
			client_area.right,
			client_area.bottom
		};

		DwmExtendFrameIntoClientArea(window, &margins);
	}

	DXGI_SWAP_CHAIN_DESC sd{};
	sd.BufferDesc.RefreshRate.Numerator = 60U; // fps
	sd.BufferDesc.RefreshRate.Denominator = 1U;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.SampleDesc.Count = 1U;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 2U;
	sd.OutputWindow = window;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	constexpr D3D_FEATURE_LEVEL levels[2] = {
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_0,
	};

	ID3D11Device* device{ nullptr };
	ID3D11DeviceContext* device_context{ nullptr };
	IDXGISwapChain* swap_chain{ nullptr };
	ID3D11RenderTargetView* render_target_view{ nullptr };
	D3D_FEATURE_LEVEL level{};

	// create our device and shit
	D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0U,
		levels,
		2U,
		D3D11_SDK_VERSION,
		&sd,
		&swap_chain,
		&device,
		&level,
		&device_context
	);

	ID3D11Texture2D* back_buffer{ nullptr };
	swap_chain->GetBuffer(0U, IID_PPV_ARGS(&back_buffer));

	if (back_buffer) {
		device->CreateRenderTargetView(back_buffer, nullptr, &render_target_view);
		back_buffer->Release();
	}
	else {
		return 1;
	}

	ShowWindow(window, cmd_show);
	UpdateWindow(window);


	// imgui menu
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	// idk if we still need this

	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX11_Init(device, device_context);

	// start feature threads manager
	features_manager::start();

	const int HOTKEY_ID_INSERT = 1;
	if (!RegisterHotKey(nullptr, HOTKEY_ID_INSERT, 0, VK_INSERT)) {
		MessageBox(nullptr, L"failed to register hotkey", L"Error", MB_ICONERROR);
		throw std::runtime_error("failed to register hotkey");
	}

	bool running = true;
	bool showed_msg = false;

	bool drew = false;

	while (running) {
		HWND targetWnd = FindWindowW(NULL, L"Counter-Strike 2");  // window class | window title
		bool cs2_window_is_foreground = ForegroundIsTarget(targetWnd) || ForegroundIsTarget(window);
		if (targetWnd == NULL) {
			if (!showed_msg) {
				showed_msg = true;
				MessageBox(nullptr, L"Counter-Strike 2 is not running", NULL, MB_ICONINFORMATION);
			}
		} else {
			showed_msg = false;
		}

		MSG msg{};
		while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			//if (msg.message == WM_QUIT) {
			if (msg.message == WM_KEYDOWN && msg.wParam == VK_END) {
				running = false;
			}

			if (cs2_window_is_foreground && msg.message == WM_HOTKEY && msg.wParam == HOTKEY_ID_INSERT) {
				globals::menu_open = !globals::menu_open;
			}
		}

		if (!running) {
			break;
		}


		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		LONG_PTR ex_style = GetWindowLongPtrW(window, GWL_EXSTYLE);
		if (cs2_window_is_foreground && globals::menu_open) {
			SetWindowLongPtrW(window, GWL_EXSTYLE, ex_style & ~WS_EX_TRANSPARENT);  // comment out when debugging

			// Cheat Logic and drawing here
			//ImGui::GetBackgroundDrawList()->AddCircleFilled({ 500, 500 }, 10.f, ImColor(1.f, 0.f, 0.f));

			// MENU HERE
			ImGui::Begin("~ banana-cs2 by .zhk_ ~");
			ImGui::Text("Menu Toggle: key.INSERT");
			ImGui::Text("Exit		: key.END");

			ImGui::Dummy(ImVec2(0.0f, 20.0f));  // 20px space
			
			//ImGui::Checkbox("Bunny Hop", &globals::bhop_enabled);
			//ImGui::Checkbox("Glow ESP", &globals::glow_enabled);
			ImGui::Checkbox("Skeleton ESP", &globals::show_skeleton_esp);
			/*int* selected = &esp::glow_config::glow_type;
			const char* items[] = { "never", "when being used", "when being looked at", "constantly" };
			const int itemCount = IM_ARRAYSIZE(items);
			if (ImGui::BeginCombo("Select Number", items[*selected]))
			{
				for (int i = 0; i < itemCount; i++)
				{
					bool isSelected = (*selected == i);
					if (ImGui::Selectable(items[i], isSelected))
						*selected = i;

					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}*/

			ImGui::Dummy(ImVec2(0.0f, 20.0f));  // 20px space
			std::string info = std::format("Handle: {} | PID: {}", driver_communication::get_driver_handle(), driver_communication::get_target_pid());
			ImGui::Text("%s", info.c_str());

			//ImGui::Dummy(ImVec2(0.0f, 10.0f));  // 20px space
			//std::string num_ge = std::format("Number of Glowing Entities: {}", esp::number_of_glowing_entities);
			//ImGui::Text("%s", num_ge.c_str());

			ImGui::End();
		} else {
			SetWindowLongPtrW(window, GWL_EXSTYLE, ex_style | WS_EX_TRANSPARENT);
		}

		esp::draw_esp(&drew);

		// rendering here
		ImGui::Render();

		constexpr float color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		device_context->OMSetRenderTargets(1U, &render_target_view, nullptr);
		device_context->ClearRenderTargetView(render_target_view, color);

		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		// Present(1U, 0U) renders with vsync |> for vsync off, use Present(0, 0)
		// rendering with vsync off can cause tearing, but overlay will be more responsive
		swap_chain->Present(0U, 0U);
	}
	// cleanup
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	// driver cleanup
	driver_communication::shutdown_driver_communication();

	if (swap_chain) {
		swap_chain->Release();
	}

	if (device_context) {
		device_context->Release();
	}

	if (device) {
		device->Release();
	}

	if (render_target_view) {
		render_target_view->Release();
	}

	DestroyWindow(window);
	UnregisterClassW(wc.lpszClassName, wc.hInstance);

	return 0;
}