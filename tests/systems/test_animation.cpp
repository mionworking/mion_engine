#include "../test_common.hpp"
#include "core/animation.hpp"

static void test_anim_player_loop_and_non_loop_finished() {
    mion::AnimPlayer ap;
    mion::AnimClip loop_clip;
    loop_clip.loop = true;
    loop_clip.frames.push_back({ { 0, 0, 8, 8 }, 0.05f });
    loop_clip.frames.push_back({ { 8, 0, 8, 8 }, 0.05f });
    ap.clips[(int)mion::ActorAnim::Idle] = loop_clip;
    ap.play(mion::ActorAnim::Idle);
    ap.advance(0.08f);
    EXPECT_FALSE(ap.finished);
    EXPECT_TRUE(ap.current_frame() != nullptr);

    mion::AnimClip once;
    once.loop = false;
    once.frames.push_back({ { 0, 0, 4, 4 }, 0.02f });
    ap.clips[(int)mion::ActorAnim::Death] = once;
    ap.play(mion::ActorAnim::Death);
    ap.advance(0.05f);
    EXPECT_TRUE(ap.finished);
}
REGISTER_TEST(test_anim_player_loop_and_non_loop_finished);

static void test_anim_player_play_same_clip_no_restart_until_finished() {
    mion::AnimPlayer ap;
    mion::AnimClip c;
    c.loop = false;
    c.frames.push_back({ { 0, 0, 8, 8 }, 0.2f });
    ap.clips[(int)mion::ActorAnim::Hurt] = c;
    ap.play(mion::ActorAnim::Hurt);
    ap.advance(0.05f);
    int fi = ap.frame_index;
    ap.play(mion::ActorAnim::Hurt);
    EXPECT_EQ(ap.frame_index, fi);
}
REGISTER_TEST(test_anim_player_play_same_clip_no_restart_until_finished);

static void test_anim_player_current_frame_null_when_empty() {
    mion::AnimPlayer ap;
    EXPECT_TRUE(ap.current_frame() == nullptr);
}
REGISTER_TEST(test_anim_player_current_frame_null_when_empty);

static void test_anim_player_puny_dir_row_updates_y() {
    mion::AnimPlayer ap;
    ap.build_puny_clips(0, 10.0f);
    int y0 = ap.clips[(int)mion::ActorAnim::Idle].frames[0].src.y;
    ap.update_puny_dir_row(2);
    int y1 = ap.clips[(int)mion::ActorAnim::Idle].frames[0].src.y;
    EXPECT_EQ(y1 - y0, 64);
}
REGISTER_TEST(test_anim_player_puny_dir_row_updates_y);
