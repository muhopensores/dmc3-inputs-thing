#pragma once

#include <spdlog/spdlog.h>

class Mods;
class Mod;

#include "D3D9Hook.hpp"
#include "WindowsMessageHook.hpp"
#if 0
#ifdef DINPUT_HOOK
#include "DInputHook.hpp"
#endif
#endif
#include "utility/ChronoAlias.hpp"
#include "imgui.h"

// Global facilitator
class ModFramework {
public:
    ModFramework();
    virtual ~ModFramework();

    bool is_valid() const {
        return m_valid;
    }

    const auto& get_mods() const {
        return m_mods;
    }

    /*const auto& get_types() const {
        return m_types;
    }*/

    const auto& get_keyboard_state() const {
        return m_last_key;
    }
	const auto& get_prompt_font() const {
		return m_prompt_font;
	}
    /*const auto& get_globals() const {
        return m_globals;
    }*/
	const auto& get_d3d9_device() const {
		return m_d3d9_hook->get_device();
	}
    Address get_module() const {
        return m_game_module;
    }

    bool is_ready() const {
        return m_game_data_initialized;
    }

	const bool get_window_focus() const {
		return m_window_focused;
	}

    void on_frame();
    void on_reset();
    bool on_message(HWND wnd, UINT message, WPARAM w_param, LPARAM l_param);
    //void on_direct_input_keys(const std::array<uint8_t, 256>& keys);

    void save_config();

    Mod* m_rr;
private:
    void draw_ui();
    void draw_about();

    bool initialize();
    void create_render_target();
    void cleanup_render_target();

    bool m_first_frame{ true };
    bool m_valid{ false };
    bool m_initialized{ false };
    bool m_draw_ui{ false };
	bool m_draw_cursor{ true };
	bool m_window_focused{ true };
#ifndef NDEBUG
    bool m_draw_debug_ui{ true };
#else
    bool m_draw_debug_ui{ false };
#endif // !NDEBUG

    std::atomic<bool> m_game_data_initialized{ false };

    std::mutex m_input_mutex{};
    
    HWND m_wnd{ 0 };
    HMODULE m_game_module{ 0 };
    //uint8_t m_menu_key{ DIK_INSERT };
	ImFont* m_prompt_font{ 0 }; // NOTE(): needs to be set before imgui::begin calls
	/*int m_mouse_x;
	int m_mouse_y;
	int m_mouse_x_dt;
	int m_mouse_y_dt;
	std::chrono::high_resolution_clock::time_point m_last_mouse_move_timestamp;*/

    //std::array<uint8_t, 256> m_last_keys{ 0 };
	WPARAM m_last_key{ 0 };
    std::unique_ptr<D3D9Hook> m_d3d9_hook{};
#ifdef DINPUT_HOOK
	//std::unique_ptr<DInputHook> m_dinput_hook; // TODO(): disabling this for now
#endif
    std::unique_ptr<WindowsMessageHook> m_windows_message_hook;
    std::shared_ptr<spdlog::logger> m_logger;
    std::string m_error{ "" };

    // Game-specific stuff
    std::unique_ptr<Mods> m_mods;
};

extern std::unique_ptr<ModFramework> g_framework;