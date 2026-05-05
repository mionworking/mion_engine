#include "test_common.hpp"

#include "components/karma_data.hpp"
#include "core/quest_state.hpp"
#include "entities/actor.hpp"
#include "entities/enemy_type.hpp"
#include "entities/ground_item.hpp"
#include "systems/drop_system.hpp"
#include "systems/enemy_death_controller.hpp"
#include "systems/simple_particles.hpp"

using namespace mion;

namespace {

// Monta um array de EnemyDef com o tipo `t` configurado com karma_drop=N.
// Demais campos vêm dos defaults de get_enemy_def.
struct DeathFixture {
    EnemyDef                defs[kEnemyTypeCount];
    DropConfig              drop_cfg;
    std::vector<GroundItem> ground_items;
    QuestState              quest_state;
    SimpleParticleSystem    particles;

    DeathFixture() {
        for (int i = 0; i < kEnemyTypeCount; ++i)
            defs[i] = get_enemy_def(static_cast<EnemyType>(i));
    }

    Actor make_player() {
        Actor a;
        a.team   = Team::Player;
        a.player = PlayerData{};
        return a;
    }

    Actor make_dead_enemy(EnemyType t) {
        Actor e;
        e.team        = Team::Enemy;
        e.enemy_type  = t;
        e.was_alive   = true;
        e.is_alive    = false;
        return e;
    }

    EnemyDeathController::DeathResult process(std::vector<Actor*>& actors, Actor& player) {
        return EnemyDeathController::process_deaths(
            actors, player, defs, drop_cfg, ground_items,
            /*run_stats=*/nullptr, quest_state, particles,
            /*audio=*/nullptr, /*room_index=*/0,
            /*stress_mode=*/false, /*rng=*/nullptr);
    }
};

} // namespace

static void test_karma_dropped_on_single_kill() {
    DeathFixture fx;
    fx.defs[static_cast<int>(EnemyType::Skeleton)].karma_drop = 10;

    Actor player = fx.make_player();
    Actor enemy  = fx.make_dead_enemy(EnemyType::Skeleton);
    std::vector<Actor*> actors = { &enemy };

    auto r = fx.process(actors, player);

    EXPECT_EQ(player.player->karma.total, 10);
    EXPECT_EQ(player.player->karma.available, 10);
    EXPECT_EQ(r.karma_gained, 10);
}
REGISTER_TEST(test_karma_dropped_on_single_kill);

static void test_karma_accumulates_across_multiple_kills() {
    DeathFixture fx;
    fx.defs[static_cast<int>(EnemyType::Skeleton)].karma_drop = 8;
    fx.defs[static_cast<int>(EnemyType::Orc)].karma_drop      = 20;

    Actor player = fx.make_player();
    Actor skel = fx.make_dead_enemy(EnemyType::Skeleton);
    Actor orc  = fx.make_dead_enemy(EnemyType::Orc);
    std::vector<Actor*> actors = { &skel, &orc };

    auto r = fx.process(actors, player);

    EXPECT_EQ(player.player->karma.total, 28);
    EXPECT_EQ(player.player->karma.available, 28);
    EXPECT_EQ(r.karma_gained, 28);
}
REGISTER_TEST(test_karma_accumulates_across_multiple_kills);

static void test_karma_drop_zero_is_noop() {
    DeathFixture fx;
    fx.defs[static_cast<int>(EnemyType::Ghost)].karma_drop = 0;

    Actor player = fx.make_player();
    Actor ghost = fx.make_dead_enemy(EnemyType::Ghost);
    std::vector<Actor*> actors = { &ghost };

    auto r = fx.process(actors, player);

    EXPECT_EQ(player.player->karma.total, 0);
    EXPECT_EQ(player.player->karma.available, 0);
    EXPECT_EQ(r.karma_gained, 0);
}
REGISTER_TEST(test_karma_drop_zero_is_noop);

static void test_karma_total_equals_available_when_only_adds() {
    DeathFixture fx;
    fx.defs[static_cast<int>(EnemyType::Skeleton)].karma_drop      = 5;
    fx.defs[static_cast<int>(EnemyType::Orc)].karma_drop           = 15;
    fx.defs[static_cast<int>(EnemyType::EliteSkeleton)].karma_drop = 30;

    Actor player = fx.make_player();
    Actor s = fx.make_dead_enemy(EnemyType::Skeleton);
    Actor o = fx.make_dead_enemy(EnemyType::Orc);
    Actor e = fx.make_dead_enemy(EnemyType::EliteSkeleton);
    std::vector<Actor*> actors = { &s, &o, &e };

    fx.process(actors, player);

    EXPECT_EQ(player.player->karma.total, 50);
    EXPECT_EQ(player.player->karma.available, 50);
    EXPECT_EQ(player.player->karma.total, player.player->karma.available);
}
REGISTER_TEST(test_karma_total_equals_available_when_only_adds);
