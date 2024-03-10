#pragma once

#include <font.hpp>
#include <memory>
#include <mesh-renderer.hpp>
#include <mixer.hpp>
#include <model.hpp>
#include <shared-data.hpp>
#include <sprite-batch.hpp>

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

  std::shared_ptr<Font> fontBig;

  std::shared_ptr<Model> worldModel;

  std::shared_ptr<Model> npcModel;

  std::shared_ptr<Model> ballModel;

  std::shared_ptr<Music> music;

  const float maxX = 1.8f;
  const float camMaxX = 12.0f;
  const float ballMaxX = 9.0f;

  float t = 0.0f;
  float maxBallHeight = 4.0f;
  float playerBallDestZ = 12.0f;
  float ballOffset = 3.0f;

  glm::vec3 camPos = glm::vec3(0.0f, 2.85f, 15.63f);
  const glm::vec3 camUp = glm::vec3(0.0f, 1.0f, 0.0f);

  glm::vec3 playerPos = glm::vec3(-maxX, 0.0f, 2.4f);
  glm::vec3 enemyPos = glm::vec3(0.0f, 0.0f, 0.0f);

  glm::vec3 ballBeginPos = glm::vec3(-ballMaxX + ballOffset, 0.0f, 0.0f);
  glm::vec3 ballEndPos = glm::vec3(-ballMaxX, 0.0f, playerBallDestZ);

  glm::vec3 ballPos = glm::vec3(0.0f, -5.0f, 0.0f);

  int score = 0;
  int highScore = 0;

  bool isPlaying = false;
  float lastTime = 0.0f;
};