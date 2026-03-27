#include <pkmpch.h>

#include "psapp.h"
#include "input/cl_input.h"
#include "core/logger.h"
#include "core/event/message_event.h"
#include "core/event/command_event.h"
#include "core/event/key_event.h"

#include <nlohmann/json.hpp>

namespace pkm {

    PsApp::PsApp() : m_running(false), m_in_battle(false) {}

    bool PsApp::init() {
        m_client = MakeRef<protocol::PsClient>();
        if (!m_client->init()) {
            PK_ERROR("Failed to initialize PsClient!");
            return false;
        }

        // MenuLayer is always at the bottom
        // TODO: need to call on attach for all layers
        m_layerstack.push_layer(new MenuLayer(m_client));

        // input thread: only pushes raw strings to queue, never touches layers
        m_input = MakeScope<CLInput>();
        m_input->set_callback([this](Event& e) {
            EventDispatcher dispatcher(e);
            dispatcher.Dispatch<CommandEvent>([this](CommandEvent& e) {
                std::string cmd = e.get_command();
                m_input_queue.push(cmd);
                return true;
            });
        });

        m_input->start();
        m_running = true;
        return true;
    }

    void PsApp::run() {
        m_client->start();

        while (m_running) {
            process_network();
            process_input();
            std::this_thread::yield();
        }

        m_client->stop();
    }

    void PsApp::shutdown() {
        m_running = false;
        m_input->stop();
    }

    void PsApp::process_network() {
        protocol::Message msg;
        while (m_client->poll(msg)) {
            on_network_message(msg);
            MessageEvent e(msg);
            push_to_layers(e);
        }
    }

    void PsApp::process_input() {
        std::string cmd;
        while (m_input_queue.pop(cmd)) {
            PK_INFO("[App] Got command: '{}'", cmd);

            if (cmd == "q" || cmd == "quit") {
                PK_INFO("Quitting...");
                m_running = false;
                return;
            }

            CommandEvent e{cmd};
            push_to_layers(e);
        }
    }

    void PsApp::push_to_layers(Event& e) {
        // dispatch top-down, stop if handled
        for (auto it = m_layerstack.end(); it != m_layerstack.begin();) {
            (*(--it))->on_event(e);
            if (e.get_handled()) break;
        }
    }

    void PsApp::on_network_message(const protocol::Message& msg) {
        // PsApp handles structural decisions, push/pop layers
        // everything else goes to layers via MessageEvent

        if (msg.type == "updatesearch" && !msg.args.empty()) {
            auto j = nlohmann::json::parse(msg.args[0]);
            if (!j["games"].is_null() && !m_in_battle) {
                std::string room = j["games"].begin().key();
                m_in_battle = true;
                m_client->send("|/join " + room);

                // push battle layer on top
                m_battle_layer = new BattleLayer(m_client, room);
                m_layerstack.push_layer(m_battle_layer);
            }
        } else if (msg.type == "win" || msg.type == "tie") {
            if (m_battle_layer) {
                m_layerstack.pop_layer(m_battle_layer);
                m_battle_layer = nullptr;
                m_in_battle = false;
                PK_INFO("[App] Battle ended, returning to menu");
            }
        }
    }
}