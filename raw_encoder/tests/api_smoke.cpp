#include "plugin.h"

int main() {
  // Just ensure symbols link
  (void)&create_encoding_queue;
  (void)&encode_frame;
  (void)&clean_up;
  return 0;
}
