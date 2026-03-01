#pragma once

#include <core/defines.h>

#include <pkmpch.h>

namespace pkm {
    
    class SSLContext {
        public:

            inline static SSLContext& get() {
                static SSLContext instance;
                return instance;
            }

            inline boost::asio::ssl::context& native_ctx() {
                return *m_client;
            }

            bool init();
            bool shutdown();

        private:
            SSLContext() = default;
            ~SSLContext() = default;

        private:
            bool m_initialized = false;
            Scope<boost::asio::ssl::context> m_client;

    };


} // namespace pkm
