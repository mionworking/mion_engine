#include "../test_common.hpp"

#include "../../src/systems/npc_actor_factory.hpp"
#include "../../src/systems/shop_input_controller.hpp"
#include "../../src/systems/town_builder.hpp"
#include "../../src/systems/town_npc_interaction.hpp"
#include "../../src/systems/town_npc_wander.hpp"
#include "../../src/world/world_context.hpp"
#include "../../src/world/world_map.hpp"
#include "../../src/world/zone_manager.hpp"

using namespace mion;

static void npc_actor_factory_links_npcs_to_runtime_actors() {
    RoomDefinition room;
    Tilemap tilemap;
    TilemapRenderer tile_renderer;
    std::vector<NpcEntity> npcs;
    ShopInventory shop;
    Actor player;
    std::vector<Actor> npc_actors;
    std::vector<Actor*> actors;

    player.sprite_scale = 2.0f;
    TownBuilder::build_town_world(room, tilemap, tile_renderer, npcs, shop, player, nullptr);
    actors.push_back(&player);
    NpcActorFactory::rebuild_npc_actors(npcs, player, npc_actors, actors);

    EXPECT_EQ(static_cast<int>(npc_actors.size()), static_cast<int>(npcs.size()));
    EXPECT_EQ(static_cast<int>(actors.size()), static_cast<int>(npcs.size()) + 1);
    EXPECT_TRUE(npcs[0].actor != nullptr);
    EXPECT_EQ(npcs[0].actor->name, "Mira");
}

static void town_npc_wander_keeps_npcs_inside_spawn_radius() {
    RoomDefinition room;
    Tilemap tilemap;
    TilemapRenderer tile_renderer;
    std::vector<NpcEntity> npcs;
    ShopInventory shop;
    Actor player;
    std::vector<Actor> npc_actors;
    std::vector<Actor*> actors;
    unsigned rng_state = 0x12345678u;

    player.sprite_scale = 2.0f;
    TownBuilder::build_town_world(room, tilemap, tile_renderer, npcs, shop, player, nullptr);
    NpcActorFactory::rebuild_npc_actors(npcs, player, npc_actors, actors);

    for (int i = 0; i < 32; ++i)
        TownNpcWanderSystem::update_town_npcs(npcs, 0.2f, rng_state);

    for (const auto& npc : npcs) {
        const float dx = npc.x - npc.spawn_x;
        const float dy = npc.y - npc.spawn_y;
        EXPECT_TRUE(dx * dx + dy * dy <= npc.wander_radius * npc.wander_radius + 0.01f);
    }
}

static void town_npc_interaction_finds_nearest_npc_and_opens_shop() {
    RoomDefinition room;
    Tilemap tilemap;
    TilemapRenderer tile_renderer;
    std::vector<NpcEntity> npcs;
    ShopInventory shop;
    Actor player;
    std::vector<Actor> npc_actors;
    std::vector<Actor*> actors;
    Camera2D camera;
    DialogueSystem dialogue;
    ResourceSystem resource;
    PlayerActionSystem player_action;
    QuestState quest_state;
    unsigned int scene_flags = 0;
    bool shop_open = false;

    player.sprite_scale = 2.0f;
    TownBuilder::build_town_world(room, tilemap, tile_renderer, npcs, shop, player, nullptr);
    NpcActorFactory::rebuild_npc_actors(npcs, player, npc_actors, actors);
    player.transform.set_position(1450.0f, 540.0f);

    WorldMap wm;
    WorldArea wa;
    wa.zone = ZoneId::Town;
    wa.room    = room;
    wa.tilemap = tilemap;
    wm.areas.push_back(std::move(wa));

    ZoneManager zm;
    zm.current = ZoneId::Town;

    WorldContext ctx;
    ctx.world_map     = &wm;
    ctx.zone_mgr      = &zm;
    ctx.tile_renderer = &tile_renderer;
    ctx.player        = &player;
    ctx.actors        = &actors;
    ctx.npc_actors    = &npc_actors;
    ctx.npcs          = &npcs;
    ctx.shop_forge    = &shop;
    ctx.camera        = &camera;
    ctx.dialogue      = &dialogue;
    ctx.resource      = &resource;
    ctx.player_action = &player_action;
    ctx.quest_state   = &quest_state;
    ctx.scene_flags   = &scene_flags;

    const auto near = TownNpcInteractionController::find_nearest_npc_for_interaction(player, npcs);
    EXPECT_TRUE(near.has_value());
    if (!near.has_value())
        return;
    TownNpcInteractionController::handle_npc_interaction(near->index, ctx, shop_open, 120);

    EXPECT_EQ(npcs[static_cast<size_t>(near->index)].name, "Forge");
    EXPECT_TRUE(shop_open);
    EXPECT_EQ(shop.selected_index, 0);
}

static void shop_input_controller_navigates_and_buys_items() {
    ShopInventory shop;
    shop.items = {
        {"Potion", ShopItemType::HpPotion, 10, 25},
        {"Crystal", ShopItemType::ManaUpgrade, 12, 5},
    };

    Actor player;
    player.gold = 50;
    player.health.current_hp = 50;
    player.health.max_hp = 100;
    bool shop_open = true;

    ShopInputController ctrl;

    OverlayInputEdges move_down{};
    move_down.down = true;
    auto r0 = ctrl.update(shop, player, move_down, nullptr, shop_open);
    EXPECT_EQ(shop.selected_index, 1);
    EXPECT_FALSE(r0.should_save);

    OverlayInputEdges confirm{};
    confirm.confirm = true;
    auto r1 = ctrl.update(shop, player, confirm, nullptr, shop_open);
    EXPECT_EQ(player.gold, 38);
    EXPECT_TRUE(r1.should_save);

    OverlayInputEdges cancel{};
    cancel.back = true;
    auto r2 = ctrl.update(shop, player, cancel, nullptr, shop_open);
    EXPECT_FALSE(shop_open);
    EXPECT_FALSE(r2.should_save);
}

REGISTER_TEST(npc_actor_factory_links_npcs_to_runtime_actors);
REGISTER_TEST(town_npc_wander_keeps_npcs_inside_spawn_radius);
REGISTER_TEST(town_npc_interaction_finds_nearest_npc_and_opens_shop);
REGISTER_TEST(shop_input_controller_navigates_and_buys_items);
