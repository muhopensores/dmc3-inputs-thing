#include <spdlog/spdlog.h>
#include "Mods.hpp"
#include "Config.hpp"

//#inlcude "YourMod.hpp"
#include "mods/SimpleMod.hpp"
#include "mods/QuicksilverShader.hpp"
#include "mods/UIButton.hpp"
#include "mods/GamepadsFix.hpp"
#include "mods/AudioStutterFix.hpp"
#include "mods/PrintfDebugging.hpp"
#include "mods/RendererReplace.hpp"
#include "mods/CustomAlolcator.hpp"
#include "mods/DebugDraw.hpp"
#include "mods/InputLog.hpp"
#include "mods/AreaJump.hpp"
#if GAMEPLAY_HOOKS
#include "mods/InertiaThings.hpp"
#include "mods/StyleSwitchFX.hpp"
#include "mods/PracticeMode.hpp"
#include "mods/BulletStop.hpp"
#include "mods/RgTimer.hpp"
#include "mods/EnemySoulEaterNoInvis.hpp"
#include "mods/EnemyStates.hpp"
#include "mods/TurnSpeed.hpp"
#include "mods/EnemyStepCooldown.hpp"
#include "mods/NoHeightRestriction.hpp"
#include "mods/EnemySpawnRate.hpp"
#endif
//#include "mods/StyleSwitcherInfo.hpp"
//#include "mods/CameraHack.hpp"

struct TypeIndexer {
    static int next_index() {
        static int count = 0;
        return count++;
    }

    template<typename T>
    static int get_index() {
        static int index = next_index();
        return index;
    }
};

#define ADD_MOD(name)                                  \
    do {                                               \
        m_mods.emplace_back(std::make_unique<name>()); \
        TypeIndexer::get_index<name>();                \
    } while (0)

Mods::Mods()
{
    //m_mods.emplace_back(std::make_unique<SimpleMod>());
    //m_mods.emplace_back(std::make_unique<YourMod>());
#ifdef DEVELOPER
    m_mods.emplace_back(std::make_unique<DeveloperTools>()); // you wish
#endif
}

void Mods::load_time_critical_mods() {
#ifdef DINPUT_HOOK
	ADD_MOD(GamepadsFix); //m_mods.emplace_back(std::make_unique<GamepadsFix>());
#endif
	ADD_MOD(CustomAlolcator); //m_mods.emplace_back(std::make_unique<CustomAlolcator>());
	//m_mods.emplace_back(std::make_unique<YourMod>());
}

void Mods::load_mods() {

	ADD_MOD(QuicksilverShader); //m_mods.emplace_back(std::make_unique<QuicksilverShader>());
	ADD_MOD(AudioStutterFix); //m_mods.emplace_back(std::make_unique<AudioStutterFix>());
    ADD_MOD(UIButton); //m_mods.emplace_back(std::make_unique<UIButton>());
	ADD_MOD(AreaJump); //m_mods.emplace_back(std::make_unique<AreaJump>());
    ADD_MOD(InputLog); //m_mods.emplace_back(std::make_unique<InputLog>()); //NOTE(): dont move this one [8]
	ADD_MOD(DebugDraw); //m_mods.emplace_back(std::make_unique<DebugDraw>()); //NOTE(): dont move this one [9]

#if GAMEPLAY_HOOKS
	ADD_MOD(InertiaThings); //m_mods.emplace_back(std::make_unique<InertiaThings>());
	ADD_MOD(StyleSwitchFX); //m_mods.emplace_back(std::make_unique<StyleSwitchFX>());
	ADD_MOD(PracticeMode); //m_mods.emplace_back(std::make_unique<PracticeMode>()); // NOTE(): dont move this one [5]
	ADD_MOD(BulletStop); //m_mods.emplace_back(std::make_unique<BulletStop>());
    ADD_MOD(RgTimer); //m_mods.emplace_back(std::make_unique<RgTimer>()); //NOTE(): dont move this one [10]
	ADD_MOD(EnemySoulEaterNoInvis); //m_mods.emplace_back(std::make_unique<EnemySoulEaterNoInvis>());
	ADD_MOD(EnemyStates); //m_mods.emplace_back(std::make_unique<EnemyStates>());
	ADD_MOD(TurnSpeed); //m_mods.emplace_back(std::make_unique<TurnSpeed>());
	ADD_MOD(EnemyStepCooldown); //m_mods.emplace_back(std::make_unique<EnemyStepCooldown>());
	ADD_MOD(NoHeightRestriction); //m_mods.emplace_back(std::make_unique<NoHeightRestriction>());
    ADD_MOD(EnemySpawnRate); //m_mods.emplace_back(std::make_unique<EnemySpawnRate>()); // ldk
#endif // !GAMEPLAY_HOOKS

#ifndef _NDEBUG
	//m_mods.emplace_back(std::make_unique<PrintfDebugging>());
#endif // _DEBUG
}

std::optional<std::string> Mods::on_initialize() const {
    for (auto& mod : m_mods) {
        spdlog::info("{:s}::on_initialize()", mod->get_name().data());

        if (auto e = mod->on_initialize(); e != std::nullopt) {
            spdlog::info("{:s}::on_initialize() has failed: {:s}", mod->get_name().data(), *e);
            return e;
        }
    }

    utility::Config cfg{ CONFIG_FILENAME };

    for (auto& mod : m_mods) {
        spdlog::info("{:s}::on_config_load()", mod->get_name().data());
        mod->on_config_load(cfg);
    }

    return std::nullopt;
}

void Mods::on_frame() const {
    for (auto& mod : m_mods) {
        mod->on_frame();
    }
    if(g_framework->m_rr) {
        g_framework->m_rr->on_frame();
    }
}

void Mods::on_draw_debug_ui() const {
    ImGui::Text("Framerates: %f", ImGui::GetIO().Framerate);
	for (auto& mod : m_mods) {
		mod->on_draw_debug_ui();
	}
}

void Mods::on_draw_ui() const {
    for (auto& mod : m_mods) {
        mod->on_draw_ui();
    }
    if(g_framework->m_rr) {
        g_framework->m_rr->on_draw_ui();
    }
}

template<typename T>
T* get_raw_pointer(Mods* mods) {
    return dynamic_cast<T*>(mods->m_mods[TypeIndexer::get_index<T>()].get());
}

template<typename T>
void custom_imgui_window(Mods* mods) {
    T* ptr = get_raw_pointer<T>(mods);
    if (ptr) {
        ptr->custom_imgui_window();
    }
}

void Mods::on_draw_custom_imgui_window() {
#if GAMEPLAY_HOOKS
    custom_imgui_window<PracticeMode>(this);
    custom_imgui_window<RgTimer>(this);
#endif
    custom_imgui_window<InputLog>(this);
    custom_imgui_window<DebugDraw>(this);
}

void Mods::on_reset(IDirect3DDevice9* pDevice, bool before) const {
    for (const auto& mod : m_mods) {
        mod->on_reset(pDevice, before);
    }
}
