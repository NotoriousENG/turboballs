#pragma once

#include <font.hpp>
#include <memory>
#include <mesh-renderer.hpp>
#include <mixer.hpp>
#include <shared-data.hpp>
#include <sprite-batch.hpp>
#include <opencv2\opencv.hpp>

class Game {
public:
  Game();
  ~Game();
  int init(SharedData *shared_data);
  int update();
  int unload();
  int close();

  std::unique_ptr<SpriteBatch> spriteBatcher;

  std::unique_ptr<MeshRenderer> meshRenderer;

  std::unique_ptr<Mixer> mixer;

  std::shared_ptr<Font> font;

  cv::Mat image;

  cv::VideoCapture capture;
};