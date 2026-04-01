#pragma once
#include <algorithm>

namespace mion {

// Unique identifier for each quest.
enum class QuestId { DefeatGrimjaw, Count };

// Lifecycle states of a quest.
enum class QuestStatus { NotStarted = 0, InProgress, Completed, Rewarded };

// Tracks status of all quests; serialized in SaveData.
struct QuestState {
    QuestStatus status[static_cast<int>(QuestId::Count)]{};

    QuestStatus get(QuestId id) const {
        return status[static_cast<int>(id)];
    }
    void set(QuestId id, QuestStatus s) {
        status[static_cast<int>(id)] = s;
    }
    bool is(QuestId id, QuestStatus s) const { return get(id) == s; }

    static int status_to_int(QuestStatus s) { return static_cast<int>(s); }
    static QuestStatus int_to_status(int v) {
        v = std::clamp(v, 0, 3);
        return static_cast<QuestStatus>(v);
    }
};

} // namespace mion
