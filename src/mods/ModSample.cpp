#include "ModSample.hpp"

// clang-format off
// only in clang/icl mode on x64, sorry
/*
static naked void detour() {
	__asm {
		mov qword ptr [ModSample::variable], rbx
		mov rax, 0xDEADBEEF
		jmp qword ptr [jmp_ret]
	}
}
*/
// clang-format on

std::optional<std::string> ModSample::on_initialize() {
  // uintptr_t base = g_framework->get_module().as<uintptr_t>();

  //if (!install_hook_offset(0xBADF00D, m_function_hook, &detour, &jmp_ret, 5)) {
  //  return a error string in case something goes wrong
  //  spdlog::error("[{}] failed to initialize", get_name());
  //  return "Failed to initialize ModSample";
  //}
  return Mod::on_initialize();
}

// during load
//void ModSample::on_config_load(const utility::Config &cfg) {
//	for (IModValue& option : m_options) {
//		option.config_load(cfg);
//	}
//}
// during save
//void ModSample::on_config_save(utility::Config &cfg) {
//	for (IModValue& option : m_options) {
//		option.config_save(cfg);
//	}
//}
// do something every frame
//void ModSample::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
//void ModSample::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
//void ModSample::on_draw_ui() {}
#endif