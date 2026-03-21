#include <pkmpch.h>

#include "cl_input.h"
#include "core/logger.h"
#include "core/event/key_event.h"

#include <iostream>

namespace pkm {

    CLInput::CLInput()
        : m_prompt(""), m_running(false) {}

    void CLInput::start() {
        m_running = true;
        m_thread = std::thread(&CLInput::run, this);
    }

    void CLInput::stop() {
        m_running = false;
        m_cv.notify_all();
        if (m_thread.joinable()) m_thread.join();
    }

    void CLInput::request(const std::string& prompt) {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_prompt = prompt;
            m_requested = true;
        }
        m_cv.notify_one();
    }

    void CLInput::run() {
        while (m_running) {
            // sleep until request arrives
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cv.wait(lock, [this] {
                return m_requested.load() || !m_running.load();
            });

            if (!m_running) break;

            std::string prompt = m_prompt;
            m_requested = false;
            lock.unlock();

            // prompt user and read response
            PK_INFO("{}", prompt.c_str());
            char c;
            while (std::cin.get(c)) {
                if (m_callback) {
                    KeyTypedEvent e(static_cast<int>(c));
                    m_callback(e);
                }
                // stop reading on enter, wait for next request
                if (c == '\n') break;
            }
        }
    }
}