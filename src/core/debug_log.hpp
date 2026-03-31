#pragma once

#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>

namespace mion {

inline long long debug_log_now_ms() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

inline void append_debug_log_line(const char* run_id, const char* hypothesis_id,
                                  const char* location, const char* message,
                                  const std::string& data_json) {
    // #region agent log
    std::fprintf(stderr, "[debug-log] %s %s %s\n", run_id, hypothesis_id, location);
    // #endregion
    std::error_code ec;
    std::filesystem::create_directories("/home/danten/Documents/G_v2/mion_engine_cpp/.cursor", ec);
    std::ofstream f("/home/danten/Documents/G_v2/mion_engine_cpp/.cursor/debug-d49b4f.log",
                    std::ios::app);
    if (f) {
        f << "{\"sessionId\":\"d49b4f\",\"runId\":\"" << run_id
          << "\",\"hypothesisId\":\"" << hypothesis_id
          << "\",\"location\":\"" << location
          << "\",\"message\":\"" << message
          << "\",\"data\":" << data_json
          << ",\"timestamp\":" << debug_log_now_ms() << "}\n";
    }

    std::ofstream f2("/tmp/mion_debug_d49b4f.log", std::ios::app);
    if (!f2) return;
    f2 << "{\"sessionId\":\"d49b4f\",\"runId\":\"" << run_id
      << "\",\"hypothesisId\":\"" << hypothesis_id
      << "\",\"location\":\"" << location
      << "\",\"message\":\"" << message
      << "\",\"data\":" << data_json
      << ",\"timestamp\":" << debug_log_now_ms() << "}\n";
}

} // namespace mion
