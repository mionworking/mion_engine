#pragma once

#include "../systems/dialogue_system.hpp"

namespace mion {

inline void register_dungeon_dialogue(DialogueSystem& dialogue) {
    {
        DialogueSequence prologue;
        prologue.id = "dungeon_prologue";
        prologue.lines = {
            {"Voice", "Few have braved these steps in years.", {180, 200, 255, 255}},
            {"Voice", "Torchlight on wet stone. Something stirs below.", {170, 210, 240, 255}},
            {"Voice", "Steel ready. The ruin remembers every footfall.", {200, 160, 120, 255}},
        };
        dialogue.register_sequence(std::move(prologue));
    }
    {
        DialogueSequence deeper;
        deeper.id = "dungeon_room2";
        deeper.lines = {
            {"Voice", "Deeper now. The walls grow older here.", {160, 180, 220, 255}},
            {"Voice", "Whatever lurks ahead has not seen daylight in years.", {200, 160, 120, 255}},
        };
        dialogue.register_sequence(std::move(deeper));
    }
    {
        DialogueSequence deeper2;
        deeper2.id = "dungeon_deeper";
        deeper2.lines = {
            {"Voice", "Another span of carved darkness. Keep the rhythm.", {140, 190, 230, 255}},
        };
        dialogue.register_sequence(std::move(deeper2));
    }
    {
        DialogueSequence relic;
        relic.id = "dungeon_rare_relic";
        relic.lines = {
            {"Voice", "A relic hums in your palm — old magic, thin but sharp.", {220, 200, 120, 255}},
        };
        dialogue.register_sequence(std::move(relic));
    }
    {
        DialogueSequence miniboss;
        miniboss.id = "miniboss_grimjaw_death";
        miniboss.lines = {
            {"Grimjaw", "Hrk… the depths… were meant… to keep you…", {200, 80, 70, 255}},
            {"Voice", "The named brute falls. What waits past him listens closer now.", {160, 200, 240, 255}},
        };
        dialogue.register_sequence(std::move(miniboss));
    }
    {
        DialogueSequence phase2;
        phase2.id = "boss_grimjaw_phase2";
        phase2.lines = {
            {"Grimjaw", "RAAAAGH! You dare…!!", {255, 80, 40, 255}},
        };
        dialogue.register_sequence(std::move(phase2));
    }
}

} // namespace mion
