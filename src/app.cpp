#define CR_HOST

#include "app.hpp"

#include <glm/glm.hpp>

#include <stdio.h>

#include <shared-data.hpp>

#ifdef EMSCRIPTEN
#include <emscripten.h>
#include <emscripten/html5.h>

static App *app_instance;

void emscripten_update() { app_instance->update(); }
#endif

#define SAMPLE_RATE 44100
#define FRAMES_PER_BUFFER 512

App::App() {
  this->is_running = true;
  // memset clear the shared data buffer
  memset(&this->shared_data, 0, sizeof(this->shared_data));
}

App::~App() {}

void App::run() {

  const auto initial_window_size = glm::vec2(800, 600);
  this->window = std::make_unique<Window>(GAME_NAME, initial_window_size.x,
                                          initial_window_size.y);
  this->renderer = std::make_unique<Renderer>(this->window.get());

  SDL_memset(&want, 0, sizeof(want)); /* or SDL_zero(want) */
  want.freq = SAMPLE_RATE;
  want.format = AUDIO_F32SYS;
  want.channels = 1;
  want.samples = FRAMES_PER_BUFFER;
  want.callback = audio_callback;

  SDL_AudioDeviceID dev = SDL_OpenAudioDevice(NULL, 1, &want, &have, 0);
  if (dev == 0) {
    SDL_Log("Failed to open audio: %s", SDL_GetError());
    // @todo implement a retry mechanism
    return;
  }

  // begin listining to audio
  SDL_PauseAudioDevice(dev, 0);

  SDL_StopTextInput(); // ensure this is off by default

  this->shared_data.input_volume = &input_volume;

#ifdef SHARED_GAME
  SDL_Log("Shared Lib: %s", GAME_LIBRARY_PATH);
  cr_plugin_open(this->game_ctx, GAME_LIBRARY_PATH);
  this->game_ctx.userdata = &this->shared_data;
#else
  this->game.init(&this->shared_data);
#endif

#ifdef EMSCRIPTEN
  app_instance = this;
  emscripten_set_main_loop(emscripten_update, 0, this->is_running);
#else
  while (this->is_running) {
    this->update();
  }
#endif

  this->onClose();
}

void App::update() {
  this->renderer->Clear();
  this->poll_events();
#ifdef SHARED_GAME
  if (cr_plugin_changed(
          this->game_ctx)) { // full teardown needed on non windows
    cr_plugin_close(this->game_ctx);
    cr_plugin_open(this->game_ctx, GAME_LIBRARY_PATH);
  }
  cr_plugin_update(this->game_ctx);
  fflush(stdout);
  fflush(stderr);
#else
  this->game.update();
#endif
  this->renderer->Present();
}

void App::onClose() {
#ifdef SHARED_GAME
  cr_plugin_close(this->game_ctx);
  SDL_Log("App closed\n");
#else
  this->game.unload();
  this->game.close();
#endif

  SDL_CloseAudioDevice(dev);
  // destroy the renderer this has to be done before the
  // window is destroyed
  this->renderer.reset();
}

void App::poll_events() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_QUIT:
      this->is_running = false;
      break;
    case SDL_KEYDOWN:
      if (event.key.keysym.sym == SDLK_BACKSPACE &&
          strlen(this->shared_data.text_input_buffer) > 0) {
        this->shared_data
            .text_input_buffer[strlen(this->shared_data.text_input_buffer) -
                               1] = '\0';
      }
      break;
    case SDL_TEXTINPUT:
      // add text to buffer
      if (strlen(this->shared_data.text_input_buffer) +
              strlen(event.text.text) <
          TEXT_BUFFER_SIZE) {
        strcat(this->shared_data.text_input_buffer, event.text.text);
      }
      break;
    default:
      break;
    }
  }
}

void audio_callback(void *userdata, Uint8 *stream, int len) {
  float *buffer = reinterpret_cast<float *>(stream);
  float max = 0.0f;

  for (int i = 0; i < len / sizeof(float); i++) {
    float absValue = std::abs(buffer[i]);
    if (absValue > max) {
      max = absValue;
    }
  }

  input_volume = max;

  SDL_Log("Max: %f", max);
}
