#include "PrintfDebugging.hpp"

// clang-format off
// only in clang/icl mode on x64, sorry

static uintptr_t g_log_printf1{ NULL };
static uintptr_t g_log_printf2{ NULL };

static void _cdecl printf_wrapper(const char* boolstr, const char* src,int id) {
	printf("\x1b[93m [CAPCOM]: \x1b[0m %d, %s, %s\n", id, src, boolstr);
}

static __declspec(naked) void log_detour1() {
	__asm {
		call printf_wrapper
		jmp qword ptr [g_log_printf1]
	}
}

// clang-format on

std::optional<std::string> PrintfDebugging::on_initialize() {
  // uintptr_t base = g_framework->get_module().as<uintptr_t>();
	//TODO(): wip
#if 0
  if (!install_hook_absolute(0x004023CF, m_debug_log_hook1, &log_detour1, &g_log_printf1, 5)) {
    spdlog::error("[{}] failed to initialize", get_name());
    return "Failed to initialize PrintfDebugging";
  }
#endif
  return Mod::on_initialize();
}

// during load
//void PrintfDebugging::on_config_load(const utility::Config &cfg) {
//	for (IModValue& option : m_options) {
//		option.config_load(cfg);
//	}
//}
// during save
//void PrintfDebugging::on_config_save(utility::Config &cfg) {
//	for (IModValue& option : m_options) {
//		option.config_save(cfg);
//	}
//}
// do something every frame
//void PrintfDebugging::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
//void PrintfDebugging::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
//void PrintfDebugging::on_draw_ui() {}
