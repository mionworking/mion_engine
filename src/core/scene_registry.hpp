#pragma once

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

#include "scene.hpp"
#include "scene_context.hpp"
#include "scene_ids.hpp"

namespace mion {

// Registry de cenas: mapeia SceneId → factory.
// Strings ficam encapsuladas aqui; o resto do código usa SceneId.
class SceneRegistry {
public:
    using Factory = std::function<std::unique_ptr<IScene>(const SceneCreateContext&)>;

    void register_scene(SceneId id, Factory factory) {
        _factories[to_string(id)] = std::move(factory);
    }

    std::unique_ptr<IScene> create(SceneId id,
                                   const SceneCreateContext& ctx) const {
        auto it = _factories.find(to_string(id));
        if (it == _factories.end()) return nullptr;
        return it->second(ctx);
    }

private:
    std::unordered_map<std::string, Factory> _factories;
};

} // namespace mion
