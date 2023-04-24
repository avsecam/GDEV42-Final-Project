#include <raylib.h>
#include <raymath.h>

#include <fstream>
#include <iostream>
#include <vector>

#include "headers/bezier.hpp"
#include "headers/level.hpp"
#include "headers/properties.hpp"
#include "headers/uihandler.hpp"

const char* LEVEL_FILENAME("level.cfg");
const char* PROPERTIES_FILENAME("properties.cfg");

const float WINDOW_WIDTH(800);
const float WINDOW_HEIGHT(600);
const char* WINDOW_TITLE("HAKENSLASH THE PLATFORMER");

const int TARGET_FPS(60);
const float TIMESTEP(1.0f / (float)TARGET_FPS);

int main() {
  Properties* properties = LoadProperties(PROPERTIES_FILENAME, TARGET_FPS);
  Level* level = Level::LoadLevel(LEVEL_FILENAME);
  level->GeneratePaths();

  Player* player = level->player;

  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
  SetTargetFPS(TARGET_FPS);

  Camera2D cameraView = {0};
  cameraView.target = {player->position.x, player->position.y};
  cameraView.offset = {WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2};
  cameraView.zoom = 1.0f;

  Vector2 cameraPos = {player->position.x, player->position.y};

  std::cout << "CamEdges " << properties->camUpperLeft.x << " "
            << properties->camUpperLeft.y << " " << properties->camLowerRight.x
            << " " << properties->camLowerRight.y << std::endl;
  std::cout << "CamType " << properties->camType << std::endl;
  std::cout << "CamDrift " << properties->camDrift << std::endl;

  float accumulator = 0.0f;
  float delta = 0.0f;
  while (!WindowShouldClose()) {
    delta = GetFrameTime();

    float windowLeft = cameraView.target.x + properties->camUpperLeft.x;
    float windowRight = cameraView.target.x + properties->camLowerRight.x;
    float windowTop = cameraView.target.y + properties->camUpperLeft.y;
    float windowBot = cameraView.target.y + properties->camLowerRight.y;
    // Move horizontally
    player->MoveHorizontal(properties);

    // Check collision, assume from left or right
    player->CollideHorizontal(level->obstacles, properties->gap);

    // Move vertically
    player->MoveVertical(properties);

    // Check top collision if moving upwards, bottom if downwards
    player->CollideVertical(level->obstacles, properties->gap);

    if (IsKeyPressed(KEY_E)) {
      if (properties->camType >= 4) {
        properties->camType = 0;
      } else {
        properties->camType += 1;
      }
    }

    if (properties->camType == 0) {
      cameraView.target = {player->position.x, player->position.y};
    } else if (properties->camType == 1) {
      cameraView.target = {player->position.x, player->position.y};
      if (player->position.x < properties->cam1UpperLeft.x) {
        cameraView.target.x = properties->cam1UpperLeft.x;
      }
      if (player->position.x > properties->cam1LowerRight.x) {
        cameraView.target.x = properties->cam1LowerRight.x;
      }
      if (player->position.y < properties->cam1UpperLeft.y) {
        cameraView.target.y = properties->cam1UpperLeft.y;
      }
      if (player->position.y > properties->cam1LowerRight.y) {
        cameraView.target.y = properties->cam1LowerRight.y;
      }
    } else if (properties->camType == 2) {
      float cameraPushX = 0.0f;
      float cameraPushY = 0.0f;

      if ((player->position.x + player->halfSizes.x) > windowRight) {
        cameraPushX = (player->position.x + player->halfSizes.x) - windowRight;
        // std::cout << "CAM PUSHING RIGHT" << std::endl;
      } else if ((player->position.x - player->halfSizes.x) < windowLeft) {
        cameraPushX = (player->position.x - player->halfSizes.x) - windowLeft;
        // std::cout << "CAM PUSHING LEFT" << std::endl;
      }
      if ((player->position.y + player->halfSizes.y) > windowBot) {
        cameraPushY = (player->position.y + player->halfSizes.y) - windowBot;
        // std::cout << "CAM PUSHING BOT" << std::endl;
      } else if ((player->position.y - player->halfSizes.y) < windowTop) {
        cameraPushY = (player->position.y - player->halfSizes.y) - windowTop;
        // std::cout << "CAM PUSHING TOP" << std::endl;
      }
      cameraView.target.x += cameraPushX;
      cameraView.target.y += cameraPushY;
    } else if (properties->camType == 3) {
      float cameraPushX = 0.0f;
      float cameraPushY = 0.0f;
      float driftX = Clamp(
        player->position.x - (windowLeft + windowRight) / 2,
        -properties->camDrift, properties->camDrift
      );
      float driftY = Clamp(
        player->position.y - (windowTop + windowBot) / 2, -properties->camDrift,
        properties->camDrift
      );

      if ((player->position.x + player->halfSizes.x) > windowRight) {
        cameraPushX = (player->position.x + player->halfSizes.x) - windowRight;
        // std::cout << "CAM PUSHING RIGHT" << std::endl;
        cameraView.target.x += cameraPushX;
      } else if ((player->position.x - player->halfSizes.x) < windowLeft) {
        cameraPushX = (player->position.x - player->halfSizes.x) - windowLeft;
        // std::cout << "CAM PUSHING LEFT" << std::endl;
        cameraView.target.x += cameraPushX;
      } else {
        cameraView.target.x += driftX;
        // std::cout << "DRIFTING HORIZONTALLY" << std::endl;
      }
      if ((player->position.y + player->halfSizes.y) > windowBot) {
        cameraPushY = (player->position.y + player->halfSizes.y) - windowBot;
        // std::cout << "CAM PUSHING BOT" << std::endl;
        cameraView.target.y += cameraPushY;
      } else if ((player->position.y - player->halfSizes.y) < windowTop) {
        cameraPushY = (player->position.y - player->halfSizes.y) - windowTop;
        // std::cout << "CAM PUSHING TOP" << std::endl;
        cameraView.target.y += cameraPushY;
      } else {
        cameraView.target.y += driftY;
        // std::cout << "DRIFTING VERTICALLY" << std::endl;
      }
    } else if (properties->camType == 4) {
      float cameraPushX = 0.0f;
      float cameraPushY = 0.0f;
      float driftY = Clamp(
        player->position.y - (windowTop + windowBot) / 2, -properties->camDrift,
        properties->camDrift
      );

      if ((player->position.x + player->halfSizes.x) > windowRight) {
        cameraPushX = (player->position.x + player->halfSizes.x) - windowRight;
        // std::cout << "CAM PUSHING RIGHT" << std::endl;
      } else if ((player->position.x - player->halfSizes.x) < windowLeft) {
        cameraPushX = (player->position.x - player->halfSizes.x) - windowLeft;
        // std::cout << "CAM PUSHING LEFT" << std::endl;
      }
      cameraView.target.x += cameraPushX;
      if (player->isGrounded) {
        if ((player->position.y + player->halfSizes.y) > windowBot) {
          cameraPushY = (player->position.y + player->halfSizes.y) - windowBot;
          // std::cout << "CAM PUSHING BOT" << std::endl;
          cameraView.target.y += cameraPushY;
        } else if ((player->position.y - player->halfSizes.y) < windowTop) {
          cameraPushY = (player->position.y - player->halfSizes.y) - windowTop;
          // std::cout << "CAM PUSHING TOP" << std::endl;
          cameraView.target.y += cameraPushY;
        } else {
          cameraView.target.y += driftY;
          // std::cout << "DRIFTING VERTICALLY" << std::endl;
        }
      }
    }

    accumulator += delta;
    while (accumulator >= TIMESTEP) {
      level->Update();
			accumulator -= TIMESTEP;
    }

    BeginDrawing();
    BeginMode2D(cameraView);
    ClearBackground(WHITE);

    level->Draw();
    if (properties->camType == 2 || properties->camType == 3 || properties->camType == 4) {
      DrawRectangleLines(
        windowLeft, windowTop, windowRight - windowLeft, windowBot - windowTop,
        RED
      );
    }

    EndDrawing();
  }

  CloseWindow();

  // Delete pointers
  delete player;
  for (Obstacle* o : level->obstacles) {
    delete o;
  }
  delete level;

  return 0;
}