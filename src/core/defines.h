#pragma once

#include <memory>

namespace pkm {
    
    template <typename T>
    using Ref<T> = std::shared_ptr<T>;

    template <typename T, typename... Args> 
    Ref<T> MakeRef(Args&& ...) {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

} // namespace pkm
