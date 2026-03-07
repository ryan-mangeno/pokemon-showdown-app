#pragma once


#include "net/connection.h"
#include "net/sslcontext.h"
#include "net/types.h"
#include "net/netconfig.h"

#include "util/json_loader.h"

#include "core/defines.h"
#include "net/connection.h"

namespace pkm::net {
    
    class WsClient {
        using ResolverResults = boost::asio::ip::tcp::resolver::results_type;
        using IOc = boost::asio::io_context;
        using BoostErr = boost::beast::error_code;
        using BoostWriteBuffer = boost::asio::const_buffer;
        using BoostReadBuffer = boost::beast::flat_buffer;

        public:
            WsClient(const NetConfig& config);
            ~WsClient();

            bool connect();
            void send(const std::string& message);
            std::string receive();
            void close();
            
        private:
            IOc m_ioc;
            Scope<WssStream> m_websocket;
            NetConfig m_config;
            ResolverResults m_endpoints;

            // TODO:
            // bool m_has_cached_endpoints;
    };


}
