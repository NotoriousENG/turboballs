#include "game.hpp"
#include "resource-paths.hpp"

#include <SDL.h>
#include <asset-manager.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <input.hpp>

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
  SDL_SetWindowTitle(SDL_GL_GetCurrentWindow(), "Turboballs");

  // map the text_input_buffer
  InputManager::SetTextInputBuffer(&shared_data->text_input_buffer[0]);
  InputManager::SetInputVolumeRef(shared_data->input_volume);
  // Get current window size
  int w, h;
  SDL_GetWindowSize(SDL_GL_GetCurrentWindow(), &w, &h);
  this->spriteBatcher = std::make_unique<SpriteBatch>(glm::vec2(w, h));

  this->meshRenderer = std::make_unique<MeshRenderer>();

  this->mixer = std::make_unique<Mixer>();

  this->font = AssetManager<Font>::getFont(RES_FONT_CYBERDYNE, 32);
  this->fontBig = AssetManager<Font>::getFont(RES_FONT_CYBERDYNE, 60);

  this->worldModel = AssetManager<Model>::get(RES_MODEL_VAPOR);

  this->npcModel = AssetManager<Model>::get(RES_MODEL_POLY);

  this->ballModel = AssetManager<Model>::get(RES_MODEL_BALL);

  this->music = AssetManager<Music>::get(RES_MUSIC_TURBOBALLS);

  for (const auto &mesh : this->npcModel->getMeshes()) {
    // scale the model up by 10
    mesh->model = glm::scale(mesh->model, glm::vec3(5.0f));
  }

  this->music->play_on_loop();

  glm::vec2 center = glm::vec2(w / 2, h / 2);
  SDL_Rect bounds = {0, 0, w, h};

  this->spriteBatcher->UpdateCamera(center,
                                    bounds); // this is needed for sprites/text

  // set clear color to night dark blue
  glClearColor(0.0f, 0.0f, 0.07f, 1.0f);

  return 0;
}

int Game::update() {

  float time = SDL_GetTicks() / 1000.0f;
  float delta = time - this->lastTime;
  this->lastTime = time;

  // INPUT:
  int num_keys;
  const Uint8 *key_state = SDL_GetKeyboardState(&num_keys);
  InputManager::Update(key_state, num_keys);

  // if enter is pressed toggle isPlaying
  if (!this->isPlaying &&
      InputManager::GetKey(SDL_SCANCODE_RETURN).IsJustPressed()) {
    this->score = 0;
    this->isPlaying = !this->isPlaying;
  }

  // clamp volume to 0.0f - 1.0f
  const float clamp_volume =
      glm::clamp(InputManager::GetInputVolume(), 0.0f, 1.0f);

  // LOGIC:

  if (isPlaying) {
    // increase t
    this->t += 0.4f * delta; // in one second t will be 0.4f

    // PLAYER:

    // set player position (x) based off input volume
    // 0 is -maxX, 1 is maxX
    this->playerPos.x = (clamp_volume * this->maxX * 2) - this->maxX;

    // CAMERA:
    // set the cam pos based off the player pos rel to max
    this->camPos.x = this->playerPos.x / this->maxX * this->camMaxX;
    glm::mat4 view = glm::lookAt(this->camPos, glm::vec3(0), this->camUp);
    this->meshRenderer->SetViewMatrix(view);

    // BALL:
    float adjX = this->playerPos.x / this->maxX * this->ballMaxX;

    glm::vec3 adjVec = glm::vec3(adjX, 0.0f, playerBallDestZ);

    if (ballEndPos.z == playerBallDestZ) {
      if (glm::distance(this->ballPos, adjVec) < 2.0f) {
        // increase score
        this->score++;
        this->t = 1.1f;
        // set high score
        if (this->score > this->highScore) {
          this->highScore = this->score;
        }
      }
      // if it is 0.1 away from dest
      else if (playerBallDestZ - this->ballPos.z < 0.1f) {
        t = 1.1f;
        isPlaying = false;
      }
    }

    // lerp ball pos
    this->ballPos = glm::mix(this->ballBeginPos, this->ballEndPos, this->t);
    // override the y to be max at t = 0.5f, 0.1 at t = 0.0f and 1.0f
    this->ballPos.y = this->maxBallHeight * 4 * this->t * (1 - this->t) + 0.1f;

    // if t is 1.0f reset t and swap ball pos
    if (this->t >= 1.0f) {
      this->t = 0.1f;
      this->ballBeginPos = this->ballPos;
      // swap ball end pos z
      this->ballEndPos.z =
          this->ballEndPos.z >= playerBallDestZ ? 0 : playerBallDestZ;

      // set ball emmision factor to a random color
      this->ballModel->getMeshes()[0]->material->emissiveFactor =
          glm::vec3((rand() % 100) / 100.0f, (rand() % 100) / 100.0f,
                    (rand() % 100) / 100.0f);

      // set a random x pos for the ballEndPos (based on cam)
      this->ballEndPos.x = (rand() % (int)this->ballMaxX * 2) - this->ballMaxX;
    }

    // ENEMY:
    // lerp the enemy z pos to always be the ballEndPos x
    this->enemyPos.x = glm::mix(this->enemyPos.x,
                                ballEndPos.x / this->ballMaxX * this->maxX, t);
  }

  // RENDER:

  // RENDER BACKGROUND TO A BUFFER TEXTURE:

  for (const auto &mesh : this->worldModel->getMeshes()) {
    this->meshRenderer->DrawMesh(mesh.get(), mesh->model);
  }

  for (const auto &mesh : this->ballModel->getMeshes()) {
    glm::mat4 xform = glm::translate(mesh->model, this->ballPos);
    this->meshRenderer->DrawMesh(mesh.get(), xform);
  }

  // RENDER TO A BLUR TEXTURE

  // RENDER THE BACKGROUND TO THE SCREEN AS A SPRITE, USING THE BLUR TO POST PROCESS

  // RENDER THE PLAYER, USING THE BLUR TO POST PROCESS

  for (const auto &mesh : this->npcModel->getMeshes()) {
    glm::mat4 xform = glm::translate(mesh->model, this->enemyPos);
    this->meshRenderer->DrawMesh(mesh.get(), xform);
  }

  for (const auto &mesh : this->npcModel->getMeshes()) {
    // translate mesh by playerPos (before rotation and scale)
    glm::mat4 xform = glm::translate(mesh->model, this->playerPos);
    this->meshRenderer->DrawMesh(mesh.get(), xform);
  }

  // RENDER THE TEXT, ALSO USING THE BLUR CUZ WHY NOT

  if (!isPlaying) {
    // render every half second
    if (SDL_GetTicks() % 1500 < 750) {
      const std::string pause_text = "Press Enter to Play";
      this->font->RenderText(this->spriteBatcher.get(), pause_text.c_str(),
                             glm::vec2(150, 300), glm::vec2(1.0f),
                             glm::vec4(0.7f, 1.0f, 0.93f, 0.8f));
    }

    this->spriteBatcher->Flush();

    const std::string title = "Turboballs";
    this->fontBig->RenderText(this->spriteBatcher.get(), title.c_str(),
                              glm::vec2(130, 200), glm::vec2(1.0f),
                              glm::vec4(0.0f, 1.0f, 1.0f, 1.0f));
  } else {
    char input_volume_percent_3_figures[6];
    sprintf(input_volume_percent_3_figures, "%.1f", clamp_volume * 100.0f);

    const std::string text =
        "Mic: " + std::string(input_volume_percent_3_figures) + '%';

    this->font->RenderText(this->spriteBatcher.get(), text.c_str(),
                           glm::vec2(0, 600 - 32), glm::vec2(1.0f),
                           glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));

    const std::string score_text = "Score: " + std::to_string(this->score);
    this->font->RenderText(this->spriteBatcher.get(), score_text.c_str(),
                           glm::vec2(0, 0), glm::vec2(1.0f),
                           glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));

    const std::string high_score_text =
        "High Score: " + std::to_string(this->highScore);
    // render high score (top right)
    this->font->RenderText(this->spriteBatcher.get(), high_score_text.c_str(),
                           glm::vec2(420, 0), glm::vec2(1.0f),
                           glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
  }

  // draw all sprites in the batch (note text is also a sprite)
  this->spriteBatcher->Flush();

  return 0;
}

int Game::unload() { return 0; }

int Game::close() {
  // clean up gl stuff
  return 0;
}
