#pragma once

#include "util.h"

#include <core/defines.h>
#include <string>

#include <nlohmann/json.hpp>
#include <fstream>
#include <tuple>
#include <core/logger.h>


namespace pkm {

    class JsonLoader {
    public:

        // Config is base class to load config types
        // derived classes must define std::tie method
        template <typename ConfigT>
        static void load(ConfigT& cfg, const char* pth) {
            std::ifstream file(pth);
            if (!file.is_open()) {
                PK_ERROR("Could not open {} during json load!", pth);
                return;
            }
            
            nlohmann::json j;
            file >> j;
            
            std::vector<nlohmann::json> values;
            for (auto& [key, value] : j.items()) {
                values.emplace_back(value);
            }
            
            size_t index = 0;
    
            std::apply([&](auto&&... args){
                ((args = values.at(index++).get<std::decay_t<decltype(args)>>()), ...);
            }, cfg.as_tuple());
    
        }
    };

}
