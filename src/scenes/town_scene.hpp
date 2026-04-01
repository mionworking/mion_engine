#pragma once
#include <cstdio>
#include <cmath>
#include <optional>
#include <random>
#include <string>
#include <vector>
#include <SDL3/SDL.h>

#include "../core/scene.hpp"
#include "../core/camera.hpp"
#include "../core/bitmap_font.hpp"
#include "../core/audio.hpp"
#include "../core/save_system.hpp"
#include "../core/engine_paths.hpp"
#include "../core/ini_loader.hpp"
#include "../core/dialogue_loader.hpp"
#include "../components/player_config.hpp"
#include "../components/talent_tree.hpp"
#include "../components/spell_book.hpp"
#include "../entities/actor.hpp"
#include "../entities/npc.hpp"
#include "../entities/shop.hpp"
#include "../world/room.hpp"
#include "../systems/movement_system.hpp"
#include "../systems/room_collision.hpp"
#include "../systems/room_flow_system.hpp"
#include "../systems/dialogue_system.hpp"
#include "../systems/dialogue_render.hpp"
#include "../systems/player_action.hpp"
#include "../systems/resource_system.hpp"
#include "../systems/shop_system.hpp"
#include "../core/ui.hpp"
#include "../core/sprite.hpp"
#include "../world/tilemap.hpp"
#include "../systems/tilemap_renderer.hpp"
#include "../systems/world_renderer.hpp"
#include "../systems/player_configurator.hpp"
#include "../systems/pause_menu_controller.hpp"
#include "../core/sprite_layout.hpp"

namespace mion {

class TownScene : public IScene {
public:
    int viewport_w = 1280;
    int viewport_h = 720;

    void set_renderer(SDL_Renderer* rr) { _renderer = rr; }
    void set_audio(AudioSystem* a) { _audio = a; }

    void enter() override {
        if (_renderer && !_tex_cache.has_value())
            _tex_cache.emplace(_renderer);

        _pending_next.clear();
        _shop_open            = false;
        _interacting_npc_hint = false;
        _prev_confirm         = false;
        _prev_cancel          = false;
        _prev_move_nav        = false;
        _prev_shop_move_y     = 0.0f;
        _shop_prev_confirm    = false;
        _shop_prev_cancel     = false;
        _camera.viewport_w    = (float)viewport_w;
        _camera.viewport_h    = (float)viewport_h;

        reset_player_config_defaults();
        reset_progression_config_defaults();
        reset_talent_tree_defaults();
        {
            IniData d = ini_load(resolve_data_path("player.ini").c_str());
            if (!d.sections.empty())
                apply_player_ini(d);
        }
        {
            IniData d = ini_load(resolve_data_path("progression.ini").c_str());
            if (!d.sections.empty())
                apply_progression_ini(d);
        }
        {
            IniData d = ini_load(resolve_data_path("talents.ini").c_str());
            if (!d.sections.empty())
                apply_talents_ini(d);
        }

        _dialogue.clear_registry();
        {
            IniData dlg = ini_load(resolve_data_path("town_dialogues.ini").c_str());
            register_dialogue_sequences_from_ini(_dialogue, dlg);
        }
        _dialogue.set_audio(_audio);

        SaveData loaded;
        const bool has_save = SaveSystem::load_default(loaded);
        if (!has_save)
            _fresh_run();
        else
            _apply_save(loaded);

        _build_town();
        {
            TextureCache* tc = _tex_cache.has_value() ? &*_tex_cache : nullptr;
            configure_player(_player, tc);
        }
        if (has_save)
            _apply_stats_after_configure(loaded);

        _actors.clear();
        _actors.push_back(&_player);
        _rebuild_npc_actors();

        if (_audio) {
            _audio->stop_all_ambient();
            _audio->play_ambient(AmbientId::TownWind, 0.18f);
            _audio->play_ambient(AmbientId::TownBirds, 0.11f);
            _audio->set_music_state(MusicState::Town);
        }

        _pause_controller = {};
        PauseMenuController::InitOptions pause_options;
        pause_options.disabled_items = {false, true, false, false};
        pause_options.on_opened      = [this]{ _shop_open = false; };
        _pause_controller.init(pause_options);
    }

    void exit() override {
        if (_audio) {
            _audio->stop_all_ambient();
            _audio->fade_music(800);
        }
        SaveData d = _capture_save();
        SaveSystem::save_default(d);
    }

    void fixed_update(float dt, const InputState& input) override {
        _pending_next.clear();

        if (_dialogue.is_active()) {
            _dialogue.fixed_update(input);
            _pause_controller.flush_input(input);
            _prev_confirm  = input.confirm_pressed;
            _prev_cancel   = input.ui_cancel_pressed;
            _prev_move_nav = input.move_y != 0.0f;
            return;
        }

        PauseMenuResult pause_result = _pause_controller.update(input);
        if (pause_result.should_quit_to_title) {
            _pending_next = "title";
            if (_audio) _audio->fade_music(400);
        }
        if (pause_result.world_paused) {
            _pause_controller.flush_input(input);
            _prev_confirm  = input.confirm_pressed;
            _prev_cancel   = input.ui_cancel_pressed;
            _prev_move_nav = input.move_y != 0.0f;
            return;
        }

        if (_shop_open) {
            _update_shop(input);
            _pause_controller.flush_input(input);
            _prev_confirm  = input.confirm_pressed;
            _prev_cancel   = input.ui_cancel_pressed;
            _prev_move_nav = input.move_y != 0.0f;
            _resource_sys.fixed_update(_actors, dt);
            _camera.follow(_player.transform.x, _player.transform.y, _room.bounds);
            _camera.step_shake(_shake_rng);
            if (_audio)
                _audio->tick_music();
            return;
        }

        const bool confirm_edge = input.confirm_pressed && !_prev_confirm;
        _prev_confirm           = input.confirm_pressed;
        _prev_cancel            = input.ui_cancel_pressed;
        _prev_move_nav          = input.move_y != 0.0f;

        _movement_sys.fixed_update(_actors, dt);
        _player_action.fixed_update(_player, input, dt, _audio, nullptr, nullptr);
        _resource_sys.fixed_update(_actors, dt);
        _col_sys.resolve_all(_actors, _room);
        _col_sys.resolve_actors(_actors, &_room);
        _update_npcs(dt);

        // Animação do player — atualiza dir_row igual ao dungeon
        if (_player.sprite_sheet) {
            _player.anim.update_puny_dir_row(
                facing_to_puny_row(_player.facing_x, _player.facing_y));
            if (_player.is_dashing())
                _player.anim.play(ActorAnim::Dash);
            else if (_player.combat.attack_phase != AttackPhase::Idle)
                _player.anim.play(ActorAnim::Attack);
            else if (_player.is_moving)
                _player.anim.play(ActorAnim::Walk);
            else
                _player.anim.play(ActorAnim::Idle);
            _player.anim.advance(dt);
        }

        _room_flow_sys.fixed_update(_actors, _player, _room);
        if (!_room_flow_sys.scene_exit_to.empty()) {
            _pending_next = _room_flow_sys.scene_exit_to;
            SaveSystem::save_default(_capture_save());
            if (_audio)
                _audio->fade_music(350);
        }

        int near = _nearest_npc_index();
        _interacting_npc_hint =
            (near >= 0
             && _dist_sq(_player.transform.x, _player.transform.y, _npcs[static_cast<size_t>(near)].x,
                         _npcs[static_cast<size_t>(near)].y)
                    <= _npcs[static_cast<size_t>(near)].interact_radius
                        * _npcs[static_cast<size_t>(near)].interact_radius);

        if (confirm_edge && near >= 0 && _interacting_npc_hint)
            _start_npc_interaction(near);

        _camera.follow(_player.transform.x, _player.transform.y, _room.bounds);
        _camera.step_shake(_shake_rng);
        if (_audio)
            _audio->tick_music();

        _pause_controller.flush_input(input);
    }

    void render(SDL_Renderer* r, float /*blend_factor*/) override {
        SDL_SetRenderDrawColor(r, 24, 40, 22, 255);
        SDL_RenderClear(r);

        // Chão com tileset de grama
        _tile_renderer.render(r, _camera, _tilemap);

        // Buildings — sprite PNG ou retângulo fallback
        for (const auto& obs : _room.obstacles) {
            float x0 = _camera.world_to_screen_x(obs.bounds.min_x);
            float y0 = _camera.world_to_screen_y(obs.bounds.min_y);
            float bw  = obs.bounds.max_x - obs.bounds.min_x;
            float bh  = obs.bounds.max_y - obs.bounds.min_y;

            auto _has = [&](const char* s){ return obs.name.find(s) != obs.name.npos; };
            const char* sprite_path = nullptr;
            if      (_has("tavern"))   sprite_path = "assets/props/building_tavern.png";
            else if (_has("forge"))    sprite_path = "assets/props/building_forge.png";
            else if (_has("elder"))    sprite_path = "assets/props/building_elder.png";
            else if (_has("fountain")) sprite_path = "assets/props/fountain.png";

            SDL_Texture* btex = (sprite_path && _tex_cache.has_value())
                                    ? _tex_cache->load(sprite_path) : nullptr;
            if (btex) {
                SDL_FRect dst = { x0, y0, bw, bh };
                SDL_RenderTexture(r, btex, nullptr, &dst);
            } else {
                float ocx = (obs.bounds.min_x + obs.bounds.max_x) * 0.5f;
                float ocy = (obs.bounds.min_y + obs.bounds.max_y) * 0.5f;
                float ohw = bw * 0.5f, ohh = bh * 0.5f;
                Uint8 R = 80, G = 90, B = 100;
                if      (_has("tavern"))   { R=120; G=90;  B=40;  }
                else if (_has("forge"))    { R=90;  G=88;  B=95;  }
                else if (_has("elder"))    { R=70;  G=50;  B=90;  }
                else if (_has("fountain")) { R=50;  G=100; B=160; }
                draw_world_rect(r, _camera, ocx, ocy, ohw, ohh, R, G, B);
            }
        }

        // Porta de saída para o dungeon
        for (const auto& door : _room.doors) {
            float dcx = (door.bounds.min_x + door.bounds.max_x) * 0.5f;
            float dcy = (door.bounds.min_y + door.bounds.max_y) * 0.5f;
            float dhw = (door.bounds.max_x - door.bounds.min_x) * 0.5f;
            float dhh = (door.bounds.max_y - door.bounds.min_y) * 0.5f;
            SDL_Texture* dtex = _tex_cache.has_value()
                                    ? _tex_cache->load("assets/props/door_open.png") : nullptr;
            if (dtex) {
                SDL_FRect dst = { _camera.world_to_screen_x(dcx - dhw),
                                  _camera.world_to_screen_y(dcy - dhh),
                                  dhw * 2.0f, dhh * 2.0f };
                SDL_RenderTexture(r, dtex, nullptr, &dst);
            } else {
                draw_world_rect(r, _camera, dcx, dcy, dhw, dhh, 80, 140, 80);
            }
        }

        // NPCs — sprite animado ou retângulo fallback.
        // Escala de NPC = mesma escala de sprite do player, para manter
        // coerência visual de tamanho entre todos os personagens 2D.
        static const struct { const char* name; const char* path; } kNpcSprites[] = {
            { "Mira",     "assets/npcs/npc_mira.png"     },
            { "Forge",    "assets/npcs/npc_forge.png"    },
            { "Villager", "assets/npcs/npc_villager.png" },
            { "Elder",    "assets/npcs/npc_elder.png"    },
        };
        for (const auto& npc : _npcs) {
            SDL_Texture* ntex = nullptr;
            if (_tex_cache.has_value()) {
                for (const auto& s : kNpcSprites) {
                    if (npc.name == s.name) { ntex = _tex_cache->load(s.path); break; }
                }
            }
            if (ntex) {
                // Idle frame 0 da linha Sul (row 0, col 12).
                // Usa o mesmo tamanho lógico de frame do layout Puny e a mesma
                // escala de sprite do player, para que NPCs tenham o mesmo
                // "tamanho em mundo".
                constexpr float kFW = (float)mion::SpriteLayout::PunyFrameW;
                constexpr float kFH = (float)mion::SpriteLayout::PunyFrameH;
                SDL_FRect src = { 12.0f * kFW, 0.0f, kFW, kFH };

                float npc_scale = _player.sprite_scale;
                if (npc_scale <= 0.0f) npc_scale = 2.0f; // fallback seguro

                float draw_w = kFW * npc_scale;
                float draw_h = kFH * npc_scale;
                SDL_FRect dst = {
                    _camera.world_to_screen_x(npc.actor ? npc.actor->transform.x : npc.x) - draw_w * 0.5f,
                    _camera.world_to_screen_y(npc.actor ? npc.actor->transform.y : npc.y) - draw_h * 0.5f,
                    draw_w, draw_h
                };
                SDL_RenderTexture(r, ntex, &src, &dst);
            } else {
                const float cx = npc.actor ? npc.actor->transform.x : npc.x;
                const float cy = npc.actor ? npc.actor->transform.y : npc.y;
                draw_world_rect(r, _camera, cx, cy, 14.0f, 18.0f,
                                npc.portrait_color.r, npc.portrait_color.g, npc.portrait_color.b);
            }
            const float label_x = (npc.actor ? npc.actor->transform.x : npc.x) - 40.0f;
            const float label_y = (npc.actor ? npc.actor->transform.y : npc.y) - 36.0f;
            draw_text(r, _camera.world_to_screen_x(label_x),
                      _camera.world_to_screen_y(label_y), npc.name.c_str(), 2, 220, 220,
                      200, 255);
        }

        {
            float cx = _player.transform.x, cy = _player.transform.y;
            SDL_Texture* tex = _player.sprite_sheet;
            const AnimFrame* af = _player.anim.current_frame();
            if (tex && af) {
                SpriteFrame frame;
                frame.texture = tex;
                frame.src     = { af->src.x, af->src.y, af->src.w, af->src.h };
                draw_sprite(r, frame,
                            _camera.world_to_screen_x(cx),
                            _camera.world_to_screen_y(cy),
                            _player.sprite_scale, _player.sprite_scale,
                            _player.facing_x < 0.0f);
            } else {
                float hw = _player.collision.half_w, hh = _player.collision.half_h;
                draw_world_rect(r, _camera, cx, cy, hw, hh, 180, 210, 255);
            }
        }

        if (_interacting_npc_hint && !_dialogue.is_active()) {
            const char* hint = "ENTER - falar";
            draw_text(r, 16.0f, (float)viewport_h - 80.0f, hint, 2, 255, 220, 120, 255);
        }

        char gold_buf[48];
        snprintf(gold_buf, sizeof(gold_buf), "Gold: %d", _player.gold);
        draw_text(r, 16.0f, 16.0f, gold_buf, 2, 255, 210, 100, 255);

        render_dialogue_ui(r, viewport_w, viewport_h, _dialogue);
        if (_shop_open)
            ShopSystem::render_shop_ui(r, _shop_forge, _player.gold, viewport_w, viewport_h);
        _pause_controller.render(r, viewport_w, viewport_h);
    }

    const char* next_scene() const override {
        return _pending_next.empty() ? nullptr : _pending_next.c_str();
    }

private:
    static constexpr int kQuestRewardGold = 120;

    // Espelho do campo SaveData::scene_flags mantido na Town.
    unsigned int             _scene_flags  = 0;

    // facing_to_puny_row() extraído para world_renderer.hpp

    RoomDefinition           _room;
    Tilemap                  _tilemap;
    TilemapRenderer          _tile_renderer;
    Actor                    _player;
    std::vector<Actor*>      _actors;
    std::vector<Actor>       _npc_actors;
    std::vector<NpcEntity>   _npcs;
    ShopInventory            _shop_forge;
    QuestState               _quest_state;
    MovementSystem           _movement_sys;
    RoomCollisionSystem      _col_sys;
    RoomFlowSystem           _room_flow_sys;
    DialogueSystem           _dialogue;
    ResourceSystem           _resource_sys;
    PlayerActionSystem       _player_action;
    Camera2D                 _camera;
    std::mt19937             _shake_rng{std::random_device{}()};
    SDL_Renderer*            _renderer = nullptr;
    AudioSystem*             _audio    = nullptr;
    std::optional<TextureCache> _tex_cache;
    std::string              _pending_next;
    bool                     _shop_open = false;
    bool                     _interacting_npc_hint = false;
    bool                     _prev_confirm         = false;
    bool                     _prev_cancel          = false;
    bool                     _prev_move_nav        = false;
    float                    _prev_shop_move_y     = 0.0f;
    bool                     _shop_prev_confirm    = false;
    bool                     _shop_prev_cancel     = false;

    PauseMenuController _pause_controller;

    static float _dist_sq(float ax, float ay, float bx, float by) {
        float dx = ax - bx, dy = ay - by;
        return dx * dx + dy * dy;
    }

    SaveData _capture_save() const {
        SaveData d;
        d.version              = kSaveFormatVersion;
        d.room_index           = 0;
        d.player_hp            = _player.health.current_hp;
        d.gold                 = _player.gold;
        d.quest_state          = _quest_state;
        d.progression          = _player.progression;
        d.talents              = _player.talents;
        d.mana                 = _player.mana;
        d.stamina              = _player.stamina;
        d.attributes           = _player.attributes;        // v4
        d.attr_points_available = _player.attributes.vigor  // v5: pontos não distribuídos
                                    + _player.attributes.forca
                                    + _player.attributes.destreza
                                    + _player.attributes.inteligencia
                                    + _player.attributes.endurance
                                    // pending_level_ups ainda não gastos
                                    + _player.progression.pending_level_ups;
        // Recalcula corretamente: pontos_disp = pending_level_ups (ainda não alocados)
        d.attr_points_available = _player.progression.pending_level_ups;
        d.scene_flags           = _scene_flags;
        return d;
    }

    void _fresh_run() {
        _quest_state        = {};
        _player.gold        = 0;
        _player.progression = ProgressionState{};
        _player.talents     = TalentState{};
    }

    void _apply_save(const SaveData& sd) {
        _player.progression = sd.progression;
        _player.talents     = sd.talents;
        _player.gold        = sd.gold;
        _quest_state        = sd.quest_state;
        _player.attributes  = sd.attributes;   // v4
        _scene_flags        = sd.scene_flags;  // v5
    }

    void _apply_stats_after_configure(const SaveData& sd) {
        int hp = sd.player_hp;
        if (hp > _player.health.max_hp)
            hp = _player.health.max_hp;
        if (hp < 1)
            hp = 1;
        _player.health.current_hp = hp;
        _player.mana    = sd.mana;
        _player.stamina = sd.stamina;
        if (_player.mana.current > _player.mana.max)
            _player.mana.current = _player.mana.max;
        if (_player.stamina.current > _player.stamina.max)
            _player.stamina.current = _player.stamina.max;
    }

    void _build_town() {
        _room.name   = "town";
        _room.bounds = {0.0f, 2400.0f, 0.0f, 1600.0f};
        _room.obstacles.clear();
        _room.doors.clear();

        // Tilemap de grama (tudo Floor)
        const int ts = 32;
        _tilemap.init(75, 50, ts);  // 75*32=2400, 50*32=1600
        for (int r = 0; r < 50; ++r)
            for (int c = 0; c < 75; ++c)
                _tilemap.set(c, r, TileType::Floor);

        if (_tex_cache.has_value()) {
            _tile_renderer.tileset        = _tex_cache->load("assets/tiles/town_tileset.png");
            _tile_renderer.floor_tile_col = 0; _tile_renderer.floor_tile_row = 0;
            _tile_renderer.wall_tile_col  = 1; _tile_renderer.wall_tile_row  = 0;
        }

        _room.add_obstacle("building_tavern", 700.0f, 200.0f, 1100.0f, 500.0f);
        _room.add_obstacle("building_forge", 1300.0f, 200.0f, 1600.0f, 500.0f);
        _room.add_obstacle("building_elder", 300.0f, 900.0f, 600.0f, 1200.0f);
        _room.add_obstacle("fountain", 1100.0f, 700.0f, 1300.0f, 900.0f);

        _room.add_door(2350.0f, 700.0f, 2390.0f, 900.0f, false, "dungeon");

        _npcs.clear();
        {
            NpcEntity mira;
            mira.x = mira.spawn_x = 440.0f;
            mira.y = mira.spawn_y = 840.0f;
            mira.name            = "Mira";
            mira.type            = NpcType::QuestGiver;
            mira.interact_radius = 52.0f;
            mira.portrait_color  = {200, 180, 80, 255};
            mira.dialogue_default       = "mira_default";
            mira.dialogue_quest_active  = "mira_quest_active";
            mira.dialogue_quest_done    = "mira_quest_done";
            mira.wander_radius = 60.0f;
            mira.wander_speed  = 28.0f;
            mira.wander_timer  = 1.2f;
            _npcs.push_back(mira);
        }
        {
            NpcEntity forge;
            forge.x = forge.spawn_x = 1450.0f;
            forge.y = forge.spawn_y = 540.0f;
            forge.name            = "Forge";
            forge.type            = NpcType::Merchant;
            forge.interact_radius = 52.0f;
            forge.portrait_color  = {220, 120, 60, 255};
            forge.dialogue_default = "forge_greeting";
            forge.wander_radius = 50.0f;
            forge.wander_speed  = 22.0f;
            forge.wander_timer  = 0.8f;
            _npcs.push_back(forge);
        }
        {
            NpcEntity v;
            v.x = v.spawn_x = 900.0f;
            v.y = v.spawn_y = 750.0f;
            v.name            = "Villager";
            v.type            = NpcType::Generic;
            v.portrait_color  = {100, 160, 90, 255};
            v.dialogue_default = "villager_a";
            v.wander_radius = 90.0f;
            v.wander_speed  = 38.0f;
            v.wander_timer  = 0.5f;
            _npcs.push_back(v);
        }
        {
            NpcEntity v;
            v.x = v.spawn_x = 1600.0f;
            v.y = v.spawn_y = 900.0f;
            v.name            = "Elder";
            v.type            = NpcType::Generic;
            v.portrait_color  = {120, 140, 100, 255};
            v.dialogue_default = "villager_b";
            v.wander_radius = 70.0f;
            v.wander_speed  = 25.0f;
            v.wander_timer  = 1.8f;
            _npcs.push_back(v);
        }

        _shop_forge.items.clear();
        _shop_forge.items.push_back({"HP Potion", ShopItemType::HpPotion, 20, 50});
        _shop_forge.items.push_back({"Stamina Potion", ShopItemType::StaminaPotion, 15, 80});
        _shop_forge.items.push_back({"Sharpening Stone", ShopItemType::AttackUpgrade, 30, 3});
        _shop_forge.items.push_back({"Mana Crystal", ShopItemType::ManaUpgrade, 25, 10});
        _shop_forge.selected_index = 0;

        _player.transform.set_position(400.0f, 800.0f);
    }

    void _rebuild_npc_actors() {
        _npc_actors.clear();
        _npc_actors.reserve(_npcs.size());
        // Mantém o player sempre no início de _actors.
        _actors.clear();
        _actors.push_back(&_player);

        for (auto& npc : _npcs) {
            Actor a;
            a.name      = npc.name;
            a.team      = Team::Enemy; // neutro para town; não participa de combate.
            a.transform.set_position(npc.x, npc.y);
            a.collision.half_w = npc.collision_half;
            a.collision.half_h = npc.collision_half;
            a.is_alive         = true;
            a.was_alive        = true;
            a.move_speed       = npc.wander_speed;
            a.sprite_sheet     = nullptr;
            a.sprite_scale     = _player.sprite_scale > 0.0f ? _player.sprite_scale : 2.0f;

            _npc_actors.push_back(a);
            npc.actor = &_npc_actors.back();
            _actors.push_back(npc.actor);
        }
    }

    // -----------------------------------------------------------------------
    // Wander + colisão de NPCs
    // -----------------------------------------------------------------------
    void _update_npcs(float dt) {
        // Números pseudo-aleatórios leves (sem std::mt19937 para não criar dependência)
        static unsigned _rng_state = 0x12345678u;
        auto _rng_next = [&]() -> unsigned {
            _rng_state ^= _rng_state << 13;
            _rng_state ^= _rng_state >> 17;
            _rng_state ^= _rng_state << 5;
            return _rng_state;
        };
        auto _rng_f01 = [&]() -> float {
            return (float)(_rng_next() & 0xFFFFu) / 65535.0f;
        };

        for (auto& npc : _npcs) {
            Actor* a = npc.actor;
            if (!a) {
                // Fallback: mantém NPC estático se não houver Actor associado.
                continue;
            }
            float& nx = a->transform.x;
            float& ny = a->transform.y;

            // --- Wander timer ---
            npc.wander_timer -= dt;
            if (npc.wander_timer <= 0.0f) {
                // Escolhe nova direção aleatória dentro do raio de spawn
                float angle = _rng_f01() * 6.2831853f; // 2π
                float tx    = npc.spawn_x + std::cos(angle) * (_rng_f01() * npc.wander_radius);
                float ty    = npc.spawn_y + std::sin(angle) * (_rng_f01() * npc.wander_radius);
                float dx    = tx - npc.x;
                float dy    = ty - npc.y;
                float len   = std::sqrt(dx * dx + dy * dy);
                if (len > 0.5f) {
                    npc.wander_dir_x = dx / len;
                    npc.wander_dir_y = dy / len;
                } else {
                    npc.wander_dir_x = 0.0f;
                    npc.wander_dir_y = 0.0f;
                }
                npc.wander_timer = 1.4f + _rng_f01() * 1.2f; // 1.4–2.6 s
            }

            // --- Mover NPC (via Actor) ---
            nx += npc.wander_dir_x * npc.wander_speed * dt;
            ny += npc.wander_dir_y * npc.wander_speed * dt;
            a->is_moving = (npc.wander_dir_x != 0.0f || npc.wander_dir_y != 0.0f);

            // --- Conter dentro do raio de spawn ---
            float sdx = nx - npc.spawn_x;
            float sdy = ny - npc.spawn_y;
            float sdist2 = sdx * sdx + sdy * sdy;
            if (sdist2 > npc.wander_radius * npc.wander_radius) {
                float sdist = std::sqrt(sdist2);
                nx = npc.spawn_x + sdx / sdist * npc.wander_radius;
                ny = npc.spawn_y + sdy / sdist * npc.wander_radius;
                npc.wander_dir_x = 0.0f;
                npc.wander_dir_y = 0.0f;
            }
            // Sincroniza posição espelhada usada em interações/render.
            npc.x = nx;
            npc.y = ny;
        }
    }

    int _nearest_npc_index() const {
        int   best   = -1;
        float best_d = 1.0e12f;
        for (int i = 0; i < static_cast<int>(_npcs.size()); ++i) {
            const NpcEntity& n = _npcs[static_cast<size_t>(i)];
            float            d = _dist_sq(_player.transform.x, _player.transform.y, n.x, n.y);
            if (d < best_d) {
                best_d = d;
                best   = i;
            }
        }
        return best;
    }

    void _start_npc_interaction(int idx) {
        NpcEntity& n = _npcs[static_cast<size_t>(idx)];
        if (n.type == NpcType::Merchant) {
            _shop_open            = true;
            _shop_forge.selected_index = 0;
            return;
        }
        if (n.type == NpcType::Generic) {
            if (!n.dialogue_default.empty())
                _dialogue.start(n.dialogue_default);
            return;
        }
        if (n.type == NpcType::QuestGiver) {
            if (_quest_state.is(QuestId::DefeatGrimjaw, QuestStatus::NotStarted)) {
                _dialogue.start("mira_quest_offer", [this]() {
                    _quest_state.set(QuestId::DefeatGrimjaw, QuestStatus::InProgress);
                    SaveSystem::save_default(_capture_save());
                });
            } else if (_quest_state.is(QuestId::DefeatGrimjaw, QuestStatus::InProgress)) {
                _dialogue.start(n.dialogue_quest_active.empty() ? "mira_quest_active"
                                                                : n.dialogue_quest_active);
            } else if (_quest_state.is(QuestId::DefeatGrimjaw, QuestStatus::Completed)) {
                _dialogue.start(n.dialogue_quest_done.empty() ? "mira_quest_done"
                                                              : n.dialogue_quest_done,
                                [this]() {
                                    _quest_state.set(QuestId::DefeatGrimjaw, QuestStatus::Rewarded);
                                    _player.gold += kQuestRewardGold;
                                    SaveSystem::save_default(_capture_save());
                                });
            } else {
                _dialogue.start(n.dialogue_default.empty() ? "mira_default" : n.dialogue_default);
            }
        }
    }

    void _update_shop(const InputState& input) {
        constexpr float th = 0.35f;
        if (! _shop_forge.items.empty()) {
            if (input.move_y < -th && _prev_shop_move_y >= -th && _shop_forge.selected_index > 0)
                --_shop_forge.selected_index;
            if (input.move_y > th && _prev_shop_move_y <= th
                && _shop_forge.selected_index < static_cast<int>(_shop_forge.items.size()) - 1)
                ++_shop_forge.selected_index;
        }
        _prev_shop_move_y = input.move_y;

        const bool buy_edge = input.confirm_pressed && !_shop_prev_confirm;
        if (buy_edge) {
            if (ShopSystem::try_buy(_player, _shop_forge, _shop_forge.selected_index) && _audio)
                _audio->play_sfx(SoundId::UiConfirm, 0.5f);
        }
        if (input.ui_cancel_pressed && !_shop_prev_cancel)
            _shop_open = false;
        _shop_prev_confirm = input.confirm_pressed;
        _shop_prev_cancel  = input.ui_cancel_pressed;
    }
};

} // namespace mion
