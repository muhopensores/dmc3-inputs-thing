#include "PracticeMode.hpp"

static kbStruct  kb_struct{};
static kbStruct  kb_struct_dt{};

static bool      g_should_launch;
static char*     g_char_state_ptr = (char*)0x01C8EDDE;
static float     g_multiplier = 10.0f;
static uintptr_t g_dmc3_glob = 0x00B6B155;
static uintptr_t kb_jmp_back;

#define STUN_MAX      60.0f // TODO(): enemies have different max values but
#define KNOCKBACK_MAX 60.0f // i'm too lazy to figure out where to grab that

// clang-format off
static __declspec(naked) void detour() {
	__asm {
		cmp byte ptr [g_should_launch], 1
		jne copyValues

		movss xmm0, dword ptr [esp+0x20]
		mulss xmm0, dword ptr [g_multiplier]
		movss dword ptr [esp+0x20], xmm0
		movss xmm1, dword ptr [eax+0x8]
		subss xmm1, xmm0
		movss dword ptr [eax+0x8], xmm1

		xorps xmm0, xmm0
		xorps xmm1, xmm1

	copyValues:
		// kb delta
		movss xmm0, dword ptr[esp + 0x1C]
		movss dword ptr[kb_struct_dt.stun], xmm0

		movss xmm0, dword ptr[esp + 0x20]
		movss dword ptr[kb_struct_dt.knockback], xmm0

		movss xmm0, dword ptr[esp + 0x24]
		movss dword ptr[kb_struct_dt.health], xmm0
		// actual kb
		movss xmm0, dword ptr [eax+0xC]
		movss dword ptr [kb_struct.stun], xmm0

		movss xmm0, dword ptr [eax+0x8]
		movss dword ptr [kb_struct.knockback], xmm0

		movss xmm0, dword ptr [eax+0x4]
		movss dword ptr [kb_struct.health], xmm0

		xorps xmm0, xmm0
	originalCode:
		mov eax, [g_dmc3_glob]
		lea eax, [eax]
		mov al, [eax]
		jmp dword ptr [kb_jmp_back]
	}
}
// clang-format on

void PracticeMode::custom_imgui_window() {

	if (!m_overlay_enabled->value()) { return; }
	ImGui::Begin("Practice mode overlay", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize);
	// TODO(): idk cant change bg opacity for some reason.
	//ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, m_overlay_transparency->value()));
	ImGui::Text("Enemy STUN:        "); ImGui::SameLine();
	ImGui::SetWindowFontScale(1.7f);
	ImGui::TextColored((ImVec4)ImColor::HSV(glm::smoothstep(0.0f, STUN_MAX, kb_struct.stun) * 0.5f, 1.0f, 1.0f), "%f", kb_struct.stun);
	ImGui::SetWindowFontScale(1.0f);

	ImGui::Text("Enemy DISPLACEMENT:"); ImGui::SameLine();
	ImGui::SetWindowFontScale(1.7f);
	ImGui::TextColored((ImVec4)ImColor::HSV(glm::smoothstep(0.0f, KNOCKBACK_MAX, kb_struct.knockback) * 0.5f, 1.0f, 1.0f), "%f", kb_struct.knockback);
	ImGui::SetWindowFontScale(1.0f);

	ImGui::Text("Enemy HEALTH:      "); ImGui::SameLine();
	ImGui::SetWindowFontScale(1.7f);
	ImGui::TextColored((ImVec4)ImColor::ImColor(10,123,212), "%.2f", kb_struct.health);
	ImGui::SetWindowFontScale(1.0f);

	ImGui::Text("Last move stun value: %f", kb_struct_dt.stun);
	ImGui::Text("Last move displacement value: %f", kb_struct_dt.knockback);
	ImGui::Text("Last move did %.3f damages", kb_struct_dt.health);
	/*ImGui::SetWindowFontScale(0.79f);
	ImGui::TextColored((ImVec4)ImColor::ImColor(147, 147, 147), "You can drag this window");
	ImGui::SetWindowFontScale(1.0f);*/
	//ImGui::PopStyleColor();
    
	// address shows locked on enemy and also soft locked if you're standing near them but aren't locked on
	uintptr_t lockedOnEnemyAddr = 0x01C8E1E4;
	uintptr_t* lockedOnEnemyAddrPtr = *(uintptr_t**)lockedOnEnemyAddr;
    if (lockedOnEnemyAddrPtr) {
		ImGui::Text("Live stats test");
        float& enemyHealth = *(float*)((uintptr_t)lockedOnEnemyAddrPtr + 0x21B0);
        ImGui::InputFloat("Enemy HP", &enemyHealth);
        float& enemyStun = *(float*)((uintptr_t)lockedOnEnemyAddrPtr + 0x21B8);
        ImGui::InputFloat("Enemy Stun", &enemyStun);
        float& enemyDisplacement = *(float*)((uintptr_t)lockedOnEnemyAddrPtr + 0x21B4);
        ImGui::InputFloat("Enemy Displacement", &enemyDisplacement);
	}

	ImGui::End();
}

std::optional<std::string> PracticeMode::on_initialize() {
  // uintptr_t base = g_framework->get_module().as<uintptr_t>();

  if (!install_hook_absolute(0x446C4C, m_function_hook, &detour, &kb_jmp_back, 5)) {
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize PracticeMode";
  }
  return Mod::on_initialize();
}

// during load
void PracticeMode::on_config_load(const utility::Config &cfg) {
	for (IModValue& option : m_options) {
		option.config_load(cfg);
	}
}
// during save
void PracticeMode::on_config_save(utility::Config &cfg) {
	for (IModValue& option : m_options) {
		option.config_save(cfg);
	}
}
// do something every frame
void PracticeMode::on_frame() {
	
	if (!(m_always_launch_tgl->value())) { g_should_launch = 0; return; }
	
	/*bool ground = Devil3SDK::pl_dante_is_grounded();*/
	g_should_launch = !(*g_char_state_ptr == 11 || *g_char_state_ptr == 13);
}
// will show up in debug window, dump ImGui widgets you want here
//void PracticeMode::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
void PracticeMode::on_draw_ui() {
	if (!ImGui::CollapsingHeader(get_name().data())) {
		return;
	}
	m_always_launch_tgl->draw("DMD always launch when on ground"); ImGui::SameLine();
	ShowHelpMarker("Always launch enemies when you are standing on the ground to practice air DMD combos");
	m_overlay_enabled->draw("Enable practice overlay"); ImGui::SameLine();
	ShowHelpMarker("Move overlay window into your prefered location by dragging");
}

/*
01C8A600 = player base (static)

+8C rotation



*/

/*
alternatively detour at 005710C0, it accesses hp for locked on enemy at +21B0
*/
