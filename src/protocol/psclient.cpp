#include <pkmpch.h>

#include "psclient.h"
#include "util/json_loader.h"
#include "core/logger.h"
#include "net/netconfig.h"
#include "net/wsclient.h"
#include "protocol/parser.h"

#include <nlohmann/json.hpp>

namespace pkm::protocol {

    PsClient::PsClient() : m_initialized(false), m_in_battle(false), m_searching(false), m_battle_room(""), m_connected(false), m_ws(nullptr) {}

    bool PsClient::init() {
        if (m_initialized) {
            PK_WARN("Already connected, no need for initializion twice");
            return true;
        }
        
        pkm::net::NetConfig ncfg;
        pkm::JsonLoader::load(ncfg, NET_CONFIG_PATH.c_str());
        m_ws = MakeRef<pkm::net::WsClient>(ncfg);
        
        if(m_ws) {
            m_initialized = true;
            return true;
        } else {
            PK_ERROR("Failed to created websocket client!");
            return false;
        }
    }

    void PsClient::shutdown() {
        m_ws->close();   
        m_connected = false;
    }

    void PsClient::run() {

        if (!m_ws->connect()) {
            PK_ERROR("Failed to run Client!");
            return;
        }
        
        m_connected = true;
        m_network_thread = std::thread(&PsClient::network_loop, this);

        while (m_connected) {
            Message msg;
            if (m_inbound.pop(msg)) {
                if (on_message) on_message(msg);
                PK_INFO("[INBOUND FROM SERVER] Type: {}", msg.type);
                dispatch(msg);
            }
        }

        m_network_thread.join();
    }
    
    void PsClient::dispatch(const Message& msg) {
        if (msg.type == "updateuser")        on_update_user(msg);
        else if (msg.type == "challstr")     on_chall_str(msg);
        else if (msg.type == "updatesearch") on_update_search(msg);
        else if (msg.type == "b")            on_battle(msg);
        else if (msg.type == "request")      on_request(msg);
        else if (msg.type == "win")          on_win(msg);
    }

    void PsClient::on_update_user(const Message& msg) {
        PK_INFO("Username: {}", msg.args[0]);
    }

    void PsClient::on_chall_str(const Message& msg) {
        if (!m_in_battle && !m_searching) {
            PK_INFO("Session verified by challstr. Searching for match...");
            m_searching = true;
            send("|/search gen9randombattle");
        }
    }

    void PsClient::on_update_search(const Message& msg) {
        if (msg.args[0].empty()) return;
        
        auto j = nlohmann::json::parse(msg.args[0]);
        
        if (j.contains("games") && !j["games"].is_null() && !m_in_battle) {
            m_battle_room = j["games"].begin().key();
            m_in_battle = true;
            m_searching = false;
            PK_INFO("Battle room assigned: {}", m_battle_room);
            send("|/join " + m_battle_room);
        } 
    }

    void PsClient::on_battle(const Message& msg) {
        m_battle_room = msg.args[0];
        send("|/join " + m_battle_room);
    }

    void PsClient::on_win(const Message& msg) {
        PK_INFO("Winner: {}", msg.args[0]);
        m_connected = false;
    }

    void PsClient::on_request(const Message& msg) {
    }

    void PsClient::network_loop() {
        PK_INFO("Network thread started.");
        while (m_connected) {
            std::string out;
            while (m_outbound.pop(out)) {
                PK_TRACE("OUTBOUND >>> {}", out);
                m_ws->send(out);
            }

            auto raw = m_ws->receive();
            if (raw.empty()) { continue;  }
            
            PK_TRACE("INBOUND <<< raw data received ({} bytes)", raw.length());

            auto msgs = protocol::parse_message(raw);
            for (auto& msg : msgs) {
                PK_TRACE("Parsed Message: type={}", msg.type);
                m_inbound.push(msg);
            }
        }
    }

    void PsClient::send(const std::string& msg) {
        // TODO: optimize
        std::string msg_cpy = msg;
        if (!m_outbound.push(msg_cpy)) {
            PK_ERROR("FAILED TO PUSH TO OUTBOUND QUEUE! Queue might be full. Command: {}", msg);
        } 
    }

}
