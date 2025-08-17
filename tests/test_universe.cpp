#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "universe.hpp"

using namespace vxl;
using Catch::Approx;

TEST_CASE("Universe place/erase/get works") {
  Universe U;
  REQUIRE_FALSE(U.get(0,0,0).has_value());
  Cube c; c.mat.colorA = {1,0,0,1};
  U.place(0,0,0, c);
  auto g = U.get(0,0,0);
  REQUIRE(g.has_value());
  REQUIRE(g->mat.colorA.x == Approx(1.0f));
  REQUIRE(U.erase(0,0,0));
  REQUIRE_FALSE(U.get(0,0,0).has_value());
}

TEST_CASE("Groups create/move") {
  Universe U;
  U.place(0,0,0); U.place(1,0,0);
  std::vector<IVec3> members = { {0,0,0}, {1,0,0} };
  U.group_create("g", members);
  REQUIRE(U.group_exists("g"));
  REQUIRE(U.group_move("g", {0,1,0}));
  REQUIRE(U.get(0,1,0).has_value());
  REQUIRE(U.get(1,1,0).has_value());
}
