#pragma once

// Game can't use virtual keys unless the menu is open.
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#include <memory>
#include <unordered_map>
#include <vector>

#include <imgui/imgui.h>

#include "sdk/Devil3.hpp"  // NOLINT
#include "sdk/ReClass.hpp" // NOLINT

#include "utility/Config.hpp"
#include "utility/Patch.hpp"

#include "ModFramework.hpp"

class IModValue {
public:
    using Ptr = std::unique_ptr<IModValue>;

    virtual ~IModValue(){};
    virtual bool draw(std::string_view name)             = 0;
    virtual void draw_value(std::string_view name)       = 0;
    virtual void config_load(const utility::Config& cfg) = 0;
    virtual void config_save(utility::Config& cfg)       = 0;
};

// Convenience classes for imgui
template <typename T> class ModValue : public IModValue {
public:
    using Ptr = std::unique_ptr<ModValue<T>>;

    static auto create(std::string_view config_name, T default_value = T{}) {
        return std::make_unique<ModValue<T>>(config_name, default_value);
    }

    ModValue(std::string_view config_name, T default_value)
        : m_config_name{config_name}, m_value{default_value} {}

    virtual ~ModValue() = default; // override {};

    virtual void config_load(const utility::Config& cfg) override {
        auto v = cfg.get<T>(m_config_name);

        if (v) {
            m_value = *v;
        }
    };

    virtual void config_save(utility::Config& cfg) override {
        cfg.set<T>(m_config_name, m_value);
    };

    operator T&() { return m_value; }

    T& value() { return m_value; }

    const auto& get_config_name() const { return m_config_name; }

protected:
    T m_value{};
    std::string m_config_name{"Default_ModValue"};
};

class ModToggle : public ModValue<bool> {
public:
    using Ptr = std::unique_ptr<ModToggle>;

    ModToggle(std::string_view config_name, bool default_value)
        : ModValue<bool>{config_name, default_value} {}

    static auto create(std::string_view config_name,
                       bool default_value = false) {
        return std::make_unique<ModToggle>(config_name, default_value);
    }

    bool draw(std::string_view name) override {
        ImGui::PushID(this);
        auto ret = ImGui::Checkbox(name.data(), &m_value);
        ImGui::PopID();

        return ret;
    }

    void draw_value(std::string_view name) override {
        ImGui::Text("%s: %i", name.data(), m_value);
    }

    bool toggle() { return m_value = !m_value; }
};

class ModFloat : public ModValue<float> {
public:
    using Ptr = std::unique_ptr<ModFloat>;

    ModFloat(std::string_view config_name, float default_value)
        : ModValue<float>{config_name, default_value} {}

    static auto create(std::string_view config_name,
                       float default_value = 0.0f) {
        return std::make_unique<ModFloat>(config_name, default_value);
    }

    bool draw(std::string_view name) override {
        ImGui::PushID(this);
        auto ret = ImGui::InputFloat(name.data(), &m_value);
        ImGui::PopID();

        return ret;
    }

    void draw_value(std::string_view name) override {
        ImGui::Text("%s: %f", name.data(), m_value);
    }
};

class ModSlider : public ModFloat {
public:
    using Ptr = std::unique_ptr<ModSlider>;

    static auto create(std::string_view config_name, float mn = 0.0f,
                       float mx = 1.0f, float default_value = 0.0f) {
        return std::make_unique<ModSlider>(config_name, mn, mx, default_value);
    }

    ModSlider(std::string_view config_name, float mn = 0.0f, float mx = 1.0f,
              float default_value = 0.0f)
        : ModFloat{config_name, default_value}, m_range{mn, mx} {}

    bool draw(std::string_view name) override {
        ImGui::PushID(this);
        auto ret =
            ImGui::SliderFloat(name.data(), &m_value, m_range.x, m_range.y);
        ImGui::PopID();

        return ret;
    }

    void draw_value(std::string_view name) override {
        ImGui::Text("%s: %f [%f, %f]", name.data(), m_value, m_range.x,
                    m_range.y);
    }

    auto& range() { return m_range; }

protected:
    Vector2f m_range{0.0f, 1.0f};
};

class ModInt32 : public ModValue<int32_t> {
public:
    using Ptr = std::unique_ptr<ModInt32>;

    static auto create(std::string_view config_name,
                       uint32_t default_value = 0) {
        return std::make_unique<ModInt32>(config_name, default_value);
    }

    ModInt32(std::string_view config_name, uint32_t default_value = 0)
        : ModValue{config_name, static_cast<int>(default_value)} {}

    bool draw(std::string_view name) override {
        ImGui::PushID(this);
        auto ret = ImGui::InputInt(name.data(), &m_value);
        ImGui::PopID();

        return ret;
    }

    void draw_value(std::string_view name) override {
        ImGui::Text("%s: %i", name.data(), m_value);
    }
};

class ModKey : public ModInt32 {
public:
    using Ptr = std::unique_ptr<ModKey>;

    static auto create(std::string_view config_name,
                       int32_t default_value = UNBOUND_KEY) {
        return std::make_unique<ModKey>(config_name, default_value);
    }

    ModKey(std::string_view config_name, int32_t default_value = UNBOUND_KEY)
        : ModInt32{config_name, static_cast<uint32_t>(default_value)} {}

    bool draw(std::string_view name) override {
        if (name.empty()) {
            return false;
        }

        ImGui::PushID(this);
        ImGui::Button(name.data());

        if (ImGui::IsItemHovered()) {
            WPARAM k = g_framework->get_keyboard_state();
            m_value  = is_erase_key(k) ? UNBOUND_KEY : k;

            ImGui::SameLine();
            ImGui::Text("Press any key");
        } else {
            ImGui::SameLine();

            if (m_value >= 0) {
                ImGui::Text("%i", m_value);
            } else {
                ImGui::Text("Not bound");
            }
        }

        ImGui::PopID();

        return true;
    }

    bool is_key_down() const {
        if (m_value < 1) {
            return false;
        }

        return g_framework->get_keyboard_state() == m_value;
    }

    bool is_key_down_once() {
        auto down = is_key_down();

        if (!m_was_key_down && down) {
            m_was_key_down = true;
            return true;
        }

        if (!down) {
            m_was_key_down = false;
        }

        return false;
    }

    bool is_erase_key(WPARAM k) const {
        switch (k) {
        case VK_ESCAPE:
        case VK_BACK:
            return true;

        default:
            return false;
        }
    }

    static constexpr WPARAM UNBOUND_KEY = 0;

protected:
    bool m_was_key_down{false};
};

class Mod {
protected:
    using ValueList = std::vector<std::reference_wrapper<IModValue>>;

public:
    virtual ~Mod(){};
    virtual std::string_view get_name() const { return "UnknownMod"; };
    // can be used for ModValues, like Mod_ValueName
    virtual std::string generate_name(std::string_view name) {
        return std::string{get_name()} + "_" + name.data();
    }

    static void show_help_marker(const char* desc) {
        ImGui::TextDisabled("(?)");
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
            ImGui::TextUnformatted(desc);
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }
    void install_patch_absolute(uintptr_t location,
                                std::unique_ptr<Patch>& patch,
                                const char* patch_bytes, uint8_t length) {
        spdlog::info("{}: Installing patch at {:x}.\n", get_name(), location);
        patch.reset(nullptr);
        std::vector<int16_t> bytes;
        while (length > 0) {
            bytes.push_back((short)(*patch_bytes) & 0x00FF);
            patch_bytes++;
            length--;
        }
        patch = Patch::create(location, bytes, true);
        // patch.reset(Patch::create_raw(location, bytes, true));
    }

    void install_patch_offset(ptrdiff_t offset, std::unique_ptr<Patch>& patch,
                              const char* patch_bytes, uint8_t length) {
        uintptr_t base     = g_framework->get_module().as<uintptr_t>();
        uintptr_t location = base + offset;
        spdlog::info("{}: Installing patch at {:x}.\n", get_name(), location);
        patch.reset(nullptr);
        std::vector<int16_t> bytes;
        while (length > 0) {
            bytes.push_back((short)(*patch_bytes) & 0x00FF);
            patch_bytes++;
            length--;
        }
        patch = Patch::create(location, bytes, true);
        // patch.reset(Patch::create_raw(location, bytes, true));
    }
    // Wrapper for easy install of hooks.
    // \param offset : offset from game exe base where hook will be installed
    // \param hook : FunctionHook object instance
    // \param detour : Function pointer to your naked detour function.
    // if your detour function is called my_detour then just pass &my_detour
    // \param ret : Pointer to a variable that will get return address
    // \param next_instruction_offset : Optional offset to the next instruction
    // to calculate return address automatically leave this blank to get return
    // address from minhook, note that minhook copies overwritten bytes
    // automatically.
    inline bool install_hook_offset(ptrdiff_t offset,
                                    std::unique_ptr<FunctionHook>& hook,
                                    void* detour, uintptr_t* ret,
                                    ptrdiff_t next_instruction_offset = 0) {
        uintptr_t base     = g_framework->get_module().as<uintptr_t>();
        uintptr_t location = base + offset;
#ifdef _DEBUG
        if (hook) {
            throw std::runtime_error(
                "Install hook called multiple times with same instance\n");
        }
        printf("Installing offset hook at location: %u\n", location);
#endif
        hook = std::make_unique<FunctionHook>(location, detour);
        if (!hook->create()) {
            spdlog::error("Failed to create hook!");
            return false;
        }

        if (next_instruction_offset) {
            *ret = location + next_instruction_offset;
        } else {
            *ret = hook->get_original();
        }
        return true;
    }

    // same deal but using absolute address
    inline bool install_hook_absolute(uintptr_t location,
                                      std::unique_ptr<FunctionHook>& hook,
                                      void* detour, uintptr_t* ret,
                                      ptrdiff_t next_instruction_offset = 0) {
#ifdef _DEBUG
        if (hook) {
            throw std::runtime_error(
                "Install hook called multiple times with same instance\n");
        }
        printf("Installing absolute hook at location: %u\n", location);
#endif
        hook = std::make_unique<FunctionHook>(location, detour);
        if (!hook->create()) {
            spdlog::error("Failed to create hook!");
            return false;
        }

        if (next_instruction_offset) {
            *ret = location + next_instruction_offset;
        } else {
            *ret = hook->get_original();
        }
        return true;
    }

    // Called when ModFramework::initialize finishes in the first render frame
    // Returns an error string if it fails
    virtual std::optional<std::string> on_initialize() { return std::nullopt; };

    // Functionally equivalent, but on_frame will always get called, on_draw_ui
    // can be disabled by ModFramework
    virtual void on_frame(){};
    virtual void on_draw_ui(){};
    virtual void on_draw_debug_ui(){};

    virtual void on_config_load(const utility::Config& cfg){};
    virtual void on_config_save(utility::Config& cfg){};

    // Game-specific callbacks
    /*
    virtual void on_pre_update_transform(RETransform* transform) {};
    virtual void on_update_transform(RETransform* transform) {};
    virtual void on_pre_update_camera_controller(RopewayPlayerCameraController*
    controller) {}; virtual void
    on_update_camera_controller(RopewayPlayerCameraController* controller) {};
    virtual void on_pre_update_camera_controller2(RopewayPlayerCameraController*
    controller) {}; virtual void
    on_update_camera_controller2(RopewayPlayerCameraController* controller) {};
    */
};
