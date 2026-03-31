#pragma once

#include <cstdio>
#include <string>

#include "dialogue.hpp"
#include "ini_loader.hpp"
#include "../systems/dialogue_system.hpp"

namespace mion {

/// Registra uma sequência por secção do INI com `line_N_speaker` / `line_N_text`.
inline void register_dialogue_sequences_from_ini(DialogueSystem& dlg,
                                                 const IniData&  ini)
{
    for (const auto& sec_pair : ini.sections) {
        const std::string& sec_id = sec_pair.first;
        const auto&        kv     = sec_pair.second;
        DialogueSequence   seq;
        seq.id = sec_id;
        for (int i = 0;; ++i) {
            char ksp[48], ktx[48];
            std::snprintf(ksp, sizeof(ksp), "line_%d_speaker", i);
            std::snprintf(ktx, sizeof(ktx), "line_%d_text", i);
            auto its = kv.find(ksp);
            auto itt = kv.find(ktx);
            if (its == kv.end() || itt == kv.end())
                break;
            DialogueLine line;
            line.speaker = its->second;
            line.text    = itt->second;
            seq.lines.push_back(std::move(line));
        }
        if (!seq.lines.empty())
            dlg.register_sequence(std::move(seq));
    }
}

} // namespace mion

