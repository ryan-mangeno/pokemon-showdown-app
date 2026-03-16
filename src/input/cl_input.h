#pragma once

#include "input.h"

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>

namespace pkm {

    class CLInput : public Input {
    public:
        CLInput(std::function<void(const std::string&)> on_response);
        ~CLInput();
        
        void start() override;
        void stop() override;
        void request(const std::string& prompt) override;

    private:
        void run();

        std::function<void(const std::string&)> m_on_response;
        
        std::thread m_thread;
        std::mutex m_mutex;
        std::condition_variable m_cv;
        std::atomic<bool> m_running{false};
        std::atomic<bool> m_requested{false};
        std::string m_prompt;
    };
}