#pragma once

#include "core/event/event.h"
namespace pkm {

    class Input {
    public:
        using EventCallbackFn = std::function<void(Event&)>;

        virtual ~Input() = default;
        virtual void start() = 0;
        virtual void stop() = 0;
        virtual void set_callback(const EventCallbackFn& callback) = 0;
        
        // called when the app needs input from the user
        // context is whatever the caller wants to pass - could be anything
        virtual void request(const std::string& prompt) = 0;
        
    };

}