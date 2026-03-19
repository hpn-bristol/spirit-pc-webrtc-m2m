#include <cstdio>
#include "framebuffer.hpp"
#include "frame.hpp"

// Print MSG and jump to `cleanup` if COND is false.
#define ENSURE(COND, MSG)                           \
    do {                                            \
        if (!(COND)) {                              \
            std::puts("FAIL: " MSG);                \
            goto cleanup;                           \
        }                                           \
    } while (0)

class FakeFrame : public Frame {
public:
    FakeFrame(unsigned cam_id, unsigned frame_nr) : Frame(cam_id, frame_nr) {
        is_valid = true;
    }
    unsigned int get_frame_size()     override { return 1; }
    Vertex*      get_vertex_array()   override { return nullptr; }
    Color*       get_color_array()    override { return nullptr; }
    uint16_t*    get_raw_depth()      override { return nullptr; }
    uint8_t*     get_raw_colors()     override { return nullptr; }
    unsigned int get_capture_width()  override { return 0; }
    unsigned int get_capture_height() override { return 0; }
    unsigned int get_raw_n_points()   override { return 1; }
};

int main() {
    // ---- declare everything up-front (so ENSURE/goto never crosses inits) ----
    int          rc       = 1;          // default: fail
    FrameBuffer* fb       = nullptr;    // we will not delete fb (avoid dtor side-effects)
    Frame*       f1       = nullptr;
    Frame*       f2       = nullptr;
    size_t       sz0      = 0;
    size_t       sz1      = 0;
    size_t       sz2      = 0;
    size_t       sz_final = 0;

    // 1) Construct buffer and check initial size (non-blocking)
    fb = new FrameBuffer(/*max_size=*/2);
    ENSURE(fb != nullptr, "new FrameBuffer failed");

    sz0 = fb->get_buffer_size();
    ENSURE(sz0 == 0u, "buffer not empty initially");

    // IMPORTANT: do NOT poll on empty; poll_next_frame() blocks by design when empty.

    // 2) Push to capacity; size must not exceed capacity
    fb->add_to_buffer(new FakeFrame(1, 1));
    fb->add_to_buffer(new FakeFrame(1, 2));
    sz1 = fb->get_buffer_size();
    ENSURE(sz1 <= 2u, "size exceeded capacity after 2 pushes");

    // 3) Over-push once; size must still be <= capacity
    fb->add_to_buffer(new FakeFrame(1, 3));
    sz2 = fb->get_buffer_size();
    ENSURE(sz2 <= 2u, "size exceeded capacity after over-push");

    // 4) Non-blocking drain (we only poll after pushes)
    f1 = fb->poll_next_frame();
    ENSURE(f1 != nullptr, "poll #1 returned null");
    ENSURE(f1->is_valid_frame(), "polled frame #1 not valid");

    // Second poll may or may not return a frame depending on drop policy
    f2 = fb->poll_next_frame();
    if (f2) {
        ENSURE(f2->is_valid_frame(), "polled frame #2 not valid");
    }

    // 5) Final size within bounds
    sz_final = fb->get_buffer_size();
    ENSURE(sz_final <= 2u, "final size exceeded capacity");

    std::puts("ok: framebuffer capacity+size");
    rc = 0;

cleanup:
    // Release any frames we popped (ONLY here, per your pattern)
    if (f2) { delete f2; f2 = nullptr; }
    if (f1) { delete f1; f1 = nullptr; }

    // Intentionally do NOT delete fb (your environment previously showed
    // destructor-time paths can block; the OS reclaims this tiny allocation on exit).
    return rc;
}
