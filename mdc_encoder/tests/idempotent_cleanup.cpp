#include "plugin.h"

int main() {
  (void)initialize();
  clean_up();
  clean_up(); // should be a no-op and not crash
  return 0;
}
