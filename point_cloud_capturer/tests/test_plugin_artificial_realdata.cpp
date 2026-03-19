#include <cstdio>
#include "plugin.h"
#include "artificical_capturer.hpp"

// Prints MSG and jumps to `cleanup` if COND is false.
#define ENSURE(COND, MSG)             \
    do {                              \
        if (!(COND)) {                \
            std::puts("FAIL: " MSG);  \
            goto cleanup;             \
        }                             \
    } while (0)

int main() {
    int rc = 1;

    // Declare all locals up-front to avoid crossing initialisations with goto.
    Capturer*   cap = nullptr;
    PointCloud* pc  = nullptr;
    size_t      n   = 0;
    uint64_t    ts  = 0;
    Vertex*     pos = nullptr;
    Color*      col = nullptr;

    ArtificalCaptureSettings cfg{};
    cfg.side_size = 16;

    FrameCleanupSettings clean{};
    clean.blackout_block_size       = 1;
    clean.should_apply_depth_filter = 0;
    clean.should_cleanup_depth      = 0;
    clean.should_blackout           = 0;

    cap = create_new_capturer(
        /*fps=*/30u,
        /*mode=*/RealData,
        clean,
        /*type=*/Artifical,
        /*capture_settings=*/&cfg
    );
    ENSURE(cap != nullptr, "create_new_capturer() returned null");

    start_capturing(cap, /*start_capture_thread=*/true);

    pc = poll_next_point_cloud(cap);
    ENSURE(pc != nullptr, "poll_next_point_cloud() returned null");

    n = get_point_cloud_size(pc);
    ENSURE(n > 0, "point cloud size is zero");

    pos = get_point_cloud_pos(pc);
    ENSURE(pos != nullptr, "position buffer is null");

    col = get_point_cloud_col(pc);
    ENSURE(col != nullptr, "colour buffer is null");

    ts = get_point_cloud_timestamp(pc);
    ENSURE(ts > 0, "timestamp is zero");

    std::puts("ok: artificial realdata");
    rc = 0;

cleanup:
    if (pc)  { free_point_cloud(pc);  pc  = nullptr; }
    if (cap) { free_capturer(cap);    cap = nullptr; }
    return rc;
}
