#pragma once
#include "Mod.hpp"
#include "sdk/ReClass.hpp"

class PracticeMode : public Mod {
public:
  PracticeMode() = default;
  // mod name string for config
  std::string_view get_name() const override { return "PracticeMode"; }
  // called by m_mods->init() you'd want to override this
  std::optional<std::string> on_initialize() override;

  // Override this things if you want to store values in the config file
  void on_config_load(const utility::Config& cfg) override;
  void on_config_save(utility::Config& cfg) override;


  void custom_imgui_window();
  void log_window();
  // on_frame() is called every frame regardless whether the gui shows up.
  void on_frame() override;
  // on_draw_ui() is called only when the gui shows up
  // you are in the imgui window here.
  void on_draw_ui() override;
  // on_draw_debug_ui() is called when debug window shows up
  //void on_draw_debug_ui() override;
private:
	const ModToggle::Ptr m_overlay_enabled{ ModToggle::create(generate_name("PracticeModeOverlay")) };
	const ModToggle::Ptr m_always_launch_tgl{ ModToggle::create(generate_name("AlwaysLaunch")) };
	const ModToggle::Ptr m_stun_log{ ModToggle::create(generate_name("StunLog")) };
	//const ModSlider::Ptr m_overlay_transparency{ ModSlider::create(generate_name("Overlay transparency"),0.0f, 1.0f, 0.5f) };
  
	// function hook instance for our detour, convinient wrapper 
	// around minhook
    std::unique_ptr<FunctionHook> m_function_hook;

  ValueList m_options{
	  *m_overlay_enabled,
	  *m_always_launch_tgl,
	  *m_stun_log,
	  //*m_overlay_transparency
  };
};