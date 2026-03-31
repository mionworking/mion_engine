#pragma once

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

#include "scene.hpp"
#include "scene_context.hpp"

namespace mion {

class SceneRegistry {
public:
    using Factory = std::function<std::unique_ptr<IScene>(const SceneCreateContext&)>;

    void register_scene(std::string id, Factory factory) {
        _factories[std::move(id)] = std::move(factory);
    }

    std::unique_ptr<IScene> create(const std::string& id,
                                   const SceneCreateContext& ctx) const {
        auto it = _factories.find(id);
        if (it == _factories.end()) return nullptr;
        return it->second(ctx);
    }

private:
    std::unordered_map<std::string, Factory> _factories;
};

} // namespace mion
