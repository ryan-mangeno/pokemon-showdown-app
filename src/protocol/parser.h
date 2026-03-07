#pragma once

#include <string>
#include <vector>

#include "message.h"

namespace pkm::protocol {
   
    std::vector<Message> parse(const std::string& raw);

}
