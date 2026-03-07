#pragma once

#include <string>
#include <vector>

namespace pkm::protocol {
    
    struct Message {
        std::string room_id;
        std::string type;
        std::vector<std::string> args; 
        
        const std::string& arg(size_t i) noexcept {
            static const std::string empty{""};
            if (i < 0 || i >= args.size()) {
                return empty;
            } else {
                return args[i];
            }
        }
    };

}
