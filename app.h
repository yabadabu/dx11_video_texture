#pragma once

#include "dx11_basic.h"
#include "video_texture.h"

class CApp {

  Render::Pipeline pipe_video;
  Render::Pipeline pipe_solid;
  Render::Mesh     quad;
  Render::Mesh     grid;
  VideoTexture     video;

  struct MVP {
    Matrix World;
    Matrix View;
    Matrix Projection;
  };
  Render::Cte<MVP> cte;

  float            camera_time = 0.0f;

public:
  bool create(HWND hWnd);
  void destroy();
  void update(float dt);
  void render();
};
