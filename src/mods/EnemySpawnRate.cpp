#include "EnemySpawnRate.hpp"
#include <inttypes.h>

#include <climits>
#include <tlhelp32.h>
/*
static naked void detour() {
	__asm {
		mov qword ptr [EnemySpawnRate::variable], rbx
		mov rax, 0xDEADBEEF
		jmp qword ptr [jmp_ret]
	}
}
*/

// clang-format off
static uintptr_t collision_handles_hack_jmp_ret{0x0041BD3D};
static int32_t g_current_collision_handles {0};
static _declspec(naked) void collision_handles_hack() {
    __asm {
        // we only get 0x64 collision handles for any level
        // store it to check for overflowing this number
        // so the game doesnt crash with too much enemies
        mov DWORD PTR [g_current_collision_handles], eax
    originalCode:
        lea edx, [eax + eax * 2]
        shl edx, 4
        jmp qword ptr [collision_handles_hack_jmp_ret]
    }
}
// clang-format on

static EnemySpawnRate* g_spawn_rate_mod{nullptr};
static int GUY_MULTIPLIER{1};
static bool LDK_BOSSFIGHTS{false};
static int BOSS_MULTIPLIER{1};

std::optional<std::string> EnemySpawnRate::on_initialize() {

    g_spawn_rate_mod = this;
    m_spawn_guy_hook = std::make_unique<FunctionHook>(0x0054ED10, &spawn_a_guy_sub_54ED10);
    m_collision_handles_hook = std::make_unique<FunctionHook>(0x0041BD37, &collision_handles_hack);

    if (m_spawn_guy_hook->create() && m_collision_handles_hook->create()) {
        return Mod::on_initialize();
    }

    return "Failed to install hook for spawn multiplier";
}

void EnemySpawnRate::on_config_load(const utility::Config& cfg) {
    GUY_MULTIPLIER  = cfg.get<int>("LDK_enemy_multiplier").value_or(1);
    BOSS_MULTIPLIER = cfg.get<int>("LDK_boss_multiplier").value_or(2);
    LDK_BOSSFIGHTS  = cfg.get<bool>("LDK_bossfights").value_or(false);
}

void EnemySpawnRate::on_config_save(utility::Config& cfg) {
    cfg.set<int> ("LDK_enemy_multiplier", GUY_MULTIPLIER);
    cfg.set<int> ("LDK_boss_multiplier",  BOSS_MULTIPLIER);
    cfg.set<bool>("LDK_bossfights",       LDK_BOSSFIGHTS);
}

// during load
// void EnemySpawnRate::on_config_load(const utility::Config &cfg) {
//	for (IModValue& option : m_options) {
//		option.config_load(cfg);
//	}
//}
// during save
// void EnemySpawnRate::on_config_save(utility::Config &cfg) {
//	for (IModValue& option : m_options) {
//		option.config_save(cfg);
//	}
//}
// do something every frame
// void EnemySpawnRate::on_frame() {}
// will show up in debug window, dump ImGui widgets you want here
// void EnemySpawnRate::on_draw_debug_ui() {}
// will show up in main window, dump ImGui widgets you want here
void EnemySpawnRate::on_draw_ui() {
    if (ImGui::CollapsingHeader("bootleg LDK")) {
        ImGui::DragInt("dudes multiplier", &GUY_MULTIPLIER, 1, 0, 5);
        if (ImGui::Checkbox("Legendary DANK knight", &LDK_BOSSFIGHTS)) {

        } ImGui::SameLine();
        show_help_marker("SOME Boss spawns are also increased in DANK mode");
        if (LDK_BOSSFIGHTS) {
            ImGui::DragInt("bosses multiplier", &BOSS_MULTIPLIER, 1, 0, 5);
        }
    }
}

void EnemySpawnRate::on_draw_debug_ui() {
    ImGui::Text("collision_handles= 0x%x", g_current_collision_handles);
}

// listing vtables here
static constexpr std::array<uintptr_t, 6> bugged_enemies {
    0x73E98C, // Giga-Pete  | CEm023 : CNonPlayer : CActor : CWork : IActor : ICollisionHandle : INonPlayer : CCom : ICom : IComAction : IComActionState
    0x742914, // Clown      | CEm037 : CNonPlayer : CActor : CWork : IActor : ICollisionHandle : INonPlayer : CCom : ICom : CComAction : IComAction : IComActionState
    0x74072C, // Horse      | CEm029 : CNonPlayer : CActor : CWork : IActor : ICollisionHandle : INonPlayer : CCom : ICom : CComAction : IComAction : IComActionState
    0x741470, // Clone      | CEm031 : CNonPlayer : CActor : CWork : IActor : ICollisionHandle : INonPlayer : CCom : ICom : CComAction : IComAction : IComActionState
    0x74182C, // Blob       | CEm032 : CNonPlayer : CActor : CWork : IActor : ICollisionHandle : INonPlayer : CCom : ICom : CComAction : IComAction : IComActionState
    0x73F268, // Dog        | CEm025 : CNonPlayer : CActor : CWork : IActor : ICollisionHandle : INonPlayer : CCom : ICom : IComAction : IComActionState
};

// nullptr dereferences in collision related logic for enemies below 
// or some other weird shit that crashes the game eventually
static constexpr std::array weird_enemies {
    uintptr_t(0x73DA90), // Birds      | CEm014 : CNonPlayer : CActor : CWork : IActor : ICollisionHandle : INonPlayer : CCom : ICom : CComAction : IComAction : IComActionState
    uintptr_t(0x73E108), // ChessPiese | CEm017 : CNonPlayer : CActor : CWork : IActor : ICollisionHandle : INonPlayer : CCom : ICom : CComAction : IComAction : IComActionState
    uintptr_t(0x73E6D0), // ChessKing? | CEm021 : CEm017 : CNonPlayer : CActor : CWork : IActor : ICollisionHandle : INonPlayer : CCom : ICom : CComAction : IComAction : IComActionState
};

static constexpr std::array boss_enemies {
    uintptr_t(0x73FEC8), // Nevan      | CEm028 : CNonPlayer : CActor : CWork : IActor : ICollisionHandle : INonPlayer : CCom : ICom : CComAction : IComAction : IComActionState
    uintptr_t(0x74107c), // Beowulf    | CEm028 : CNonPlayer : CActor : CWork : IActor : ICollisionHandle : INonPlayer : CCom : ICom : CComAction : IComAction : IComActionState
    uintptr_t(0x73f744), // A&R        | CEm026 : CNonPlayer : CActor : CWork : IActor : ICollisionHandle : INonPlayer : CCom : ICom : CComAction : IComAction : IComActionState
    uintptr_t(0x74248C), // Vergil     | CEm035 : CNonPlayer : CActor : CWork : IActor : ICollisionHandle : INonPlayer : CCom : ICom : IComAction : IComActionState
    uintptr_t(0x741e38), // Lady       | CEm034 : CNonPlayer : CActor : CWork : IActor : ICollisionHandle : INonPlayer : CCom : ICom : IComAction : IComActionState
};

uintptr_t __fastcall EnemySpawnRate::spawn_a_guy_sub_54ED10_internal(uintptr_t p_this, float* a2, uintptr_t a3) {
    uintptr_t res = NULL;
    res = m_spawn_guy_hook->get_original<decltype(spawn_a_guy_sub_54ED10)>()(p_this, a2, a3);
    int multiplier = GUY_MULTIPLIER;
    if (!res) {
        return res;
    }
    uintptr_t vtable_pointer = *(uintptr_t*)res;
#ifndef _NDEBUG
    printf("enemy ptr: %" PRIxPTR ", enemy vtable: %" PRIxPTR ";\n", res, vtable_pointer);
#endif // !_NDEBUG

    // check for bugged enemies
    for (uintptr_t enemy : bugged_enemies) {
        if (vtable_pointer == enemy) {
            return res;
        }
    }

    // only double the weird enemies
    for (uintptr_t enemy : weird_enemies) {
        if ((vtable_pointer == enemy) && GUY_MULTIPLIER > 1) {
            res = m_spawn_guy_hook->get_original<decltype(spawn_a_guy_sub_54ED10)>()(p_this, a2, a3);
            return res;
        }
    }

    // boss fights mode
    for (uintptr_t enemy : boss_enemies) {
        if (vtable_pointer == enemy && LDK_BOSSFIGHTS) {
            multiplier = BOSS_MULTIPLIER;
        }
    }

    for (int i = 0; i < multiplier - 1; i++) {
        // std::this_thread::sleep_for(std::chrono::milliseconds(12));
        if(g_current_collision_handles >= 0x62) { 
            spdlog::error("[{}] spawn_guy exceeds collision handles", this->get_name());
            break;
        }
        res = m_spawn_guy_hook->get_original<decltype(spawn_a_guy_sub_54ED10)>()(p_this, a2, a3);
        if (!res) {
            spdlog::error("[{}] spawn_guy returned nullptr", this->get_name());
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
        (b < -1 && a >= INT_MAX / b && a <= INT_MIN / b)) {
        result = a * b;
    } else {
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