#pragma once 

#include <filesystem>
#include <string_view>

namespace pkm {
    
    constexpr std::string_view ROOT_DIR = PROJECT_ROOT;
    
    inline std::filesystem::path config_path() {
        return std::filesystem::path(ROOT_DIR) / "config" / "config.json";
    }
}
