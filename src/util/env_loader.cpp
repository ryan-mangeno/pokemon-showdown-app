#include <pkmpch.h>

#include <fstream>
#include <string>
#include <cstdlib> // for setenv

#include "core/logger.h"

namespace pkm {
    void load_env(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            PK_ERROR("Could not open .env file at {}", path);
            return;
        }

        std::string line;
        while (std::getline(file, line)) {
            // Ignore comments and empty lines
            if (line.empty() || line[0] == '#') continue;

            auto delimiter_pos = line.find('=');
            if (delimiter_pos != std::string::npos) {
                std::string key = line.substr(0, delimiter_pos);
                std::string value = line.substr(delimiter_pos + 1);
                
                // setenv(key, value, overwrite)
                setenv(key.c_str(), value.c_str(), 1);
                PK_TRACE("Loaded Env Var: {}", key);
            }
        }
    }
}