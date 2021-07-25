#pragma once
#include "Mod.hpp"
#include "sdk/ReClass.hpp"

struct D3DDevicePtr {
	IDirect3DDevice9* device;
};


class QuicksilverShader : public Mod {
public:
  QuicksilverShader() = default;
  // mod name string for config
  std::string_view get_name() const override { return "QuicksilverShader"; }
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
	enum shader_type {
		DEFAULT,
		SEPIA,
		NEGAMONO,
		DFN,
		GREEN_SCREEN,
		OWN,
		MAX
	};

	int m_current_type = 0;

	std::array<const char*, 5> m_shader_programs = {
		// default shader
		"ps_1_1\n\
		def c0,  1.0000,  1.000,  1.000,  0.000\n\
		tex t0 ; there was some japanese comment here\n\
		mul_x2 r0, v0, t0\n\
		add r0.rgb, c0, -r0", // default shader end
		// sepia shader // TODO(): make it less terrible
		"ps_1_2\n\
		def c0,  0.2999,  0.587,  0.114,  0.000\n\
		def c1,  1.0000,  0.760,  0.600,  0.950\n\
		tex t0\n\
		mov r0, t0\n\
		dp3 r0.rgb, t0, c0\n\
		mul r0.rgb, r0, c1\n\
		mul_x2 r0.rgb, r0, c1.w", // sepia shader end
		// devil3_negamono shader
		"ps_1_1\n\
		def c0,  1.0000,  1.000,  1.000,  0.000\n\
		def c1,  0.299,  0.587,  0.114,  0.000 ; v = 0.299*R + 0.587*G + 0.114*G \n\
		tex t0 \n\
		mul_x2 r0, v0, t0\n\
		add r0.rgb, c0, -r0 \n\
		dp3_sat r0.rgb, c1, r0", // devil3_negamono end
		// deep fried negative shader
		"ps_1_2\n\
		def c0,  0.2999,  0.587,  0.114,  0.000\n\
		def c1,  1.0000,  1.000,  1.000,  0.000\n\
		tex t0\n\
		mul_x2 r0, v0, t0\n\
		add r0.rgb, c1, -r0\n\
		dp3 r0.rgb, r0, c0\n\
		mul r0, r0, r0 ; dont laugh\n\
		mul r0, r0, r0 ; there is no\n\
		mul r0, r0, r0 ; pow() instruction\n\
		mul r0, r0, r0 ; on ps_1_x", // dfn shader end
		// green screen shader
		"ps_1_2\n\
		def c0,  0.000,  1.000,  0.0,  1.000\n\
		mov r0, c0" // green screen shader end
	};

	char m_own_shader[1024];
	char m_error_string[1024];
  // function hook instance for our detour, convinient wrapper 
  // around minhook
	void create_shader_program(shader_type type);
	void create_shader_program(const char* shader);
};