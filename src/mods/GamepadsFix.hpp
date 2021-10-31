#if DINPUT_HOOK
#pragma once
#include "Mod.hpp"
#include "sdk/ReClass.hpp"

class GamepadsFix : public Mod {
public:
	GamepadsFix() = default;
  // mod name string for config
  std::string_view get_name() const override { return "GamepadFixes"; }
  // called by m_mods->init() you'd want to override this
  std::optional<std::string> on_initialize() override;

  // Override this things if you want to store values in the config file
  //void on_config_load(const utility::Config& cfg) override;
  //void on_config_save(utility::Config& cfg) override;

  // on_frame() is called every frame regardless whether the gui shows up.
  void on_frame() override;
  // on_draw_ui() is called only when the gui shows up
  // you are in the imgui window here.
  void on_draw_ui() override;
  // on_draw_debug_ui() is called when debug window shows up
  //void on_draw_debug_ui() override;

  bool     m_focused       = true;
  char     pad[3]          { 0 };
  int      m_gamepad_index = 0;
  uint32_t m_time          = 0;

protected:
	//int __cdecl Dinput8Create_sub_404BB0_internal(HWND hWnd);
	static int _cdecl Dinput8Create_sub_404BB0(HWND hWnd);
private:
	constexpr uintptr_t dinput8_create_sub_address() { return 0x404BB0; };
	// TODO(): maybe vibration support?
	constexpr uintptr_t gamepad_vibration_plr_address() { return 0x006C0F19; };
  std::unique_ptr<FunctionHook> m_dinput8_create_hook;

  bool m_led_animation = false;
  //std::unique_ptr<FunctionHook> m_function_hook;

};
#endif