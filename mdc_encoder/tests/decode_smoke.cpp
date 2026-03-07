#include "plugin.h"
#include <cstdint>

static int run_once(char* data, uint32_t size) {
  DracoMDCDecoder* dec = decode_pc(data, size);
  if (!dec) return 10;
  const auto n = get_n_points(dec);
  if (n == 0) {
    if (get_point_array(dec) != nullptr) return 12;
    if (get_color_array(dec) != nullptr) return 13;
  }
  free_decoder(dec);
  return 0;
}

int main() {
  const char bogus[] = "not draco";
  if (int rc = run_once(const_cast<char*>(bogus), (uint32_t)sizeof(bogus))) return rc;
  if (int rc = run_once(nullptr, 0)) return rc;
  if (int rc = run_once(nullptr, 16)) return rc;
  return 0;
}
