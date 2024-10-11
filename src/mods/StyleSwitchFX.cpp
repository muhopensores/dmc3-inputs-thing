#include "StyleSwitchFX.hpp"
#include "utility/Scan.hpp"
#include "utility/Compressed.hpp"
#include "../Sfx.cpp"
#include "d3dx9.h"

static CPlDante* g_char_ptr     = nullptr;
static cCameraCtrl* g_cam_ptr   = nullptr;
#if 0 // TODO coat textures
static Devil3Texture* g_texture = nullptr;
static IDirect3DTexture9* g_texture_original = nullptr;
#endif

std::mutex g_style_switch_mutex {};

#if 0
static constexpr void* GET_PL_DANTE_TEXTURE_DECODE() {
	return (void*)0x024F9640;
}
#endif

#ifdef SND_HACK
#define SND_CREATE_VOX(name) VoxObj* name()
typedef SND_CREATE_VOX(snd_create_vox);
SND_CREATE_VOX(SndCreateVoxStub) {
	return nullptr;
}
static snd_create_vox* SndCreateVox_ = SndCreateVoxStub;
#define SndCreateVox SndCreateVox_
#endif

// TODO(): ability to change vfx that's played ?
static int       g_vfx_id = 218;
static int       g_vfx_bank = 3;
static int       g_vfx_a3 = 8;
static int       prev_style;
static int*      current_style = (int*)0xB6B220;
static bool      g_enable_mod;
static bool      g_enable_sound;
#if 0 // TODO coat textures
static bool      g_enable_textures;
static bool      g_textures_not_found{false};
#endif
static uintptr_t detour_jmpback;

struct SfxPreset {
	const char* name;
	int16_t a1,a2;
	int a3 = 7;
};

static std::array sfx_presets = {
	SfxPreset { "Electricity SFX", 0, 63, 0 },
	SfxPreset { "CLANG", 0, 75, 0 },
	SfxPreset { "Crazy Combo start SFX", 0, 80, 0 },
	SfxPreset { "Enemy DT SFX", 0, 90, 0 },
	SfxPreset { "Player DT start SFX", 1, 80, 0 },
	//SfxPreset { "Custom", -1, -1, -1 }
};

static SfxPreset g_sfx_preset = sfx_presets[0];

struct VfxPreset {
    const char* name;
    uint8_t id;
    int bank, a3;
};

std::array vfx_presets = {
    VfxPreset { "Circly thing fx", 218, 3, 8 },
    VfxPreset { "Crazy Combo fx",  144, 3, 8 },
    VfxPreset { "DT fx",          0xDC, 3, 8 },
};

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

#if 0
static std::array<IDirect3DTexture9*, DANTE_STYLES::STYLE_MAX> g_style_textures {};
#endif

static void play_effect(int style) {
	typedef int(__cdecl* cEffectBase_sub_67FE80)(char a1, int a2, int *a3, int a4);
	static cEffectBase_sub_67FE80 c_efb_1 = (cEffectBase_sub_67FE80)0x67FE80;

	typedef int (__thiscall* cEffect2_sub_5A49E0)(CPlDante*);
	static cEffect2_sub_5A49E0 c_efb_2 = (cEffect2_sub_5A49E0)0x5A49E0;

	//int*   current_style = (int*)0xB6B220;
	int a1 = c_efb_1(g_vfx_bank, g_vfx_id, 0, g_vfx_a3);
	if (a1) {
		CPlDante* pl = (CPlDante*)g_char_ptr;
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

#if 0 // TODO coat textures
void style_switch_efx_clear_textures() {
    if(!g_enable_textures) {return;}
    auto device = g_framework->get_d3d9_device();
    HRESULT hr{};
    for (IDirect3DTexture9* texture : g_style_textures) {
        if (texture) {
            texture->Release();
            texture = nullptr;
        }
    }
    if (g_texture_original) {
        g_texture_original->Release();
        g_texture_original = nullptr;
    }
    if(g_texture) {
        g_texture = nullptr;
    }
}

void style_switch_efx_load_textures() {
    if(!g_enable_textures) {return;}
    auto device = g_framework->get_d3d9_device();
    HRESULT hr{};
    hr = D3DXCreateTextureFromFileA(device, "native\\texture\\SM.dds", &g_style_textures[SWORDMASTER]);
    assert(SUCCEEDED(hr));
    if(FAILED(hr)) {
        g_enable_textures = false;
        g_textures_not_found = true;
    }
    hr = D3DXCreateTextureFromFileA(device, "native\\texture\\TS.dds", &g_style_textures[TRICKSTER]);
    assert(SUCCEEDED(hr));
    hr = D3DXCreateTextureFromFileA(device, "native\\texture\\GS.dds", &g_style_textures[GUNSLINGER]);
    assert(SUCCEEDED(hr));
    hr = D3DXCreateTextureFromFileA(device, "native\\texture\\RG.dds", &g_style_textures[ROYALGUARD]);
    assert(SUCCEEDED(hr));
    hr = D3DXCreateTextureFromFileA(device, "native\\texture\\QS.dds", &g_style_textures[QUICKSILVER]);
    assert(SUCCEEDED(hr));
    hr = D3DXCreateTextureFromFileA(device, "native\\texture\\DG.dds", &g_style_textures[GERMANWORD]);
    assert(SUCCEEDED(hr));
}
#endif

std::optional<std::string> StyleSwitchFX::on_initialize() {
    // TODOOOOO(important): add soloud audio
    // g_char_ptr = (CPlDante*)0x1C8A600;
    g_char_ptr = devil3_sdk::get_pl_dante();
    srand(time(0));
    // g_cam_ptr = (CCameraCtrl*)0x01371978;

    // m_sound_file_mem = utility::DecompressFileFromMemory(sfx_compressed_data,sfx_compressed_size);

#ifdef SND_TODO
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

    SndCreateVox = (snd_create_vox*)GetProcAddress(snd, "CreateVox");
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
#endif
#if 0
    auto decompressed     = utility::DecompressFileFromMemoryWithSize(sfx_compressed_data, sfx_compressed_size);
    m_sound_file_mem      = std::get<0>(decompressed);
    m_sound_file_mem_size = std::get<1>(decompressed);
#endif
#ifdef SND_TODO
    m_vox->load_mem((unsigned char*)m_sound_file_mem, m_sound_file_mem_size);
    m_vox->set_volume(1.0f);
#endif
    return Mod::on_initialize();
}

// during load
void StyleSwitchFX::on_config_load(const utility::Config& cfg) {
    g_enable_mod      = cfg.get<bool>("StyleSwitchFXenabled").value_or(false);
    g_enable_sound    = cfg.get<bool>("StyleSwitchSoundEnabled").value_or(false);
#if 0 // TODO coat textures
    g_enable_textures = cfg.get<bool>("StyleSwitchTexturesEnabled").value_or(false);
#endif

    g_vfx_id   = cfg.get<int>("StyleSwitchVfxId").value_or(218);
    g_vfx_bank = cfg.get<int>("StyleSwitchVfxBank").value_or(3);
    g_vfx_a3   = cfg.get<int>("StyleSwitchVfxIdk").value_or(8);

    g_sfx_preset.a1 = cfg.get<int16_t>("StyleSwitchSfxBank").value_or(0);
    g_sfx_preset.a2 = cfg.get<int16_t>("StyleSwitchSfxId").value_or(63);
    g_sfx_preset.a3 = cfg.get<int>("StyleSwitchSfxIdk").value_or(0);

    for (int i = 0; i < DANTE_STYLES::STYLE_MAX; i++) {
        g_style_colors[i] = cfg.get<glm::vec4>(g_style_names[i]).value_or(g_default_colors[i]);
    }
}

// during save
void StyleSwitchFX::on_config_save(utility::Config& cfg) {
    cfg.set<bool>("StyleSwitchFXenabled",       g_enable_mod);
    cfg.set<bool>("StyleSwitchSoundEnabled",    g_enable_sound);
#if 0 // TODO coat textures
    cfg.set<bool>("StyleSwitchTexturesEnabled", g_enable_textures);
#endif

    cfg.set<int>("StyleSwitchVfxId",   g_vfx_id);
    cfg.set<int>("StyleSwitchVfxBank", g_vfx_bank);
    cfg.set<int>("StyleSwitchVfxIdk",  g_vfx_a3);

    cfg.set<int16_t>("StyleSwitchSfxId",   g_sfx_preset.a1);
    cfg.set<int16_t>("StyleSwitchSfxBank", g_sfx_preset.a2);
    cfg.set<int>    ("StyleSwitchSfxIdk",  g_sfx_preset.a3);

    for (int i = 0; i < DANTE_STYLES::STYLE_MAX; i++) {
        cfg.set<glm::vec4>(g_style_names[i], g_style_colors[i]);
    }
}

// do something every frame
void StyleSwitchFX::on_frame() {
    if (!g_enable_mod) {
        return;
    }
    if ((g_char_ptr->pad_0000) != 0x744D38) {
        prev_style = *current_style;
        return;
    }
#if 0 // TODO coat textures
    if ((g_char_ptr->pad_0000) != 0x744D38) {
        g_texture = nullptr;
        return;
    }
#endif
    if (*current_style != prev_style) {
        if (g_vfx_id == 218) {
            g_vfx_a3 = 1;
            play_effect(*current_style);
            g_vfx_a3 = 2;
            play_effect(*current_style);
        }
        else {
            play_effect(*current_style);
        }
        play_sound();
#if 0 // TODO coat textures
        change_texture(*current_style);
#endif
        prev_style = *current_style;
    }
}

static int16_t safe_truncate_to_int16(int value) {
    if (value >= std::numeric_limits<int16_t>::min() && value <= std::numeric_limits<int16_t>::max()) {
        return static_cast<int16_t>(value);
    } else {
        // Handle overflow/underflow
        if (value > std::numeric_limits<int16_t>::max()) {
            return std::numeric_limits<int16_t>::max();
        } else {
            return std::numeric_limits<int16_t>::min();
        }
    }
}

// will show up in debug window, dump ImGui widgets you want here
// void StyleSwitchFX::on_draw_debug_ui() {}
void StyleSwitchFX::on_draw_ui() {
    if (!ImGui::CollapsingHeader(get_name().data())) {
        return;
    }

    ImGui::Checkbox("Enable style switch effects", &g_enable_mod);
    if (g_enable_mod) {

        int item_current_idx = 0;
        static int selected_combobox_index = 0;
        if (ImGui::BeginCombo("VFX preset", vfx_presets[selected_combobox_index].name)) {
            for (size_t i = 0; i < vfx_presets.size(); i++) {
                const bool is_selected = (item_current_idx == i);

                if (ImGui::Selectable(vfx_presets[i].name, is_selected)) {
                    item_current_idx  = i;
                    selected_combobox_index = i;
                    g_vfx_id         = vfx_presets[i].id;
                    g_vfx_bank       = vfx_presets[i].bank;
                    g_vfx_a3         = vfx_presets[i].a3;
                }
                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
        ImGui::InputInt("g_vfx_id", &g_vfx_id);
        ImGui::InputInt("g_vfx_bank", &g_vfx_bank);
        ImGui::InputInt("g_vfx_a3", &g_vfx_a3);
    }

    ImGui::Checkbox("Enable sound effect", &g_enable_sound);
    if (g_enable_sound) {
        int item_current_idx = 0;
        static int selected_combobox_index = 0;
        if (ImGui::BeginCombo("SFX preset", sfx_presets[selected_combobox_index].name)) {
            for (size_t i = 0; i < sfx_presets.size(); i++) {
                const bool is_selected = (item_current_idx == i);

                if (ImGui::Selectable(sfx_presets[i].name, is_selected)) {
                    item_current_idx  = i;
                    selected_combobox_index = i;
                    g_sfx_preset.name = sfx_presets[i].name;
                    g_sfx_preset.a1   = sfx_presets[i].a1;
                    g_sfx_preset.a2   = sfx_presets[i].a2;
                    g_sfx_preset.a3   = sfx_presets[i].a3;
                }
                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
        int pa1 = g_sfx_preset.a1;
        int pa2 = g_sfx_preset.a2;

        if (ImGui::InputInt("sfx bank", &pa1)) {
            g_sfx_preset.a1 = safe_truncate_to_int16(pa1);
        }
        if (ImGui::InputInt("sfx id", &pa2)) {
            g_sfx_preset.a2 = safe_truncate_to_int16(pa2);
        }
        ImGui::InputInt("idk", &g_sfx_preset.a3);
        ImGui::SameLine(); 

        if (ImGui::Button("test play sound")) {
            devil3_sdk::play_sound(g_sfx_preset.a1, g_sfx_preset.a2, g_sfx_preset.a3);
        }
    }

#if 0 // TODO : textured coats
    if (g_textures_not_found) {
        ImGui::Text("Could not load coat textures from DMC3_ROOT\\native\\texture");
        ImGui::Text("Check for permission errors if the files are there, or something idk");
    }
    else {
        ImGui::Checkbox("Change coat color along with style", &g_enable_textures);
    }
#endif
#if SND_TODO
    if (m_vox != nullptr) {
        static float snd_volume = 1.0f;
        if (ImGui::DragFloat("Audio volume", &snd_volume, 0.1f, 0.0f, 10.0f)) {
            snd_volume = glm::clamp(snd_volume, 0.0f, 10.0f);
            m_vox->set_volume(snd_volume);
        }
        ImGui::Checkbox("3D audio effects", &m_3d_audio);
    }
#endif

    ImGui::Text("Customize style colors");

    for (int i = 0; i < STYLE_MAX; i++) {
        ImGui::ColorEdit3(g_style_names[i], (float*)&g_style_colors[i]);
    }
}

void StyleSwitchFX::play_sound()
{
	if (!g_enable_sound) { return; }

    devil3_sdk::play_sound(g_sfx_preset.a1, g_sfx_preset.a2, rand() % 3);

#if SND_TODO
	if (m_vox) {
		cCameraCtrl* camera = devil3_sdk::get_cam_ctrl();
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
#endif
		//PlaySound((LPCSTR)m_sound_file_mem, NULL, SND_MEMORY | SND_ASYNC);
#ifdef SND_TODO
	}
#endif
}

#if 0 // TODO coat textures
void StyleSwitchFX::change_texture(int style) {
    // const std::lock_guard<std::mutex> lock(g_style_switch_mutex);
    if (g_textures_not_found) {
        return;
    }
    if (!g_enable_textures) {
        return;
    }
    if (!g_texture) {

        TextureTableEntry* tex_entry = (TextureTableEntry*)0x252F750;
        while (tex_entry->ptrD3Texture != NULL) {
            if (tex_entry->ptrD3Texture->decodeDataPointer == GET_PL_DANTE_TEXTURE_DECODE() && tex_entry->ptrD3Texture->texturePointer) {
                g_texture          = tex_entry->ptrD3Texture;
                g_texture_original = g_texture->texturePointer;
                break;
            }
            tex_entry++;
        }
    }
    if (!g_texture) {
        return;
    }

    while (InterlockedCompareExchange((volatile unsigned int*)&g_texture->texturePointer, (unsigned int)g_style_textures[style], (unsigned int)g_texture->texturePointer) != (unsigned int)g_style_textures[style]) {
    }
    // tp->Release();
}
#endif
