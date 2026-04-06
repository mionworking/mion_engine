#include "../test_common.hpp"

#include "../../src/systems/town_config_loader.hpp"

using namespace mion;

static void town_config_loader_loads_runtime_config() {
    TownConfigLoader::load_town_player_and_progression_config();

    EXPECT_GT(g_player_config.base_hp, 0);
    EXPECT_GT(g_player_config.base_move_speed, 0.0f);
}

static void town_config_loader_registers_dialogues_from_ini() {
    DialogueSystem dialogue;

    TownConfigLoader::load_town_dialogues(dialogue);

    EXPECT_TRUE(dialogue._sequences.find("mira_default") != dialogue._sequences.end());
    EXPECT_TRUE(dialogue._sequences.find("forge_greeting") != dialogue._sequences.end());
}

REGISTER_TEST(town_config_loader_loads_runtime_config);
REGISTER_TEST(town_config_loader_registers_dialogues_from_ini);
