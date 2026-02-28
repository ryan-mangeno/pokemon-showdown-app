#pragma once

#include <core/defines.h>

namespace pkm {
    
    class SSLContext {
        public:
        
            static Ref<SSLContext>& get() {
               if (!s_initialized) {
                    // TODO: logging 
               } 
            }

            bool init();
            bool shutdown();

        private:
            SSLContext();
            
            Ref<SSLContext> m_ctx = nullptr;
            bool s_initialized = false;

    };


} // namespace pkm
