#include "../test_common.hpp"
#include "core/audio.hpp"
#include "systems/simple_particles.hpp"
#include "systems/world_audio_system.hpp"
#include "world/world_area.hpp"
#include <SDL3/SDL.h>
#include <cstdlib>
#include <random>

static void test_sfx_distance_attenuation_quadratic() {
    EXPECT_NEAR(mion::sfx_distance_attenuation(0.f, 600.f), 1.0f, 0.001f);
    EXPECT_NEAR(mion::sfx_distance_attenuation(600.f * 600.f, 600.f), 0.0f, 0.001f);
    EXPECT_NEAR(mion::sfx_distance_attenuation(300.f * 300.f, 600.f), 0.25f, 0.001f);
}
REGISTER_TEST(test_sfx_distance_attenuation_quadratic);

static void test_particles_spawn_and_update_reduces_count() {
    std::mt19937 rng(42);
    mion::SimpleParticleSystem ps;
    ps.spawn_burst(0.0f, 0.0f, 20, 255, 0, 0, 10.0f, 12.0f, rng);
    EXPECT_TRUE(ps.live_particle_count() > 0);
    int n0 = ps.live_particle_count();
    for (int i = 0; i < 120; ++i)
        ps.update(0.05f);
    EXPECT_TRUE(ps.live_particle_count() <= n0);
}
REGISTER_TEST(test_particles_spawn_and_update_reduces_count);

static void test_audio_integration_optional_env() {
    if (!std::getenv("MION_AUDIO_INTEGRATION_TESTS"))
        return;
    if (SDL_Init(SDL_INIT_AUDIO) != 0)
        return;
    mion::AudioSystem audio;
    if (!audio.init()) {
        SDL_Quit();
        return;
    }
    audio.set_master_volume(0.01f);
    audio.play_sfx(mion::SoundId::UiConfirm, 0.01f);
    for (int i = 0; i < 5; ++i)
        audio.tick_music();
    audio.shutdown();
    SDL_Quit();
    EXPECT_TRUE(true);
}
REGISTER_TEST(test_audio_integration_optional_env);

// Fase 7 audit: zone cross calls the same entry points as WorldScene::_handle_zone_transition.
static void test_world_audio_init_for_zone_optional_env() {
    if (!std::getenv("MION_AUDIO_INTEGRATION_TESTS"))
        return;
    if (SDL_Init(SDL_INIT_AUDIO) != 0)
        return;
    mion::AudioSystem audio;
    if (!audio.init()) {
        SDL_Quit();
        return;
    }
    audio.set_master_volume(0.01f);
    mion::WorldAudioSystem::init_for_zone(audio, mion::ZoneId::Town);
    for (int i = 0; i < 12; ++i)
        audio.tick_music();
    EXPECT_TRUE(audio.current_music_state() == mion::MusicState::Town);

    mion::WorldAudioSystem::init_for_zone(audio, mion::ZoneId::Transition);
    for (int i = 0; i < 12; ++i)
        audio.tick_music();
    EXPECT_TRUE(audio.current_music_state() == mion::MusicState::Town);

    mion::WorldAudioSystem::init_for_zone(audio, mion::ZoneId::DungeonRoom0);
    for (int i = 0; i < 12; ++i)
        audio.tick_music();
    EXPECT_TRUE(audio.current_music_state() == mion::MusicState::DungeonCalm);

    audio.shutdown();
    SDL_Quit();
}
REGISTER_TEST(test_world_audio_init_for_zone_optional_env);

static void test_audio_music_state_optional_env() {
    if (!std::getenv("MION_AUDIO_INTEGRATION_TESTS"))
        return;
    if (SDL_Init(SDL_INIT_AUDIO) != 0)
        return;
    mion::AudioSystem audio;
    if (!audio.init()) {
        SDL_Quit();
        return;
    }

    audio.set_master_volume(0.01f);
    audio.set_music_state(mion::MusicState::Town);
    for (int i = 0; i < 8; ++i)
        audio.tick_music();
    EXPECT_TRUE(audio.current_music_state() == mion::MusicState::Town);

    audio.set_music_state(mion::MusicState::DungeonCombat);
    for (int i = 0; i < 220; ++i) {
        audio.tick_music();
        SDL_Delay(10);
    }
    EXPECT_TRUE(audio.current_music_state() == mion::MusicState::DungeonCombat);

    audio.set_music_state(mion::MusicState::None);
    for (int i = 0; i < 8; ++i)
        audio.tick_music();
    EXPECT_TRUE(audio.current_music_state() == mion::MusicState::None);

    audio.shutdown();
    SDL_Quit();
}
REGISTER_TEST(test_audio_music_state_optional_env);

static void test_audio_ambient_and_spatial_optional_env() {
    if (!std::getenv("MION_AUDIO_INTEGRATION_TESTS"))
        return;
    if (SDL_Init(SDL_INIT_AUDIO) != 0)
        return;
    mion::AudioSystem audio;
    if (!audio.init()) {
        SDL_Quit();
        return;
    }

    audio.set_master_volume(0.01f);
    audio.play_ambient(mion::AmbientId::DungeonDrips, 0.1f);
    audio.play_ambient(mion::AmbientId::DungeonTorch, 0.08f);
    for (int i = 0; i < 6; ++i)
        audio.tick_music();
    audio.stop_ambient(mion::AmbientId::DungeonTorch);
    audio.play_sfx_pitched(mion::SoundId::UiConfirm, 0.05f);
    audio.play_sfx_at(mion::SoundId::Hit, 100.0f, 100.0f, 90.0f, 90.0f, 500.0f, 0.2f);
    audio.stop_all_ambient();
    for (int i = 0; i < 6; ++i)
        audio.tick_music();

    audio.shutdown();
    SDL_Quit();
    EXPECT_TRUE(true);
}
REGISTER_TEST(test_audio_ambient_and_spatial_optional_env);
