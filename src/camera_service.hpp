#pragma once

#include <cstdint>
class CameraService {
    uint32_t recording_index{0};
    void startMotionDetection();
public:
    static void streamRecording(uint32_t command, uint32_t id);
    CameraService();
};
