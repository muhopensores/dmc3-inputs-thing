#include "AudioStutterFix.hpp"
#if THREAD_AUDIO_FIX
#include <deque>

void snd_thread_func();
static AudioStutterFix* g_af_ptr;
std::atomic<bool> g_thread_kill_signal = false;

void AudioStutterFix::toggle(bool enable) {
	if (enable) {
		g_thread_kill_signal = false;
		auto th = std::thread(snd_thread_func);
		th.detach();
		memset(m_error_txt_buf, 0, 1024);
		m_hooked1 = m_function_hook_lots_snd_shit->create();
		if (!m_hooked1) {
			spdlog::error("[{}] failed to initialize m_function_hook_lots_snd_shit", get_name());
			sprintf_s(m_error_txt_buf, 1024, "failed to initialize m_function_hook_lots_snd_shit");
			return;
		}
		m_hooked2 = m_function_hook3->create();
		if (!m_hooked2) {
			spdlog::error("[{}] failed to initialize m_function_hook3", get_name());
			sprintf_s(m_error_txt_buf, 1024, "failed to initialize m_function_hook3");
		}
	} 
	else {
		g_thread_kill_signal = true;
		memset(m_error_txt_buf, 0, 1024);
		if (m_hooked1) {
			if (!m_function_hook_lots_snd_shit->disable()) {
				spdlog::error("[{}] failed to initialize m_function_hook_lots_snd_shit", get_name());
				sprintf_s(m_error_txt_buf, 1024, "failed to initialize m_function_hook_lots_snd_shit");
				return;
			}
		}
		if (m_hooked2) {
			if (!m_function_hook3->disable()) {
				spdlog::error("[{}] failed to initialize m_function_hook3", get_name());
				sprintf_s(m_error_txt_buf, 1024, "failed to initialize m_function_hook3");
			}
		}
	}
}

std::optional<std::string> AudioStutterFix::on_initialize() {

	// WARNING(): dirty hack to only init once here:
	static bool init = false;
	if (init) {
		return Mod::on_initialize();
	}
	m_function_hook_lots_snd_shit = std::make_unique<FunctionHook>(get_snd_whatever_ptr(), &sub_404A70);

	/*bool hooked = m_function_hook_lots_snd_shit->create();
	if (!hooked) {
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize AudioStutterFix";
	}*/
	
	m_function_hook3 = std::make_unique<FunctionHook>(get_snd_start_thread_ptr(), &SND_filename_prepare_sub_404950);
	
	/*hooked = m_function_hook3->create();
	if (!hooked) {
		spdlog::error("[{}] failed to initialize", get_name());
		return "Failed to initialize AudioStutterFix";
	}*/
#if 0

  if (!install_hook_absolute(0x10001379, m_function_hook, &detour, &jmp_ret, 0xC)) {
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize AudioStutterFix";
  }

  if (!install_hook_absolute(0x1000134F, m_function_hook4, &detour3, &jmp_ret3, 0x6)) {
	  spdlog::error("[{}] failed to initialize", get_name());
	  return "Failed to initialize AudioStutterFix";
  }

  if (!install_hook_absolute(0x100013AA, m_function_hook5, &detour4, &jmp_ret4, 0x6)) {
	  spdlog::error("[{}] failed to initialize", get_name());
	  return "Failed to initialize AudioStutterFix";
  }
#endif
  init = true;
  g_af_ptr = this;


  return Mod::on_initialize();
}

std::atomic<bool> g_signal;

struct th_args {
	int a1;
	char filename[1000];
};

th_args g_th_args;

struct sub_whatever_args {
	int  a1;
	char a2;
	signed int a3;
	void* a5;
	void* a6;
};

std::deque<sub_whatever_args> g_deq;

static void snd_thread_func() {
	while (true) {
		
		if (g_signal) {
			g_af_ptr->m_function_hook3->get_original<decltype(AudioStutterFix::SND_filename_prepare_sub_404950)>()(g_th_args.a1, g_th_args.filename);
			g_signal = false;
		}

		if (!g_deq.empty() && !g_signal) {
			auto& args = g_deq.front();
			g_af_ptr->m_function_hook_lots_snd_shit->get_original<decltype(AudioStutterFix::sub_404A70)>()(args.a1, args.a2, args.a3, args.a5/*, args.a6*/);
			g_deq.pop_front();
		}

		if (g_thread_kill_signal) {
			g_thread_kill_signal = false;
			break;
		}
		Sleep(2);
	}
#ifdef _DEBUG
	printf("Killing sound stutter fix thread\n");
#endif
}

void __cdecl AudioStutterFix::sub_404A70_internal(int a1, char a3, signed int a4, void* a5/*, void* a6*/)
{
	g_deq.push_back({ a1, a3, a4, a5/*, a6*/});

	return;
}

int __cdecl AudioStutterFix::SND_filename_prepare_sub_404950_internal(int a1, const char * filename)
{

	/*void** dword_7814C4 = (void**)0x007814C4;
	dword_7814C4[6 * a1] = 0;
	void** dword_7814C0 = (void**)0x007814C0;
	dword_7814C0[6 * a1] = 0;*/
	
	g_th_args.a1 = a1;
	strcpy(g_th_args.filename, filename);

	g_signal = true;

	//g_af_ptr->m_function_hook3->get_original<decltype(AudioStutterFix::SND_filename_prepare_sub_404950)>()(a1, filename);
	return 1;
}

int __cdecl AudioStutterFix::SND_filename_prepare_sub_404950(int a1, const char * filename)
{
	return g_af_ptr->SND_filename_prepare_sub_404950_internal(a1, filename);
}

void __cdecl AudioStutterFix::sub_404A70(int a1, char a3, signed int a4, void* a5/*, void* a6*/)
{
	return g_af_ptr->sub_404A70_internal(a1,a3, a4, a5/*, a6*/);
}

// during load
void AudioStutterFix::on_config_load(const utility::Config &cfg) {
	for (IModValue& option : m_options) {
		option.config_load(cfg);
	}
	toggle(m_audio_fix_enable->value());
}
// during save
void AudioStutterFix::on_config_save(utility::Config &cfg) {
	for (IModValue& option : m_options) {
		option.config_save(cfg);
	}
}
// do something every frame
//void AudioStutterFix::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
//void AudioStutterFix::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
void AudioStutterFix::on_draw_ui() {
	if (!ImGui::CollapsingHeader(get_name().data())) {
		return;
	}
	if (m_audio_fix_enable->draw("Fix audio stutters with StyleSwitcher ForceMode0=1")) {
		toggle(m_audio_fix_enable->value());
	}
	ImGui::TextWrapped("Attempts to fix StyleSwitcher stutters by creating special "
		"watchdog thread for game audio, may cause some inconsistencies, probably will");
	if (m_error_txt_buf[1]) { 
		ImGui::TextColored(ImVec4(1.0f,0.0f,0.0f,1.0f),"Error: %s", m_error_txt_buf); 
	};
}
#else

std::optional<std::string> AudioStutterFix::on_initialize() {

	// WARNING(): dirty hack to only init once here:
	static bool init = false;
	if (init) {
		return Mod::on_initialize();
	}
	HMODULE snd = GetModuleHandle("snd.drv");
	if (!snd) {
		spdlog::info("[AudioStutterFix]: snd.drv is not found\n");
		printf("[AudioStutterFix]: snd.drv is not found\n");
		return Mod::on_initialize();
	}
	FARPROC snd_proc = GetProcAddress(snd, "IsSndDrvSexy");
	if (!snd_proc) {
		spdlog::info("[AudioStutterFix]: snd.drv is not custom, skipping audio fixes.\n");
		printf("[AudioStutterFix]: snd.drv is not custom, skipping audio fixes.\n");
		return Mod::on_initialize();
	}
	/* patching out: 
	6A 64 - push 64
	FF D7 - call edi
	*/
	std::vector<int16_t> bytes; bytes.resize(0x4);
	std::fill(bytes.begin(), bytes.end(), 0x90);
	m_disable_sleep1 = new Patch(0x00404987, bytes, true);
	m_disable_sleep2 = new Patch(0x00404998, bytes, true);

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
#endif