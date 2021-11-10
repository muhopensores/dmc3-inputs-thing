#pragma once
#include "Mod.hpp"
#include "sdk/ReClass.hpp"
class AudioStutterFix : public Mod {
public:
  AudioStutterFix() = default;
  // mod name string for config
  std::string_view get_name() const override { return "AudioStutterFix"; }
  // called by m_mods->init() you'd want to override this
  std::optional<std::string> on_initialize() override;

  // Override this things if you want to store values in the config file
  void on_config_load(const utility::Config& cfg) override;
  void on_config_save(utility::Config& cfg) override;

  // on_frame() is called every frame regardless whether the gui shows up.
  //void on_frame() override;
  // on_draw_ui() is called only when the gui shows up
  // you are in the imgui window here.
  void on_draw_ui() override;
  // on_draw_debug_ui() is called when debug window shows up
  //void on_draw_debug_ui() override;
	static int __cdecl SND_filename_prepare_sub_404950(int a1, const char *filename);
	static void __cdecl sub_404A70(int a1, char a3, signed int a4, void* a5/*, void* a6*/);
    std::unique_ptr<FunctionHook> m_function_hook3;
    std::unique_ptr<FunctionHook> m_function_hook_lots_snd_shit;
protected:
	void __cdecl sub_404A70_internal(int a1, char a3, signed int a4, void* a5/*, void* a6*/);
	int __cdecl SND_filename_prepare_sub_404950_internal(int a1, const char *filename);
private:
	bool m_hooked1 = false;
	bool m_hooked2 = false;
	const ModToggle::Ptr m_audio_fix_enable{ ModToggle::create(generate_name("m_audio_fix_enable")) };

	ValueList m_options{
		*m_audio_fix_enable
	};

    std::unique_ptr<FunctionHook> m_function_hook;
    std::unique_ptr<FunctionHook> m_function_hook2;
    std::unique_ptr<FunctionHook> m_function_hook4;
    std::unique_ptr<FunctionHook> m_function_hook5;
	constexpr uintptr_t get_snd_start_thread_ptr() { return 0x00404950; };
	constexpr uintptr_t get_snd_whatever_ptr() { return 0x00404A70; };

	char m_error_txt_buf[1024]{ 0 };

	void toggle(bool enable);
	
  // function hook instance for our detour, convinient wrapper 
  // around minhook
};