#pragma once

#include <string>

namespace pkm::net {
    /*
    WebSocket (Inbound): Server gives challstr
    HTTPS (Outbound/Inbound): You send credentials + challstr to the Login Server; it gives you an assertion
    WebSocket (Outbound): You send the assertion back to the original server to  verify identity
    */
    std::string request_assertion(const std::string& username, 
                                  const std::string& password, 
                                  const std::string& challstr);

} // namespace pkm::net
