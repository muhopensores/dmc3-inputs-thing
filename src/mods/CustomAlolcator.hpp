#pragma once
#include "Mod.hpp"
#include "sdk/ReClass.hpp"
class CustomAlolcator : public Mod {
public:
  CustomAlolcator() = default;
  // mod name string for config
  std::string_view get_name() const override { return "CustomAlolcator"; }
  // called by m_mods->init() you'd want to override this
  std::optional<std::string> on_initialize() override;

  // Override this things if you want to store values in the config file
  //void on_config_load(const utility::Config& cfg) override;
  //void on_config_save(utility::Config& cfg) override;

  // on_frame() is called every frame regardless whether the gui shows up.
  //void on_frame() override;
  // on_draw_ui() is called only when the gui shows up
  // you are in the imgui window here.
  void on_draw_ui() override;
  // on_draw_debug_ui() is called when debug window shows up
  //void on_draw_debug_ui() override;
  bool mem_patch_applied{ false };

protected:
	uintptr_t __fastcall sub_6D4580_internal(uintptr_t p_this, uintptr_t a2, uintptr_t a3);
	static uintptr_t __fastcall sub_6D4580(uintptr_t p_this, uintptr_t a2, uintptr_t a3);
private:
	std::unique_ptr<FunctionHook> m_alloc_hook;
  // function hook instance for our detour, convinient wrapper 
  // around minhook
  // std::unique_ptr<FunctionHook> m_function_hook;
};

extern CustomAlolcator* g_custom_alolcator; // NOTE(): WARNING WARNING big global NOT static i repeat NOT STATIC