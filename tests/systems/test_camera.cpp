#include "../test_common.hpp"
#include "core/camera.hpp"
#include <random>

static void test_camera_follow_clamps() {
    mion::Camera2D cam;
    cam.viewport_w = 200.0f;
    cam.viewport_h = 150.0f;
    mion::WorldBounds b{ 0.0f, 400.0f, 0.0f, 300.0f };
    cam.follow(200.0f, 150.0f, b);
    EXPECT_NEAR(cam.x, 100.0f, 0.01f);
    EXPECT_NEAR(cam.y, 75.0f, 0.01f);
    cam.follow(0.0f, 0.0f, b);
    EXPECT_NEAR(cam.x, 0.0f, 0.01f);
    EXPECT_NEAR(cam.y, 0.0f, 0.01f);
    cam.follow(1000.0f, 1000.0f, b);
    EXPECT_NEAR(cam.x, 200.0f, 0.01f);
    EXPECT_NEAR(cam.y, 150.0f, 0.01f);
}
REGISTER_TEST(test_camera_follow_clamps);

static void test_camera_world_to_screen_no_shake() {
    mion::Camera2D cam;
    cam.x = 50.0f;
    cam.y = 40.0f;
    EXPECT_NEAR(cam.world_to_screen_x(150.0f), 100.0f, 0.01f);
    EXPECT_NEAR(cam.world_to_screen_y(90.0f), 50.0f, 0.01f);
}
REGISTER_TEST(test_camera_world_to_screen_no_shake);

static void test_camera_shake_steps_down() {
    std::srand(12345);
    mion::Camera2D cam;
    cam.trigger_shake(10.0f, 4);
    int start = cam.shake_remaining;
    std::mt19937 rng(12345);
    for (int i = 0; i < 4; ++i)
        cam.step_shake(rng);
    EXPECT_EQ(start, 4);
    EXPECT_EQ(cam.shake_remaining, 0);
}
REGISTER_TEST(test_camera_shake_steps_down);
