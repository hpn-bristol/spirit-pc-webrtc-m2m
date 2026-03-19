#include <cstdio>
#include "plugin.h"
#include "artificical_capturer.hpp"

#define ENSURE(COND, MSG)                           \
    do {                                            \
        if (!(COND)) {                              \
            std::puts("FAIL: " MSG);                \
            goto cleanup;                           \
        }                                           \
    } while (0)

int main() {
    // ---- declare up-front (so ENSURE/goto never crosses inits) ----
    int            rc = 1;
    MultiCapturer* mc = nullptr;
    uint32_t       sz0 = 0, sz1 = 0;
    void*          cal0 = nullptr;
    void*          cal1 = nullptr;

    // Artificial needs only side_size; we do not start capturing for these calls.
    ArtificalCaptureSettings a0{}; a0.side_size = 9;
    ArtificalCaptureSettings a1{}; a1.side_size = 13;
    void* settings[2] = { &a0, &a1 };

    FrameCleanupSettings clean{};
    clean.blackout_block_size       = 1;
    clean.should_apply_depth_filter = 0;
    clean.should_cleanup_depth      = 0;
    clean.should_blackout           = 0;

    mc = create_new_multi_capturer(
        /*fps=*/30u, /*mode=*/RealData, clean, /*type=*/Artifical,
        /*n_settings=*/2u, /*capture_settings=*/settings
    );
    ENSURE(mc != nullptr, "create_new_multi_capturer returned null");

    // Calibration invariants for Artificial:
    //   size == 0  ⇒  ptr == nullptr
    //   free_capturer_calibration(Artifical, ptr) must be safe on nullptr
    sz0  = get_calibration_size_for_capturer(mc, 0u);
    cal0 = get_calibration_for_capturer(mc, 0u);
    if (sz0 == 0u) {
        ENSURE(cal0 == nullptr, "size 0 but non-null calibration ptr (idx 0)");
    }
    free_capturer_calibration(Artifical, cal0);  // must be a no-op for Artificial

    sz1  = get_calibration_size_for_capturer(mc, 1u);
    cal1 = get_calibration_for_capturer(mc, 1u);
    if (sz1 == 0u) {
        ENSURE(cal1 == nullptr, "size 0 but non-null calibration ptr (idx 1)");
    }
    free_capturer_calibration(Artifical, cal1);  // must be a no-op for Artificial

    std::puts("ok: calibration invariants");
    rc = 0;

cleanup:
    if (mc) { free_multi_capturer(mc); mc = nullptr; }
    return rc;
}
