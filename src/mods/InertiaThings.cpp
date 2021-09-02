#include "InertiaThings.hpp"

static InertiaThings* g_it_ptr;

// TODO(): dt check dmc3se.exe+188CE17

static float chached_velocity = 0.0f;

typedef void (__thiscall* _skystar_shit)(CPlDante*, uint16_t, float);

_skystar_shit oShit;

static void skystar_shit(CPlDante* pl, int a, float t)
{
	oShit = (_skystar_shit)g_it_ptr->m_vel_skystar_hook->get_original();
	oShit(pl, a, t);
	printf("[%s] air velocity: (%f, %f)\n", __FUNCTION__, pl->momentumMagnitude, pl->momentumDelta);
	if (pl->momentumMagnitude < 33.0f) {
		chached_velocity = 45.3f;
	}
	else {
		chached_velocity = pl->momentumMagnitude;
	}
}

static __declspec(naked) void ss_wrap() {
	__asm {
		mov     eax, [esp+0x8]
		push    eax
		mov     eax, [esp+0x8]
		push    eax
		mov     eax, [esp+0x14]
		push    eax
		call    skystar_shit
		mov     eax, [esp+0xC]
		add     esp, 0x14
		mov     [esp], eax

		ret
	}
}
bool InertiaThings::install_inertia_patches_and_hooks(bool state) {
	if (state) {

		m_vel_set_hook = std::make_unique<FunctionHook>(plr_vel_set_fptr(), &plr_veloicty_set_sub_5A6210);
		m_vel_set_zero_hook = std::make_unique<FunctionHook>(plr_vel_set_zero_fptr(), &plr_velocity_zero_sub_5A6230);
		m_vel_lookup_hook = std::make_unique<FunctionHook>(plr_velocity_lookup_table_something(), &plr_velocity_lookup_table_something_sub_5A4CB0);
		m_vel_skystar_hook = std::make_unique<FunctionHook>(plr_sky_star_velocity_fptr(), &ss_wrap);

		bool hooked =
			m_vel_set_hook->create()
			&& m_vel_set_zero_hook->create()
			&& m_vel_lookup_hook->create()
			&& m_vel_skystar_hook->create();

		if (!hooked) { 
			spdlog::error("Failed to create() inertia hooks\n");
			return false;
		}

		toggle_patches(state);

		return true;
	}
	else {

		m_vel_set_hook.reset();
		m_vel_set_zero_hook.reset();
		m_vel_lookup_hook.reset();
		m_vel_skystar_hook.reset();

		//if (!unhooked) { 
			//spdlog::error("Failed to remove() inertia hooks\n");
			//return false;
		//}

		toggle_patches(state);

		return true;
	}
}
std::optional<std::string> InertiaThings::on_initialize() {
	
	g_it_ptr          = this;

	m_ar_rave_patch[0] = Patch::create(0x005B8164, get_forward_angle_patch_bytes(), false);
	m_ar_rave_patch[1] = Patch::create(0x005B82B4, get_forward_angle_patch_bytes(), false);

	m_reb_rave_patches[0] = Patch::create(0x005AF954, get_forward_angle_patch_bytes(), false);
	m_reb_rave_patches[1] = Patch::create(0x005AFAA4, get_forward_angle_patch_bytes(), false);
	m_reb_rave_patches[2] = Patch::create(0x005AFBF4, get_forward_angle_patch_bytes(), false);
	m_reb_rave_patches[3] = Patch::create(0x005AFD44, get_forward_angle_patch_bytes(), false);

	m_guitar_rave_patches[0] = Patch::create(0x005BA194, get_forward_angle_patch_bytes(), false);
	m_guitar_rave_patches[1] = Patch::create(0x005BA2C4, get_forward_angle_patch_bytes(), false);
	m_guitar_rave_patches[2] = Patch::create(0x005BA1A3, get_air_guitar01_patch_bytes(), false);
	m_guitar_rave_patches[3] = Patch::create(0x005BA2D3, get_air_guitar02_patch_bytes(), false);

	return Mod::on_initialize();
}

static void debug_log_anim(const char* function) {
	short cur_anim = Devil3SDK::pl_dante_get_human_anim_table()->current_anim;
	printf("[%s] have this current_anim:(%d)\n", function, cur_anim);
}

// during load
void InertiaThings::on_config_load(const utility::Config &cfg) {
	for (IModValue& option : m_options) {
		option.config_load(cfg);
	}
	install_inertia_patches_and_hooks(m_enable_inertia->value());
}
// during save
void InertiaThings::on_config_save(utility::Config &cfg) {
	for (IModValue& option : m_options) {
		option.config_save(cfg);
	}
}
// do something every frame
//void InertiaThings::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
//void InertiaThings::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
void InertiaThings::on_draw_ui() {

	if (!ImGui::CollapsingHeader(get_name().data())) {
		return;
	}
	if (m_enable_inertia->draw("Enable inertia")) {
		install_inertia_patches_and_hooks(m_enable_inertia->value());
	}

//#ifndef NDEBUG
	ImGui::Checkbox("Show debug stats", &m_stats);
	if (m_stats) {
		ImGui::Text("cached_vel: %f", chached_velocity);
		CPlDante* c_pl_dante = Devil3SDK::get_pl_dante();

		if (c_pl_dante) {
			ImGui::Text("Player data:");
			ImGui::Text("velocity: %f", c_pl_dante->momentumMagnitude);
			ImGui::Text("velocity_delta: %f", c_pl_dante->momentumDelta);
			ImGui::Text("Weight: %f", c_pl_dante->weightMaybe);
			ImGui::Text("position (%.2f, %.2f, %.2f", c_pl_dante->Poistion.x, c_pl_dante->Poistion.y, c_pl_dante->Poistion.z);
			ImGui::Text("delta position (%.2f, %.2f, %.2f", c_pl_dante->DeltaP.x, c_pl_dante->DeltaP.y, c_pl_dante->DeltaP.z);
			ImGui::Text("angle: %d", c_pl_dante->angle);
			ImGui::Text("another direction: %d", c_pl_dante->anotherDirection);
			ImGui::Text("idk direction: %d", c_pl_dante->idkDirection);
			ImGui::Text("someOtherdirection: %d", c_pl_dante->someOtherDirection);

		}
		auto human_atbl = Devil3SDK::pl_dante_get_human_anim_table();
		ImGui::Text("Animation data:");
		ImGui::Text("current_anim: %d", human_atbl->current_anim);
		ImGui::Text("bank_id: %d", human_atbl->bank_id);
		ImGui::Text("motion: %d", human_atbl->motion_id);
		ImGui::Text("current_anim: %d", human_atbl->current_anim);
		ImGui::Text("time: %f", human_atbl->time);
		ImGui::Text("anim_frame: %f", human_atbl->anim_frame);
		ImGui::Text("loop_point: %f", human_atbl->loop_point);

		auto dt1_atbl = Devil3SDK::pl_dante_get_dt1_anim_table();
		ImGui::Text("DT1 Animation data:");
		ImGui::Text("bank_id: %d", dt1_atbl->bank_id);
		ImGui::Text("motion: %d", dt1_atbl->motion_id);
		ImGui::Text("current_anim: %d", dt1_atbl->current_anim);
		ImGui::Text("time: %f", dt1_atbl->time);
		ImGui::Text("anim_frame: %f", dt1_atbl->anim_frame);
		ImGui::Text("loop_point: %f", dt1_atbl->loop_point);

		auto dt2_atbl = Devil3SDK::pl_dante_get_dt2_anim_table();
		ImGui::Text("DT2 Animation data:");
		ImGui::Text("bank_id: %d", dt2_atbl->bank_id);
		ImGui::Text("motion: %d", dt2_atbl->motion_id);
		ImGui::Text("current_anim: %d", dt2_atbl->current_anim);
		ImGui::Text("time: %f", dt2_atbl->time);
		ImGui::Text("anim_frame: %f", dt2_atbl->anim_frame);
		ImGui::Text("loop_point: %f", dt2_atbl->loop_point);
	}
//#endif
}

void InertiaThings::toggle_patches(bool state)
{
	m_ar_rave_patch[0]->toggle(state);
	m_ar_rave_patch[1]->toggle(state);

	m_reb_rave_patches[0]->toggle(state);
	m_reb_rave_patches[1]->toggle(state);
	m_reb_rave_patches[2]->toggle(state);
	m_reb_rave_patches[3]->toggle(state);

	m_guitar_rave_patches[0]->toggle(state);
	m_guitar_rave_patches[1]->toggle(state);
	m_guitar_rave_patches[2]->toggle(state);
	m_guitar_rave_patches[3]->toggle(state);
}

void InertiaThings::plr_overwrite_velocity(CPlDante * p_dante)
{
	float vel = std::max(p_dante->momentumMagnitude, chached_velocity);
	//chached_velocity  = vel;
	chached_velocity *= vel >= 20.0f ? 0.1050f : 0.90f;
	chached_velocity = glm::clamp(chached_velocity, 0.0f, 9.0f);
	p_dante->momentumMagnitude = chached_velocity;
}

void __fastcall InertiaThings::plr_veloicty_set_sub_5A6210_internal(CPlDante * p_this, void* unused, float vel_m, float vel_d)
{
#ifndef NDEBUG
	printf("[%s] setting velocity to (%f, %f)\n", __FUNCTION__, vel_m, vel_d);
	//m_vel_set_hook->get_original<decltype(plr_veloicty_set_sub_5A6210)>()(p_this, vel_m, vel_d);
#endif // DEBUG
	
	typedef void (__thiscall* _set_velocity)(CPlDante*, float, float);
	_set_velocity orig_fn = (_set_velocity)m_vel_set_hook->get_original();
	orig_fn(p_this, vel_m, vel_d);

	if (!Devil3SDK::pl_dante_is_grounded()) { 
		chached_velocity = 0.0f;
		return; 
	}
	
	auto predicate = [](MOTION_ID id) { 
		return Devil3SDK::pl_dante_check_animation_id(id);
	};
	bool check = std::any_of(m_moves_should_conserve.begin(), m_moves_should_conserve.end(), predicate);
	if (check) {
		chached_velocity = p_this->momentumMagnitude;
		return;
	}

#ifndef NDEBUG
	debug_log_anim(__FUNCTION__);
#endif

	check = std::any_of(m_moves_dont_touch.begin(), m_moves_dont_touch.end(), predicate);
	if (check) { return; }

	/*auto predicate = [](MOTION_ID id) { return id == uass.current_anim;	};
	const bool check = std::any_of(m_moves_dont_touch.begin(), m_moves_dont_touch.end(), predicate);
	if (check) { return; }*/

	plr_overwrite_velocity(p_this);
	//float vel = std::max(p_this->momentumMagnitude, chached_velocity);
	//chached_velocity  = vel;
	//chached_velocity *= vel >= 20.0f ? 0.1050f : 0.90f;
	//p_this->momentumMagnitude = chached_velocity;

}

void __fastcall InertiaThings::plr_veloicty_set_sub_5A6210(CPlDante * p_this, void* unused, float vel_m, float vel_d)
{
	return g_it_ptr->plr_veloicty_set_sub_5A6210_internal(p_this, unused, vel_m, vel_d);
}

void __fastcall InertiaThings::plr_velocity_zero_sub_5A6230_internal(CPlDante * p_this)
{
	
#ifndef NDEBUG
	printf("[%s] setting velocity to (0, 0)\n", __FUNCTION__);
	debug_log_anim(__FUNCTION__);
#endif

	m_vel_set_zero_hook->get_original<decltype(plr_velocity_zero_sub_5A6230)>()(p_this);

	/*auto predicate = [](MOTION_ID id) { 
		return (id == uass.current_anim) || (id == uass_prev.current_anim);	
	};*/

	auto predicate = [](MOTION_ID id) { 
		return Devil3SDK::pl_dante_check_animation_id(id);	
	};

	const bool check = std::any_of(m_moves_should_zero.begin(), m_moves_should_zero.end(), predicate);
	if (check) { return; }


	plr_overwrite_velocity(p_this);
	//float vel = std::max(p_this->momentumMagnitude, chached_velocity);
	//chached_velocity  = vel;
	//chached_velocity *= vel >= 20.0f ? 0.1050f : 0.90f;
	//chached_velocity *= 0.9f;
	//p_this->momentumMagnitude = chached_velocity;
}

void __fastcall InertiaThings::plr_velocity_zero_sub_5A6230(CPlDante * p_this)
{
	return g_it_ptr->plr_velocity_zero_sub_5A6230_internal(p_this);
}

void __fastcall InertiaThings::plr_velocity_lookup_table_something_sub_5A4CB0_internal(CPlDante * p_this)
{
	float cache_velocity = 0.0f;
	m_vel_lookup_hook->get_original<decltype(plr_velocity_lookup_table_something_sub_5A4CB0)>()(p_this);
	cache_velocity = std::max(chached_velocity, p_this->momentumMagnitude);
	//p_this->momentumMagnitude *= 2.0;
	//p_this->momentumDelta *= 2.0;
#ifndef NDEBUG
	printf("[%s] setting velocity to (%f, %f)\n", __FUNCTION__, p_this->momentumMagnitude, p_this->momentumDelta);
	debug_log_anim(__FUNCTION__);
#endif

	auto predicate = [](MOTION_ID id) { return Devil3SDK::pl_dante_check_animation_id(id); };
	const bool check = std::any_of(m_moves_should_conserve.begin(), m_moves_should_conserve.end(), predicate);
	if (check) {
		chached_velocity = cache_velocity;
		//p_this->momentumMagnitude = cache_velocity;
	}
	//p_this->momentumMagnitude = chached_velocity;
}

void __fastcall InertiaThings::plr_velocity_lookup_table_something_sub_5A4CB0(CPlDante * p_this)
{
	return g_it_ptr->plr_velocity_lookup_table_something_sub_5A4CB0_internal(p_this);
}
