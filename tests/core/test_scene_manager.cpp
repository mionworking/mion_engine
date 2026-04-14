#include "../test_common.hpp"
#include <memory>
#include "core/input.hpp"
#include "core/register_scenes.hpp"
#include "core/scene.hpp"
#include "core/scene_context.hpp"
#include "core/scene_ids.hpp"
#include "core/scene_registry.hpp"

struct SceneTestMockScene : mion::IScene {
    int         enter_count  = 0;
    int         exit_count   = 0;
    int         update_count = 0;
    mion::SceneId pending_next = mion::SceneId::kNone;
    bool          request_next = false;
    int*          external_exit_count = nullptr;

    void enter() override { ++enter_count; }
    void exit() override {
        ++exit_count;
        if (external_exit_count) ++*external_exit_count;
    }
    void fixed_update(float, const mion::InputState&) override {
        ++update_count;
        if (request_next) pending_next = mion::SceneId::kWorld;
    }
    void render(SDL_Renderer*, float) override {}
    mion::SceneId next_scene() const override { return pending_next; }
    void clear_next_scene_request() override {
        pending_next = mion::SceneId::kNone;
    }
};

static void test_scene_manager_enter_exit_on_set() {
    mion::SceneManager mgr;
    int exits = 0;

    auto a = std::make_unique<SceneTestMockScene>();
    SceneTestMockScene* pa = a.get();
    pa->external_exit_count = &exits;
    mgr.set(std::move(a));
    EXPECT_EQ(pa->enter_count, 1);
    EXPECT_EQ(exits, 0);

    auto b = std::make_unique<SceneTestMockScene>();
    SceneTestMockScene* pb = b.get();
    pb->external_exit_count = &exits;
    mgr.set(std::move(b));
    EXPECT_EQ(exits, 1);
    EXPECT_EQ(pb->enter_count, 1);

    mgr.set(nullptr);
    EXPECT_EQ(exits, 2);
}
REGISTER_TEST(test_scene_manager_enter_exit_on_set);

static void test_scene_manager_fixed_update_returns_next_id() {
    mion::SceneManager mgr;
    auto s = std::make_unique<SceneTestMockScene>();
    SceneTestMockScene* p = s.get();
    p->request_next = true;
    mgr.set(std::move(s));
    mion::InputState in;
    mion::SceneId id = mgr.fixed_update(0.016f, in);
    EXPECT_TRUE(id != mion::SceneId::kNone);
    EXPECT_EQ(id, mion::SceneId::kWorld);
}
REGISTER_TEST(test_scene_manager_fixed_update_returns_next_id);

static void test_scene_manager_clear_pending_transition() {
    mion::SceneManager mgr;
    auto s = std::make_unique<SceneTestMockScene>();
    SceneTestMockScene* p = s.get();
    p->request_next = true;
    mgr.set(std::move(s));

    mion::InputState in;
    EXPECT_EQ(mgr.fixed_update(0.016f, in), mion::SceneId::kWorld);
    mgr.clear_pending_transition();
    EXPECT_EQ(p->pending_next, mion::SceneId::kNone);
    EXPECT_EQ(p->next_scene(), mion::SceneId::kNone);
}
REGISTER_TEST(test_scene_manager_clear_pending_transition);

static void test_scene_registry_unknown_id() {
    mion::SceneRegistry reg;
    mion::SceneCreateContext ctx;
    EXPECT_TRUE(reg.create(mion::SceneId::kNone, ctx) == nullptr);
}
REGISTER_TEST(test_scene_registry_unknown_id);

static void test_register_default_scenes_creates_title_and_dungeon() {
    mion::SceneRegistry reg;
    mion::register_default_scenes(reg);

    mion::SceneCreateContext ctx;
    ctx.viewport_w = 960;
    ctx.viewport_h = 540;

    EXPECT_TRUE(reg.create(mion::SceneId::kTitle, ctx) != nullptr);
    EXPECT_TRUE(reg.create(mion::SceneId::kWorld, ctx) != nullptr);
    EXPECT_TRUE(reg.create(mion::SceneId::kCredits, ctx) != nullptr);
}
REGISTER_TEST(test_register_default_scenes_creates_title_and_dungeon);
