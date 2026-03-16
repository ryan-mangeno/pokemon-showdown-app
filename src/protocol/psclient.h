#pragma once

#include "net/wsclient.h"
#include "core/thread/queue.h"
#include "core/defines.h"
#include "message.h"

#include <thread>
namespace pkm::protocol {

    class PsClient {
        public:
            PsClient();
            ~PsClient() = default;

            bool init();
            void shutdown();
            void run();

            void send(const std::string& msg);

            // set by app
            std::function<void(const protocol::Message&)> on_message;
        
        private:
            void dispatch(const Message& msg);
            void on_update_user(const Message& msg);
            void on_chall_str(const Message& msg);
            void on_update_search(const Message& msg);
            void on_battle(const Message& msg);
            void on_win(const Message& msg);
            void on_request(const Message& msg);

            void network_loop();

        private:
            Ref<pkm::net::WsClient> m_ws;
            
            bool m_initialized;
            bool m_searching;
            // TODO: both should be atomic + handle setting logic
            bool m_connected;
            bool m_in_battle;
            

            std::string m_battle_room;

            // Network thread  ->  SPSCQueue<Message>   ->  Main thread   (inbound)
            // Main thread     ->  SPSCQueue<string>    ->  Network thread (outbound)
            pkm::core::SPSCQueue<Message>      m_inbound{256};
            pkm::core::SPSCQueue<std::string>  m_outbound{64};
            std::thread m_network_thread;

    };
    

}
