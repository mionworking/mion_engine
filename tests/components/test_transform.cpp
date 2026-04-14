#include "../test_common.hpp"
#include "components/transform.hpp"

static void test_transform() {
    mion::Transform2D t;
    EXPECT_EQ(t.x, 0.0f);
    EXPECT_EQ(t.y, 0.0f);

    t.set_position(10.0f, 20.0f);
    EXPECT_EQ(t.x, 10.0f);
    EXPECT_EQ(t.y, 20.0f);

    t.translate(5.0f, -3.0f);
    EXPECT_EQ(t.x, 15.0f);
    EXPECT_EQ(t.y, 17.0f);
}
REGISTER_TEST(test_transform);
