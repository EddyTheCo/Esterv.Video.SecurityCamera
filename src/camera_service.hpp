#pragma once

#include <cstdint>
class CameraService {
  uint32_t recording_index{0};
  void startMotionDetection();

public:
  static void streamRecording(uint8_t command, uint8_t id);
  CameraService();
};
