#include <pkmpch.h>

#include <unistd.h>

#include "wsclient.h"
#include "util/json_loader.h"
#include "net/sslcontext.h"

namespace pkm::net {
    
    WsClient::WsClient(const NetConfig& config) : m_ioc(), m_config(config), m_websocket(nullptr), m_strand(boost::asio::make_strand(m_ioc)) {
        
        SSLContext& ssl_ctx = SSLContext::get();
        ssl_ctx.init();
        
        auto& [port, host, ws_path] = m_config;

        auto results = pkm::net::resolve(m_ioc, host, port);
        if (!ok(results)) {
            PK_ERROR("Could not initialize Client, please retry!");
        } else {
            m_endpoints = value(results);

            m_websocket = MakeScope<WssStream>(m_ioc, ssl_ctx.native_ctx()); 
            SSL_set_tlsext_host_name(m_websocket->next_layer().native_handle(), host.c_str());
        }
        
    }

    WsClient::~WsClient() {
    }
    
    bool WsClient::connect() {
        return ::pkm::net::connect(*m_websocket, m_endpoints, m_config.ps_server_url, m_config.ps_websocket_path);
    }

    void WsClient::send(const std::string& message) {
        std::string formatted_msg = message;
        if (formatted_msg.back() != '\n') formatted_msg += '\n';

        boost::asio::post(m_strand, [this, formatted_msg]() {
            
            bool write_in_progress = !m_internal_write_queue.empty();
            m_internal_write_queue.push_back(formatted_msg);

            if (!write_in_progress) {
                do_write();
            }
        });
    }

    void WsClient::do_write() {
        // write the first thing in our internal queue
        m_websocket->async_write(
            boost::asio::buffer(m_internal_write_queue.front()),
            boost::asio::bind_executor(m_strand, [this](BoostErr ec, std::size_t) {
                if (!ec) {
                    m_internal_write_queue.pop_front();
                    
                    if (!m_internal_write_queue.empty()) {
                        do_write();
                    }
                } else {
                    PK_ERROR("Write error: {}", ec.message());
                }
            })
        );
    }

    std::string WsClient::receive() {
        BoostReadBuffer buf;
        m_websocket->read(buf);
        std::string msg = boost::beast::buffers_to_string(buf.data());
        return msg;
    }

    void WsClient::close() {
        if (!m_websocket) return;

        BoostErr ec;

        if (!m_websocket->is_open()) {
            PK_INFO("WebSocket already closed");
            return;
        }

        m_websocket->close(boost::beast::websocket::close_code::normal, ec);

        if (!ec) {
            PK_INFO("WebSocket closed successfully with proper handshake.");
            return;
        }
        if (ec == boost::beast::websocket::error::closed ||
            ec == boost::asio::error::eof ||
            ec.message() == "stream truncated")
        {
            PK_INFO("WebSocket already closed or server disconnected before handshake.");
            return;
        }

        PK_ERROR("WebSocket close failed: {}", ec.message());
       
        auto& sslStream   = m_websocket->next_layer();  
        auto& basicStream = sslStream.next_layer();    
        auto& tcpSocket   = basicStream.socket();      

        ::close(tcpSocket.native_handle());

        PK_INFO("WebSocket forcibly closed via native handle."); 

    }

}
