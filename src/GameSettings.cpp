#include "GameSettings.h"

#include <toml++/toml.hpp>

#include "Util/TOML.h"

namespace
{
    void SetSetting(RE::Setting* setting, const std::string& name, const toml::node& node)
    {
        if (setting) {
            switch (node.type()) {
            case toml::node_type::boolean:
                {
                    bool value = *node.value<bool>();
                    setting->data.b = value;
                    SKSE::log::info("Set {} = {}", name, value);
                    break;
                }
            case toml::node_type::integer:
                {
                    int32_t value = *node.value<int32_t>();
                    setting->data.i = value;
                    SKSE::log::info("Set {} = {}", name, value);
                    break;
                }
            case toml::node_type::floating_point:
                {
                    float value = *node.value<float>();
                    setting->data.f = value;
                    SKSE::log::info("Set {} = {}", name, value);
                    break;
                }
            case toml::node_type::string:
                {
                    // NOTE: Does this cause a memory leak��
                    auto free_str = new std::string{ std::move(*node.value<std::string>()) };
                    setting->data.s = free_str->data();
                    SKSE::log::info("Set {} = {}", name, *free_str);
                    break;
                }
            default:
                SKSE::log::warn("Unknown data type for {}", name);
                break;
            }
        } else {
            SKSE::log::warn("Unknown setting for {}", name);
        }
    }

    void LoadFromOneFile(const std::string& path)
    {
        auto collection = RE::GameSettingCollection::GetSingleton();

        auto data = toml::parse_file(path);
        for (auto& [key, value] : data) {
            std::string name{ key.str() };
            auto        setting = collection->GetSetting(name.c_str());
            SetSetting(setting, name, value);
        }
    }
}

void GameSettings::Load()
{
    if (!std::filesystem::exists(root)) {
        return;
    }

    for (auto& entry : std::filesystem::directory_iterator{ root }) {
        if (!entry.is_regular_file()) {
            continue;
        }

        auto& path = entry.path();
        if (path.extension().native() != L".toml"sv) {
            continue;
        }

        auto path_str = path.generic_string();
        try {
            LoadFromOneFile(path_str);
            SKSE::log::info("Successfully loaded \"{}\".", path_str);
        } catch (const toml::parse_error& e) {
            auto msg = std::format("Failed to load \"{}\" (error occurred at line {}, column {}): {}.", path_str,
                e.source().begin.line, e.source().begin.column, e.what());
            SKSE::stl::report_and_fail(msg);
        } catch (const std::exception& e) {
            auto msg = std::format("Failed to load \"{}\": {}.", path_str, e.what());
            SKSE::stl::report_and_fail(msg);
        }
    }
}

const std::filesystem::path GameSettings::root{ "Data/SKSE/Plugins/ccld_GameSettingsOverride"sv };
