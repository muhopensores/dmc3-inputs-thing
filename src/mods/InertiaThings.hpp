#pragma once
#include "Mod.hpp"
#include "sdk/ReClass.hpp"

#include "utility/Patch.hpp"

class InertiaThings : public Mod {
public:
  InertiaThings() = default;
  // mod name string for config
  std::string_view get_name() const override { return "InertiaThings"; }
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
  //static void __stdcall hk_sky_star_velocity_sub_5A5EC0(CPlDante* p_this, uint16_t angle, float vec4_tweak);
  std::unique_ptr<FunctionHook> m_vel_skystar_hook;

  bool install_inertia_patches_and_hooks(bool hook);

  enum MOTION_ID : uint16_t {

	  WALLRUN_END_N = 1559,
	  WALLRUN_END_R = 2071,
	  WALLRUN_END_L = 2583,


	  BEO_VOLCANO_1 = 529,
	  BEO_VOLCANO_2 = 273,

	  DIVEKICK     = 1037,
	  NEUTRAL_JUMP = 1024,
	  FORWARD_JUMP = 1280,
	  STINGER_JUMP = 3584,

	  REB_HELMBREAKER = 2563,
	  REB_HIGHTIME_1 = 2819,
	  REB_HIGHTIME_2 = 3331,


	  SKYSTAR = 2839,

	  BEO_DIVEKICK_1 = 3079,
	  BEO_DIVEKICK_2 = 3335,

	  BEO_LAUNCH_1 = 3847,
	  BEO_LAUNCH_2 = 4103,
	  BEO_LAUNCH_3 = 4359,

	  BEO_RI_1 = 785,
	  BEO_RI_2 = 1809,

	  BEO_STINGER_1 = 7687,
	  BEO_STINGER_2 = 7943,
	  BEO_STINGER_3 = 14087,
	  BEO_STINGER_4 = 14599, // charges
	  BEO_STINGER_5 = 7943,
	  BEO_STINGER_6 = 8455,
	  BEO_STINGER_7 = 14343,

	  NEV_AIRRAID_NEUTRAL = 13318,
	  NEV_AIRRAID_BACK = 13830,
	  NEV_AIRRAID_FORWARD = 13574,
	  NEV_AIRRAID_RIGHT = 14342,
	  NEV_AIRRAID_LEFT = 14086,
	  NEV_AIRRAID_TURN = 13574,
	  NEV_AIRRAID_FLY = 14598,
	  
	  NEV_AIRRAID_SPIN_1 = 6406,
	  NEV_AIRRAID_SPIN_2 = 6662,
	  NEV_AIRRAID_SPIN_3 = 6918,

	  NEV_AIRRAID_START = 15110,
	  
	  ANR_LAUNCH_1 = 4613,
	  ANR_LAUNCH_2 = 4869,

	  TRICK_GROUND = 3863,
	  TRICK_AIR = 4119,

	  AIR_HIKE = 4352,
	  JC = 8448,
  };

private:
	bool m_hooked = false;
	bool m_stats = false;
	const ModToggle::Ptr m_enable_inertia{ ModToggle::create(generate_name("Inertia")) };

	void toggle_patches(bool state);

	constexpr uintptr_t plr_vel_set_fptr() { return (uintptr_t)0x5A6210; };
	constexpr uintptr_t plr_vel_set_zero_fptr() { return (uintptr_t)0x5A6230; };
	constexpr uintptr_t plr_velocity_lookup_table_something() { return (uintptr_t)0x5A4CB0; };
	constexpr uintptr_t plr_sky_star_velocity_fptr() { return (uintptr_t)0x5A5EC0; };
	constexpr uintptr_t plr_anim_lookup_fptr() { return (uintptr_t)0x41AC62; };
	
	const std::vector<int16_t> get_forward_angle_patch_bytes() { 
	    // mov dx, [esi+284C]
		return { 0x66, 0x8B, 0x96, 0x4C, 0x28, 0x00, 0x00 }; 
	};

	const std::vector<int16_t> get_air_guitar01_patch_bytes() {
		return { 0xE8, 0x48, 0xAB, 0xFE, 0xFF };
	};
	const std::vector<int16_t> get_air_guitar02_patch_bytes() {
		return { 0xE8, 0x18, 0xAA, 0xFE, 0xFF };
	};


	std::array<std::unique_ptr<Patch>, 4> m_ar_rave_patch;
	std::array<std::unique_ptr<Patch>, 4> m_reb_rave_patches;
	std::array<std::unique_ptr<Patch>, 4> m_guitar_rave_patches;

	const std::array<MOTION_ID, 7 > m_moves_should_conserve = {
		AIR_HIKE, JC, BEO_DIVEKICK_1, NEV_AIRRAID_SPIN_2, 
		WALLRUN_END_L, WALLRUN_END_R, WALLRUN_END_N
	};

	const std::array<MOTION_ID, 12> m_moves_should_zero = {
		REB_HELMBREAKER, REB_HIGHTIME_1, REB_HIGHTIME_2,
		BEO_VOLCANO_1, BEO_VOLCANO_2, BEO_LAUNCH_1, 
		BEO_LAUNCH_2, BEO_LAUNCH_3, BEO_RI_1, BEO_RI_2,
		ANR_LAUNCH_1, ANR_LAUNCH_2 };

	const std::array<MOTION_ID, 24> m_moves_dont_touch = {
		BEO_DIVEKICK_1, BEO_DIVEKICK_2, TRICK_AIR,
		TRICK_GROUND, BEO_STINGER_1, BEO_STINGER_2,
		BEO_STINGER_3, NEV_AIRRAID_NEUTRAL,
		NEV_AIRRAID_BACK, NEV_AIRRAID_FORWARD,
		NEV_AIRRAID_RIGHT, NEV_AIRRAID_LEFT,
		NEV_AIRRAID_TURN, NEV_AIRRAID_FLY,
		NEV_AIRRAID_SPIN_1, NEV_AIRRAID_SPIN_2,
		NEV_AIRRAID_SPIN_3, NEV_AIRRAID_START, 
		STINGER_JUMP, DIVEKICK, BEO_STINGER_4,
		BEO_STINGER_5, BEO_STINGER_6
	};

	ValueList m_options{
		*m_enable_inertia
	};

protected:

	static void plr_overwrite_velocity(CPlDante* p_dante);

	//void __thiscall VelocitySet_sub_5A6210(CPlDante *this, float a2, float a3)
	// NOTE(): we cant define function prototype as __thiscall
	// so we'll pretend it's fastcall, seems to work
	void __fastcall plr_veloicty_set_sub_5A6210_internal(CPlDante* p_this, void* unused, float vel_m, float vel_d);
	static void __fastcall plr_veloicty_set_sub_5A6210(CPlDante* p_this, void* unused, float vel_m, float vel_d);

	//void __thiscall VelocitySetZero_sub_5A6230(CPlDante *this)
	void __fastcall plr_velocity_zero_sub_5A6230_internal(CPlDante* p_this);
	static void __fastcall plr_velocity_zero_sub_5A6230(CPlDante* p_this);

	//int __thiscall VelocityLookupTableSomething_sub_5A4CB0(CPlDante *this)
	void __fastcall plr_velocity_lookup_table_something_sub_5A4CB0_internal(CPlDante* p_this);
	static void __fastcall  plr_velocity_lookup_table_something_sub_5A4CB0(CPlDante* p_this);

	//void __thiscall skyStarVelocity_sub_5A5EC0(CPlDante *pl, int a2, float a3)

	std::unique_ptr<FunctionHook> m_vel_set_hook;
	std::unique_ptr<FunctionHook> m_vel_set_zero_hook;
	std::unique_ptr<FunctionHook> m_vel_lookup_hook;

	std::unique_ptr<FunctionHook> m_anim_lookup_hook;

};