#include "../test_common.hpp"

#include "../../src/core/dungeon_dialogue.hpp"

using namespace mion;

static void dungeon_dialogue_registry_registers_expected_sequences() {
    DialogueSystem dialogue;

    register_dungeon_dialogue(dialogue);

    EXPECT_TRUE(dialogue.has_sequence("dungeon_prologue"));
    EXPECT_TRUE(dialogue.has_sequence("dungeon_room2"));
    EXPECT_TRUE(dialogue.has_sequence("boss_grimjaw_phase2"));
}

REGISTER_TEST(dungeon_dialogue_registry_registers_expected_sequences);
