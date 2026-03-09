#pragma once
#include <stdint.h>
#include "point_cloud_data.h"   // Geometry types

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Frame Frame;

typedef enum FrameMode : int {
    RealData = 0,
    RawData  = 1,
    Both     = 2
} FrameMode;

typedef struct FrameCleanupSettings {
    unsigned int blackout_block_size;
    int should_apply_depth_filter;
    int should_cleanup_depth;
    int should_blackout;
} FrameCleanupSettings;

typedef void (*FrameReadyCallback)(
    unsigned int capturer_index,
    Frame* frame_ptr,
    int is_valid
);

#ifdef __cplusplus
}
#endif
