static void update_yaw_pitch(
      float dx,
      float dy,
      float sensitivity,
      float *yaw,
      float *pitch
  ) {
      *yaw   += dx * sensitivity;
      *pitch -= dy * sensitivity;

      if (*pitch > 89.0f)  *pitch = 89.0f;
      if (*pitch < -89.0f) *pitch = -89.0f;
  }

