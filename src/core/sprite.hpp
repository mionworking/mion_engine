#pragma once
#include <SDL3/SDL.h>
#include <stb_image.h>
#include <string>
#include <unordered_map>

namespace mion {

// Loads and caches textures using stb_image (PNG, JPG, BMP, TGA).
// One instance per renderer.
class TextureCache {
public:
    explicit TextureCache(SDL_Renderer* r) : _renderer(r) {}

    ~TextureCache() {
        for (auto& [k, tex] : _cache)
            SDL_DestroyTexture(tex);
    }

    // Returns a cached texture or loads from disk.
    // Returns nullptr if the file does not exist — actor renders a rectangle as fallback.
    SDL_Texture* load(const std::string& path) {
        auto it = _cache.find(path);
        if (it != _cache.end()) return it->second;

        int w, h, channels;
        unsigned char* data = stbi_load(path.c_str(), &w, &h, &channels, 4);
        if (!data) {
            SDL_Log("TextureCache: failed to load '%s': %s", path.c_str(), stbi_failure_reason());
            _cache[path] = nullptr;
            return nullptr;
        }

        SDL_Surface* surf = SDL_CreateSurfaceFrom(w, h, SDL_PIXELFORMAT_RGBA32,
                                                  data, w * 4);
        SDL_Texture* tex  = surf ? SDL_CreateTextureFromSurface(_renderer, surf) : nullptr;
        SDL_DestroySurface(surf);
        stbi_image_free(data);

        if (!tex) SDL_Log("TextureCache: failed to create texture '%s'", path.c_str());
        _cache[path] = tex;
        return tex;
    }

private:
    SDL_Renderer* _renderer;
    std::unordered_map<std::string, SDL_Texture*> _cache;
};

// A sprite frame: a crop of a spritesheet or a full texture.
struct SpriteFrame {
    SDL_Texture* texture = nullptr;
    SDL_Rect     src     = {0, 0, 0, 0};  // {0,0,0,0} = full texture
    float        pivot_x = 0.5f;
    float        pivot_y = 0.5f;
};

// Renders a SpriteFrame centered at (screen_x, screen_y).
inline void draw_sprite(SDL_Renderer* r,
                        const SpriteFrame& frame,
                        float screen_x, float screen_y,
                        float scale_x = 1.0f, float scale_y = 1.0f,
                        bool  flip_h  = false)
{
    if (!frame.texture) return;

    float tw, th;
    SDL_GetTextureSize(frame.texture, &tw, &th);

    float sw = (frame.src.w > 0) ? (float)frame.src.w : tw;
    float sh = (frame.src.h > 0) ? (float)frame.src.h : th;

    SDL_FRect dst = {
        screen_x - sw * scale_x * frame.pivot_x,
        screen_y - sh * scale_y * frame.pivot_y,
        sw * scale_x,
        sh * scale_y
    };

    SDL_FlipMode flip = flip_h ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;

    if (frame.src.w > 0) {
        SDL_FRect src_f = { (float)frame.src.x, (float)frame.src.y,
                            (float)frame.src.w, (float)frame.src.h };
        SDL_RenderTextureRotated(r, frame.texture, &src_f, &dst, 0.0, nullptr, flip);
    } else {
        SDL_RenderTextureRotated(r, frame.texture, nullptr, &dst, 0.0, nullptr, flip);
    }
}

} // namespace mion
