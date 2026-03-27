#pragma once
#include <vector>
#include <optional>
#include <cstdio>
#include <algorithm>
#include <cmath>
#include <SDL3/SDL.h>

#include "../core/scene.hpp"
#include "../core/camera.hpp"
#include "../core/bitmap_font.hpp"
#include "../core/sprite.hpp"
#include "../core/audio.hpp"
#include "../world/room.hpp"
#include "../world/tilemap.hpp"
#include "../entities/actor.hpp"
#include "../entities/enemy_type.hpp"
#include "../systems/room_collision.hpp"
#include "../systems/melee_combat.hpp"
#include "../systems/enemy_ai.hpp"
#include "../systems/tilemap_renderer.hpp"
#include "../systems/lighting.hpp"
#include "../systems/pathfinder.hpp"

namespace mion {

// ---------------------------------------------------------------
// Helpers de render (privados à cena)
// ---------------------------------------------------------------
static void _draw_rect(SDL_Renderer* r, const Camera2D& cam,
                       float cx, float cy, float hw, float hh,
                       Uint8 R, Uint8 G, Uint8 B, Uint8 A = 255)
{
    SDL_SetRenderDrawColor(r, R, G, B, A);
    SDL_FRect rect = { cam.world_to_screen_x(cx-hw), cam.world_to_screen_y(cy-hh),
                       hw*2.0f, hh*2.0f };
    SDL_RenderFillRect(r, &rect);
}

static void _draw_outline(SDL_Renderer* r, const Camera2D& cam,
                          float cx, float cy, float hw, float hh,
                          Uint8 R, Uint8 G, Uint8 B)
{
    SDL_SetRenderDrawColor(r, R, G, B, 255);
    SDL_FRect rect = { cam.world_to_screen_x(cx-hw), cam.world_to_screen_y(cy-hh),
                       hw*2.0f, hh*2.0f };
    SDL_RenderRect(r, &rect);
}

static void _draw_hp_bar(SDL_Renderer* r, const Camera2D& cam,
                         float cx, float cy, float hw, int hp, int max_hp)
{
    const float bw = hw*2.0f, bh = 5.0f;
    const float by = cy - 22.0f;
    SDL_SetRenderDrawColor(r, 50, 50, 50, 255);
    SDL_FRect bg = { cam.world_to_screen_x(cx-hw), cam.world_to_screen_y(by), bw, bh };
    SDL_RenderFillRect(r, &bg);
    float ratio = (max_hp > 0) ? (float)hp / max_hp : 0.0f;
    SDL_SetRenderDrawColor(r, (Uint8)(255*(1-ratio)), (Uint8)(255*ratio), 30, 255);
    SDL_FRect fill = { cam.world_to_screen_x(cx-hw), cam.world_to_screen_y(by),
                       bw*ratio, bh };
    SDL_RenderFillRect(r, &fill);
}

// Renderiza um actor — sprite se disponível, senão retângulo colorido
static void _render_actor(SDL_Renderer* r, const Camera2D& cam, const Actor& a)
{
    // Continua renderizando mortos até a animação de morte terminar
    bool death_done = !a.is_alive && a.anim.finished;
    if (death_done) return;

    float cx = a.transform.x, cy = a.transform.y;
    float hw = a.collision.half_w,  hh = a.collision.half_h;

    // --- Caminho sprite ---
    SDL_Texture* tex = static_cast<SDL_Texture*>(a.sprite_sheet);
    if (tex) {
        const AnimFrame* af = a.anim.current_frame();
        if (af) {
            SpriteFrame frame;
            frame.texture = tex;
            frame.src     = { af->src.x, af->src.y, af->src.w, af->src.h };
            draw_sprite(r, frame,
                        cam.world_to_screen_x(cx),
                        cam.world_to_screen_y(cy),
                        1.0f, 1.0f, a.facing_x < 0.0f);

            if (a.is_alive) {
                _draw_hp_bar(r, cam, cx, cy, hw, a.health.current_hp, a.health.max_hp);
                if (a.combat.is_in_active_phase()) {
                    AABB hb = a.melee_hit_box.bounds_at(cx, cy, a.facing_x, a.facing_y);
                    float hcx=(hb.min_x+hb.max_x)*0.5f, hcy=(hb.min_y+hb.max_y)*0.5f;
                    float hhw=(hb.max_x-hb.min_x)*0.5f, hhh=(hb.max_y-hb.min_y)*0.5f;
                    _draw_outline(r, cam, hcx, hcy, hhw, hhh, 255, 60, 60);
                }
            }
            return;
        }
    }

    // --- Fallback retângulo ---
    if (!a.is_alive) return;

    if (a.team == Team::Player) {
        switch (a.combat.attack_phase) {
            case AttackPhase::Idle:     _draw_rect(r,cam,cx,cy,hw,hh,180,210,255); break;
            case AttackPhase::Startup:  _draw_rect(r,cam,cx,cy,hw,hh,255,220, 50); break;
            case AttackPhase::Active:   _draw_rect(r,cam,cx,cy,hw,hh,255, 60, 60); break;
            case AttackPhase::Recovery: _draw_rect(r,cam,cx,cy,hw,hh, 80, 80,220); break;
        }
    } else {
        if (a.combat.hurt_stun_remaining_seconds > 0.0f)
            _draw_rect(r,cam,cx,cy,hw,hh,255,255,255);
        else
            _draw_rect(r,cam,cx,cy,hw,hh,180,40,40);
    }

    _draw_outline(r,cam,cx,cy,hw,hh,220,220,220);

    float fx = cx + a.facing_x*(hw+6.0f);
    float fy = cy + a.facing_y*(hh+6.0f);
    SDL_SetRenderDrawColor(r,255,255,100,255);
    SDL_RenderLine(r, cam.world_to_screen_x(cx), cam.world_to_screen_y(cy),
                      cam.world_to_screen_x(fx), cam.world_to_screen_y(fy));

    _draw_hp_bar(r,cam,cx,cy,hw, a.health.current_hp, a.health.max_hp);

    if (a.combat.is_in_active_phase()) {
        AABB hb = a.melee_hit_box.bounds_at(cx,cy,a.facing_x,a.facing_y);
        float hcx=(hb.min_x+hb.max_x)*0.5f, hcy=(hb.min_y+hb.max_y)*0.5f;
        float hhw=(hb.max_x-hb.min_x)*0.5f, hhh=(hb.max_y-hb.min_y)*0.5f;
        _draw_outline(r,cam,hcx,hcy,hhw,hhh,255,60,60);
    }
}

// ---------------------------------------------------------------
// DungeonScene
// ---------------------------------------------------------------
class DungeonScene : public IScene {
public:
    int   viewport_w = 1280;
    int   viewport_h = 720;

    void set_renderer(SDL_Renderer* r) { _renderer = r; }
    void set_audio(AudioSystem* a)     { _audio    = a; }

    void enter() override {
        // TextureCache inicializado uma vez (persiste across restarts)
        if (_renderer && !_tex_cache.has_value())
            _tex_cache.emplace(_renderer);

        _build_room();
        _pathfinder.build_nav(_tilemap, _room);
        _reset_all();

        _game_over  = false;
        _go_timer   = 0.0f;
        _hitstop    = 0;
        _next_scene = "";
        _camera.viewport_w = (float)viewport_w;
        _camera.viewport_h = (float)viewport_h;

        if (_renderer) _lighting.init(_renderer, viewport_w, viewport_h);
        if (_audio)    _audio->play_music("assets/audio/dungeon_ambient.wav");
    }

    void exit() override {
        _lighting.destroy();
        if (_audio) _audio->stop_music();
    }

    void fixed_update(float dt, const InputState& input) override {
        if (_game_over) {
            _go_timer += dt;
            if (_go_timer >= 1.0f && input.attack_pressed) {
                _reset_all();
                _game_over = false;
                _go_timer  = 0.0f;
                _hitstop   = 0;
            }
            return;
        }

        if (_hitstop > 0) { --_hitstop; return; }

        // Reseta flag de movimento para este tick
        for (auto* a : _actors) a->is_moving = false;

        // Knockback
        for (auto* a : _actors) if (a->is_alive) a->step_knockback(dt);

        // Player
        if (_player.is_alive) {
            float nx, ny;
            input.normalized_movement(nx, ny);
            _player.transform.translate(nx * _player.move_speed * dt,
                                        ny * _player.move_speed * dt);
            _player.set_facing(nx, ny);
            _player.is_moving = (nx != 0.0f || ny != 0.0f);

            if (input.attack_pressed && _player.combat.is_attack_idle()) {
                _player.combat.begin_attack();
                if (_audio) _audio->play_sfx(SoundId::PlayerAttack);
            }
        }

        for (auto* a : _actors) if (a->is_alive) a->combat.advance_time(dt);

        _enemy_ai.fixed_update(_actors, dt, &_pathfinder);
        _col_sys.resolve_all(_actors, _room);
        _col_sys.resolve_actors(_actors);
        _combat_sys.fixed_update(_actors, dt);

        // Efeitos de hit
        if (_combat_sys.pending_hitstop > 0) {
            _hitstop = _combat_sys.pending_hitstop;
            if (_combat_sys.player_hit_landed)
                _camera.trigger_shake(6.0f, 8);
        }
        if (!_combat_sys.last_events.empty() && _audio)
            _audio->play_sfx(SoundId::Hit);

        // Detecta mortes neste frame (para áudio) e atualiza was_alive
        for (auto* a : _actors) {
            if (a->was_alive && !a->is_alive) {
                if (_audio) {
                    if (a->team == Team::Enemy)
                        _audio->play_sfx(SoundId::EnemyDeath);
                    else
                        _audio->play_sfx(SoundId::PlayerDeath);
                }
            }
            a->was_alive = a->is_alive;
        }

        if (!_player.is_alive) { _game_over = true; _go_timer = 0.0f; }

        // Drive animation
        _update_animations(dt);

        _camera.follow(_player.transform.x, _player.transform.y, _room.bounds);
        _camera.step_shake();

        if (_audio) _audio->tick_music();
    }

    void render(SDL_Renderer* r) override {
        SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
        SDL_RenderClear(r);

        _tile_renderer.render(r, _camera, _tilemap);

        for (const auto& obs : _room.obstacles) {
            float ocx=(obs.bounds.min_x+obs.bounds.max_x)*0.5f;
            float ocy=(obs.bounds.min_y+obs.bounds.max_y)*0.5f;
            float ohw=(obs.bounds.max_x-obs.bounds.min_x)*0.5f;
            float ohh=(obs.bounds.max_y-obs.bounds.min_y)*0.5f;
            _draw_rect(r,_camera,ocx,ocy,ohw,ohh,60,55,70);
            _draw_outline(r,_camera,ocx,ocy,ohw,ohh,90,85,100);
        }

        for (const auto* a : _actors) _render_actor(r, _camera, *a);

        // HP numérico
        for (const auto* a : _actors) {
            if (!a->is_alive) continue;
            char buf[8];
            SDL_snprintf(buf, sizeof(buf), "%d", a->health.current_hp);
            float sx = _camera.world_to_screen_x(a->transform.x)
                       - text_width(buf, 1) * 0.5f;
            float sy = _camera.world_to_screen_y(a->transform.y) - 36.0f;
            draw_text(r, sx, sy, buf, 1, 200, 200, 200);
        }

        if (_player.is_alive) {
            float lx = _camera.world_to_screen_x(_player.transform.x);
            float ly = _camera.world_to_screen_y(_player.transform.y);
            _lighting.render(r, lx, ly);
        }

        if (_game_over) _render_game_over(r);
    }

    const char* next_scene() const override { return _next_scene; }

private:
    RoomDefinition      _room;
    Actor               _player;
    std::vector<Actor>  _enemies;    // inimigos tipados (Feature 2)
    std::vector<Actor*> _actors;
    RoomCollisionSystem _col_sys;
    MeleeCombatSystem   _combat_sys;
    EnemyAISystem       _enemy_ai;
    Pathfinder          _pathfinder;  // Feature 3
    Camera2D            _camera;
    Tilemap             _tilemap;
    TilemapRenderer     _tile_renderer;
    LightingSystem      _lighting;
    SDL_Renderer*       _renderer   = nullptr;
    AudioSystem*        _audio      = nullptr;  // Feature 4 (não-proprietário)
    std::optional<TextureCache> _tex_cache;     // Feature 1

    bool        _game_over  = false;
    float       _go_timer   = 0.0f;
    int         _hitstop    = 0;
    const char* _next_scene = "";

    // -------------------------------------------------------------------
    // Spawn e reset
    // -------------------------------------------------------------------

    void _spawn_enemy(EnemyType type, float x, float y, const char* name) {
        const EnemyDef& def = get_enemy_def(type);
        Actor a;
        a.name               = name;
        a.team               = Team::Enemy;
        a.enemy_type         = type;
        a.attack_damage      = def.damage;
        a.aggro_range        = def.aggro_range;
        a.attack_range_ai    = def.attack_range;
        a.stop_range_ai      = def.stop_range;
        a.move_speed         = def.move_speed;
        a.knockback_friction = def.knockback_friction;
        a.collision          = def.collision;
        a.hurt_box           = def.hurt_box;
        a.melee_hit_box      = def.melee_hit_box;
        a.health             = { def.max_hp, def.max_hp };
        a.combat.attack_startup_duration_seconds  = def.startup_sec;
        a.combat.attack_active_duration_seconds   = def.active_sec;
        a.combat.attack_recovery_duration_seconds = def.recovery_sec;
        a.combat.reset_for_spawn();
        a.transform.set_position(x, y);
        a.is_alive           = true;
        a.was_alive          = true;
        a.knockback_vx       = 0;
        a.knockback_vy       = 0;
        // Escala o timer inicial para distribuir replans entre inimigos
        a.path_replan_timer  = (float)_enemies.size() * 0.1f;

        // Spritesheet (Feature 1)
        if (_tex_cache.has_value()) {
            a.sprite_sheet = _tex_cache->load(def.sprite_sheet_path);
        }
        a.anim.build_default_clips(def.frame_w, def.frame_h,
                                   def.idle_frames, def.walk_frames,
                                   def.attack_frames, def.hurt_frames,
                                   def.death_frames, def.frame_fps);

        _enemies.push_back(std::move(a));
    }

    void _reset_all() {
        // Player
        _player.name         = "player";
        _player.team         = Team::Player;
        _player.attack_damage = 10;
        _player.melee_hit_box = { 22.0f, 14.0f, 28.0f };
        _player.is_alive     = true;
        _player.was_alive    = true;
        _player.move_speed   = 200.0f;
        _player.knockback_vx = 0; _player.knockback_vy = 0;
        _player.transform.set_position(400.0f, 600.0f);
        _player.health = { 100, 100 };
        _player.combat.reset_for_spawn();

        // Sprite do player (Feature 1)
        if (_tex_cache.has_value()) {
            _player.sprite_sheet = _tex_cache->load("assets/sprites/player.png");
        }
        _player.anim.build_default_clips(32, 32, 4, 4, 4, 2, 4, 8.0f);

        // Inimigos (Feature 2)
        _enemies.clear();
        _spawn_enemy(EnemyType::Skeleton, 900.0f,  600.0f, "skeleton_01");
        _spawn_enemy(EnemyType::Orc,      1100.0f, 400.0f, "orc_01");
        _spawn_enemy(EnemyType::Ghost,    700.0f,  300.0f, "ghost_01");

        // Reconstrói o vetor de ponteiros (invalidado após _enemies.push_back)
        _actors.clear();
        _actors.push_back(&_player);
        for (auto& e : _enemies) _actors.push_back(&e);
    }

    void _build_room() {
        _room.name   = "dungeon_01";
        _room.bounds = { 0, 1600, 0, 1200 };
        _room.obstacles.clear();
        _room.add_obstacle("pillar_nw",  200,  200,  280,  280);
        _room.add_obstacle("pillar_ne", 1320,  200, 1400,  280);
        _room.add_obstacle("pillar_sw",  200,  920,  280, 1000);
        _room.add_obstacle("pillar_se", 1320,  920, 1400, 1000);
        _room.add_obstacle("wall_mid",   680,  500,  920,  540);

        const int TS = 32;
        int cols = (int)(_room.bounds.max_x / TS) + 1;
        int rows = (int)(_room.bounds.max_y / TS) + 1;
        _tilemap.init(cols, rows, TS);
        _tilemap.fill(0, 0, cols-1, rows-1, TileType::Floor);
        _tilemap.fill(0,      0,      cols-1, 0,      TileType::Wall);
        _tilemap.fill(0,      rows-1, cols-1, rows-1, TileType::Wall);
        _tilemap.fill(0,      0,      0,      rows-1, TileType::Wall);
        _tilemap.fill(cols-1, 0,      cols-1, rows-1, TileType::Wall);
    }

    // -------------------------------------------------------------------
    // Drive de animação — chamado uma vez por fixed_update
    // -------------------------------------------------------------------
    void _update_animations(float dt) {
        for (auto* a : _actors) {
            if (!a->is_alive) {
                a->anim.play(ActorAnim::Death);
            } else if (a->combat.is_hurt_stunned()) {
                a->anim.play(ActorAnim::Hurt);
            } else if (!a->combat.is_attack_idle()) {
                a->anim.play(ActorAnim::Attack);
            } else if (a->is_moving) {
                a->anim.play(ActorAnim::Walk);
            } else {
                a->anim.play(ActorAnim::Idle);
            }
            a->anim.advance(dt);
        }
    }

    // -------------------------------------------------------------------
    // Game over
    // -------------------------------------------------------------------
    void _render_game_over(SDL_Renderer* r) {
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(r, 0, 0, 0, 180);
        SDL_FRect ov = { 0, 0, (float)viewport_w, (float)viewport_h };
        SDL_RenderFillRect(r, &ov);

        const int   scale = 4;
        const char* msg1  = "GAME OVER";
        const char* msg2  = "PRESS SPACE TO RESTART";
        float cx = viewport_w * 0.5f, cy = viewport_h * 0.5f;

        draw_text(r, cx - text_width(msg1,scale)*0.5f, cy - 40, msg1, scale, 220, 40, 40);
        if (_go_timer >= 1.0f)
            draw_text(r, cx - text_width(msg2,2)*0.5f, cy + 30, msg2, 2, 180, 180, 180);
    }
};

} // namespace mion
