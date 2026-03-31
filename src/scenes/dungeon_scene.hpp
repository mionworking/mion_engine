#pragma once
#include <array>
#include <vector>
#include <optional>
#include <random>
#include <string>
#include <cstdio>
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <SDL3/SDL.h>

#include "../core/scene.hpp"
#include "../core/camera.hpp"
#include "../core/bitmap_font.hpp"
#include "../core/sprite.hpp"
#include "../core/audio.hpp"
#include "../world/room.hpp"
#include "../world/tilemap.hpp"
#include "../world/dungeon_rules.hpp"
#include "../world/room_loader.hpp"
#include "../entities/actor.hpp"
#include "../entities/enemy_type.hpp"
#include "../systems/room_collision.hpp"
#include "../systems/melee_combat.hpp"
#include "../systems/enemy_ai.hpp"
#include "../systems/tilemap_renderer.hpp"
#include "../systems/lighting.hpp"
#include "../systems/world_renderer.hpp"
#include "../systems/player_configurator.hpp"
#include "../core/pause_menu.hpp"
#include "../systems/dungeon_hud.hpp"
#include "../systems/skill_tree_ui.hpp"
#include "../systems/attribute_screen_ui.hpp"
#include "../systems/pathfinder.hpp"
#include "../systems/player_action.hpp"
#include "../systems/movement_system.hpp"
#include "../systems/resource_system.hpp"
#include "../systems/status_effect_system.hpp"
#include "../systems/room_flow_system.hpp"
#include "../systems/enemy_spawner.hpp"
#include "../systems/room_manager.hpp"
#include "../systems/dungeon_world_renderer.hpp"
#include "../systems/screen_fx.hpp"
#include "../systems/projectile_system.hpp"
#include "../systems/drop_system.hpp"
#include "../systems/simple_particles.hpp"
#include "../systems/floating_text.hpp"
#include "../systems/dialogue_system.hpp"
#include "../core/save_system.hpp"
#include "../core/quest_state.hpp"
#include "../core/run_stats.hpp"
#include "../core/engine_paths.hpp"
#include "../core/ini_loader.hpp"
#include "../components/player_config.hpp"
#include "../entities/projectile.hpp"
#include "../entities/ground_item.hpp"
#include "../components/talent_tree.hpp"
#include "../components/spell_defs.hpp"
#include "../core/ui.hpp"

namespace mion {

// ---------------------------------------------------------------
// DungeonScene
// ---------------------------------------------------------------
class DungeonScene : public IScene {
public:
    int   viewport_w = 1280;
    int   viewport_h = 720;

    static constexpr float kFootstepPlayerDist = 44.0f;
    static constexpr float kFootstepEnemyDist  = 56.0f;

    void set_renderer(SDL_Renderer* r) { _renderer = r; }
    void set_audio(AudioSystem* a)     { _audio    = a; }
    void set_show_autosave_indicator(bool v) { _show_autosave_indicator = v; }
    void set_run_stats(RunStats* p) { _run_stats = p; }
    void set_difficulty(DifficultyLevel* p) { _difficulty = p; }
    void set_enemy_spawn_count(int count) { _requested_enemy_count = std::max(0, count); }
    void set_rng(std::mt19937* r) { _rng = r; }

    void enable_stress_mode(int enemy_count) {
        _stress_mode = true;
        _requested_enemy_count = std::max(3, enemy_count);
    }

    int enemy_count() const { return (int)_enemies.size(); }

    int living_enemy_count() const {
        return (int)std::count_if(_enemies.begin(), _enemies.end(),
                                  [](const Actor& a) { return a.is_alive; });
    }

    void enter() override {
        // TextureCache inicializado uma vez (persiste across restarts)
        if (_renderer && !_tex_cache.has_value())
            _tex_cache.emplace(_renderer);

        _load_data_files();
        _register_dungeon_dialogue();
        _dialogue.set_audio(_audio);
        {
            SaveData          loaded;
            if (SaveSystem::load_default(loaded))
                _apply_save_and_build_world(loaded);
            else
                _full_run_reset_initial(false);
        }

        _death_snapshot_done         = false;
        _death_fade_remaining        = 0.f;
        _prev_level_choice_pending   = false;
        _floating_texts.texts.clear();
        _screen_flash_duration       = 0.25f;
        _hitstop            = 0;
        _pending_next_scene.clear();
        _scene_exit_pending = false;
        _scene_exit_timer   = 0.f;
        _scene_exit_target.clear();
        _skill_tree_open = false;
        _build_skill_tree_columns();
        _pause_menu.init({"RESUME", "SKILL TREE", "SETTINGS", "QUIT TO MENU"});
        _pause_menu.on_item_selected(0, [this]{ _pause_menu.paused = false; });
        _pause_menu.on_item_selected(1, [this]{
            _skill_tree_open = true;
            _pause_menu.paused = false;
            _st_selected_col = 0;
            _st_selected_row = 0;
            _st_clamp_cursor();
        });
        _pause_menu.on_item_selected(2, [this]{ _pause_menu.settings_open = true; });
        _pause_menu.on_item_selected(3, [this]{
            _pending_next_scene = "title";
            _pause_menu.paused  = false;
            if (_audio) _audio->fade_music(400);
        });
        _camera.viewport_w = (float)viewport_w;
        _camera.viewport_h = (float)viewport_h;

        if (_renderer) _lighting.init(_renderer, viewport_w, viewport_h);
        if (_audio) {
            _audio->stop_all_ambient();
            _audio->play_ambient(AmbientId::DungeonDrips, 0.22f);
            _audio->play_ambient(AmbientId::DungeonTorch, 0.14f);
            _audio->set_music_state(MusicState::DungeonCalm);
        }
        _sync_footstep_anchors();

        if (_run_stats)
            _run_stats->max_level_reached =
                std::max(_run_stats->max_level_reached, _player.progression.level);
    }

    void exit() override {
        _lighting.destroy();
        if (_audio) {
            _audio->stop_all_ambient();
            _audio->stop_music();
        }
        for (SDL_Texture* t : _tiled_textures) SDL_DestroyTexture(t);
        _tiled_textures.clear();
    }

    // Repete cada linha da spritesheet `mult` vezes para simular mais frames.
    // Chamado após enter() no render_stress para testar impacto de sheets maiores.
    void tile_sprite_sheets(int mult) {
        if (mult <= 1 || !_renderer) return;

        // Cache: pares (original, tiled) — busca linear, poucos tipos de textura
        struct TexPair { void* orig; SDL_Texture* tiled; };
        std::vector<TexPair> cache;

        // Cria (ou recupera do cache) versão N-vezes mais larga da textura
        auto get_tiled = [&](void* orig_ptr, float orig_w, float orig_h) -> SDL_Texture* {
            for (const auto& p : cache)
                if (p.orig == orig_ptr) return p.tiled;

            int new_w = (int)(orig_w * mult);
            SDL_Texture* orig  = static_cast<SDL_Texture*>(orig_ptr);
            SDL_Texture* tiled = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_RGBA8888,
                                                   SDL_TEXTUREACCESS_TARGET,
                                                   new_w, (int)orig_h);
            if (!tiled) { cache.push_back({orig_ptr, nullptr}); return nullptr; }

            SDL_SetTextureBlendMode(tiled, SDL_BLENDMODE_BLEND);
            SDL_SetRenderTarget(_renderer, tiled);
            SDL_SetRenderDrawColor(_renderer, 0, 0, 0, 0);
            SDL_RenderClear(_renderer);
            for (int i = 0; i < mult; ++i) {
                SDL_FRect dst = { orig_w * (float)i, 0.0f, orig_w, orig_h };
                SDL_RenderTexture(_renderer, orig, nullptr, &dst);
            }
            SDL_SetRenderTarget(_renderer, nullptr);

            _tiled_textures.push_back(tiled);
            cache.push_back({orig_ptr, tiled});
            return tiled;
        };

        // Repete os frames de cada clip, ajustando o x pela largura original
        auto expand_anim = [&](AnimPlayer& anim, int orig_w) {
            for (auto& clip : anim.clips) {
                if (clip.frames.empty()) continue;
                auto src = clip.frames;
                clip.frames.clear();
                clip.frames.reserve(src.size() * (size_t)mult);
                for (int m = 0; m < mult; ++m)
                    for (auto f : src) {
                        f.src.x += m * orig_w;
                        clip.frames.push_back(f);
                    }
            }
        };

        // Player
        if (_player.sprite_sheet) {
            float tw = 0, th = 0;
            SDL_GetTextureSize(static_cast<SDL_Texture*>(_player.sprite_sheet), &tw, &th);
            SDL_Texture* t = get_tiled(_player.sprite_sheet, tw, th);
            if (t) { _player.sprite_sheet = t; expand_anim(_player.anim, (int)tw); }
        }

        // Inimigos
        for (auto& e : _enemies) {
            if (!e.sprite_sheet) continue;
            float tw = 0, th = 0;
            SDL_GetTextureSize(static_cast<SDL_Texture*>(e.sprite_sheet), &tw, &th);
            int ow = (int)tw;
            void* op = e.sprite_sheet;
            SDL_Texture* t = get_tiled(op, tw, th);
            if (t) { e.sprite_sheet = t; expand_anim(e.anim, ow); }
        }
    }

    void fixed_update(float dt, const InputState& input) override {
        _pending_next_scene.clear();

        if (_autosave_flash > 0.f) {
            _autosave_flash -= dt;
            if (_autosave_flash < 0.f) _autosave_flash = 0.f;
        }

        if (_boss_intro_pending) {
            _boss_intro_timer += dt;
            if (_boss_intro_timer >= kBossIntroDuration)
                _boss_intro_pending = false;
        }
        if (_screen_flash_timer > 0.0f) {
            _screen_flash_timer -= dt;
            if (_screen_flash_timer < 0.0f)
                _screen_flash_timer = 0.0f;
        }

        if (_hitstop > 0) {
            --_hitstop;
            _flush_menu_input_prev(input);
            return;
        }

        if (_dialogue.is_active()) {
            _dialogue.fixed_update(input);
            _flush_menu_input_prev(input);
            return;
        }

        if (_handle_pause_and_skill_ui(input)) {
            _flush_menu_input_prev(input);
            return;
        }

        // Level-up: abre tela de atributos automaticamente
        if (_player.progression.level_choice_pending()) {
            if (!_attr_screen_open) {
                _attr_screen_open = true;
                _attr_selected = 0;
                _floating_texts.spawn_level_up(_player.transform.x, _player.transform.y);
                if (_audio) _audio->play_sfx(SoundId::UiConfirm, 0.8f);
            }
            // Enquanto a tela estiver aberta, bloqueia mundo
            _flush_menu_input_prev(input);
            return;
        }

        _prev_upgrade_1 = input.upgrade_1;
        _prev_upgrade_2 = input.upgrade_2;
        _prev_upgrade_3 = input.upgrade_3;

        // Skill tree: pausa mundo enquanto houver pontos pendentes
        if (_talent_selection_pending()) {
            _flush_menu_input_prev(input);
            return;
        }
        _prev_talent_1 = input.talent_1_pressed;
        _prev_talent_2 = input.talent_2_pressed;
        _prev_talent_3 = input.talent_3_pressed;

        // 1. Reset de flags + knockback (antes de qualquer input/IA)
        const int _hp_track0 = _player.health.current_hp;
        const int _gold_track0 = _player.gold;
        _movement_sys.fixed_update(_actors, dt);

        // 2. Player intents (input → ações) + spawn de projéteis
        InputState play_in = input;
        if (_boss_intro_pending && _boss_intro_timer < kBossIntroDuration && !_stress_mode) {
            play_in.move_x = play_in.move_y = 0.0f;
            play_in.attack_pressed          = false;
            play_in.confirm_pressed         = false;
            play_in.ranged_pressed          = false;
            play_in.parry_pressed           = false;
            play_in.dash_pressed            = false;
            play_in.spell_1_pressed         = false;
            play_in.spell_2_pressed         = false;
            play_in.spell_3_pressed         = false;
            play_in.spell_4_pressed         = false;
            play_in.upgrade_1               = false;
            play_in.upgrade_2               = false;
            play_in.upgrade_3               = false;
            play_in.talent_1_pressed        = false;
            play_in.talent_2_pressed        = false;
            play_in.talent_3_pressed        = false;
        }
        int spell_casts_frame = 0;
        _player_action.fixed_update(_player, play_in, dt, _audio, &_projectiles, &_actors,
                                    &spell_casts_frame);

        // Atualiza timer de cast para animação (0.4 s após cada cast)
        if (spell_casts_frame > 0)
            _player_cast_timer = 0.4f;
        else
            _player_cast_timer = std::max(0.0f, _player_cast_timer - dt);

        // 3. Resources tick (stamina regen) + status effects tick
        _resource_sys.fixed_update(_actors, dt);
        _status_sys.fixed_update(_actors, dt);

        // 4. Avança timers de combate
        for (auto* a : _actors) if (a->is_alive) a->combat.advance_time(dt);

        // 5. IA de inimigos (+ projéteis inimigos)
        _enemy_ai.fixed_update(_actors, dt, &_pathfinder, &_projectiles);

        if (!_stress_mode) {
            for (auto& e : _enemies) {
                if (e.name != "Grimjaw") continue;
                if (e.boss_phase == 2 && !_boss_phase2_triggered) {
                    _boss_phase2_triggered = true;
                    _camera.trigger_shake(18.0f, 30);
                    _dialogue.start("boss_grimjaw_phase2");
                    _screen_flash_timer     = 0.3f;
                    _screen_flash_duration  = 0.3f;
                    _screen_flash_color     = {255, 200, 50, 180};
                }
            }
        }

        // 6. Colisão
        _col_sys.resolve_all(_actors, _room);
        _col_sys.resolve_actors(_actors, &_room);

        const float _cam_aud_x = _camera.x + _camera.viewport_w * 0.5f;
        const float _cam_aud_y = _camera.y + _camera.viewport_h * 0.5f;

        // 6b. Projéteis
        _projectile_sys.fixed_update(_projectiles, _actors, _room, dt);
        if (_projectile_sys.projectile_hit_actor) {
            if (_rng) _particles.spawn_burst(_projectile_sys.last_hit_world_x,
                                   _projectile_sys.last_hit_world_y,
                                   12, 255, 210, 120, 80.f, 200.f, *_rng);
        }
        for (const auto& ev : _projectile_sys.last_hit_events) {
            for (auto* ac : _actors) {
                if (ac->name != ev.target_name) continue;
                ac->hit_flash_timer = 0.15f;
                _floating_texts.spawn_damage(ac->transform.x, ac->transform.y, ev.damage);
                if (ac->team == Team::Player) {
                    _screen_flash_timer    = 0.25f;
                    _screen_flash_duration = 0.25f;
                    _screen_flash_color    = {180, 20, 20, 120};
                }
                if (_audio)
                    _audio->play_sfx_at(SoundId::Hit, ac->transform.x, ac->transform.y, _cam_aud_x,
                                        _cam_aud_y, 620.f, 1.0f);
                break;
            }
        }

        // 7. Resolve combate melee
        _combat_sys.fixed_update(_actors, dt);

        // 8. Efeitos de hit (hitstop, camera shake, áudio)
        if (_combat_sys.pending_hitstop > 0) {
            _hitstop = _combat_sys.pending_hitstop;
            if (_combat_sys.player_hit_landed)
                _camera.trigger_shake(6.0f, 8);
        }
        if (_combat_sys.parry_success && _audio)
            _audio->play_sfx(SoundId::Parry, 0.85f);

        if (!_combat_sys.last_events.empty()) {
            for (const auto& ev : _combat_sys.last_events) {
                for (auto* ac : _actors) {
                    if (ac->name == ev.target_name) {
                        ac->hit_flash_timer = 0.15f;
                        _floating_texts.spawn_damage(ac->transform.x, ac->transform.y, ev.damage);
                        if (ac->team == Team::Player) {
                            _screen_flash_timer    = 0.25f;
                            _screen_flash_duration = 0.25f;
                            _screen_flash_color    = {180, 20, 20, 120};
                        }
                        if (_audio)
                            _audio->play_sfx_at(SoundId::Hit, ac->transform.x, ac->transform.y,
                                                _cam_aud_x, _cam_aud_y, 620.f, 1.0f);
                        if (_rng) _particles.spawn_burst(ac->transform.x, ac->transform.y,
                                                 14, 255, 200, 160, 60.f, 180.f, *_rng);
                        break;
                    }
                }
            }
        }

        // 9. Mortes → drops + XP
        for (auto* a : _actors) {
            if (a->was_alive && !a->is_alive) {
                if (a->team == Team::Enemy) {
                    if (_rng) _particles.spawn_burst(a->transform.x, a->transform.y,
                                           22, 200, 60, 80, 40.f, 140.f, *_rng);
                    const EnemyDef& def = _enemy_defs[static_cast<int>(a->enemy_type)];
                    if (_rng) DropSystem::on_enemy_died(_ground_items, a->transform.x, a->transform.y, def,
                                              _drop_config, *_rng, -1, -1);
                    int gained = _player.progression.add_xp(
                        dungeon_rules::xp_per_enemy_kill(_room_index));
                    _player.talents.pending_points += gained;
                    if (_run_stats)
                        _run_stats->max_level_reached =
                            std::max(_run_stats->max_level_reached, _player.progression.level);
                    if (!_stress_mode && a->name == "Grimjaw") {
                        if (_run_stats)
                            _run_stats->boss_defeated = true;
                        _post_mortem_dialogue_id = "miniboss_grimjaw_death";
                        if (_quest_state.is(QuestId::DefeatGrimjaw, QuestStatus::InProgress)) {
                            _quest_state.set(QuestId::DefeatGrimjaw, QuestStatus::Completed);
                            _persist_save();
                        }
                    }
                    if (_run_stats)
                        _run_stats->enemies_killed++;
                }
                if (_audio) {
                    if (a->team == Team::Enemy)
                        _audio->play_sfx(SoundId::EnemyDeath);
                    else
                        _audio->play_sfx(SoundId::PlayerDeath);
                }
            }
            a->was_alive = a->is_alive;
        }

        {
            const bool lcp = _player.progression.level_choice_pending();
            if (lcp && !_prev_level_choice_pending) {
                // abertura da attr screen já tratada acima
            }
            _prev_level_choice_pending = lcp;
        }

        if (DropSystem::pickup_near_player(_player, _ground_items, _drop_config)
            && !_stress_mode && !_dialogue.is_active())
            _dialogue.start("dungeon_rare_relic");

        // 10. Fluxo de sala
        _room_flow_sys.fixed_update(_actors, _player, _room);
        if (!_room_flow_sys.scene_exit_to.empty()) {
            const std::string& target = _room_flow_sys.scene_exit_to;
            if (target == "title" || target == "town") {
                if (!_scene_exit_pending) {
                    _scene_exit_pending = true;
                    _scene_exit_timer   = 0.4f;
                    _scene_exit_target  = target;
                    _persist_save();
                    if (_audio) _audio->fade_music(400);
                }
            } else {
                _pending_next_scene = target;
            }
        } else if (_room_flow_sys.transition_requested) {
            if (!_stress_mode && _room_index == 0 && !_first_door_dialogue_done
                && !_dialogue.is_active()) {
                _dialogue.start("dungeon_first_door", [this]() {
                    _first_door_dialogue_done = true;
                    _advance_room();
                });
            } else {
                _advance_room();
            }
        }

        if (!_post_mortem_dialogue_id.empty() && !_dialogue.is_active()
            && _pending_next_scene.empty()) {
            std::string id = _post_mortem_dialogue_id;
            _post_mortem_dialogue_id.clear();
            _dialogue.start(id);
        }
        if (_pending_room_blurb && !_dialogue.is_active() && _pending_next_scene.empty()) {
            _pending_room_blurb = false;
            _dialogue.start("dungeon_deeper");
        }

        if (_scene_exit_pending) {
            _scene_exit_timer -= dt;
            if (_scene_exit_timer <= 0.f) {
                std::string target = _scene_exit_target;
                _scene_exit_pending = false;
                _scene_exit_target.clear();
                if (target == "town" && !_stress_mode && _run_stats && _run_stats->boss_defeated
                    && _quest_state.is(QuestId::DefeatGrimjaw, QuestStatus::Completed)) {
                    _snapshot_last_run_to_save();
                    _pending_next_scene = "victory";
                } else {
                    _pending_next_scene = std::move(target);
                }
            }
        }

        if (_audio && !_stress_mode) {
            MusicState target = MusicState::DungeonCalm;
            bool        boss_alive = false;
            if (_room_index == 3) {
                for (const auto& e : _enemies) {
                    if (e.is_alive && e.name == "Grimjaw") {
                        boss_alive = true;
                        break;
                    }
                }
            }
            if (boss_alive)
                target = MusicState::DungeonBoss;
            else {
                bool any_aggro = false;
                for (const auto& e : _enemies) {
                    if (!e.is_alive) continue;
                    float dx = _player.transform.x - e.transform.x;
                    float dy = _player.transform.y - e.transform.y;
                    if (dx * dx + dy * dy < e.aggro_range * e.aggro_range) {
                        any_aggro = true;
                        break;
                    }
                }
                target = any_aggro ? MusicState::DungeonCombat : MusicState::DungeonCalm;
            }
            if (target != _audio->current_music_state())
                _audio->set_music_state(target);
        }

        if (_run_stats) {
            if (_hp_track0 > _player.health.current_hp)
                _run_stats->damage_taken += _hp_track0 - _player.health.current_hp;
            if (_player.gold > _gold_track0)
                _run_stats->gold_collected += _player.gold - _gold_track0;
            _run_stats->time_seconds += dt;
            _run_stats->spells_cast += spell_casts_frame;
        }

        if (!_player.is_alive) {
            if (!_death_snapshot_done) {
                _death_snapshot_done = true;
                _snapshot_last_run_to_save();
                _death_fade_remaining = kDeathFadeSeconds;
            }
        }
        if (_death_fade_remaining > 0.f) {
            _death_fade_remaining -= dt;
            if (_death_fade_remaining <= 0.f) {
                _death_fade_remaining = 0.f;
                if (_pending_next_scene.empty())
                    _pending_next_scene = "game_over";
            }
        }

        // 11. Animações + partículas
        _particles.update(dt);
        _update_animations(dt);

        // 12. Câmera
        _camera.follow(_player.transform.x, _player.transform.y, _room.bounds);
        if (_rng) _camera.step_shake(*_rng);

        if (_audio) _audio->tick_music();

        if (_audio && !_stress_mode && _player.is_alive) {
            const float acx = _camera.x + _camera.viewport_w * 0.5f;
            const float acy = _camera.y + _camera.viewport_h * 0.5f;
            float       pdx = _player.transform.x - _player.footstep_prev_x;
            float       pdy = _player.transform.y - _player.footstep_prev_y;
            if (_player.is_moving) {
                _player.footstep_accum_dist += std::sqrt(pdx * pdx + pdy * pdy);
                while (_player.footstep_accum_dist >= kFootstepPlayerDist) {
                    _player.footstep_accum_dist -= kFootstepPlayerDist;
                    _audio->play_sfx(SoundId::FootstepPlayer, 0.26f);
                }
            } else
                _player.footstep_accum_dist = 0.f;
            _player.footstep_prev_x = _player.transform.x;
            _player.footstep_prev_y = _player.transform.y;

            for (auto& e : _enemies) {
                if (!e.is_alive) continue;
                float edx = e.transform.x - e.footstep_prev_x;
                float edy = e.transform.y - e.footstep_prev_y;
                if (e.is_moving) {
                    e.footstep_accum_dist += std::sqrt(edx * edx + edy * edy);
                    while (e.footstep_accum_dist >= kFootstepEnemyDist) {
                        e.footstep_accum_dist -= kFootstepEnemyDist;
                        _audio->play_sfx_at(SoundId::FootstepEnemy, e.transform.x, e.transform.y, acx,
                                            acy, 480.f, 0.2f);
                    }
                } else
                    e.footstep_accum_dist = 0.f;
                e.footstep_prev_x = e.transform.x;
                e.footstep_prev_y = e.transform.y;
            }
        }

        _player.hit_flash_timer = std::max(0.f, _player.hit_flash_timer - dt);
        for (auto& e : _enemies) e.hit_flash_timer = std::max(0.f, e.hit_flash_timer - dt);

        // Animação: player
        if (_player.sprite_sheet) {
            if (!_player.is_alive)
                _player.anim.play(ActorAnim::Death);
            else if (_player.is_dashing())
                _player.anim.play(ActorAnim::Dash);
            else if (_player_cast_timer > 0.0f)
                _player.anim.play(ActorAnim::Cast);
            else if (_player.combat.attack_phase != AttackPhase::Idle)
                _player.anim.play(ActorAnim::Attack);
            else if (_player.combat.hurt_stun_remaining_seconds > 0.0f)
                _player.anim.play(ActorAnim::Hurt);
            else if (_player.is_moving)
                _player.anim.play(ActorAnim::Walk);
            else
                _player.anim.play(ActorAnim::Idle);
            _player.anim.advance(dt);
        }

        // Animação: enemies
        for (auto& e : _enemies) {
            if (!e.sprite_sheet) continue;
            if (!e.is_alive)
                e.anim.play(ActorAnim::Death);
            else if (e.combat.attack_phase != AttackPhase::Idle)
                e.anim.play(ActorAnim::Attack);
            else if (e.combat.hurt_stun_remaining_seconds > 0.0f)
                e.anim.play(ActorAnim::Hurt);
            else if (e.is_moving)
                e.anim.play(ActorAnim::Walk);
            else
                e.anim.play(ActorAnim::Idle);
            e.anim.advance(dt);
        }

        _floating_texts.tick(dt);

        _flush_menu_input_prev(input);
    }

    void render(SDL_Renderer* r, float /*blend_factor*/) override {
        SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
        SDL_RenderClear(r);

        TextureCache* tex_cache = _tex_cache.has_value() ? &*_tex_cache : nullptr;
        DungeonWorldRenderer::render(r, {
            _camera,
            _room,
            _tilemap,
            _tile_renderer,
            tex_cache,
            _actors,
            _projectiles,
            _ground_items,
            _particles,
            _floating_texts,
            _lighting,
            _player,
            living_enemy_count() == 0,
        });

        render_dungeon_hud(r, viewport_w, viewport_h, _player, _room_index);

        if (_show_autosave_indicator && _autosave_flash > 0.f) {
            const char* tag = "Saved";
            float       tx  = (float)viewport_w - text_width(tag, 1) - 12.f;
            float       ty  = (float)viewport_h - 22.f;
            Uint8       al  = (Uint8)std::min(220.f, 90.f + 200.f * (_autosave_flash / 1.35f));
            draw_text(r, tx, ty, tag, 1, 140, 200, 160, al);
        }

        if (_player.is_alive && _attr_screen_open)
            render_attribute_screen(r, viewport_w, viewport_h,
                                    _player.attributes,
                                    _player.progression.pending_level_ups,
                                    _attr_selected);

        ScreenFx::render_boss_intro(
            r, viewport_w, viewport_h,
            !_stress_mode && _room_index == 3 && _boss_intro_pending,
            _boss_intro_timer, kBossIntroDuration);
        ScreenFx::render_death_fade(
            r, viewport_w, viewport_h, _player.is_alive,
            _death_fade_remaining, kDeathFadeSeconds);
        ScreenFx::render_screen_flash(
            r, viewport_w, viewport_h,
            _screen_flash_timer, _screen_flash_duration, _screen_flash_color);

        if (_dialogue.is_active())
            _dialogue.render(r, viewport_w, viewport_h);

        if (_attr_screen_open)
            render_attribute_screen(r, viewport_w, viewport_h,
                                    _player.attributes,
                                    _player.progression.pending_level_ups,
                                    _attr_selected);
        if (_skill_tree_open)
            render_skill_tree_overlay(r, viewport_w, viewport_h,
                                      _player.talents,
                                      _st_selected_col, _st_selected_row,
                                      _st_col_indices);
        _pause_menu.render(r, viewport_w, viewport_h);
    }

    const char* next_scene() const override {
        return _pending_next_scene.empty() ? "" : _pending_next_scene.c_str();
    }

    void clear_next_scene_request() override {
        _pending_next_scene.clear();
    }

private:
    static constexpr float kBossIntroDuration = 1.8f;
    static constexpr float kDeathFadeSeconds    = 1.5f;

    RoomDefinition      _room;
    Actor               _player;
    std::vector<Actor>  _enemies;    // inimigos tipados (Feature 2)
    std::vector<Actor*> _actors;
    RoomCollisionSystem _col_sys;
    MeleeCombatSystem   _combat_sys;
    EnemyAISystem       _enemy_ai;
    Pathfinder          _pathfinder;
    PlayerActionSystem  _player_action;
    MovementSystem      _movement_sys;
    ResourceSystem      _resource_sys;
    StatusEffectSystem  _status_sys;
    RoomFlowSystem      _room_flow_sys;
    EnemyDef            _enemy_defs[kEnemyTypeCount]{};
    DropConfig          _drop_config{};
    IniData             _rooms_ini{};
    DialogueSystem      _dialogue;
    ProjectileSystem    _projectile_sys;
    std::vector<Projectile> _projectiles;
    std::vector<GroundItem> _ground_items;
    int                 _room_index = 0;
    bool                _prev_upgrade_1 = false;
    bool                _prev_upgrade_2 = false;
    bool                _prev_upgrade_3 = false;
    bool                _prev_talent_1 = false;
    bool                _prev_talent_2 = false;
    bool                _prev_talent_3 = false;
    Camera2D            _camera;
    Tilemap             _tilemap;
    TilemapRenderer     _tile_renderer;
    LightingSystem      _lighting;
    SDL_Renderer*       _renderer   = nullptr;
    AudioSystem*        _audio      = nullptr;  // Feature 4 (não-proprietário)
    std::mt19937*       _rng        = nullptr;  // owned pela stack de main()
    std::optional<TextureCache> _tex_cache;     // Feature 1
    std::vector<SDL_Texture*>   _tiled_textures; // criadas por tile_sprite_sheets, destruídas no exit()

    bool                  _show_autosave_indicator = false;
    float                 _autosave_flash          = 0.f;
    bool                  _first_door_dialogue_done = false;
    bool                  _pending_room_blurb        = false;
    std::string           _post_mortem_dialogue_id;
    SimpleParticleSystem  _particles;

    FloatingTextSystem  _floating_texts;
    bool                  _death_snapshot_done       = false;
    float                 _death_fade_remaining      = 0.f;
    bool                  _prev_level_choice_pending = false;

    RunStats*   _run_stats   = nullptr;
    DifficultyLevel* _difficulty = nullptr;
    int         _hitstop    = 0;
    std::string _pending_next_scene;
    int         _requested_enemy_count = 3;
    bool        _stress_mode = false;

    // Fade + transição suave para outra cena (town / title)
    bool        _scene_exit_pending = false;
    float       _scene_exit_timer   = 0.f;
    std::string _scene_exit_target;

    QuestState _quest_state{};

    PauseMenu        _pause_menu;
    bool             _prev_ui_left = false;
    bool             _prev_ui_right = false;
    bool             _prev_tab = false;
    bool             _prev_menu_confirm = false;
    bool             _skill_tree_open = false;
    int              _st_selected_col = 0;
    int              _st_selected_row = 0;
    std::vector<int> _st_col_indices[3];

    // Tela de distribuição de atributos (level-up)
    bool             _attr_screen_open = false;
    int              _attr_selected    = 0;  // 0-4: Vigor/Força/Destreza/Intel/Endurance

    bool        _boss_phase2_triggered = false;
    bool        _boss_intro_pending    = false;
    float       _boss_intro_timer      = 0.0f;
    float       _player_cast_timer     = 0.0f;  // segundos restantes da anim. Cast
    float       _screen_flash_timer    = 0.0f;
    float       _screen_flash_duration = 0.25f;
    SDL_Color   _screen_flash_color    = {255, 255, 255, 0};

    void _snapshot_last_run_to_save() {
        if (_stress_mode || !_run_stats)
            return;
        SaveData d{};
        if (!SaveSystem::load_default(d))
            return;
        d.last_run_stats = *_run_stats;
        SaveSystem::save_default(d);
    }

    void _flush_menu_input_prev(const InputState& in) {
        _pause_menu.flush_input(in);
        _prev_ui_left        = in.ui_left_pressed;
        _prev_ui_right       = in.ui_right_pressed;
        _prev_tab            = in.skill_tree_pressed;
        _prev_menu_confirm   = in.confirm_pressed;
    }

    void _build_skill_tree_columns() {
        for (int c = 0; c < 3; ++c)
            _st_col_indices[c].clear();
        for (int i = 0; i < kTalentCount; ++i) {
            const int disc = static_cast<int>(g_talent_nodes[static_cast<size_t>(i)].discipline);
            if (disc >= 0 && disc < 3)
                _st_col_indices[disc].push_back(i);
        }
    }

    void _st_clamp_cursor() {
        const int nc = static_cast<int>(_st_col_indices[_st_selected_col].size());
        if (nc <= 0) {
            _st_selected_row = 0;
            return;
        }
        if (_st_selected_row >= nc)
            _st_selected_row = nc - 1;
        if (_st_selected_row < 0)
            _st_selected_row = 0;
    }

    bool _handle_pause_and_skill_ui(const InputState& input) {
        const bool esc_edge   = input.pause_pressed       && !_pause_menu.prev_esc;
        const bool tab_edge   = input.skill_tree_pressed  && !_prev_tab;
        const bool up_edge    = input.ui_up_pressed        && !_pause_menu.prev_up;
        const bool down_edge  = input.ui_down_pressed      && !_pause_menu.prev_down;
        const bool left_edge  = input.ui_left_pressed      && !_prev_ui_left;
        const bool right_edge = input.ui_right_pressed     && !_prev_ui_right;
        const bool conf_edge  = input.confirm_pressed      && !_prev_menu_confirm;
        const bool back_edge  = input.ui_cancel_pressed    && !_pause_menu.prev_cancel;

        // --- Attribute Screen (level-up) ---
        if (_attr_screen_open) {
            if (up_edge)   _attr_selected = (_attr_selected + 4) % 5;
            if (down_edge) _attr_selected = (_attr_selected + 1) % 5;
            if (conf_edge) {
                if (_player.progression.level_choice_pending()) {
                    const char* names[5] = {"+VIG","+FOR","+DES","+INT","+END"};
                    switch (_attr_selected) {
                    case 0: _player.attributes.vigor++;        break;
                    case 1: _player.attributes.forca++;        break;
                    case 2: _player.attributes.destreza++;     break;
                    case 3: _player.attributes.inteligencia++; break;
                    case 4: _player.attributes.endurance++;    break;
                    default: break;
                    }
                    _player.progression.pending_level_ups--;
                    recompute_player_derived_stats(
                        _player.derived, _player.attributes, _player.progression,
                        _player.talents, _player.equipment,
                        g_player_config.melee_damage, g_player_config.ranged_damage);
                    const int base_hp = _stress_mode ? 10000 : g_player_config.base_hp;
                    _player.health.max_hp = base_hp + _player.derived.hp_max_bonus;
                    if (_player.health.current_hp > _player.health.max_hp)
                        _player.health.current_hp = _player.health.max_hp;
                    _player.mana.max    = g_player_config.base_mana_max    + _player.derived.mana_max_bonus;
                    _player.stamina.max = g_player_config.base_stamina_max + _player.derived.stamina_max_bonus;
                    _floating_texts.spawn_upgrade(_player.transform.x, _player.transform.y,
                                                  names[_attr_selected]);
                    _persist_save();
                }
                if (!_player.progression.level_choice_pending())
                    _attr_screen_open = false;
            }
            if (esc_edge || back_edge)
                _attr_screen_open = false;
            _flush_menu_input_prev(input);
            return true;
        }

        if (_skill_tree_open) {
            if (esc_edge || tab_edge)
                _skill_tree_open = false;
            else {
                if (left_edge) {
                    _st_selected_col = (_st_selected_col + 2) % 3;
                    _st_clamp_cursor();
                }
                if (right_edge) {
                    _st_selected_col = (_st_selected_col + 1) % 3;
                    _st_clamp_cursor();
                }
                const int nrows = static_cast<int>(_st_col_indices[_st_selected_col].size());
                if (up_edge && nrows > 0)
                    _st_selected_row = (_st_selected_row + nrows - 1) % nrows;
                if (down_edge && nrows > 0)
                    _st_selected_row = (_st_selected_row + 1) % nrows;
                if (conf_edge && !_stress_mode && nrows > 0) {
                    const auto& col = _st_col_indices[_st_selected_col];
                    const int   ti  = col[static_cast<size_t>(_st_selected_row)];
                    if (_try_spend_talent(static_cast<TalentId>(ti)))
                        _persist_save();
                }
            }
            return true;
        }

        if (tab_edge && !_player.progression.level_choice_pending()
            && !_talent_selection_pending()) {
            _skill_tree_open = !_skill_tree_open;
            if (_skill_tree_open) {
                _st_selected_col = 0;
                _st_selected_row = 0;
                _st_clamp_cursor();
            }
        }

        bool blocked_open = _player.progression.level_choice_pending()
                            || _talent_selection_pending();
        if (blocked_open)
            return false;

        return _pause_menu.handle_input(input);
    }

    // -------------------------------------------------------------------
    // Spawn e reset
    // -------------------------------------------------------------------

    int _enemy_spawn_budget() const {
        return EnemySpawner::spawn_budget(
            _room_index, _stress_mode, _requested_enemy_count, _difficulty);
    }

    bool _talent_selection_pending() const {
        return _player.talents.pending_points > 0
            && _player.talents.has_unlockable_options();
    }

    int _collect_spendable_talent_ids(TalentId out[3]) const {
        int n = 0;
        for (int i = 0; i < kTalentCount && n < 3; ++i) {
            const auto id = static_cast<TalentId>(i);
            if (_player.talents.can_spend(id))
                out[n++] = id;
        }
        return n;
    }

    bool _try_spend_talent(TalentId id) {
        if (!_player.talents.try_unlock(id))
            return false;
        if (id == TalentId::ArcaneReservoir) {
            _player.mana.max += 30.0f;
            _player.mana.current += 30.0f;
            if (_player.mana.current > _player.mana.max)
                _player.mana.current = _player.mana.max;
        } else if (id == TalentId::ManaFlow) {
            _player.mana.regen_rate += 8.0f;
        }
        _sync_spell_unlocks_from_talents();
        return true;
    }

    void _sync_spell_unlocks_from_talents() {
        _player.spell_book.sync_from_talents(_player.talents);
    }

    void _rebuild_actor_pointers() {
        _actors.clear();
        _actors.push_back(&_player);
        for (auto& e : _enemies) _actors.push_back(&e);
    }

    void _sync_footstep_anchors() {
        _player.footstep_prev_x     = _player.transform.x;
        _player.footstep_prev_y     = _player.transform.y;
        _player.footstep_accum_dist = 0.f;
        for (auto& e : _enemies) {
            e.footstep_prev_x     = e.transform.x;
            e.footstep_prev_y     = e.transform.y;
            e.footstep_accum_dist = 0.f;
        }
    }

    void _configure_player_state(bool reset_health_to_max) {
        TextureCache* tc = _tex_cache.has_value() ? &*_tex_cache : nullptr;
        PlayerConfigureOptions opts;
        opts.reset_health_to_max = reset_health_to_max;
        opts.stress_mode         = _stress_mode;
        configure_player(_player, tc, opts);

        if (_stress_mode)
            RoomManager::place_player_at_room_center(_player, _room);
        else
            RoomManager::place_player_intro_spawn(_player, _room);
    }

    void _spawn_enemies_for_budget(int budget) {
        TextureCache* tc = _tex_cache.has_value() ? &*_tex_cache : nullptr;
        const EnemySpawnResult result = EnemySpawner::spawn_for_budget(
            _enemies, _room, _tilemap, _pathfinder, _player, _enemy_defs, tc,
            _room_index, budget, _stress_mode, _difficulty);
        _boss_phase2_triggered = result.boss_phase2_triggered;
        _boss_intro_pending    = result.boss_intro_pending;
        _boss_intro_timer      = result.boss_intro_timer;
        _rebuild_actor_pointers();
        _sync_footstep_anchors();
    }

    SaveData _capture_save_data() const {
        SaveData d;
        d.version     = kSaveFormatVersion;
        d.room_index  = _room_index;
        d.player_hp   = _player.health.current_hp;
        d.gold        = _player.gold;
        d.quest_state = _quest_state;
        d.progression = _player.progression;
        d.talents = _player.talents;
        d.mana    = _player.mana;
        d.stamina = _player.stamina;
        return d;
    }

    void _persist_save() {
        if (_stress_mode) return;
        SaveData d = _capture_save_data();
        SaveData disk{};
        if (SaveSystem::load_default(disk))
            d.victory_reached = disk.victory_reached;
        if (_difficulty)
            d.difficulty = static_cast<int>(*_difficulty);
        SaveSystem::save_default(d);
        if (_show_autosave_indicator) _autosave_flash = 1.35f;
    }

    void _apply_save_and_build_world(const SaveData& sd) {
        RoomManager::reset_room_runtime(_projectiles, _ground_items);
        _room_index           = sd.room_index;
        _player.progression   = sd.progression;
        _player.talents       = sd.talents;
        _player.attributes    = sd.attributes;
        _player.gold          = sd.gold;
        _quest_state        = sd.quest_state;
        if (_difficulty)
            *_difficulty = static_cast<DifficultyLevel>(std::clamp(sd.difficulty, 0, 2));
        RoomManager::build_room(_room, _tilemap, _room_index, _rooms_ini);
        _load_room_visuals();
        _pathfinder.build_nav(_tilemap, _room);
        _configure_player_state(false);
        RoomManager::place_player_for_loaded_room(_player, _room, _stress_mode, _room_index);

        int hp = sd.player_hp;
        if (hp > _player.health.max_hp) hp = _player.health.max_hp;
        if (hp < 1) hp = 1;
        _player.health.current_hp = hp;

        _player.mana    = sd.mana;
        _player.stamina = sd.stamina;
        if (_player.mana.current > _player.mana.max) _player.mana.current = _player.mana.max;
        if (_player.stamina.current > _player.stamina.max)
            _player.stamina.current = _player.stamina.max;

        _player.spell_book = SpellBookState{};
        _sync_spell_unlocks_from_talents();

        _spawn_enemies_for_budget(_enemy_spawn_budget());
        _first_door_dialogue_done = (_room_index > 0);
        _post_mortem_dialogue_id.clear();
        _pending_room_blurb       = false;
    }

    void _load_data_files() {
        reset_progression_config_defaults();
        reset_player_config_defaults();
        reset_talent_tree_defaults();

        for (int i = 0; i < kEnemyTypeCount; ++i)
            _enemy_defs[i] = get_enemy_def(static_cast<EnemyType>(i));
        _drop_config = DropConfig{};

        {
            IniData d = ini_load(resolve_data_path("progression.ini").c_str());
            if (!d.sections.empty())
                apply_progression_ini(d);
        }
        {
            IniData d = ini_load(resolve_data_path("player.ini").c_str());
            if (!d.sections.empty())
                apply_player_ini(d);
        }
        {
            IniData d = ini_load(resolve_data_path("talents.ini").c_str());
            if (!d.sections.empty())
                apply_talents_ini(d);
        }

        {
            IniData d = ini_load(resolve_data_path("enemies.ini").c_str());
            static std::array<std::string, kEnemyTypeCount> enemy_sprite_paths;
            static const char* kEnemyIniSecs[kEnemyTypeCount] = {
                "skeleton",      "orc",           "ghost",
                "archer",        "patrol_guard",  "elite_skeleton",
                "boss_grimjaw",
            };
            for (int i = 0; i < kEnemyTypeCount; ++i)
                apply_enemy_ini_section(d, kEnemyIniSecs[i], _enemy_defs[i],
                                        &enemy_sprite_paths[static_cast<size_t>(i)]);
        }

        // spells.ini
        {
            IniData d = ini_load(resolve_data_path("spells.ini").c_str());
            apply_spell_ini_section(d, "frost_bolt",
                                    g_spell_defs[static_cast<int>(SpellId::FrostBolt)]);
            apply_spell_ini_section(d, "bolt",
                                    g_spell_defs[static_cast<int>(SpellId::FrostBolt)]);
            apply_spell_ini_section(d, "nova",
                                    g_spell_defs[static_cast<int>(SpellId::Nova)]);
            apply_spell_ini_section(
                d, "chain_lightning",
                g_spell_defs[static_cast<int>(SpellId::ChainLightning)]);
            apply_spell_ini_section(d, "multi_shot",
                                    g_spell_defs[static_cast<int>(SpellId::MultiShot)]);
            apply_spell_ini_section(d, "poison_arrow",
                                    g_spell_defs[static_cast<int>(SpellId::PoisonArrow)]);
            apply_spell_ini_section(d, "strafe",
                                    g_spell_defs[static_cast<int>(SpellId::Strafe)]);
            apply_spell_ini_section(d, "cleave",
                                    g_spell_defs[static_cast<int>(SpellId::Cleave)]);
            apply_spell_ini_section(d, "battle_cry",
                                    g_spell_defs[static_cast<int>(SpellId::BattleCry)]);
        }

        // items.ini
        {
            IniData d = ini_load(resolve_data_path("items.ini").c_str());
            _drop_config.drop_chance_pct = d.get_int  ("drops", "drop_chance_pct", _drop_config.drop_chance_pct);
            _drop_config.pickup_radius   = d.get_float("drops", "pickup_radius",   _drop_config.pickup_radius);
            _drop_config.health_bonus    = d.get_int  ("drops", "health_bonus",    _drop_config.health_bonus);
            _drop_config.damage_bonus    = d.get_int  ("drops", "damage_bonus",    _drop_config.damage_bonus);
            _drop_config.speed_bonus     = d.get_float("drops", "speed_bonus",     _drop_config.speed_bonus);
            _drop_config.lore_drop_chance_pct =
                d.get_int("drops", "lore_drop_chance_pct", _drop_config.lore_drop_chance_pct);
        }

        _rooms_ini = ini_load(resolve_data_path("rooms.ini").c_str());
    }

    void _register_dungeon_dialogue() {
        {
            DialogueSequence prologue;
            prologue.id = "dungeon_prologue";
            prologue.lines = {
                { "Voice", "Few have braved these steps in years.", {180, 200, 255, 255} },
                { "Voice", "Torchlight on wet stone. Something stirs below.", {170, 210, 240, 255} },
                { "Voice", "Steel ready. The ruin remembers every footfall.", {200, 160, 120, 255} },
            };
            _dialogue.register_sequence(std::move(prologue));
        }
        {
            DialogueSequence deeper;
            deeper.id = "dungeon_room2";
            deeper.lines = {
                { "Voice", "Deeper now. The walls grow older here.", {160, 180, 220, 255} },
                { "Voice", "Whatever lurks ahead has not seen daylight in years.", {200, 160, 120, 255} },
            };
            _dialogue.register_sequence(std::move(deeper));
        }
        {
            DialogueSequence first;
            first.id = "dungeon_first_door";
            first.lines = {
                { "Voice", "The east gate only opens when the hall is silent.", {150, 200, 255, 255} },
                { "Voice", "Step through — the stone will remember your stride.", {180, 170, 140, 255} },
            };
            _dialogue.register_sequence(std::move(first));
        }
        {
            DialogueSequence deeper2;
            deeper2.id = "dungeon_deeper";
            deeper2.lines = {
                { "Voice", "Another span of carved darkness. Keep the rhythm.", {140, 190, 230, 255} },
            };
            _dialogue.register_sequence(std::move(deeper2));
        }
        {
            DialogueSequence relic;
            relic.id = "dungeon_rare_relic";
            relic.lines = {
                { "Voice", "A relic hums in your palm — old magic, thin but sharp.", {220, 200, 120, 255} },
            };
            _dialogue.register_sequence(std::move(relic));
        }
        {
            DialogueSequence mb;
            mb.id = "miniboss_grimjaw_death";
            mb.lines = {
                { "Grimjaw", "Hrk… the depths… were meant… to keep you…", {200, 80, 70, 255} },
                { "Voice", "The named brute falls. What waits past him listens closer now.", {160, 200, 240, 255} },
            };
            _dialogue.register_sequence(std::move(mb));
        }
        {
            DialogueSequence ph2;
            ph2.id = "boss_grimjaw_phase2";
            ph2.lines = {
                { "Grimjaw", "RAAAAGH! You dare…!!", {255, 80, 40, 255} },
            };
            _dialogue.register_sequence(std::move(ph2));
        }
    }

    void _full_run_reset_initial(bool clear_saved_progress) {
        if (clear_saved_progress)
            SaveSystem::remove_default_saves();
        if (_run_stats)
            _run_stats->reset();
        _room_index = 0;
        RoomManager::reset_room_runtime(_projectiles, _ground_items);
        _player.progression = ProgressionState{};
        _player.talents     = TalentState{};
        _player.gold        = 0;
        _quest_state        = {};
        RoomManager::build_room(_room, _tilemap, _room_index, _rooms_ini);
        _load_room_visuals();
        _pathfinder.build_nav(_tilemap, _room);
        _configure_player_state(true);
        _spawn_enemies_for_budget(_enemy_spawn_budget());
        if (!_stress_mode)
            _dialogue.start("dungeon_prologue");
        _first_door_dialogue_done = false;
        _post_mortem_dialogue_id.clear();
        _pending_room_blurb = false;
    }

    void _advance_room() {
        RoomManager::begin_next_room(_room_index, _run_stats);
        RoomManager::reset_room_runtime(_projectiles, _ground_items);
        RoomManager::build_room(_room, _tilemap, _room_index, _rooms_ini);
        _load_room_visuals();
        _pathfinder.build_nav(_tilemap, _room);
        RoomManager::place_player_for_room_advance(_player, _room, _stress_mode);
        _spawn_enemies_for_budget(_enemy_spawn_budget());
        if (!_stress_mode && _room_index == 2)
            _dialogue.start("dungeon_room2");
        else if (!_stress_mode && _room_index >= 3)
            _pending_room_blurb = true;
        _persist_save();
        _sync_footstep_anchors();
    }

    void _load_room_visuals() {
        if (_tex_cache.has_value()) {
            _tile_renderer.tileset      = _tex_cache->load("assets/tiles/dungeon_tileset.png");
            _tile_renderer.floor_tile_col = 0; _tile_renderer.floor_tile_row = 0;
            _tile_renderer.wall_tile_col  = 1; _tile_renderer.wall_tile_row  = 0;
        } else {
            _tile_renderer.tileset = nullptr;
        }
    }

    // _facing_to_puny_row extracted to world_renderer.hpp → facing_to_puny_row()

    // Drive de animação — chamado uma vez por fixed_update
    // -------------------------------------------------------------------
    void _update_animations(float dt) {
        for (auto* a : _actors) {
            a->anim.update_puny_dir_row(facing_to_puny_row(a->facing_x, a->facing_y));
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

};

} // namespace mion
