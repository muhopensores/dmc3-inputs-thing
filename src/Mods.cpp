#include <spdlog/spdlog.h>
#include "Mods.hpp"
#include "Config.hpp"

//#inlcude "YourMod.hpp"
#include "mods/SimpleMod.hpp"
#include "mods/QuicksilverShader.hpp"
#include "mods/InertiaThings.hpp"
#include "mods/StyleSwitchFX.hpp"
#include "mods/PracticeMode.hpp"
#include "mods/BulletStop.hpp"
#include "mods/UIButton.hpp"
#include "mods/GamepadsFix.hpp"
#include "mods/InputLog.hpp"
#include "mods/AudioStutterFix.hpp"
#include "mods/PrintfDebugging.hpp"
#include "mods/DebugDraw.hpp"
#include "mods/RgTimer.hpp"
#include "mods/NoHeightRestriction.hpp"
// #include "mods/EnemySoulEaterNoInvis.hpp" // affects other enemies, commented out until I find some enemy ID compare
#include "mods/TurnSpeed.hpp"
#include "mods/EnemyStepCooldown.hpp"
#include "mods/EnemyStates.hpp"
#include "mods/EnemySpawnRate.hpp"
#include "mods/CustomAlolcator.hpp"
#include "mods/RendererReplace.hpp"
#include "mods/AreaJump.hpp"
//#include "mods/StyleSwitcherInfo.hpp"
//#include "mods/CameraHack.hpp"

#define SPEEDRUN_MODE 1

Mods::Mods()
{
    //m_mods.emplace_back(std::make_unique<SimpleMod>());
    //m_mods.emplace_back(std::make_unique<YourMod>());
#ifdef DEVELOPER
    m_mods.emplace_back(std::make_unique<DeveloperTools>()); // you wish
#endif
}

void Mods::load_time_critical_mods() {
	m_mods.emplace_back(std::make_unique<GamepadsFix>());
	m_mods.emplace_back(std::make_unique<CustomAlolcator>());
	//m_mods.emplace_back(std::make_unique<YourMod>());
}

void Mods::load_mods() {
#if 1
	m_mods.emplace_back(std::make_unique<QuicksilverShader>());
	m_mods.emplace_back(std::make_unique<AudioStutterFix>());
#ifndef SPEEDRUN_MODE
	m_mods.emplace_back(std::make_unique<InertiaThings>());
	m_mods.emplace_back(std::make_unique<StyleSwitchFX>());
	m_mods.emplace_back(std::make_unique<PracticeMode>()); // NOTE(): dont move this one [5]
	m_mods.emplace_back(std::make_unique<BulletStop>());
#else
	m_mods.emplace_back(std::make_unique<Mod>());
	m_mods.emplace_back(std::make_unique<Mod>());
	m_mods.emplace_back(std::make_unique<Mod>());
	m_mods.emplace_back(std::make_unique<Mod>());
#endif // !SPEEDRUN_MODE
	m_mods.emplace_back(std::make_unique<UIButton>());
#ifndef SPEEDRUN_MODE
	m_mods.emplace_back(std::make_unique<InputLog>()); //NOTE(): dont move this one [8]
	m_mods.emplace_back(std::make_unique<DebugDraw>()); //NOTE(): dont move this one [9]
    m_mods.emplace_back(std::make_unique<RgTimer>()); //NOTE(): dont move this one [10]
	// m_mods.emplace_back(std::make_unique<EnemySoulEaterNoInvis>());
	m_mods.emplace_back(std::make_unique<EnemyStates>());
	m_mods.emplace_back(std::make_unique<TurnSpeed>());
	m_mods.emplace_back(std::make_unique<EnemyStepCooldown>());
	m_mods.emplace_back(std::make_unique<NoHeightRestriction>());
#else 
	m_mods.emplace_back(std::make_unique<Mod>());
	m_mods.emplace_back(std::make_unique<Mod>());
	m_mods.emplace_back(std::make_unique<Mod>());
	m_mods.emplace_back(std::make_unique<Mod>());
	m_mods.emplace_back(std::make_unique<Mod>());
	m_mods.emplace_back(std::make_unique<Mod>());
	m_mods.emplace_back(std::make_unique<Mod>());
#endif // !SPEEDRUN_MODE

	//m_mods.emplace_back(std::make_unique<StyleSwitcherInfo>()); // crashes half the time on boot, will replace
	m_mods.emplace_back(std::make_unique<EnemySpawnRate>()); // ldk

#ifndef SPEEDRUN_MODE
	m_mods.emplace_back(std::make_unique<AreaJump>());
#else 
	m_mods.emplace_back(std::make_unique<Mod>());
#endif // !SPEEDRUN_MODE


#endif

#ifndef _NDEBUG
	//m_mods.emplace_back(std::make_unique<PrintfDebugging>());
#endif // _DEBUG
	//m_mods.emplace_back(std::make_unique<CameraHack>());
	//m_mods.emplace_back(std::make_unique<YourMod>());
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
}

void Mods::on_draw_debug_ui() const {
	for (auto& mod : m_mods) {
		mod->on_draw_debug_ui();
	}
}

void Mods::on_draw_ui() const {
    for (auto& mod : m_mods) {
        mod->on_draw_ui();
    }
    //g_framework->m_rr->on_draw_ui();
}


void Mods::on_draw_custom_imgui_window() const {
    PracticeMode* p = dynamic_cast<PracticeMode*>(m_mods[6].get()); // epic footguns akimbo
    InputLog* l     = dynamic_cast<InputLog*>(m_mods[9].get());     // epic footguns akimbo part2
    DebugDraw* d    = dynamic_cast<DebugDraw*>(m_mods[10].get());   // epic footguns akimbo part3
    RgTimer* t      = dynamic_cast<RgTimer*>(m_mods[11].get());

    if (p) {
        p->custom_imgui_window();
    }
    if (l) {
        l->custom_imgui_window();
    }
    if (d) {
        d->custom_imgui_window();
    }
    if (t) {
        t->custom_imgui_window();
    }
}