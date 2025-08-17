#include <catch2/catch_test_macros.hpp>
#include "commands.hpp"
#include <sstream>

using namespace vxl;

TEST_CASE("Commands basic") {
  Universe U;
  Selection S;
  std::ostringstream out;
  CommandRegistry R;
  register_builtin_commands(R);

  CommandContext ctx{
    U, S,
    [&](const std::string& s){ out << s << "\n"; },
    [](){}, [](){}
  };

  REQUIRE(R.run_line("place 0 0 0 color=#ff0000ff", ctx));
  REQUIRE(U.get(0,0,0).has_value());
  REQUIRE(R.run_line("select 0 0 0", ctx));
  REQUIRE(R.run_line("fill solid #00ff00ff", ctx));
  auto c = U.get(0,0,0);
  REQUIRE(c.has_value());
  REQUIRE(c->mat.colorA.y == Approx(1.0f));
  REQUIRE(R.run_line("move 0 1 0", ctx));
  REQUIRE(U.get(0,1,0).has_value());
}
