#include "StyleSwitchFX.hpp"
#include "utility/Scan.hpp"
#include "utility/Compressed.hpp"
#include "../Sfx.cpp"

static CPlDante* g_char_ptr = nullptr;
static cCameraCtrl* g_cam_ptr = nullptr;

#pragma comment(lib, "Winmm.lib")

#define SND_CREATE_VOX(name) VoxObj* name()
typedef SND_CREATE_VOX(snd_create_vox);
SND_CREATE_VOX(SndCreateVoxStub) {
	return nullptr;
}
static snd_create_vox* SndCreateVox_ = SndCreateVoxStub;
#define SndCreateVox SndCreateVox_

// TODO(): ability to change vfx that's played ?
static int       g_vfx_id = 218;
static int       g_vfx_bank = 3;
static int       g_vfx_a3 = 8;
static int       prev_style;
static int*      current_style = (int*)0xB6B220;
static bool      g_enable_mod;
static bool      g_enable_sound;
static uintptr_t detour_jmpback;

enum DANTE_STYLES {
	SWORDMASTER, GUNSLINGER, TRICKSTER, ROYALGUARD, QUICKSILVER, GERMANWORD, STYLE_MAX
};

constexpr std::array g_style_names {
	"Swordmaster", "Gunslinger", "Trickster", "Royal guard", "Quicksilver", "German word"
};

static std::array<glm::vec4, STYLE_MAX> g_default_colors{
	glm::vec4(0.700f, 0.0f,   0.0f,   1.0f), // swordmaster
	glm::vec4(0.065f, 0.4f,   0.941f, 1.0f), // gunslinger
	glm::vec4(1.0f,   0.706f, 0.0f,   1.0f), // trickster
	glm::vec4(0.0f,   0.786f, 0.0f,   1.0f), // royalguard
	glm::vec4(1.0f,   1.0f,   1.0f,   1.0f), // quicksilver
	glm::vec4(0.941f, 0.065f, 0.658,  1.0f), // german word
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
	int a1 = c_efb_1(g_vfx_bank, g_vfx_id, 0, g_vfx_a3);
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
	// TODOOOOO(important): add soloud audio
	g_char_ptr = (CPlDante*)0x1C8A600;
	//g_cam_ptr = (CCameraCtrl*)0x01371978;

	m_sound_file_mem = utility::DecompressFileFromMemory(sfx_compressed_data,sfx_compressed_size);
	
	HMODULE snd = GetModuleHandle("snd.drv");
	if (!snd) {
		spdlog::info("[StyleSwitchFX]: snd.drv not found\n");
		printf("[StyleSwitchFX]: snd.drv not found\n");
		return Mod::on_initialize();
	}
	FARPROC snd_proc = GetProcAddress(snd, "IsSndDrvSexy");
	if (!snd_proc) {
		spdlog::info("[StyleSwitchFX]: not using custom snd.drv\n");
		printf("[StyleSwitchFX]: not using custom snd.drv\n");
		return Mod::on_initialize();
	}

	SndCreateVox = (snd_create_vox *)GetProcAddress(snd, "CreateVox");
	if (!SndCreateVox) {
		spdlog::info("[StyleSwitchFX]: could not GetProcAddress of CreateVox. this should not happen\n");
		printf("[StyleSwitchFX]: could not GetProcAddress of CreateVox. this should not happen\n");
		return Mod::on_initialize();
	}
	m_vox = SndCreateVox();
	if (m_vox) {
		spdlog::info("Got VoxObj from snd.drv! Nice\n");
		printf("Got VoxObj from snd.drv! Nice\n");
	}
	auto decompressed= utility::DecompressFileFromMemoryWithSize(sfx_compressed_data,sfx_compressed_size);
	m_sound_file_mem = std::get<0>(decompressed);
	m_sound_file_mem_size = std::get<1>(decompressed);
	m_vox->load_mem((unsigned char*)m_sound_file_mem, m_sound_file_mem_size);
	m_vox->set_volume(1.0f);
  return Mod::on_initialize();
}

// during load
void StyleSwitchFX::on_config_load(const utility::Config &cfg) {
	g_enable_mod = cfg.get<bool>("StyleSwitchFXenabled").value_or(false);
	g_enable_sound = cfg.get<bool>("StyleSwitchSoundEnabled").value_or(false);
	
	for (int i = 0; i < DANTE_STYLES::STYLE_MAX; i++) {
		g_style_colors[i] = cfg.get<glm::vec4>(g_style_names[i]).value_or(g_default_colors[i]);
	}
}
// during save
void StyleSwitchFX::on_config_save(utility::Config &cfg) {
	cfg.set<bool>("StyleSwitchFXenabled", g_enable_mod);
	cfg.set<bool>("StyleSwitchSoundEnabled", g_enable_sound);

	for (int i = 0; i < DANTE_STYLES::STYLE_MAX; i++) {
		cfg.set<glm::vec4>(g_style_names[i], g_style_colors[i]);
	}
}

// do something every frame
void StyleSwitchFX::on_frame() {
	if (!g_enable_mod) { return; }
	if ((g_char_ptr->pad_0000) != 0x744D38) { return; }
	if (*current_style != prev_style) {
		play_effect(*current_style);
		if (g_enable_sound) { 
			play_sound();
		}
		prev_style = *current_style;
	}
}
// will show up in debug window, dump ImGui widgets you want here
//void StyleSwitchFX::on_draw_debug_ui() {}
void StyleSwitchFX::on_draw_ui() {
	if (!ImGui::CollapsingHeader(get_name().data())) {
		return;
	}
	ImGui::InputInt("g_vfx_id",   &g_vfx_id);
	ImGui::InputInt("g_vfx_bank", &g_vfx_bank);
	ImGui::InputInt("g_vfx_a3", &g_vfx_a3);

	ImGui::Checkbox("Enable style switch effects", &g_enable_mod);
	
	ImGui::Checkbox("Enable sound effect", &g_enable_sound);
	if (m_vox != nullptr) {
		static float snd_volume = 1.0f;
		if (ImGui::DragFloat("Audio volume", &snd_volume,0.1f,0.0f,10.0f)) {
			snd_volume = glm::clamp(snd_volume, 0.0f, 10.0f);
			m_vox->set_volume(snd_volume);
		}
		ImGui::Checkbox("3D audio effects", &m_3d_audio);
	}
	
	ImGui::Text("Customize style colors");

	for (int i = 0; i < STYLE_MAX; i++) {
		ImGui::ColorEdit3(g_style_names[i], (float*)&g_style_colors[i]);
	}
}

void StyleSwitchFX::play_sound()
{
	if (m_vox) {
		cCameraCtrl* camera = Devil3SDK::get_cam_ctrl();
		if (!camera || camera == (cCameraCtrl*)-1) { return; };
		g_cam_ptr = camera;
		//glm::vec3 at = g_char_ptr->Poistion - g_cam_ptr->pos;

		// soloud 3d wants audio shit in meters and dmc3 is cm iirc
		glm::vec3 cam = g_cam_ptr->pos * 0.01f;
		glm::vec3 plr = g_char_ptr->Poistion * 0.01f;
		glm::vec3 at = cam - plr;

		/*
		m_vox->set_listener3d(g_cam_ptr->pos.x, g_cam_ptr->pos.y, g_cam_ptr->pos.z,
			at.x, at.y, at.z, g_cam_ptr->upVector.x, g_cam_ptr->upVector.y, g_cam_ptr->upVector.z);
		m_vox->play3d(g_char_ptr->Poistion.x, g_char_ptr->Poistion.y, g_char_ptr->Poistion.z, m_vox->m_volume);
		*/
		m_vox->set_listener3d(cam.x, cam.y, cam.z,
			at.x, at.y, at.z, g_cam_ptr->upVector.x, g_cam_ptr->upVector.y, g_cam_ptr->upVector.z);
		m_vox->play3d(plr.x, plr.y, plr.z, m_vox->m_volume);

	}
	else {
		PlaySound((LPCSTR)m_sound_file_mem, NULL, SND_MEMORY | SND_ASYNC);
	}
}
