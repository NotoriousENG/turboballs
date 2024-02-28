#pragma once

#include <memory>

#include "renderer.hpp"
#include "window.hpp"

#include "shared-data.hpp"

#ifdef SHARED_GAME
#include <cr.h>
#else
#include <game.hpp>
#endif

#ifdef EMSCRIPTEN
void emscripten_update();
#endif

class App {
public:
  App();
  ~App();
  void run();
  void update();
  void onClose();
  void poll_events();

private:
  bool is_running;
  std::unique_ptr<Window> window;
  std::unique_ptr<Renderer> renderer;

  SDL_AudioSpec want, have;
  SDL_AudioDeviceID dev;

  SharedData shared_data;

#ifdef SHARED_GAME
  cr_plugin game_ctx;
#else
  Game game;
#endif
};

static float input_volume = 0.5f;

void audio_callback(void *userdata, Uint8 *stream, int len);