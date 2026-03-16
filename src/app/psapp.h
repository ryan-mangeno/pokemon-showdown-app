#pragma once

#include "protocol/psclient.h"
#include "protocol/message.h"
#include "input/input.h"
#include "protocol/battlestate.h"

#include <string>

namespace pkm {

    class PsApp {
    public:
        PsApp();
        ~PsApp() = default;

        bool init();
        void run();
        void shutdown();

    private:
        void on_message(const protocol::Message& msg);
        void on_battle_request(const protocol::Message& msg);

    private:
        Scope<protocol::PsClient>   m_client;
        Scope<Input>                m_input;
        protocol::BattleState       m_state;

        std::string m_battle_room;
        bool m_running;
    };

}