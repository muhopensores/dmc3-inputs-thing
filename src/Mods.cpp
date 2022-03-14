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
#include "mods/GamepadsFix.hpp" // seems broken
#include "mods/InputLog.hpp"
#include "mods/AudioStutterFix.hpp"
#include "mods/PrintfDebugging.hpp"
#include "mods/DebugDraw.hpp"
#include "mods/NoHeightRestriction.hpp"
#include "mods/EnemySoulEaterNoInvis.hpp"
//#include "mods/StyleSwitcherInfo.hpp"
//#include "mods/CameraHack.hpp"


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
	//m_mods.emplace_back(std::make_unique<YourMod>());
}

void Mods::load_mods() {
	m_mods.emplace_back(std::make_unique<QuicksilverShader>());
	m_mods.emplace_back(std::make_unique<AudioStutterFix>());
	m_mods.emplace_back(std::make_unique<InertiaThings>());
	m_mods.emplace_back(std::make_unique<StyleSwitchFX>());
	m_mods.emplace_back(std::make_unique<PracticeMode>()); // NOTE(): dont move this one
	m_mods.emplace_back(std::make_unique<BulletStop>());
	m_mods.emplace_back(std::make_unique<UIButton>());
	m_mods.emplace_back(std::make_unique<InputLog>()); //NOTE(): dont move this one
	m_mods.emplace_back(std::make_unique<DebugDraw>());
	m_mods.emplace_back(std::make_unique<NoHeightRestriction>());
	m_mods.emplace_back(std::make_unique<EnemySoulEaterNoInvis>());
	//m_mods.emplace_back(std::make_unique<StyleSwitcherInfo>()); // crashes half the timeon boot, will replace

#ifdef _DEBUG
	m_mods.emplace_back(std::make_unique<PrintfDebugging>());
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
}

void Mods::on_draw_custom_imgui_window() const {
	PracticeMode* p = dynamic_cast<PracticeMode*>(m_mods[5].get()); // epic footguns akimbo
	InputLog* l = dynamic_cast<InputLog*>(m_mods[8].get()); // epic footguns akimbo part2
	DebugDraw* d = dynamic_cast<DebugDraw*>(m_mods[9].get()); // epic footguns akimbo part3
	p->custom_imgui_window();
	l->custom_imgui_window();
	d->custom_imgui_window();
}