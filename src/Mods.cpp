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
//#include "mods/GamepadTriggersFix.hpp" // seems broken
#include "mods/InputLog.hpp"

Mods::Mods()
{
    //m_mods.emplace_back(std::make_unique<SimpleMod>());
	m_mods.emplace_back(std::make_unique<QuicksilverShader>());
	m_mods.emplace_back(std::make_unique<InertiaThings>());
	m_mods.emplace_back(std::make_unique<StyleSwitchFX>());
	m_mods.emplace_back(std::make_unique<PracticeMode>()); // NOTE(): dont move this one
	m_mods.emplace_back(std::make_unique<BulletStop>());
	m_mods.emplace_back(std::make_unique<UIButton>());
	m_mods.emplace_back(std::make_unique<InputLog>());
	//m_mods.emplace_back(std::make_unique<GamepadTriggersFix>()); seems broken
    //m_mods.emplace_back(std::make_unique<YourMod>());

#ifdef DEVELOPER
    m_mods.emplace_back(std::make_unique<DeveloperTools>()); // you wish
#endif
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
	PracticeMode* p = dynamic_cast<PracticeMode*>(m_mods[3].get()); // epic footguns akimbo
	InputLog* l = dynamic_cast<InputLog*>(m_mods[6].get()); // epic footguns akimbo part2
	p->custom_imgui_window();
	l->custom_imgui_window();
}