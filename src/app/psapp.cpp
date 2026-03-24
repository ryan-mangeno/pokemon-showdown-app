#include <pkmpch.h>

#include "psapp.h"
#include "protocol/parser.h"
#include "input/cl_input.h"
#include "core/logger.h"
#include "core/event/key_event.h"

namespace pkm {

    PsApp::PsApp() : m_running(false) {}

    bool PsApp::init() {
        m_client = MakeScope<protocol::PsClient>();
        if (!m_client->init()) {
            PK_ERROR("Failed to initialize PsClient!");
            return false;
        }

        m_input = MakeScope<CLInput>();
        m_input->set_callback ( [this](Event& e) {
            EventDispatcher dispatcher(e);
            dispatcher.Dispatch<KeyTypedEvent>([this](KeyTypedEvent& e) {
                char c = e.get_keycode();
                if (c == '\n') {
                    // submit buffer
                    m_client->send(m_battle_room + "|/choose move " + m_input_buffer);
                    m_input_buffer.clear();
                } else if (c == 127) {
                    // backspace
                    if (!m_input_buffer.empty()) m_input_buffer.pop_back();
                } else {
                    m_input_buffer += c;
                }
                return true;
            });
        } );
        
        // TODO: change to event system like input
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
        m_input->stop();
        m_client->shutdown();
        m_running = false;
    }

    void PsApp::on_message(const protocol::Message& msg) {
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