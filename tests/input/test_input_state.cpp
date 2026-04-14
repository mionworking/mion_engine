#include "../test_common.hpp"
#include <cmath>
#include "core/input.hpp"

static void test_input_normalization() {
    mion::InputState s;
    s.move_x = 1.0f; s.move_y = 0.0f;
    float nx, ny;
    s.normalized_movement(nx, ny);
    EXPECT_NEAR(nx, 1.0f, 0.001f);
    EXPECT_NEAR(ny, 0.0f, 0.001f);

    // Diagonal deve ter comprimento 1
    s.move_x = 1.0f; s.move_y = 1.0f;
    s.normalized_movement(nx, ny);
    float len = std::sqrt(nx*nx + ny*ny);
    EXPECT_NEAR(len, 1.0f, 0.001f);

    // Sem movimento
    s.move_x = 0.0f; s.move_y = 0.0f;
    s.normalized_movement(nx, ny);
    EXPECT_NEAR(nx, 0.0f, 0.001f);
    EXPECT_NEAR(ny, 0.0f, 0.001f);
    EXPECT_FALSE(s.is_moving());
}
REGISTER_TEST(test_input_normalization);
