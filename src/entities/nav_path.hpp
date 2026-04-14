#pragma once
#include <vector>

namespace mion {

// Caminho de waypoints — preenchido pelo Pathfinder, consumido pelos sistemas de IA inimiga.
struct Path {
    struct Point { float x, y; };
    std::vector<Point> waypoints;
    int  current = 0;
    bool valid   = false;

    bool  done()    const { return !valid || current >= (int)waypoints.size(); }
    Point next_wp() const { return done() ? Point{0,0} : waypoints[current]; }
    void  advance()       { if (!done()) ++current; }
    void  reset()         { waypoints.clear(); current = 0; valid = false; }
};

} // namespace mion
