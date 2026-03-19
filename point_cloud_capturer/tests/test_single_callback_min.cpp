#include <cstdio>
#include <atomic>
#include <chrono>
#include <thread>

#include "plugin.h"
#include "artificical_capturer.hpp"

#define ENSURE(COND, MSG)                           \
    do {                                            \
        if (!(COND)) {                              \
            std::puts("FAIL: " MSG);                \
            goto cleanup;                           \
        }                                           \
    } while (0)

using namespace std::chrono_literals;

namespace {
    std::atomic<unsigned> g_seen{0};

    void on_frame(unsigned /*capturer_id*/, Frame* f, int valid) {
        if (f && valid) {
            g_seen.fetch_add(1, std::memory_order_relaxed);
        }
    }
}

int main() {
    // ---- declare up-front ----
    int        rc  = 1;
    Capturer*  cap = nullptr;
    unsigned   seen = 0;

    ArtificalCaptureSettings cfg{}; cfg.side_size = 10;

    FrameCleanupSettings clean{};
    clean.blackout_block_size       = 1;
    clean.should_apply_depth_filter = 0;
    clean.should_cleanup_depth      = 0;
    clean.should_blackout           = 0;

    cap = create_new_capturer(30u, RealData, clean, Artifical, &cfg);
    ENSURE(cap != nullptr, "create_new_capturer returned null");

    register_frame_ready_callback(cap, &on_frame);
    start_capturing(cap, /*start_capture_thread=*/true);

    // Bounded spin: prove a callback arrives without calling any blocking poll.
    {
        const auto t0 = std::chrono::steady_clock::now();
        do {
            seen = g_seen.load(std::memory_order_relaxed);
            if (seen > 0) break;
            std::this_thread::sleep_for(10ms);
        } while (std::chrono::steady_clock::now() - t0 < 1500ms);
    }
    ENSURE(seen > 0, "no single-capturer callback within 1.5 s");

    std::puts("ok: single-capturer callback");
    rc = 0;

cleanup:
    if (cap) { free_capturer(cap); cap = nullptr; }
    return rc;
}
