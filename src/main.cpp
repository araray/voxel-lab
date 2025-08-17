#include "app.hpp"
#include <iostream>

int main() {
  try {
    vxl::App app;
    return app.run();
  } catch (const std::exception& e) {
    std::cerr << "Fatal: " << e.what() << "\n";
    return 1;
  }
}
