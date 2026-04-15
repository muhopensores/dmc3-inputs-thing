#include "StyleSwitchFX.hpp"
#include "utility/Scan.hpp"
#include "utility/Compressed.hpp"
#include "d3dx9.h"
#include "CustomAlolcator.hpp"

#include <dsound.h>
#include <mmsystem.h>

#define DEVIL4_SFX 69
#define SOUND_BUFFERS_MAX 4

#define WAV_PATH "native\\sound\\style_switch.wav"

static CPlDante* g_char_ptr     = nullptr;
static cCameraCtrl* g_cam_ptr   = nullptr;
static float g_pan_distance = 1000.0f;
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
    SfxPreset { "\\native\\sound\\style_switch.wav", DEVIL4_SFX, DEVIL4_SFX, DEVIL4_SFX },
	SfxPreset { "Custom", -1, -1, -1 }
};

static SfxPreset g_sfx_preset = sfx_presets[0];
static size_t g_sfx_preset_index = 0;

struct VfxPreset {
    const char* name;
    uint8_t id;
    int bank, a3;
};

static constexpr std::array vfx_presets = {
    VfxPreset { "Circly thing fx", 218, 3, 8 },
    VfxPreset { "Crazy Combo fx",  144, 3, 8 },
    VfxPreset { "DT fx",          0xDC, 3, 8 },
    VfxPreset { "Custom",            0, 0, 0 },
};

static size_t g_vfx_preset_index = 0;

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

struct CustomSoundData {
    struct WavData {
        WAVEFORMATEX fmt;
        const uint8_t* data;
        DWORD size;
        uint8_t* raw_bytes;
    };

    static constexpr LPDIRECTSOUND8* g_devil3_directsound = (LPDIRECTSOUND8*)0x00833240;
    static inline LPDIRECTSOUNDBUFFER devil3_sbuf[4]{};
    static inline WavData custom_style_switch_sound{};

    CustomSoundData() {
        char buffer[MAX_PATH] = {};
        GetCurrentDirectoryA(MAX_PATH, buffer);
        auto err = load_custom_sfx(&custom_style_switch_sound,  devil3_sbuf);
        if (err.has_value()) {
            throw std::runtime_error(err.value());
        }
    }
    ~CustomSoundData() {
        unload_custom_sfx(&custom_style_switch_sound, devil3_sbuf);
    }

    CustomSoundData(const CustomSoundData&)            = delete;
    CustomSoundData(CustomSoundData&&)                 = delete;
    CustomSoundData& operator=(const CustomSoundData&) = delete;
    CustomSoundData& operator=(CustomSoundData&&)      = delete;

    static std::optional<const char*> load_wav_from_memory(const uint8_t* bytes, size_t byte_count, WavData* out) {
        if (byte_count < 44) {
            return ("Too small for WAV\n");
        }
        if (memcmp(bytes, "RIFF", 4) || memcmp(bytes + 8, "WAVE", 4)) {
            return ("Not a WAV\n");
            
        }

        /* Own a copy so the caller can free the original */
        out->raw_bytes = (uint8_t*)malloc(byte_count);
        if (out->raw_bytes) {
            memcpy(out->raw_bytes, bytes, byte_count);
        } else {
            return "Not enough memories";
        }

        memset(&out->fmt, 0, sizeof(out->fmt));
        out->data = nullptr;
        out->size = 0;

        DWORD pos = 12;
        while (pos + 8 <= (DWORD)byte_count) {
            uint32_t id  = *(uint32_t*)(out->raw_bytes + pos);
            uint32_t csz = *(uint32_t*)(out->raw_bytes + pos + 4);
            pos += 8;
            if (pos + csz > (DWORD)byte_count)
                break;

            if (id == *(uint32_t*)"fmt " && csz >= sizeof(PCMWAVEFORMAT)) {
                out->fmt.wFormatTag      = *(uint16_t*)(out->raw_bytes + pos);
                out->fmt.nChannels       = *(uint16_t*)(out->raw_bytes + pos + 2);
                out->fmt.nSamplesPerSec  = *(uint32_t*)(out->raw_bytes + pos + 4);
                out->fmt.nAvgBytesPerSec = *(uint32_t*)(out->raw_bytes + pos + 8);
                out->fmt.nBlockAlign     = *(uint16_t*)(out->raw_bytes + pos + 12);
                out->fmt.wBitsPerSample  = *(uint16_t*)(out->raw_bytes + pos + 14);
            } else if (id == *(uint32_t*)"data") {
                out->data = out->raw_bytes + pos;
                out->size = csz;
            }
            pos += csz;
            if (pos & 1)
                pos++;
        }

        if (!out->data) {
            free(out->raw_bytes);
            return ("No data chunk\n");
        }
        return std::nullopt;
    }

    static std::optional<const char*> load_wav_file(const char* path, WavData* out) {
        FILE* f = fopen(path, "rb");
        if (!f) {
            return "Cannot open " WAV_PATH;
        }
        fseek(f, 0, SEEK_END);
        long sz = ftell(f);
        fseek(f, 0, SEEK_SET);
        auto buf = (uint8_t*)malloc(sz);
        if (buf == NULL) {
            fclose(f);
            return "not enough memories";
        }
        else {
            fread(buf, 1, sz, f);
            fclose(f);
            std::optional<const char*> ok = load_wav_from_memory(buf, sz, out);
            free(buf);
            return ok;
        }
    }

    static LONG calc_pan(cCameraCtrl* cam, Vector3f sound_pos, float max_dist) {
        glm::vec3 p_up(cam->upVector.x, cam->upVector.y, cam->upVector.z);
        glm::vec3 p_cam(cam->pos.x, cam->pos.y, cam->pos.z);
        glm::vec3 p_look(cam->lookat.x, cam->lookat.y, cam->lookat.z);

        glm::vec3 forward = glm::normalize(p_look - p_cam);
        glm ::vec3 right  = glm::normalize(glm::cross(forward, p_up));

        glm::vec3 to_sound = sound_pos - p_cam;

        float pan_val = glm::dot(to_sound, right);

        // Scale to DirectSound (-10000 to +10000)
        pan_val     = glm::clamp(pan_val, -max_dist, max_dist);
        LONG ds_pan = (LONG)(pan_val / max_dist * 10000.0f);

        return ds_pan;
    }

    static LONG calc_volume(float sx, float sy, float sz,
                            float lx, float ly, float lz,
                            float max_dist) {
        float dx   = sx - lx;
        float dy   = sy - ly;
        float dz   = sz - lz;
        float dist = sqrtf(dx * dx + dy * dy + dz * dz);
        if (dist >= max_dist)
            return DSBVOLUME_MIN;
        if (dist <= 0.0f)
            return DSBVOLUME_MAX;
        return (LONG)(dist / max_dist * -10000.0f);
    }

    static std::optional<const char*> load_custom_sfx(WavData* wav, LPDIRECTSOUNDBUFFER* dsound_buffers) {

        std::optional<const char*> ok = load_wav_file(WAV_PATH, wav);
        if (ok.has_value()) {
            return ok;
        }

        DSBUFFERDESC bd  = {};
        bd.dwSize        = sizeof(bd);
        bd.dwFlags       = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPAN;
        bd.dwBufferBytes = wav->size;
        bd.lpwfxFormat   = &wav->fmt;
        auto dsound      = *g_devil3_directsound;

        for (int i = 0; i < SOUND_BUFFERS_MAX; i++) {

            HRESULT hr = dsound->CreateSoundBuffer(&bd, &dsound_buffers[i], nullptr);
            if (FAILED(hr)) {
                return "Failed to create dsound buffer";
            }

            // fill sound buffers
            void *p1, *p2;
            DWORD s1, s2;
            hr = dsound_buffers[i]->Lock(0, wav->size, &p1, &s1, &p2, &s2, 0);
            if (SUCCEEDED(hr)) {
                memcpy(p1, wav->data, s1);
                if (p2)
                    memcpy(p2, wav->data + s1, s2);
                dsound_buffers[i]->Unlock(p1, s1, p2, s2);
            } else {
                return "Failed to fill sound buffers";
            }
        }
        return std::nullopt;
    }

    static void unload_custom_sfx(WavData* wav, LPDIRECTSOUNDBUFFER* dsound_buffers) {
        free(wav->raw_bytes);
        for (int i = 0; i < SOUND_BUFFERS_MAX; i++) {
            dsound_buffers[i]->Stop();
            dsound_buffers[i]->Release();
            dsound_buffers[i] = nullptr;
        }
    }
};
std::unique_ptr<CustomSoundData> m_custom_sound;

std::optional<std::string> StyleSwitchFX::on_initialize() {
    // HACK() for style switcher with more memory patch
    if(g_mem_patch_applied) {
        // max out all styles, cba to look where it loads save data
        struct StyleData {
            uint32_t N0000004F { 0x2 }; //0x0000
            uint32_t N00000050 { 0x2 }; //0x0004
            uint32_t N00000051 { 0x2 }; //0x0008
            uint32_t N00000052 { 0x2 }; //0x000C
            uint32_t N00000053 { 0x2 }; //0x0010
            uint32_t N00000054 { 0x2 }; //0x0014
            uint32_t N00000055 { 0x0 }; //0x0018
            float N00000056 { 99999.0f }; //0x001C
            float N00000057 { 99999.0f }; //0x0020
            float N00000058 { 99999.0f }; //0x0024
            float N00000059 { 99999.0f }; //0x0028
            float N0000005A { 99999.0f }; //0x002C
            float N0000005B { 0.0f }; //0x0030
        }; //Size: 0x0034
        StyleData s1;
        // send those bytes to where style switcher expects them
        memcpy((void*)0x00C38B6C, &s1, sizeof(s1));
    }

    // TODOOOOO(important): add soloud audio
    // g_char_ptr = (CPlDante*)0x1C8A600;
    g_char_ptr = devil3_sdk::get_pl_dante();
    // why did i need srand?
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
    //return std::nullopt;
#if 0
    auto decompressed     = utility::DecompressFileFromMemoryWithSize(sfx_compressed_data, sfx_compressed_size);
    m_sound_file_mem      = std::get<0>(decompressed);
    m_sound_file_mem_size = std::get<1>(decompressed);
    if (!load_wav_from_memory((uint8_t*)m_sound_file_mem, m_sound_file_mem_size, &g_devil4_style_switch_sound)) {
        return "Failed to load devil4 style switch";
    }
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
    g_vfx_preset_index = cfg.get<size_t>("StyleSwitchVfxIndex").value_or(0);

    g_sfx_preset.a1 = cfg.get<int16_t>("StyleSwitchSfxBank").value_or(0);
    g_sfx_preset.a2 = cfg.get<int16_t>("StyleSwitchSfxId").value_or(63);
    g_sfx_preset.a3 = cfg.get<int>("StyleSwitchSfxIdk").value_or(0);
    g_sfx_preset_index = cfg.get<size_t>("StyleSwitchSfxIndex").value_or(0);

    for (int i = 0; i < DANTE_STYLES::STYLE_MAX; i++) {
        g_style_colors[i] = cfg.get<glm::vec4>(g_style_names[i]).value_or(g_default_colors[i]);
    }

    if (g_sfx_preset.a1 == DEVIL4_SFX && g_sfx_preset.a2 == DEVIL4_SFX && g_sfx_preset.a3 == DEVIL4_SFX) {
        if (!m_custom_sound) {
            m_custom_sound = std::make_unique<CustomSoundData>();
        }
        else {
            assert("CustomSoundData exists wtf?");
        }
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
    cfg.set<size_t>("StyleSwitchVfxIndex", g_vfx_preset_index);

    cfg.set<int16_t>("StyleSwitchSfxId", g_sfx_preset.a2);
    cfg.set<int16_t>("StyleSwitchSfxBank", g_sfx_preset.a1);
    cfg.set<int> ("StyleSwitchSfxIdk", g_sfx_preset.a3);
    cfg.set<size_t>("StyleSwitchSfxIndex", g_sfx_preset_index);

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
        static int selected_combobox_index = g_vfx_preset_index;
        if (ImGui::BeginCombo("VFX preset", vfx_presets[selected_combobox_index].name)) {
            for (size_t i = 0; i < vfx_presets.size(); i++) {
                const bool is_selected = (item_current_idx == i);

                if (ImGui::Selectable(vfx_presets[i].name, is_selected)) {
                    item_current_idx  = i;
                    selected_combobox_index = i;
                    g_vfx_preset_index = selected_combobox_index;
                    if(vfx_presets[i].id > 0) {
                        g_vfx_id = vfx_presets[i].id;
                    }
                    if (vfx_presets[i].bank > 0) {
                        g_vfx_bank = vfx_presets[i].bank;
                    }
                    if (vfx_presets[i].a3 > 0) {
                        g_vfx_a3 = vfx_presets[i].a3;
                    }
                }
                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
        if (g_vfx_preset_index == 3) {
            ImGui::InputInt("g_vfx_id", &g_vfx_id);
            ImGui::InputInt("g_vfx_bank", &g_vfx_bank);
            ImGui::InputInt("g_vfx_a3", &g_vfx_a3);
        }
    }

    ImGui::Checkbox("Enable sound effect", &g_enable_sound);
    if (g_enable_sound) {
        int item_current_idx = 0;
        static int selected_combobox_index = g_sfx_preset_index;
        if (ImGui::BeginCombo("SFX preset", sfx_presets[selected_combobox_index].name)) {
            for (size_t i = 0; i < sfx_presets.size(); i++) {
                const bool is_selected = (item_current_idx == i);

                if (ImGui::Selectable(sfx_presets[i].name, is_selected)) {
                    item_current_idx  = i;
                    selected_combobox_index = i;
                    g_sfx_preset_index = selected_combobox_index;
                    g_sfx_preset.name = sfx_presets[i].name;
                    if ( sfx_presets[i].a1 != -1) {
                        g_sfx_preset.a1 = sfx_presets[i].a1;
                    }
                    if ( sfx_presets[i].a2 != -1) {
                        g_sfx_preset.a2 = sfx_presets[i].a2;
                    }
                    if ( sfx_presets[i].a3 != -1) {
                        g_sfx_preset.a3 = sfx_presets[i].a3;
                    }
                }
                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
        if (g_sfx_preset_index == 5) {
            ImGui::Text("Put style_switch.wav file into .\\native\\sound\\style_switch.wav");
            static const char* error_message = nullptr;
            if (ImGui::Button("Reload custom sound")) {
                if (!m_custom_sound) {
                    try {
                        m_custom_sound = std::make_unique<CustomSoundData>();
                    } catch (std::runtime_error e) {
                        spdlog::error(e.what());
                        m_custom_sound = nullptr;
                        error_message = e.what();
                    }
                }
            }
            if (m_custom_sound && (error_message == nullptr)) {
                ImGui::TextColored(ImColor(IM_COL32(85,217,133,255)), "file loaded");
                if (ImGui::Button("Test play sound")) {
                    m_custom_sound->devil3_sbuf[0]->Play(0, 0, 0);
                }
            }
        }
        if (g_sfx_preset_index == 6) {
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

void StyleSwitchFX::on_draw_debug_ui()
{
    ImGui::Text("cPlDante: %p", g_char_ptr);
    ImGui::SliderFloat("g_pan_dist", &g_pan_distance, 0.0f, 99999.0f);
}
void StyleSwitchFX::play_sound()
{
	if (!g_enable_sound) { return; }

    if (g_sfx_preset.a1 == DEVIL4_SFX &&
        g_sfx_preset.a2 == DEVIL4_SFX &&
        g_sfx_preset.a3 == DEVIL4_SFX) {
        static int counter = 0;
        int index = counter % 4;
        static constexpr float max_dist = 10000.0f;
        cCameraCtrl* camera = devil3_sdk::get_cam_ctrl();
		if (!camera || camera == (cCameraCtrl*)-1) { return; };
        auto p = g_char_ptr->Poistion;
        auto l = camera->pos;
        LONG pan = CustomSoundData::calc_pan(camera, Vector3f{p.x, p.y, p.z}, g_pan_distance);
        LONG vol = CustomSoundData::calc_volume(p.x, p.y, p.z, l.x, l.y, l.z, 20000.0f);
#ifndef _NDEBUG
        printf("  pan = %d  (DSOUND: -10000=left, 0=center, +10000=right)\n", pan);
        printf("  vol = %d  (DSOUND: -10000=silent, 0=full)\n", vol);
#endif // !_NDEBUG
        DWORD status;
        m_custom_sound->devil3_sbuf[index]->GetStatus(&status);
        if(status & DSBSTATUS_PLAYING) {
             m_custom_sound->devil3_sbuf[index]->Stop();
             m_custom_sound->devil3_sbuf[index]->SetCurrentPosition(0);
        }
        m_custom_sound->devil3_sbuf[index]->SetPan(pan);
        m_custom_sound->devil3_sbuf[index]->SetVolume(vol);
        m_custom_sound->devil3_sbuf[index]->Play(0, 0, 0);
        counter += 1;
    }
    else {
        devil3_sdk::play_sound(g_sfx_preset.a1, g_sfx_preset.a2, rand() % 3);
    }

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
