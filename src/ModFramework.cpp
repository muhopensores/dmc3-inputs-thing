#include <spdlog/sinks/basic_file_sink.h>

#include <imgui/imgui.h>

#include <Windowsx.h>


// ours with XInput removed
#include "fw-imgui/imgui_impl_win32.h"
#include "fw-imgui/imgui_impl_dx9.h"

#include "utility/Module.hpp"

#include "Mods.hpp"

#include "LicenseStrings.hpp"
#include "ModFramework.hpp"

#include "Config.hpp"
#include "mods/UIButton.hpp"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

std::unique_ptr<ModFramework> g_framework{};

ModFramework::ModFramework()
    : m_game_module{ GetModuleHandle(0) },
    m_logger{ spdlog::basic_logger_mt("ModFramework", LOG_FILENAME, true) }
{
    spdlog::set_default_logger(m_logger);
    spdlog::flush_on(spdlog::level::info);
    spdlog::info(LOG_ENTRY);

#ifdef DEBUG
    spdlog::set_level(spdlog::level::debug);
#endif
	std::this_thread::sleep_for(std::chrono::seconds(1));

    m_d3d9_hook = std::make_unique<D3D9Hook>();

	m_d3d9_hook->on_reset    ([this](D3D9Hook& hook) { on_reset(); });
	m_d3d9_hook->on_end_scene([this](D3D9Hook& hook) { on_frame(); });

    m_valid = m_d3d9_hook->hook();

    if (m_valid) {
        spdlog::info("Hooked D3D9");
    }
}

ModFramework::~ModFramework() {

}

void SetVisibleCursorWINAPI(bool visible)
{

	CURSORINFO info = { sizeof(CURSORINFO), 0, nullptr, {} };
	if (!GetCursorInfo(&info))
	{
		throw std::system_error(std::error_code(static_cast<int>(GetLastError()), std::system_category()), "GetCursorInfo");
	}

	bool isvisible = (info.flags & CURSOR_SHOWING) != 0;
	if (isvisible != visible)
	{
		ShowCursor(visible);
	}
}

static bool IsCursorVisibleWINAPI() {
	CURSORINFO info = { sizeof(CURSORINFO), 0, nullptr, {} };
	if (!GetCursorInfo(&info))
	{
		throw std::exception("GetCursorInfo");
	}

	return (info.flags & CURSOR_SHOWING) != 0;
}

void ModFramework::on_frame() {
    spdlog::debug("on_frame");

    if (!m_initialized) {
        if (!initialize()) {
            spdlog::error("Failed to initialize ModFramework");
            return;
        }

        spdlog::info("ModFramework initialized");
        m_initialized = true;
        return;
    }

	//SetVisibleCursorWINAPI(m_draw_ui);

	ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    if (m_error.empty() && m_game_data_initialized) {
        m_mods->on_frame();
		m_mods->on_draw_custom_imgui_window();
    }

    draw_ui();

    ImGui::EndFrame();
    ImGui::Render();

	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

    //ID3D11DeviceContext* context = nullptr;
    //m_d3d11_hook->get_device()->GetImmediateContext(&context);

    //context->OMSetRenderTargets(1, &m_main_render_target_view, NULL);

    //ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void ModFramework::on_reset() {
    spdlog::info("Reset!");
	if (!m_initialized) { return; }
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui_ImplDX9_InvalidateDeviceObjects();
	//ImGui_ImplDX9_InvalidateDeviceObjects();
    // Crashes if we don't release it at this point.
    //cleanup_render_target();
    m_initialized = false;
}

// FIXME(): some buttons still go through even when the game window is not in focus
bool ModFramework::on_message(HWND wnd, UINT message, WPARAM w_param, LPARAM l_param) {
    if (!m_initialized) {
        return true;
    }

	if (message == WM_KEYDOWN) {
		
		m_last_key = w_param;

		if (w_param == VK_INSERT) {
			m_draw_ui = !m_draw_ui;
		}
		if (w_param == UIButton::ui_button_get_wparam()) {
			m_draw_ui = !m_draw_ui;
		}
	}
	/*if (message == WM_MOUSEMOVE) {
		m_draw_cursor = true;
		int cur_x = GET_X_LPARAM(l_param);
		int cur_y = GET_Y_LPARAM(l_param);

		m_mouse_x_dt = cur_x - m_mouse_x;
		m_mouse_y_dt = cur_y - m_mouse_y;
		m_mouse_x = cur_x;
		m_mouse_y = cur_y;
		m_last_mouse_move_timestamp = std::chrono::high_resolution_clock::now();
	}*/

    if (m_draw_ui && ImGui_ImplWin32_WndProcHandler(wnd, message, w_param, l_param) != 0) {
        // If the user is interacting with the UI we block the message from going to the game.
        auto& io = ImGui::GetIO();

        if (io.WantCaptureMouse || io.WantCaptureKeyboard || io.WantTextInput) {
            return false;
        }
    }

    return true;
}

// this is unfortunate.
/*void ModFramework::on_direct_input_keys(const std::array<uint8_t, 256>& keys) {
    if (keys[m_menu_key] && m_last_keys[m_menu_key] == 0) {
        std::lock_guard _{ m_input_mutex };
        m_draw_ui = !m_draw_ui;

        // Save the config if we close the UI
        if (!m_draw_ui && m_game_data_initialized) {
            save_config();
        }
    }

    m_last_keys = keys;
}*/

void ModFramework::save_config() {
    spdlog::info("Saving config to file");

    utility::Config cfg{};

    for (auto& mod : m_mods->get_mods()) {
        mod->on_config_save(cfg);
    }

    if (!cfg.save(CONFIG_FILENAME)) {
        spdlog::info("Failed to save config");
        return;
    }

    spdlog::info("Saved config");
}

void ModFramework::draw_ui() {
    //std::lock_guard _{ m_input_mutex };

    if (!m_draw_ui) {
        //m_dinput_hook->acknowledge_input();
        //ImGui::GetIO().MouseDrawCursor = false;
        return;
    }

    auto& io = ImGui::GetIO();


    ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_::ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(410, 500), ImGuiCond_::ImGuiCond_Once);

    ImGui::Begin("ModFramework", &m_draw_ui);

#ifdef GIT_HASH
	ImGui::Text("Version: %s", GIT_HASH);
	ImGui::Text("Date: %s", GIT_DATE);
#endif
    ImGui::Text("Menu Key: Insert");
	
	if (ImGui::Button("Save config")) {
		save_config();
	}
    draw_about();

    if (m_error.empty() && m_game_data_initialized) {
        m_mods->on_draw_ui();
    }
    else if (!m_game_data_initialized) {
        ImGui::TextWrapped("ModFramework is currently initializing...");
    }
    else if(!m_error.empty()) {
        ImGui::TextWrapped("ModFramework error: %s", m_error.c_str());
    }

    ImGui::End();
}

void ModFramework::draw_about() {
    if (!ImGui::CollapsingHeader("About")) {
        return;
    }

    ImGui::TreePush("About");

    ImGui::Text("Authors: Siyan, endneo, cheburrat0r, deepdarkkapustka");
	ImGui::Text("https://github.com/muhopensores/dmc3-inputs-thing/");
	ImGui::NewLine();
    ImGui::Text("Inspired by RE2Framework/Kanan project.");
    ImGui::Text("https://github.com/praydog/RE2-Mod-Framework");
    ImGui::Text("https://github.com/cursey/kanan-new");

    if (ImGui::CollapsingHeader("Licenses")) {
        ImGui::TreePush("Licenses");

        if (ImGui::CollapsingHeader("glm")) {
            ImGui::TextWrapped(license::glm);
        }

        if (ImGui::CollapsingHeader("imgui")) {
            ImGui::TextWrapped(license::imgui);
        }

        if (ImGui::CollapsingHeader("minhook")) {
            ImGui::TextWrapped(license::minhook);
        }

        if (ImGui::CollapsingHeader("spdlog")) {
            ImGui::TextWrapped(license::spdlog);
        }

        ImGui::TreePop();
    }

    ImGui::TreePop();
}

bool ModFramework::initialize() {
    if (m_initialized) {
        return true;
    }

    spdlog::info("Attempting to initialize");

    auto device = m_d3d9_hook->get_device();

    // Wait.
    if (device == nullptr) {
        spdlog::info("Device is null.");
        return false;
    }

	D3DDEVICE_CREATION_PARAMETERS devParams{ 0 };
	auto hr = device->GetCreationParameters(&devParams);
	if (SUCCEEDED(hr)) {
		if (devParams.hFocusWindow) {
			spdlog::info("[D3D Device init] D3DDEVICE_CREATION_PARAMETERS hFocusWindow={0:x}\n", (void*)devParams.hFocusWindow);
			m_wnd = devParams.hFocusWindow;
		}
		else {
			spdlog::info("[D3D Device present] D3DDEVICE_CREATION_PARAMETERS hFocusWindow= is NULL\n");
			return false;
		}
	}

    // Explicitly call destructor first
    m_windows_message_hook.reset();
    m_windows_message_hook = std::make_unique<WindowsMessageHook>(m_wnd);
    m_windows_message_hook->on_message = [this](auto wnd, auto msg, auto wParam, auto lParam) {
        return on_message(wnd, msg, wParam, lParam);
    };

    // just do this instead of rehooking because there's no point.
    if (m_first_frame) {
        m_dinput_hook = std::make_unique<DInputHook>(m_wnd);
    }
    else {
        m_dinput_hook->set_window(m_wnd);
    }

    //spdlog::info("Creating render target");

    //create_render_target();

    spdlog::info("Window Handle: 0x{0:x}", (uintptr_t)m_wnd);
    spdlog::info("Initializing ImGui");

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    spdlog::info("Initializing ImGui Win32");

    if (!ImGui_ImplWin32_Init(m_wnd)) {
        spdlog::error("Failed to initialize ImGui.");
        return false;
    }

    spdlog::info("Initializing ImGui D3D9");

    if (!ImGui_ImplDX9_Init(device)) {
        spdlog::error("Failed to initialize ImGui.");
        return false;
    }

    ImGui::StyleColorsDark();

    if (m_first_frame) {
        m_first_frame = false;

        spdlog::info("Starting game data initialization thread");

        // Game specific initialization stuff
        std::thread init_thread([this]() {
            m_mods = std::make_unique<Mods>();

            auto e = m_mods->on_initialize();

            if (e) {
                if (e->empty()) {
                    m_error = "An unknown error has occurred.";
                }
                else {
                    m_error = *e;
                }
            }

            m_game_data_initialized = true;
        });

        init_thread.detach();
    }

    return true;
}

void ModFramework::create_render_target() {
    /*cleanup_render_target();

    ID3D11Texture2D* back_buffer{ nullptr };
    if (m_d3d11_hook->get_swap_chain()->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&back_buffer) == S_OK) {
        m_d3d11_hook->get_device()->CreateRenderTargetView(back_buffer, NULL, &m_main_render_target_view);
        back_buffer->Release();
    }*/
}

void ModFramework::cleanup_render_target() {
    /*if (m_main_render_target_view != nullptr) {
        m_main_render_target_view->Release();
        m_main_render_target_view = nullptr;
    }*/
}

