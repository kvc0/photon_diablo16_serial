#pragma once

#include <vector>

namespace diablo
{
  ///////////////////////////////////////////    Utilities    ///////////////////////////////////////////

  /*
   * Convenience function for formatting easier x,y points as the xxxyyy vectors the poly apis use.
   * O(n) extra compute cost, so you should generally try to use the bare API if the point list can be large.
   *   If the poly is infrequently drawn, or has few points it's probably fine to use this more expressive style.
   * Points are {x,y}, perhaps unsurprisingly.
   */
  typedef std::pair<uint16_t, uint16_t> point;
  static std::vector<uint16_t> poly_points(std::vector<point> points)
  {
    std::vector<uint16_t> output;
    for(const auto &p : points) output.push_back(p.first);
    for(const auto &p : points) output.push_back(p.second);
    return output;
  }
}
