#include <cstdio>
#include <atomic>
#include <chrono>
#include <thread>

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

using namespace std::chrono_literals;

namespace {
    std::atomic<unsigned> g_hits{0};

    void on_frame(unsigned /*idx*/, Frame* f, int valid) {
        if (f && valid) {
            g_hits.fetch_add(1, std::memory_order_relaxed);
        }
    }
}

int main() {
    int rc = 1;

    // Up-front declarations (avoid goto crossing initialisations).
    MultiCapturer* mc   = nullptr;
    unsigned       seen = 0;
    auto           t0   = std::chrono::steady_clock::now();

    ArtificalCaptureSettings a1{}; a1.side_size = 10;
    ArtificalCaptureSettings a2{}; a2.side_size = 12;

    FrameCleanupSettings clean{};
    clean.blackout_block_size       = 1;
    clean.should_apply_depth_filter = 0;
    clean.should_cleanup_depth      = 0;
    clean.should_blackout           = 0;

    void* settings[2] = { &a1, &a2 };

    mc = create_new_multi_capturer(
        /*fps=*/30u,
        /*mode=*/RealData,
        clean,
        /*type=*/Artifical,
        /*n_settings=*/2u,
        /*capture_settings=*/settings
    );
    ENSURE(mc != nullptr, "create_new_multi_capturer() returned null");

    ENSURE(register_frame_ready_callback_for_capturer(mc, 0u, &on_frame),
           "register callback for capturer 0 failed");
    ENSURE(register_frame_ready_callback_for_capturer(mc, 1u, &on_frame),
           "register callback for capturer 1 failed");

    start_capturing_multi(mc, /*start_capture_thread=*/true);

    // Simple bounded wait using an atomic; avoids condition_variable/unique_lock.
    while (std::chrono::steady_clock::now() - t0 < 2000ms) {
        seen = g_hits.load(std::memory_order_relaxed);
        if (seen > 0) break;
        std::this_thread::sleep_for(10ms);
    }
    ENSURE(seen > 0, "no callbacks observed within 2 seconds");

    std::puts("ok: multicapturer callbacks");
    rc = 0;

cleanup:
    if (mc) { free_multi_capturer(mc); mc = nullptr; }
    return rc;
}
