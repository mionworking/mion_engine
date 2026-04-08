#pragma once
#include <array>
#include <vector>
#include <optional>
#include <random>
#include <string>
#include <cstdio>
#include <algorithm>
#include <cmath>
#include <SDL3/SDL.h>

#include "../core/scene.hpp"
#include "../core/camera.hpp"
#include "../core/bitmap_font.hpp"
#include "../core/sprite.hpp"
#include "../core/audio.hpp"
#include "../core/quest_state.hpp"
#include "../core/run_stats.hpp"
#include "../core/locale.hpp"
#include "../core/engine_paths.hpp"
#include "../core/dungeon_dialogue.hpp"
#include "../core/ini_loader.hpp"
#include "../core/ui.hpp"
#include "../core/debug_log.hpp"

#include "../world/room.hpp"
#include "../world/tilemap.hpp"
#include "../world/dungeon_rules.hpp"
#include "../world/world_map.hpp"
#include "../world/world_map_builder.hpp"
#include "../world/world_context.hpp"
#include "../world/zone_manager.hpp"

#include "../entities/actor.hpp"
#include "../entities/enemy_type.hpp"
#include "../entities/projectile.hpp"
#include "../entities/ground_item.hpp"
#include "../entities/npc.hpp"
#include "../entities/shop.hpp"

#include "../components/player_config.hpp"
#include "../components/talent_tree.hpp"
#include "../components/spell_defs.hpp"
#include "../components/spell_book.hpp"

#include "../systems/room_collision.hpp"
#include "../systems/melee_combat.hpp"
#include "../systems/enemy_ai.hpp"
#include "../systems/tilemap_renderer.hpp"
#include "../systems/lighting.hpp"
#include "../systems/world_renderer.hpp"
#include "../systems/player_configurator.hpp"
#include "../systems/animation_driver.hpp"
#include "../systems/dungeon_hud.hpp"
#include "../systems/skill_tree_ui.hpp"
#include "../systems/attribute_screen_ui.hpp"
#include "../systems/attribute_levelup_controller.hpp"
#include "../systems/pause_menu_controller.hpp"
#include "../systems/skill_tree_controller.hpp"
#include "../systems/equipment_screen_controller.hpp"
#include "../systems/player_action.hpp"
#include "../systems/movement_system.hpp"
#include "../systems/resource_system.hpp"
#include "../systems/status_effect_system.hpp"
#include "../systems/enemy_spawner.hpp"
#include "../systems/screen_fx.hpp"
#include "../systems/projectile_system.hpp"
#include "../systems/drop_system.hpp"
#include "../systems/footstep_audio_system.hpp"
#include "../systems/simple_particles.hpp"
#include "../systems/floating_text.hpp"
#include "../systems/dialogue_system.hpp"
#include "../systems/dialogue_render.hpp"
#include "../systems/boss_state_controller.hpp"
#include "../systems/combat_fx_controller.hpp"
#include "../systems/run_stats_tracker.hpp"
#include "../systems/enemy_death_controller.hpp"
#include "../systems/dungeon_config_loader.hpp"
#include "../systems/town_config_loader.hpp"
#include "../systems/town_builder.hpp"
#include "../systems/town_npc_interaction.hpp"
#include "../systems/town_npc_wander.hpp"
#include "../systems/npc_actor_factory.hpp"
#include "../systems/town_world_renderer.hpp"
#include "../systems/dungeon_world_renderer.hpp"
#include "../systems/shop_system.hpp"
#include "../systems/shop_input_controller.hpp"
#include "../systems/area_entry_system.hpp"
#include "../systems/world_save_controller.hpp"
#include "../systems/world_audio_system.hpp"
#include "../systems/overlay_input.hpp"
#include "../core/scene_ids.hpp"

namespace mion {

class WorldScene : public IScene {
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
    void set_locale(LocaleSystem* l) { _locale = l; }
    void set_enemy_spawn_count(int count) { _requested_enemy_count = std::max(0, count); }
    void set_rng(std::mt19937* r) { _rng = r; }

    void enable_stress_mode(int enemy_count) {
        _stress_mode = true;
        _requested_enemy_count = std::max(3, enemy_count);
    }

    int enemy_count() const {
        int n = 0;
        for (const auto& area : _world_map.areas)
            n += static_cast<int>(area.enemies.size());
        return n;
    }

    int living_enemy_count() const {
        int n = 0;
        for (const auto& area : _world_map.areas)
            for (const auto& e : area.enemies)
                if (e.is_alive) ++n;
        return n;
    }

    const Actor& player_actor() const { return _player; }

    // Widen sprite sheets (render stress); duplicates DungeonScene behavior over all areas.
    void tile_sprite_sheets(int mult);

    // ------------------------------------------------------------------
    // enter()
    // ------------------------------------------------------------------
    void enter() override {
        if (_renderer && !_tex_cache.has_value())
            _tex_cache.emplace(_renderer);

        // 1. Load configs
        // DungeonConfigLoader already calls CommonPlayerProgressionLoader —
        // no need to call TownConfigLoader::load_town_player_and_progression_config separately.
        DungeonConfigLoader::load_dungeon_static_data(
            _enemy_defs, _drop_config, _rooms_ini, _enemy_sprite_paths);

        // 2. Register dialogues
        register_dungeon_dialogue(_dialogue);
        TownConfigLoader::load_town_dialogues(_dialogue);
        _dialogue.set_audio(_audio);

        // 3. Build WorldMap
        TextureCache* tc = _tex_cache.has_value() ? &*_tex_cache : nullptr;
        _world_map = WorldMapBuilder::build(
            _rooms_ini, tc, _npcs, _shop_forge, _player, _tile_renderer);

        SaveData sd;
        const bool has_save = _load_save_or_start_fresh(sd);
        _configure_player_for_session(tc, has_save);
        _initialize_runtime_after_enter();

        if (_run_stats)
            _run_stats->max_level_reached =
                std::max(_run_stats->max_level_reached, _player.progression.level);
    }

    // ------------------------------------------------------------------
    // exit()
    // ------------------------------------------------------------------
    void exit() override {
        _lighting.destroy();
        if (_audio) {
            _audio->stop_all_ambient();
            _audio->stop_music();
        }
        for (SDL_Texture* t : _tiled_textures) SDL_DestroyTexture(t);
        _tiled_textures.clear();
    }

    // ------------------------------------------------------------------
    // fixed_update()
    // ------------------------------------------------------------------
    void fixed_update(float dt, const InputState& input) override {
        _pending_next_scene.clear();
        const OverlayInputEdges overlay_input = _overlay_input.capture(input, _pause_controller);

        if (_autosave_flash > 0.f) {
            _autosave_flash -= dt;
            if (_autosave_flash < 0.f) _autosave_flash = 0.f;
        }

        _screen_flash.tick(dt);
        _player.potion.update(dt);
        _zone_mgr.update(_player.transform.x, _player.transform.y, _world_map);

        if (_zone_mgr.just_changed)
            _handle_zone_transition();

        // --- Hitstop ---
        if (_hitstop > 0) {
            --_hitstop;
            _sync_gameplay_input_history(input);
            _flush_overlay_input(input);
            return;
        }

        // --- Dialogue ---
        if (_dialogue.is_active()) {
            _dialogue.fixed_update(input);
            _sync_gameplay_input_history(input);
            _flush_overlay_input(input);
            return;
        }

        // --- Pause ---
        if (_handle_pause_ui(overlay_input)) {
            _sync_gameplay_input_history(input);
            _flush_overlay_input(input);
            return;
        }

        // --- Attribute level-up controller ---
        {
            _attr_controller.sync_open_from_progression(_player);
            AttributeLevelUpResult r = _attr_controller.update(_player, overlay_input, _stress_mode);
            if (r.just_opened) {
                _floating_texts.spawn_level_up(_player.transform.x, _player.transform.y);
                if (_audio) _audio->play_sfx(SoundId::UiConfirm, 0.8f);
            }
            if (r.should_save) {
                static const char* kAttrNames[5] = {"+VIG", "+FOR", "+DES", "+INT", "+END"};
                if (r.applied_index >= 0 && r.applied_index < 5)
                    _floating_texts.spawn_upgrade(_player.transform.x, _player.transform.y,
                                                  kAttrNames[r.applied_index]);
                _persist_save();
            }
            if (r.world_paused) {
                _sync_gameplay_input_history(input);
                _flush_overlay_input(input);
                return;
            }
        }

        // --- Skill tree ---
        {
            SkillTreeResult r = _skill_tree_controller.update(_player, overlay_input, _stress_mode);
            if (r.should_save)
                _persist_save();
            if (r.world_paused) {
                _sync_gameplay_input_history(input);
                _flush_overlay_input(input);
                return;
            }
        }

        // --- Equipment screen ---
        {
            EquipmentResult er = _equipment_controller.update(_player, overlay_input, dt);
            if (er.should_save)
                _persist_save();
            if (er.world_paused) {
                _sync_gameplay_input_history(input);
                _flush_overlay_input(input);
                return;
            }
        }

        // --- Shop (town) ---
        if (_shop_open && _zone_mgr.in_town()) {
            const ShopInputResult shop_result = _update_shop(overlay_input);
            if (shop_result.should_save)
                _persist_save();
            _resource_sys.fixed_update(_actors, dt);
            _camera.follow(_player.transform.x, _player.transform.y, _world_map.total_bounds());
            if (_rng) _camera.step_shake(*_rng);
            if (_audio) _audio->tick_music();
            _sync_gameplay_input_history(input);
            _flush_overlay_input(input);
            return;
        }

        // Potion use (edge: pressed this frame, not last)
        if (input.potion_pressed && !_prev_potion)
            _player.potion.use(_player.health);

        const int gold0 = _player.gold;
        int       damage_taken_frame = 0;

        // 1. Movement
        _movement_sys.fixed_update(_actors, dt);

        // 2. Player input
        InputState play_in = input;
        if (!_stress_mode && _boss_state.intro_pending
            && _boss_state.intro_timer < BossState::kIntroDuration
            && _zone_mgr.current == ZoneId::Boss) {
            play_in.move_x = play_in.move_y = 0.0f;
            play_in.attack_pressed  = false;
            play_in.confirm_pressed = false;
            play_in.ranged_pressed  = false;
            play_in.parry_pressed   = false;
            play_in.dash_pressed    = false;
            play_in.spell_1_pressed = false;
            play_in.spell_2_pressed = false;
            play_in.spell_3_pressed = false;
            play_in.spell_4_pressed = false;
        }
        int spell_casts_frame = 0;
        _player_action.fixed_update(_player, play_in, dt, _audio, &_projectiles, &_actors,
                                    &spell_casts_frame);
        if (spell_casts_frame > 0)
            _player_cast_timer = 0.4f;
        else
            _player_cast_timer = std::max(0.0f, _player_cast_timer - dt);

        // 3. Resources + status
        _resource_sys.fixed_update(_actors, dt);
        _status_sys.fixed_update(_actors, dt);
        for (auto* a : _actors) if (a->is_alive) a->combat.advance_time(dt);

        // 4. AI + combat (dungeon only)
        if (_zone_mgr.in_dungeon()) {
            const WorldArea* cur_area = _world_map.area_at(
                _player.transform.x, _player.transform.y);

            float nav_ox = 0.f, nav_oy = 0.f;
            if (cur_area) {
                nav_ox = cur_area->offset_x;
                nav_oy = cur_area->offset_y;
            }
            _enemy_ai.fixed_update(
                _actors, dt,
                cur_area ? const_cast<Pathfinder*>(&cur_area->pathfinder) : nullptr,
                &_projectiles, nav_ox, nav_oy);
            _boss_state.update(_all_enemies(), _enemy_defs, _camera, _dialogue, _screen_flash,
                               dt, _stress_mode);

            if (cur_area) {
                _projectile_sys.fixed_update(
                    _projectiles, _actors, cur_area->room,
                    cur_area->offset_x, cur_area->offset_y, dt);
            } else {
                _projectile_sys.last_hit_events.clear();
            }

            const float cam_aud_x = _camera.x + _camera.viewport_w * 0.5f;
            const float cam_aud_y = _camera.y + _camera.viewport_h * 0.5f;

            CombatFxController::apply_projectile_hit_fx(
                _projectile_sys, _actors, _particles, _floating_texts,
                _screen_flash, _audio, cam_aud_x, cam_aud_y, _rng);

            _combat_sys.fixed_update(_actors, dt);
            damage_taken_frame += RunStatsTracker::damage_taken_for_actor(
                _player.name, _combat_sys.last_events, _projectile_sys.last_hit_events);
            CombatFxController::apply_combat_feedback(_combat_sys, _camera, _audio, _hitstop);
            CombatFxController::apply_melee_hit_fx(
                _combat_sys, _actors, _particles, _floating_texts,
                _screen_flash, _audio, cam_aud_x, cam_aud_y, _rng);

            auto dr = EnemyDeathController::process_deaths(
                _actors, _player, _enemy_defs, _drop_config, _ground_items,
                _run_stats, _quest_state, _particles, _audio,
                _zone_mgr.room_index_equiv(), _stress_mode, _rng);
            if (dr.xp_gained > 0)
                debug_log("XP gained=%d", dr.xp_gained);
            if (!dr.post_mortem_dialogue_id.empty())
                _scene_transition.queue_post_mortem_dialogue(dr.post_mortem_dialogue_id);
            if (dr.quest_completed || dr.boss_defeated)
                _persist_save();
        }
        for (auto* a : _actors) a->was_alive = a->is_alive;

        {
            const bool lcp = _player.progression.level_choice_pending();
            if (lcp && !_prev_level_choice_pending) {
                debug_log("Level choice pending: level=%d", _player.progression.level);
            }
            _prev_level_choice_pending = lcp;
        }

        if (_zone_mgr.in_dungeon()) {
            if (DropSystem::pickup_near_player(_player, _ground_items, _drop_config))
                _rare_relic_dialogue_pending = true;
        }

        if (_zone_mgr.in_dungeon()
            && _rare_relic_dialogue_pending
            && !_rare_relic_dialogue_shown
            && !_stress_mode
            && !_dialogue.is_active()) {
            _dialogue.start(DungeonDialogueId::kRareRelic);
            _rare_relic_dialogue_pending = false;
            _rare_relic_dialogue_shown = true;
        }

        // 5. NPCs (town only)
        if (_zone_mgr.in_town()) {
            TownNpcWanderSystem::update_town_npcs(_npcs, dt, _npc_rng_state);

            const std::optional<TownNpcInteractionController::NearbyNpcResult> near =
                TownNpcInteractionController::find_nearest_npc_for_interaction(_player, _npcs);
            _interacting_npc_hint = near.has_value() && near->in_interaction_range;

            const bool confirm_edge = input.confirm_pressed && !_prev_confirm;
            if (confirm_edge && near.has_value() && near->in_interaction_range) {
                TownNpcInteractionController::handle_npc_interaction(
                    near->index, _make_context(), _shop_open, kQuestRewardGold, [this]() {
                        _persist_save();
                    });
            }
        } else {
            _interacting_npc_hint = false;
        }

        // 6. Broadphase collision — always
        {
            AABB query = _player.collision.bounds_at(_player.transform.x, _player.transform.y);
            query.min_x -= 96.f; query.min_y -= 96.f;
            query.max_x += 96.f; query.max_y += 96.f;
            auto obs = _world_map.get_obstacles_near(query);
            _col_sys.resolve_all(_actors, obs, _world_map.total_bounds());
            _col_sys.resolve_actors(_actors, nullptr);
        }

        // 7. AreaEntrySystem — detect entering new areas, spawn enemies + dialogue
        _area_entry.update(_zone_mgr, _make_context(),
                           [](WorldArea& area, WorldContext& ctx) {
            if (!ctx.zone_mgr) return;

            // Dialogue on first entry
            const char* dlg_id = nullptr;
            if      (area.zone == ZoneId::DungeonRoom0) dlg_id = DungeonDialogueId::kPrologue;
            else if (area.zone == ZoneId::DungeonRoom1) dlg_id = DungeonDialogueId::kRoom2;
            else if (area.zone == ZoneId::DungeonRoom2) dlg_id = DungeonDialogueId::kDeeper;
            debug_log("[AreaEntry] zone=%d dlg=%s ctx.dialogue=%p active=%d",
                      (int)area.zone, dlg_id ? dlg_id : "none",
                      (void*)ctx.dialogue,
                      ctx.dialogue ? (int)ctx.dialogue->is_active() : -1);
            if (dlg_id && ctx.dialogue && !ctx.dialogue->is_active())
                ctx.dialogue->start(dlg_id);

            int budget = EnemySpawner::spawn_budget(
                ctx.zone_mgr->room_index_equiv(),
                ctx.stress_mode ? *ctx.stress_mode : false,
                ctx.requested_enemies ? *ctx.requested_enemies : 3,
                ctx.difficulty);
            auto sr = EnemySpawner::spawn_for_budget(
                area.enemies, area.room, area.tilemap, area.pathfinder,
                *ctx.player, ctx.enemy_defs, ctx.tex_cache,
                ctx.zone_mgr->room_index_equiv(), budget,
                ctx.stress_mode ? *ctx.stress_mode : false,
                ctx.difficulty,
                area.offset_x, area.offset_y);
            if (ctx.boss_state)
                ctx.boss_state->apply_spawn_result(
                    sr.boss_intro_pending, sr.boss_intro_timer, sr.boss_phase2_triggered);
        });
        if (_zone_mgr.just_changed)
            _rebuild_all_actors();

        // Post-mortem dialogue (boss death → victory after lines finish)
        if (auto id = _scene_transition.take_post_mortem_dialogue_to_start(
                _dialogue, !_pending_next_scene.empty());
            !id.empty()) {
            _dialogue.start(id, [this]() {
                _scene_transition.schedule_scene_exit(SceneId::kVictory, 0.8f);
            });
        }

        // 8. Audio + animations + camera
        if (_audio && !_stress_mode)
            WorldAudioSystem::update(*_audio, _zone_mgr, _player, _all_enemies(), _enemy_defs);

        RunStatsTracker::FrameStatsDelta frame_stats{};
        frame_stats.damage_taken = damage_taken_frame;
        frame_stats.gold_collected = (_player.gold > gold0) ? (_player.gold - gold0) : 0;
        frame_stats.spells_cast = spell_casts_frame;
        frame_stats.time_seconds = dt;
        RunStatsTracker::apply_frame_delta(_run_stats, frame_stats);

        if (auto next = _death_flow.tick(_player.is_alive, dt, [this]() { _snapshot_last_run(); }); !next.empty())
            _pending_next_scene = std::move(next);

        _particles.update(dt);

        AnimationDriver::update_player_dungeon_anim(_player, _player_cast_timer, dt);
        for (auto& area : _world_map.areas)
            for (auto& e : area.enemies)
                AnimationDriver::update_basic_actor_anim(e, dt);
        if (_zone_mgr.in_town())
            AnimationDriver::update_player_town_anim(_player, dt);

        _floating_texts.tick(dt);

        _camera.follow(_player.transform.x, _player.transform.y, _world_map.total_bounds());
        if (_rng) _camera.step_shake(*_rng);
        if (_audio) _audio->tick_music();

        if (_audio && !_stress_mode && _player.is_alive) {
            FootstepAudioSystem::update_footsteps(
                *_audio, _player, _all_enemies(), _camera,
                kFootstepPlayerDist, kFootstepEnemyDist);
        }

        _player.hit_flash_timer = std::max(0.f, _player.hit_flash_timer - dt);
        for (auto& area : _world_map.areas)
            for (auto& e : area.enemies)
                e.hit_flash_timer = std::max(0.f, e.hit_flash_timer - dt);

        // Scene exit handling
        if (auto next = _scene_transition.tick_scene_exit(dt); !next.empty())
            _pending_next_scene = std::move(next);

        _sync_gameplay_input_history(input);
        _flush_overlay_input(input);
    }

    // ------------------------------------------------------------------
    // render()
    // ------------------------------------------------------------------
    void render(SDL_Renderer* r, float /*blend_factor*/) override {
        SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
        SDL_RenderClear(r);

        TextureCache* tc = _tex_cache.has_value() ? &*_tex_cache : nullptr;

        auto visible = _world_map.areas_in_view(_camera);
        for (const auto* area : visible) {
            if (area->zone == ZoneId::Town) {
                _tile_renderer.render(r, _camera, area->tilemap);
                TownRenderContext town_ctx;
                town_ctx.camera     = &_camera;
                town_ctx.room       = &area->room;
                town_ctx.npcs       = &_npcs;
                town_ctx.tex_cache  = tc;
                town_ctx.player_sprite_scale = _player.sprite_scale;
                TownWorldRenderer::render_town_world(r, town_ctx);
            } else {
                TilemapRenderer area_tile_renderer;
                if (tc) {
                    area_tile_renderer.tileset = tc->load("assets/tiles/dungeon_tileset.png");
                    area_tile_renderer.floor_tile_col = 0; area_tile_renderer.floor_tile_row = 0;
                    area_tile_renderer.wall_tile_col  = 1; area_tile_renderer.wall_tile_row  = 0;
                    if (area->zone == ZoneId::Transition) {
                        area_tile_renderer.floor_tileset   = tc->load("assets/tiles/corridor_floor.png");
                        area_tile_renderer.floor_strip_cols = 6;
                    }
                }
                // Render dungeon area with offset applied to camera
                Camera2D offset_cam = _camera;
                offset_cam.x -= area->offset_x;
                offset_cam.y -= area->offset_y;

                // Inimigos em coords de mundo; o player é desenhado depois com `_camera`.
                std::vector<Actor*> area_actors;
                for (auto& e : const_cast<WorldArea*>(area)->enemies)
                    area_actors.push_back(&e);

                int living = 0;
                for (const auto& e : area->enemies)
                    if (e.is_alive) ++living;

                DungeonWorldRenderInputs din = {
                    offset_cam,
                    _camera,
                    area->room,
                    area->tilemap,
                    area_tile_renderer,
                    tc,
                    area_actors,
                    _projectiles,
                    _ground_items,
                    _particles,
                    _floating_texts,
                    _lighting,
                    _player,
                    living == 0,
                    false,
                };
                DungeonWorldRenderer::render(r, din);
            }
        }

        // Player sprite (world coords)
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

        if (!_stress_mode && _player.is_alive
            && (_zone_mgr.in_dungeon() || _zone_mgr.current == ZoneId::Transition)) {
            const float lx = _camera.world_to_screen_x(_player.transform.x);
            const float ly = _camera.world_to_screen_y(_player.transform.y);
            _lighting.render(r, lx, ly);
        }

        // HUD
        if (_zone_mgr.in_dungeon()) {
            render_dungeon_hud(
                r, viewport_w, viewport_h, _player, _zone_mgr.room_index_equiv(), _locale);
        } else if (_zone_mgr.in_town()) {
            if (_interacting_npc_hint && !_dialogue.is_active()) {
                const char* hint = tr("town_npc_interact_hint");
                draw_text(r, 16.0f, static_cast<float>(viewport_h) - 80.0f, hint, 2, 255, 220, 120, 255);
            }
            char gold_buf[48];
            SDL_snprintf(gold_buf, sizeof(gold_buf), tr("town_gold_label"), _player.gold);
            draw_text(r, 16.0f, 16.0f, gold_buf, 2, 255, 210, 100, 255);
        }

        if (_show_autosave_indicator && _autosave_flash > 0.f) {
            const char* tag = tr("ui_saved");
            float tx = static_cast<float>(viewport_w) - text_width(tag, 1) - 12.f;
            float ty = static_cast<float>(viewport_h) - 22.f;
            Uint8 al = static_cast<Uint8>(std::min(220.f, 90.f + 200.f * (_autosave_flash / 1.35f)));
            draw_text(r, tx, ty, tag, 1, 140, 200, 160, al);
        }

        if (_player.is_alive)
            _attr_controller.render(_player, r, viewport_w, viewport_h);

        ScreenFx::render_boss_intro(
            r, viewport_w, viewport_h,
            !_stress_mode && _zone_mgr.current == ZoneId::Boss && _boss_state.intro_pending,
            _boss_state.intro_timer, BossState::kIntroDuration, _locale);
        ScreenFx::render_death_fade(
            r, viewport_w, viewport_h, _player.is_alive,
            _death_flow.fade_remaining, PlayerDeathFlow::kFadeDuration);
        ScreenFx::render_screen_flash(r, viewport_w, viewport_h, _screen_flash);

        _skill_tree_controller.render(_player, r, viewport_w, viewport_h);
        _equipment_controller.render(_player, r, viewport_w, viewport_h);

        if (_dialogue.is_active())
            render_dialogue_ui(r, viewport_w, viewport_h, _dialogue, _locale);

        if (_shop_open)
            ShopSystem::render_shop_ui(
                r, _shop_forge, _player.gold, viewport_w, viewport_h, _locale);

        _pause_controller.render(r, viewport_w, viewport_h);
    }

    const char* next_scene() const override {
        return _pending_next_scene.empty() ? "" : _pending_next_scene.c_str();
    }

    void clear_next_scene_request() override {
        _pending_next_scene.clear();
    }

private:
    static constexpr int kQuestRewardGold = 120;

    // Owns transition state for post-mortem dialogue -> delayed scene exit.
    struct SceneTransitionFlow {
        std::string pending_post_mortem_dialogue_id;
        bool        scene_exit_pending = false;
        float       scene_exit_timer   = 0.f;
        std::string scene_exit_target;

        void reset() {
            pending_post_mortem_dialogue_id.clear();
            scene_exit_pending = false;
            scene_exit_timer   = 0.f;
            scene_exit_target.clear();
        }

        void queue_post_mortem_dialogue(std::string id) {
            pending_post_mortem_dialogue_id = std::move(id);
        }

        std::string take_post_mortem_dialogue_to_start(const DialogueSystem& dialogue,
                                                       bool scene_transition_pending) {
            if (pending_post_mortem_dialogue_id.empty() || dialogue.is_active() || scene_transition_pending)
                return {};
            return std::move(pending_post_mortem_dialogue_id);
        }

        void schedule_scene_exit(std::string t, float delay) {
            scene_exit_pending = true;
            scene_exit_timer   = delay;
            scene_exit_target  = std::move(t);
        }

        std::string tick_scene_exit(float dt) {
            if (!scene_exit_pending) return {};
            scene_exit_timer -= dt;
            if (scene_exit_timer > 0.f) return {};
            scene_exit_pending = false;
            return std::move(scene_exit_target);
        }
    };

    // === World ===
    WorldMap            _world_map;
    ZoneManager         _zone_mgr;
    AreaEntrySystem     _area_entry;

    // === Actors ===
    Actor               _player;
    std::vector<Actor*> _actors;
    std::vector<NpcEntity>  _npcs;
    std::vector<Actor>  _npc_actors;
    ShopInventory       _shop_forge;
    QuestState          _quest_state{};
    unsigned int        _scene_flags = 0;

    // === Always-active systems ===
    MovementSystem      _movement_sys;
    PlayerActionSystem  _player_action;
    ResourceSystem      _resource_sys;
    StatusEffectSystem  _status_sys;
    RoomCollisionSystem _col_sys;
    DialogueSystem      _dialogue;
    Camera2D            _camera;
    TilemapRenderer     _tile_renderer;

    // === Dungeon systems ===
    MeleeCombatSystem   _combat_sys;
    EnemyAISystem       _enemy_ai;
    ProjectileSystem    _projectile_sys;
    std::vector<Projectile>  _projectiles;
    std::vector<GroundItem>  _ground_items;
    LightingSystem      _lighting;
    BossState           _boss_state;
    SimpleParticleSystem _particles;
    FloatingTextSystem  _floating_texts;
    ScreenFlashState    _screen_flash;

    // === Config data ===
    EnemyDef            _enemy_defs[kEnemyTypeCount]{};
    DropConfig          _drop_config{};
    IniData             _rooms_ini{};
    std::array<std::string, kEnemyTypeCount> _enemy_sprite_paths{};

    // === UI / overlay ===
    PauseMenuController        _pause_controller;
    AttributeLevelUpController _attr_controller;
    SkillTreeController        _skill_tree_controller;
    EquipmentScreenController  _equipment_controller;
    OverlayInputTracker        _overlay_input;

    // === State flags ===
    bool    _shop_open = false;
    bool    _interacting_npc_hint = false;
    bool    _stress_mode = false;
    int     _requested_enemy_count = 3;
    float   _player_cast_timer = 0.0f;
    bool    _prev_potion    = false;
    bool    _prev_confirm   = false;
    PlayerDeathFlow _death_flow;
    bool    _prev_level_choice_pending = false;
    float   _autosave_flash = 0.f;
    int     _hitstop = 0;
    bool    _rare_relic_dialogue_pending = false;
    bool    _rare_relic_dialogue_shown = false;
    std::string            _pending_next_scene;
    SceneTransitionFlow    _scene_transition;
    bool                   _show_autosave_indicator = false;
    unsigned    _npc_rng_state = 0x12345678u;
    ShopInputController _shop_input;

    // === Non-owning ===
    SDL_Renderer*    _renderer   = nullptr;
    AudioSystem*     _audio      = nullptr;
    RunStats*        _run_stats  = nullptr;
    DifficultyLevel* _difficulty = nullptr;
    std::mt19937*    _rng        = nullptr;
    LocaleSystem*    _locale     = nullptr;
    std::optional<TextureCache> _tex_cache;
    std::vector<SDL_Texture*> _tiled_textures;

    // ------------------------------------------------------------------
    // Helpers
    // ------------------------------------------------------------------

    WorldContext _make_context() {
        WorldContext ctx;
        ctx.world_map    = &_world_map;
        ctx.zone_mgr     = &_zone_mgr;
        ctx.area_entry   = &_area_entry;
        ctx.player       = &_player;
        ctx.enemies      = nullptr; // enemies live per-area
        ctx.actors       = &_actors;
        ctx.npcs         = &_npcs;
        ctx.npc_actors   = &_npc_actors;
        ctx.shop_forge   = &_shop_forge;
        ctx.projectiles  = &_projectiles;
        ctx.ground_items = &_ground_items;
        ctx.dialogue     = &_dialogue;
        ctx.camera       = &_camera;
        ctx.audio        = _audio;
        ctx.run_stats    = _run_stats;
        ctx.difficulty   = _difficulty;
        ctx.quest_state  = &_quest_state;
        ctx.scene_flags  = &_scene_flags;
        ctx.rooms_ini    = &_rooms_ini;
        ctx.enemy_defs   = _enemy_defs;
        ctx.drop_config  = &_drop_config;
        ctx.stress_mode  = &_stress_mode;
        ctx.requested_enemies = &_requested_enemy_count;
        ctx.tile_renderer = &_tile_renderer;
        ctx.resource      = &_resource_sys;
        ctx.player_action = &_player_action;
        ctx.tex_cache     = _tex_cache.has_value() ? &*_tex_cache : nullptr;
        ctx.boss_state    = &_boss_state;
        return ctx;
    }

    std::vector<Actor*>& _all_enemies() {
        _cached_all_enemies.clear();
        for (auto& area : _world_map.areas)
            for (auto& e : area.enemies)
                _cached_all_enemies.push_back(&e);
        return _cached_all_enemies;
    }
    std::vector<Actor*> _cached_all_enemies;

    void _rebuild_all_actors() {
        _actors.clear();
        _actors.push_back(&_player);
        for (auto& area : _world_map.areas)
            for (auto& e : area.enemies)
                _actors.push_back(&e);
        NpcActorFactory::rebuild_npc_actors(_npcs, _player, _npc_actors, _actors);
    }

    void _fresh_run() {
        _quest_state        = {};
        _scene_flags        = 0;
        _player.gold        = 0;
        _player.progression = ProgressionState{};
        _player.talents     = TalentState{};
        _player.transform.set_position(400.f, 800.f);
    }

    bool _load_save_or_start_fresh(SaveData& sd) {
        const bool has_save = WorldSaveController::load_default_save(sd);
        if (!has_save) {
            _fresh_run();
            return false;
        }

        WorldContext wctx = _make_context();
        WorldSaveController::apply_world_save(wctx, sd);
        if (sd.player_world_x == 0.f && sd.player_world_y == 0.f)
            _player.transform.set_position(400.f, 800.f);
        return true;
    }

    void _configure_player_for_session(TextureCache* tc, bool has_save) {
        PlayerConfigureOptions opts;
        opts.reset_health_to_max = !has_save;
        opts.preserve_resources  = has_save;
        opts.stress_mode         = _stress_mode;
        configure_player(_player, tc, opts);

        if (has_save) {
            if (_player.health.current_hp > _player.health.max_hp) _player.health.current_hp = _player.health.max_hp;
            if (_player.health.current_hp < 1)                     _player.health.current_hp = 1;
        }

        _player.spell_book = SpellBookState{};
        _player.spell_book.sync_from_talents(_player.talents);
    }

    void _initialize_runtime_after_enter() {
        _rebuild_all_actors();

        _zone_mgr.update(_player.transform.x, _player.transform.y, _world_map);
        if (_audio)
            WorldAudioSystem::init_for_zone(*_audio, _zone_mgr.current);

        if (_renderer) _lighting.init(_renderer, viewport_w, viewport_h);
        _pause_controller = {};
        _pause_controller.set_locale(_locale);
        _pause_controller.init();
        _skill_tree_controller = {};
        _skill_tree_controller.set_locale(_locale);
        _skill_tree_controller.rebuild_columns();
        _attr_controller = {};
        _attr_controller.set_locale(_locale);
        _equipment_controller = {};
        _equipment_controller.set_locale(_locale);
        _camera.viewport_w = static_cast<float>(viewport_w);
        _camera.viewport_h = static_cast<float>(viewport_h);

        _death_flow.reset();
        _floating_texts.texts.clear();
        _screen_flash  = {};
        _hitstop       = 0;
        _pending_next_scene.clear();
        _scene_transition.reset();
        _prev_level_choice_pending = false;
        _rare_relic_dialogue_pending = false;
        _rare_relic_dialogue_shown = false;
        _sync_footstep_anchors();
    }

    void _persist_save() {
        if (!WorldSaveController::persist(_make_context(), _show_autosave_indicator, _autosave_flash))
            debug_log("WorldScene: failed to persist save");
    }

    void _snapshot_last_run() {
        if (!WorldSaveController::snapshot_last_run(_make_context()))
            debug_log("WorldScene: failed to snapshot last run");
    }

    void _handle_zone_transition() {
        if (_audio)
            WorldAudioSystem::init_for_zone(*_audio, _zone_mgr.current);

        if (_zone_mgr.current == ZoneId::Boss) {
            bool boss_exists = false;
            for (const auto& area : _world_map.areas) {
                if (area.zone != ZoneId::Boss) continue;
                for (const auto& e : area.enemies)
                    if (e.is_alive
                        && _enemy_defs[static_cast<int>(e.enemy_type)].is_zone_boss) {
                        boss_exists = true;
                        break;
                    }
            }
            if (!boss_exists)
                _boss_state = {};
        }
    }

    bool _handle_pause_ui(const OverlayInputEdges& edges) {
        PauseMenuResult r = _pause_controller.update(edges);
        if (r.should_open_skill_tree)
            _skill_tree_controller.open();
        if (r.should_quit_to_title) {
            _persist_save();
            _pending_next_scene = SceneId::kTitle;
            if (_audio) _audio->fade_music(400);
        }
        return r.world_paused;
    }

    void _flush_overlay_input(const InputState& in) {
        _overlay_input.flush(in, _pause_controller);
    }

    void _sync_gameplay_input_history(const InputState& in) {
        _prev_potion    = in.potion_pressed;
        _prev_confirm   = in.confirm_pressed;
    }

    void _sync_footstep_anchors() {
        _player.footstep_prev_x     = _player.transform.x;
        _player.footstep_prev_y     = _player.transform.y;
        _player.footstep_accum_dist = 0.f;
        for (auto& area : _world_map.areas)
            for (auto& e : area.enemies) {
                e.footstep_prev_x     = e.transform.x;
                e.footstep_prev_y     = e.transform.y;
                e.footstep_accum_dist = 0.f;
            }
    }

    ShopInputResult _update_shop(const OverlayInputEdges& input) {
        return _shop_input.update(_shop_forge, _player, input, _audio, _shop_open);
    }

    const char* tr(const std::string& key) const {
        return _locale ? _locale->get(key) : key.c_str();
    }
};

inline void WorldScene::tile_sprite_sheets(int mult) {
    if (mult <= 1 || !_renderer) return;

    struct TexPair {
        SDL_Texture* orig;
        SDL_Texture* tiled;
    };
    std::vector<TexPair> cache;

    auto get_tiled = [&](SDL_Texture* orig_ptr, float orig_w, float orig_h) -> SDL_Texture* {
        for (const auto& p : cache)
            if (p.orig == orig_ptr) return p.tiled;

        int         new_w = (int)(orig_w * mult);
        SDL_Texture* orig  = orig_ptr;
        SDL_Texture* tiled = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_RGBA8888,
                                               SDL_TEXTUREACCESS_TARGET, new_w, (int)orig_h);
        if (!tiled) {
            cache.push_back({orig_ptr, nullptr});
            return nullptr;
        }

        SDL_SetTextureBlendMode(tiled, SDL_BLENDMODE_BLEND);
        SDL_SetRenderTarget(_renderer, tiled);
        SDL_SetRenderDrawColor(_renderer, 0, 0, 0, 0);
        SDL_RenderClear(_renderer);
        for (int i = 0; i < mult; ++i) {
            SDL_FRect dst = {orig_w * (float)i, 0.0f, orig_w, orig_h};
            SDL_RenderTexture(_renderer, orig, nullptr, &dst);
        }
        SDL_SetRenderTarget(_renderer, nullptr);

        _tiled_textures.push_back(tiled);
        cache.push_back({orig_ptr, tiled});
        return tiled;
    };

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

    if (_player.sprite_sheet) {
        float tw = 0, th = 0;
        SDL_GetTextureSize(_player.sprite_sheet, &tw, &th);
        SDL_Texture* t = get_tiled(_player.sprite_sheet, tw, th);
        if (t) {
            _player.sprite_sheet = t;
            expand_anim(_player.anim, (int)tw);
        }
    }

    for (auto& area : _world_map.areas) {
        for (auto& e : area.enemies) {
            if (!e.sprite_sheet) continue;
            float tw = 0, th = 0;
            SDL_GetTextureSize(e.sprite_sheet, &tw, &th);
            int ow = (int)tw;
            SDL_Texture* t = get_tiled(e.sprite_sheet, tw, th);
            if (t) {
                e.sprite_sheet = t;
                expand_anim(e.anim, ow);
            }
        }
    }
}

} // namespace mion
