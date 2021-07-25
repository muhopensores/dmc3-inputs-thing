#include "StyleSwitchFX.hpp"
#include "utility/Scan.hpp"
#include "utility/Compressed.hpp"
#include "../Sfx.cpp"

static CPlDante* g_char_ptr = nullptr;
static CCameraCtrl* g_cam_ptr = nullptr;

#pragma comment(lib, "Winmm.lib")

// TODO(): ability to change vfx that's played ?
//static int       g_vfx_id;
static int       prev_style;
static int*      current_style = (int*)0xB6B220;
static bool      g_enable_mod;
static bool      g_enable_sound;
static uintptr_t detour_jmpback;

enum DANTE_STYLES {
	SWORDMASTER, GUNSLINGER, TRICKSTER, ROYALGUARD, QUICKSILVER, GERMANWORD, STYLE_MAX
};

constexpr std::array g_style_names{
	"Swordmaster", "Gunslinger", "Trickster", "Royal guard", "Quicksilver", "German word"
};

static std::array<glm::vec4, STYLE_MAX> g_style_colors{
	glm::vec4(0.700f, 0.0f,   0.0f,   1.0f), // swordmaster
	glm::vec4(0.065f, 0.4f,   0.941f, 1.0f), // gunslinger
	glm::vec4(1.0f,   0.706f, 0.0f,   1.0f), // trickster
	glm::vec4(0.0f,   0.786f, 0.0f,   1.0f), // royalguard
	glm::vec4(1.0f,   1.0f,   1.0f,   1.0f), // quicksilver
	glm::vec4(0.941f, 0.065f, 0.658,  1.0f), // german word
};

static void play_effect(int style) {
	typedef int(__cdecl* cEffectBase_sub_67FE80)(char a1, int a2, int *a3, int a4);
	static cEffectBase_sub_67FE80 c_efb_1 = (cEffectBase_sub_67FE80)0x67FE80;

	typedef int (__thiscall* cEffect2_sub_5A49E0)(CPlDante*);
	static cEffect2_sub_5A49E0 c_efb_2 = (cEffect2_sub_5A49E0)0x5A49E0;

	//int*   current_style = (int*)0xB6B220;
	int a1 = c_efb_1(03, 218, 0, 8);
	if (a1) {
		CPlDante* pl = (CPlDante*)0x01C8A600;
		int a2 = c_efb_2(pl);
		int colorama = ImGui::ColorConvertFloat4ToU32(g_style_colors[style]);
		__asm {
			pushad
			mov     esi, [pl]
			mov     edi, [a1]
			mov     eax, [a2]
			mov     edx, [esi + eax * 4 + 12B8h]
			mov     eax, [edx + 0F8h]
			mov     dword ptr[edi + 0A0h], 0
			mov     ecx, [colorama]
			mov     [edi + 0xA8], ecx
			mov     dword ptr[edi + 0xA4], 2

			mov     [edi + 90h], eax
			popad
		}
	}
}
#if 0
// clang-format off
static __declspec(naked) void detour() {
	__asm {

		pushad

		call play_effect

		popad

	originalCode:
		lea     eax, [eax+ecx]
		push    eax
		mov     ecx, [esp+8]

		jmp     qword ptr [detour_jmpback]
	}
}
// clang-format on
#endif

std::optional<std::string> StyleSwitchFX::on_initialize() {
	//HMODULE base = g_framework->get_module().as<HMODULE>();
	/*auto style_switch = utility::scan(0x1000, UINT_MAX - 0x1000 - 4, "8D 04 08 50 8B 4C 24 08 E8 ?? ?? ?? ?? 6A 01 8B 4C 24 04");
	if (style_switch.has_value()) {
		if (!install_hook_absolute(*style_switch, m_function_hook, &detour, &detour_jmpback, 8)) {
			spdlog::error("[{}] failed to initialize", get_name());
			return "Failed to initialize StyleSwitchFX";
		}
	}*/
	g_char_ptr = (CPlDante*)0x1C8A600;
	g_cam_ptr = (CCameraCtrl*)0x01371978;

	m_sound_file_mem = utility::DecompressFileFromMemory(sfx_compressed_data,sfx_compressed_size);

  return Mod::on_initialize();
}

// during load
void StyleSwitchFX::on_config_load(const utility::Config &cfg) {
	g_enable_mod = cfg.get<bool>("StyleSwitchFXenabled").value_or(false);
	g_enable_sound = cfg.get<bool>("StyleSwitchSoundEnabled").value_or(false);
}
// during save
void StyleSwitchFX::on_config_save(utility::Config &cfg) {
	cfg.set<bool>("StyleSwitchFXenabled", g_enable_mod);
	cfg.set<bool>("StyleSwitchSoundEnabled", g_enable_sound);
}

// do something every frame
void StyleSwitchFX::on_frame() {
	if (!g_enable_mod) { return; }
	if ((g_char_ptr->pad_0000) != 0x744D38) { return; }
	if (*current_style != prev_style) {
		play_effect(*current_style);
		if (g_enable_sound) { play_sound(); }
		prev_style = *current_style;
	}
}
// will show up in debug window, dump ImGui widgets you want here
//void StyleSwitchFX::on_draw_debug_ui() {}
void StyleSwitchFX::on_draw_ui() {
	if (!ImGui::CollapsingHeader(get_name().data())) {
		return;
	}

	ImGui::Checkbox("Enable style switch effects", &g_enable_mod);
	
	ImGui::Checkbox("Enable sound effect", &g_enable_sound);
	/*if (g_enable_sound) {
		ImGui::Combo("Sound effect",
			&m_sound_effect,
			"DMC4 Style Switch\0DMC4 Finger snap\0DMC3 Switch port effect\0\0");
	}*/
	
	ImGui::Text("Customize style colors");

	for (int i = 0; i < STYLE_MAX; i++) {
		ImGui::ColorEdit3(g_style_names[i], (float*)&g_style_colors[i]);
	}
}

void StyleSwitchFX::play_sound()
{
	PlaySound((LPCSTR)m_sound_file_mem, NULL, SND_MEMORY | SND_ASYNC);
}
