#pragma once

#include <vector>

#include "../core/sprite.hpp"
#include "../entities/actor.hpp"
#include "../entities/npc.hpp"
#include "../entities/shop.hpp"
#include "../world/room.hpp"
#include "../world/tilemap.hpp"
#include "tilemap_renderer.hpp"

namespace mion {

namespace TownBuilder {

inline void build_town_world(RoomDefinition& room,
                             Tilemap& tilemap,
                             TilemapRenderer& tile_renderer,
                             std::vector<NpcEntity>& npcs,
                             ShopInventory& shop_forge,
                             Actor& player,
                             TextureCache* tex_cache) {
    room.name   = "town";
    room.bounds = {0.0f, 2400.0f, 0.0f, 1600.0f};
    room.obstacles.clear();
    room.doors.clear();

    const int tile_size = 32;
    tilemap.init(75, 50, tile_size);
    for (int r = 0; r < 50; ++r)
        for (int c = 0; c < 75; ++c)
            tilemap.set(c, r, TileType::Floor);

    if (tex_cache) {
        tile_renderer.tileset        = tex_cache->load("assets/tiles/town_tileset.png");
        tile_renderer.floor_tile_col = 0;
        tile_renderer.floor_tile_row = 0;
        tile_renderer.wall_tile_col  = 1;
        tile_renderer.wall_tile_row  = 0;
    } else {
        tile_renderer.tileset = nullptr;
    }

    room.add_obstacle("building_tavern", 700.0f, 200.0f, 1100.0f, 500.0f);
    room.add_obstacle("building_forge", 1300.0f, 200.0f, 1600.0f, 500.0f);
    room.add_obstacle("building_elder", 300.0f, 900.0f, 600.0f, 1200.0f);
    room.add_obstacle("fountain", 1100.0f, 700.0f, 1300.0f, 900.0f);

    room.add_door(2350.0f, 700.0f, 2390.0f, 900.0f, false, "dungeon");

    npcs.clear();
    {
        NpcEntity mira;
        mira.x = mira.spawn_x = 440.0f;
        mira.y = mira.spawn_y = 840.0f;
        mira.name             = "Mira";
        mira.type             = NpcType::QuestGiver;
        mira.interact_radius  = 52.0f;
        mira.portrait_color   = {200, 180, 80, 255};
        mira.dialogue_default      = "mira_default";
        mira.dialogue_quest_active = "mira_quest_active";
        mira.dialogue_quest_done   = "mira_quest_done";
        mira.wander_radius = 60.0f;
        mira.wander_speed  = 28.0f;
        mira.wander_timer  = 1.2f;
        npcs.push_back(mira);
    }
    {
        NpcEntity forge;
        forge.x = forge.spawn_x = 1450.0f;
        forge.y = forge.spawn_y = 540.0f;
        forge.name             = "Forge";
        forge.type             = NpcType::Merchant;
        forge.interact_radius  = 52.0f;
        forge.portrait_color   = {220, 120, 60, 255};
        forge.dialogue_default = "forge_greeting";
        forge.wander_radius = 50.0f;
        forge.wander_speed  = 22.0f;
        forge.wander_timer  = 0.8f;
        npcs.push_back(forge);
    }
    {
        NpcEntity villager;
        villager.x = villager.spawn_x = 900.0f;
        villager.y = villager.spawn_y = 750.0f;
        villager.name             = "Villager";
        villager.type             = NpcType::Generic;
        villager.portrait_color   = {100, 160, 90, 255};
        villager.dialogue_default = "villager_a";
        villager.wander_radius = 90.0f;
        villager.wander_speed  = 38.0f;
        villager.wander_timer  = 0.5f;
        npcs.push_back(villager);
    }
    {
        NpcEntity elder;
        elder.x = elder.spawn_x = 1600.0f;
        elder.y = elder.spawn_y = 900.0f;
        elder.name             = "Elder";
        elder.type             = NpcType::Generic;
        elder.portrait_color   = {120, 140, 100, 255};
        elder.dialogue_default = "villager_b";
        elder.wander_radius = 70.0f;
        elder.wander_speed  = 25.0f;
        elder.wander_timer  = 1.8f;
        npcs.push_back(elder);
    }

    shop_forge.items.clear();
    shop_forge.items.push_back({"HP Potion", ShopItemType::HpPotion, 20, 50});
    shop_forge.items.push_back({"Stamina Potion", ShopItemType::StaminaPotion, 15, 80});
    shop_forge.items.push_back({"Sharpening Stone", ShopItemType::AttackUpgrade, 30, 3});
    shop_forge.items.push_back({"Mana Crystal", ShopItemType::ManaUpgrade, 25, 10});
    shop_forge.selected_index = 0;

    player.transform.set_position(400.0f, 800.0f);
}

} // namespace TownBuilder

} // namespace mion
