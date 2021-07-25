#pragma once

#include "Mod.hpp"

class Mods {
public:
    Mods();
    virtual ~Mods() {}

    std::optional<std::string> on_initialize() const;

    void on_frame() const;
    void on_draw_ui() const;
    void on_draw_debug_ui() const;
	void on_draw_custom_imgui_window() const;

    const auto& get_mods() const {
        return m_mods;
    }

private:
    std::vector<std::unique_ptr<Mod>> m_mods;
};