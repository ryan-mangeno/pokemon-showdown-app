#pragma once

#include <memory>
#include <string_view>

namespace pkm {
    
    template <typename T>
    using Ref = std::shared_ptr<T>;

    template <typename T, typename... Args> 
    Ref<T> MakeRef(Args&&... args) {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

    template <typename T>
    using Scope = std::unique_ptr<T>;

    template <typename T, typename... Args> 
    Scope<T> MakeScope(Args&&... args) {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }
    
    // for loading json for different config files
    template<typename Derived>
    struct Config { 
        auto as_tuple() {
            return static_cast<Derived*>(this)->as_tuple_impl();
        }
    };
    
} // namespace pkm
