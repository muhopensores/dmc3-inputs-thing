#pragma once
#include "Mod.hpp"
#include "sdk/ReClass.hpp"
#include "utility/Patch.hpp"
class NoHeightRestriction : public Mod {
public:
  NoHeightRestriction() = default;
  // mod name string for config
  std::string_view get_name() const override { return "NoHeightRestriction"; }
  // called by m_mods->init() you'd want to override this
  std::optional<std::string> on_initialize() override;
  
  // Override this things if you want to store values in the config file
  void on_config_load(const utility::Config& cfg) override;
  void on_config_save(utility::Config& cfg) override;

  // on_frame() is called every frame regardless whether the gui shows up.
  // void on_frame() override;
  // on_draw_ui() is called only when the gui shows up
  // you are in the imgui window here.
  void on_draw_ui() override;
  // on_draw_debug_ui() is called when debug window shows up
  // void on_draw_debug_ui() override;
  
private:
  // function hook instance for our detour, convinient wrapper
  // around minhook
  // std::unique_ptr<FunctionHook> m_function_hook;
  static uintptr_t jmp_ret;
  static bool modEnabled;
  void toggle(bool enable);
  std::unique_ptr<Patch> noHeightRestrictionPatch1;
  std::unique_ptr<Patch> noHeightRestrictionPatch2;
  std::unique_ptr<Patch> noHeightRestrictionPatch3;
};