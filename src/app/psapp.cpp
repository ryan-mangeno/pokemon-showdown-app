#include <pkmpch.h>

#include "psapp.h"
#include "protocol/parser.h"
#include "input/cl_input.h"
#include "core/logger.h"

namespace pkm {

    PsApp::PsApp() : m_running(false) {}

    bool PsApp::init() {
        m_client = MakeScope<protocol::PsClient>();
        if (!m_client->init()) {
            PK_ERROR("Failed to initialize PsClient!");
            return false;
        }

        m_input = MakeScope<CLInput>([this](const std::string& response) {
            // TODO: temporary to get input and cli working
            PK_INFO("Input from app: {}", response);
            m_client->send(m_battle_room + "|/choose move " + response);
        });

        m_client->on_message = [this](const protocol::Message& msg) {
            on_message(msg);
        };

        m_input->start();
        m_running = true;
        return true;
    }

    void PsApp::run() {
        m_client->run();
    }

    void PsApp::shutdown() {
        m_running = false;
        m_input->stop();
        m_client->shutdown();
    }

    void PsApp::on_message(const protocol::Message& msg) {
        PK_INFO("PsApp on_message: {}", msg.type);
        m_state.apply(msg);
        if (msg.type == "request") {
            on_battle_request(msg);
        } else if (msg.type == "updatesearch" && !msg.args[0].empty()) {
            auto j = nlohmann::json::parse(msg.args[0]);
            if (!j["games"].is_null()) {
                m_battle_room = j["games"].begin().key();
            }
        }
    }

    void PsApp::on_battle_request(const protocol::Message& msg) {
        // TODO: formatting for now
        std::string prompt = "Your turn!\n";
        auto& moves = m_state.available_moves();
        for (size_t i = 0; i < moves.size(); i++) {
            prompt += "[" + std::to_string(i+1) + "] " + moves[i].name;
            prompt += " (" + std::to_string(moves[i].pp) + "/" + 
                    std::to_string(moves[i].max_pp) + " pp)";
            if (moves[i].disabled) prompt += " [DISABLED]";
            prompt += "\n";
        }
        m_input->request(prompt);
    }
}