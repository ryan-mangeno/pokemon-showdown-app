#pragma once 

#include <filesystem>
#include <string_view>

namespace pkm {
    // project root is made during cmake build 
    constexpr std::string_view ROOT_DIR = PROJECT_ROOT;
    
}
