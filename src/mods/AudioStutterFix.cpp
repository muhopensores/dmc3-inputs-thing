#include "AudioStutterFix.hpp"
#include "sdk/VoxObj.hpp"
#include <vector>
#include <string>
#include <future>
#include <mutex>
#include <filesystem>
#include <unordered_map>
#include "utility/Patch.hpp"

struct Devil3BgmChannels {
	// NOTE(): initialized in dmc3se.exe WinMain
	VoxObj* channel[4];
	size_t num_channels;
};
Devil3BgmChannels* game_snd_channels{ nullptr };
FunctionHook* cCustomize_hack{ nullptr }; // cCustomize screen doesnt stop the bgm for some reason
uintptr_t c_cusomize_return = 0x00444190; // dmc3se.exe+44190 - 85 F6                 - test esi,esi

static void stop_sound() {
	if (!game_snd_channels) { return; }
	/*for (size_t i = 0; i < game_snd_channels->num_channels; i++) {
		
	}*/
	game_snd_channels->channel[0]->pause();
}

__declspec(naked) void c_cusomize_detour() {
	__asm {
		pushad
		call stop_sound
		popad
		push 0000017Ah
		jmp DWORD PTR [c_cusomize_return]
	}
}



FILE* __cdecl fopen_ours_real(const char* FileName, const char* Mode) {
    //FILE* ret = g_file_store->at(FileName);
    return nullptr;
}

static uintptr_t fopen_jmp_back {};
__declspec(naked) void assembler_jump() {
    __asm { 
        mov ebx, fopen_ours_real 
        jmp DWORD PTR [fopen_jmp_back]
    }
}

std::optional<std::string> AudioStutterFix::on_initialize() {

	// WARNING(): dirty hack to only init once here:
	static bool init = false;
	if (init) {
		return Mod::on_initialize();
	}
	/* patching out: 
	6A 64 - push 64
	FF D7 - call edi
	*/
	m_enabled = true;

	std::vector<int16_t> bytes; bytes.resize(0x4);
	std::fill(bytes.begin(), bytes.end(), 0x90);
	m_disable_sleep1 = new Patch(0x00404987, bytes, true);
	m_disable_sleep2 = new Patch(0x00404998, bytes, true);
    
    return Mod::on_initialize();
#if 0 // TODO(): removed due to custom snd.drv issues
	game_snd_channels = (Devil3BgmChannels*)0x0832DBC;
	cCustomize_hack = new FunctionHook(0x0044418B, &c_cusomize_detour);
	cCustomize_hack->create();
#endif
#if 0
    try {
        g_file_store = std::make_unique<FileStore>();
        loadOggFilesParallel(".\\native\\sound", g_file_store.get());
    }
    catch (const std::exception& e) {
        spdlog::error("Failed to load native\\sound\\ogg; Exception info {}", e.what());
    }

    uintptr_t handle = (uintptr_t)GetModuleHandleA("snd.drv");
    ptrdiff_t offset = 0x11FF;
    const uintptr_t address = handle + offset;

    if (!install_hook_absolute(address, g_fopen_hook, &assembler_jump, &fopen_jmp_back, 6)) {
        return "failed to install snd.drv@fopen hook";
    }
#endif

	return Mod::on_initialize();
}

void AudioStutterFix::on_draw_ui() {
	if (!ImGui::CollapsingHeader(get_name().data())) {
		return;
	}
	if (m_enabled) {
		ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 0.8f), "Using custom SND.DRV");
		ImGui::TextWrapped("Audio stutter patch is applied.");
	}
	else {
		ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 0.8f), "Not using custom SND.DRV");
		ImGui::TextWrapped("Audio stutter patch is not applied.");
	}
}