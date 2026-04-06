#include "../test_common.hpp"

#include "../../src/core/dungeon_dialogue.hpp"

using namespace mion;

static void dungeon_dialogue_registry_registers_expected_sequences() {
    DialogueSystem dialogue;

    register_dungeon_dialogue(dialogue);

    EXPECT_TRUE(dialogue._sequences.find("dungeon_prologue") != dialogue._sequences.end());
    EXPECT_TRUE(dialogue._sequences.find("dungeon_first_door") != dialogue._sequences.end());
    EXPECT_TRUE(dialogue._sequences.find("boss_grimjaw_phase2") != dialogue._sequences.end());
}

REGISTER_TEST(dungeon_dialogue_registry_registers_expected_sequences);
