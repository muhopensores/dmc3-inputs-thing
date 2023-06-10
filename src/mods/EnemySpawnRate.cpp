#include "EnemySpawnRate.hpp"



#include <tlhelp32.h>
#include <climits>
// clang-format off
// only in clang/icl mode on x64, sorry
/*
static naked void detour() {
	__asm {
		mov qword ptr [EnemySpawnRate::variable], rbx
		mov rax, 0xDEADBEEF
		jmp qword ptr [jmp_ret]
	}
}
*/
// clang-format on
static EnemySpawnRate* g_spawn_rate_mod{ nullptr };
static int GUY_MULTIPLIER{ 3 };
std::optional<std::string> EnemySpawnRate::on_initialize() {

	g_spawn_rate_mod = this;
	m_spawn_guy_hook = std::make_unique<FunctionHook>(0x0054ED10, &spawn_a_guy_sub_54ED10);
	

	if (m_spawn_guy_hook->create() ) {
		return Mod::on_initialize();
	}
	else {
		return "Failed to install hook for spawn multiplier";
	}
}

// during load
//void EnemySpawnRate::on_config_load(const utility::Config &cfg) {
//	for (IModValue& option : m_options) {
//		option.config_load(cfg);
//	}
//}
// during save
//void EnemySpawnRate::on_config_save(utility::Config &cfg) {
//	for (IModValue& option : m_options) {
//		option.config_save(cfg);
//	}
//}
// do something every frame
//void EnemySpawnRate::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
//void EnemySpawnRate::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
void EnemySpawnRate::on_draw_ui() {
	if (ImGui::CollapsingHeader("bootleg ldk")) {
		ImGui::DragInt("multiplier", &GUY_MULTIPLIER, 1, 0, 10);
	}
}

uintptr_t __fastcall EnemySpawnRate::spawn_a_guy_sub_54ED10_internal(uintptr_t p_this, float* a2, uintptr_t a3)
{
	uintptr_t res = NULL;
	for (int32_t i = 0; i < GUY_MULTIPLIER; i++) {
		res = m_spawn_guy_hook->get_original<decltype(spawn_a_guy_sub_54ED10)>()(p_this, a2, a3);
		std::this_thread::sleep_for(std::chrono::milliseconds(12));
		if (!res) { 
			MessageBox(NULL, "spawn_guy returned nullptr", "wew", MB_ICONERROR);
			break; 
		}
	}
	return res;
}

static uint64_t mul_and_check_overflow(uint64_t a, uint64_t b) {
	uint32_t result = 0;
	if ((b > 0 && a <= INT_MAX / b && a >= INT_MIN / b) ||
		(b == 0) ||
		(b == -1 && a >= -INT_MAX) ||
		(b < -1 && a >= INT_MAX / b && a <= INT_MIN / b))
	{
		result = a * b;
	}
	else
	{
		result = 0xDEADBEEF;
	}
	return result;
}

uintptr_t __fastcall EnemySpawnRate::spawn_a_guy_sub_54ED10(uintptr_t p_this, float* a2, uintptr_t a3) {
	return g_spawn_rate_mod->spawn_a_guy_sub_54ED10_internal(p_this, a2, a3);
}

#if 0
uintptr_t __fastcall EnemySpawnRate::cHeapFrameSomething_probably_sub_6D40B0_internal(uintptr_t p_this, cHeapFrame* a2, unsigned int a1, uintptr_t a3)
{
	auto res = m_alloc_hook->get_original<decltype(cHeapFrameSomething_probably_sub_6D40B0)>()(p_this,a2, a1, a3);
	if (!res) {
		if (a2) {
			//a2->ptr = malloc(a3);
			a2->scratch = malloc(a3*4);
			memset(a2->scratch, 0x7ff8dead, a3 * 4);
			res = (uintptr_t)a2->scratch;
			a2->num_refs += 1;
		}
	}
	return res;
}

uintptr_t __fastcall EnemySpawnRate::cHeapFrameSomething_probably_sub_6D40B0(uintptr_t p_this, cHeapFrame* a2, unsigned int a1, uintptr_t a3)
{
	return g_spawn_rate_mod->cHeapFrameSomething_probably_sub_6D40B0_internal(p_this, a2, a1,a3);
}
#endif