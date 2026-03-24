#pragma once

#include "net/connection.h"
#include "net/sslcontext.h"
#include "net/types.h"
#include "net/netconfig.h"

#include "util/json_loader.h"

#include "core/defines.h"
#include "net/connection.h"
#include "core/thread/queue.h"

#include <boost/asio/strand.hpp>
namespace pkm::net {
    
    class WsClient {
        using ResolverResults = boost::asio::ip::tcp::resolver::results_type;
        using IOc = boost::asio::io_context;
        using BoostErr = boost::beast::error_code;
        using BoostWriteBuffer = boost::asio::const_buffer;
        using BoostReadBuffer = boost::beast::flat_buffer;
        using BoostStrand = boost::asio::strand<boost::asio::io_context::executor_type>;

        public:
            WsClient(const NetConfig& config);
            ~WsClient();

            bool connect();
            void send(const std::string& message);
            std::string receive();
            void close();

            inline IOc& get_ioc() { return m_ioc; }
            inline bool is_open() { return m_websocket->is_open(); }

            template <typename T, typename ParseFunc>
            void start_read_loop(pkm::SPSCQueue<T>& inbound_queue, ParseFunc parser) {
                m_websocket->async_read(m_read_buf, 
                    boost::asio::bind_executor(m_strand, 
                    [this, &inbound_queue, parser](BoostErr ec, std::size_t) {
                        if (!ec) {
                            std::string raw_msg = boost::beast::buffers_to_string(m_read_buf.data());
                            m_read_buf.consume(m_read_buf.size());

                            auto msgs = parser(raw_msg);
                            
                            for (auto& m : msgs) {
                                inbound_queue.push(m); 
                            }

                            start_read_loop(inbound_queue, parser); 
                        } else {
                            if (ec == boost::beast::websocket::error::closed) {
                                PK_INFO("WebSocket closed cleanly by server.");
                            } else {
                                PK_ERROR("Read error: {}", ec.message());
                            }
                        }
                    }));
            }
        
        private:
            void do_write();

        private:
            IOc m_ioc;
            Scope<WssStream> m_websocket;
            NetConfig m_config;
            ResolverResults m_endpoints;
            BoostStrand m_strand;

            // internal buffer and queue Asio uses
            BoostReadBuffer m_read_buf;
            std::deque<std::string> m_internal_write_queue;

            // TODO:
            // bool m_has_cached_endpoints;
    };


}
