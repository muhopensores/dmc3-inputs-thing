#pragma once
#include "Mod.hpp"
#include "sdk/ReClass.hpp"

#include "sdk/VoxObj.hpp"

class StyleSwitchFX : public Mod {
public:
  StyleSwitchFX() = default;
  // mod name string for config
  std::string_view get_name() const override { return "StyleSwitchFX"; }
  // called by m_mods->init() you'd want to override this
  std::optional<std::string> on_initialize() override;

  // Override this things if you want to store values in the config file
  void on_config_load(const utility::Config& cfg) override;
  void on_config_save(utility::Config& cfg) override;

  // on_frame() is called every frame regardless whether the gui shows up.
  void on_frame() override;
  // on_draw_ui() is called only when the gui shows up
  // you are in the imgui window here.
  void on_draw_ui() override;
  // on_draw_debug_ui() is called when debug window shows up
  //void on_draw_debug_ui() override;
private:

  // function hook instance for our detour, convinient wrapper 
  // around minhook
  std::unique_ptr<FunctionHook> m_function_hook;

  int m_sound_effect;
  void* m_sound_file_mem;
  unsigned int m_sound_file_mem_size;
  void play_sound();
  void change_texture(int style);
  //VoxObj* //m_vox = nullptr;
  //bool m_3d_audio = false;
};

extern std::mutex g_style_switch_mutex;
void style_switch_efx_clear_textures();
void style_switch_efx_load_textures();
