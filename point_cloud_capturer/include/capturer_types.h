#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef enum CAPTURE_TYPE : int {
    Artifical = 0,
    RealSense = 1,
    PrerecordedRealSense = 2,
    Kinect = 3,
    PrerecordedKinect = 4,
    PlyFiles = 5
} CAPTURE_TYPE;

typedef enum CAPTURER_SETUP_CODE : int {
	StartedCorrectly = 0,
	CameraDisconnected = 1,
	BackendError = 2,
	InvalidValue = 3,
	WrongApiCallSeq = 4,
	NotImpl = 5,
	DeviceInRecovery = 6,
	UnknownException = 7
} CAPTURER_SETUP_CODE;

#ifdef __cplusplus
}
#endif
