#pragma once

#include "message.h"
#include "ident.h"
#include <string_view>
#include <vector>

namespace pkm::protocol {
    
    std::vector<Message> parse_message(std::string_view raw);
    Ident parse_ident(std::string_view ident);

}