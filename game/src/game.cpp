#include "game.hpp"
#include "resource-paths.hpp"

#include <SDL.h>
#include <asset-manager.hpp>
#include <input.hpp>
#include <opencv2\opencv.hpp>

static Game *game; // this is dirty but it works for now

#ifdef SHARED_GAME
#include <cassert>
#include <cr.h>

static int loaded_timestamp = 0;

CR_EXPORT int cr_main(struct cr_plugin *ctx, enum cr_op operation) {
  assert(ctx);

  gladLoadGLES2Loader((GLADloadproc)SDL_GL_GetProcAddress);
  // get a random int
  switch (operation) {
  case CR_LOAD:
    loaded_timestamp = SDL_GetTicks();
    game = new Game();
    game->init((SharedData *)ctx->userdata);
    return printf("loaded %i\n", loaded_timestamp);
  case CR_UNLOAD:
    game->unload();
    return printf("unloaded %i\n", loaded_timestamp);
  case CR_CLOSE:
    game->close();
    delete game;
    return printf("closed %i\n", loaded_timestamp);
  case CR_STEP:
    return game->update();
  }
  return 0;
}
#endif

Game::Game() { game = this; }

Game::~Game() {}

int Game::init(SharedData *shared_data) {
  SDL_Log("Game init");
  SDL_SetWindowTitle(SDL_GL_GetCurrentWindow(), "Tink's World");

  // map the text_input_buffer
  InputManager::SetTextInputBuffer(&shared_data->text_input_buffer[0]);
  // Get current window size
  int w, h;
  SDL_GetWindowSize(SDL_GL_GetCurrentWindow(), &w, &h);
  this->spriteBatcher = std::make_unique<SpriteBatch>(glm::vec2(w, h));

  this->meshRenderer = std::make_unique<MeshRenderer>();

  this->mixer = std::make_unique<Mixer>();

  this->font = AssetManager<Font>::getFont(RES_FONT_VERA, 32);

  cv::namedWindow("Display window");

  SDL_Log("Before capture\n");
  capture = cv::VideoCapture(cv::CAP_ANY);
  SDL_Log("After capture\n");

#ifndef EMSCRIPTEN
  this->mixer->ToggleMute();
#endif

  glm::vec2 center = glm::vec2(w / 2, h / 2);
  SDL_Rect bounds = {0, 0, w, h};

  this->spriteBatcher->UpdateCamera(center, bounds);

  return 0;
}

int Game::update() {
  int num_keys;
  const Uint8 *key_state = SDL_GetKeyboardState(&num_keys);
  InputManager::Update(key_state, num_keys);

  this->meshRenderer->Draw();

  this->font->RenderText(this->spriteBatcher.get(), "Hello, Cube!",
                         glm::vec2(200, 200), glm::vec2(1.0f),
                         glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));

  // draw all sprites in the batch (note text is also a sprite)
  this->spriteBatcher->Flush();

  if (!capture.isOpened()) {
    SDL_Log("Camera Closed\n");
  } else {
    SDL_Log("Camera Open\n");
    capture >> image;

    imshow("Display window", image);
    cv::waitKey(25);
  }

  return 0;
}

int Game::unload() { return 0; }

int Game::close() {
  // clean up gl stuff
  return 0;
}
