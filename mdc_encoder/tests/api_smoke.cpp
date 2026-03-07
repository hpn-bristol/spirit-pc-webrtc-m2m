#include "plugin.h"

// No-op callbacks matching the exported signatures
static void on_description_done(Description* dsc,
                                char* raw_data_ptr,
                                uint32_t n_points_in_total,
                                uint32_t dsc_size,
                                uint32_t capturer_id,
                                uint32_t frame_nr,
                                uint32_t dsc_nr,
                                uint64_t timestamp) {
  (void)dsc; (void)raw_data_ptr; (void)n_points_in_total; (void)dsc_size;
  (void)capturer_id; (void)frame_nr; (void)dsc_nr; (void)timestamp;
}

static void on_free_point_cloud(PointCloud* pc) {
  (void)pc; // We never enqueue a PC in this smoke test
}

int main() {
  // API presence + callability
  (void)initialize();

  EncodingQueue* q = create_encoding_queue(1);
  if (!q) return 1;

  // Register callbacks to validate ABI (function pointer types)
  register_description_done_callback(q, on_description_done);
  register_free_pc_callback(q, on_free_point_cloud);

  free_encoding_queue(q);

  // Clean up DLL state (harmless no-op if not initialized)
  clean_up();
  return 0;
}
