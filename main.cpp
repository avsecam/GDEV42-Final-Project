#include <raylib.h>
#include <raymath.h>

#include <fstream>
#include <iostream>
#include <vector>

#include "headers/bezier.hpp"
#include "headers/level.hpp"
#include "headers/properties.hpp"
#include "headers/uihandler.hpp"
#include "headers/enemies.hpp"

const char *LEVEL_FILENAME("level.cfg");
const char *PROPERTIES_FILENAME("properties.cfg");

const float WINDOW_WIDTH(1280);
const float WINDOW_HEIGHT(720);
const char *WINDOW_TITLE("HAKENSLASH THE PLATFORMER");

const int TARGET_FPS(60);
const float TIMESTEP(1.0f / (float)TARGET_FPS);

int main()
{
  Properties *properties = LoadProperties(PROPERTIES_FILENAME, TARGET_FPS);
  Level *level = Level::LoadLevel(LEVEL_FILENAME);
  level->GeneratePaths();

  Player *player = level->player;

	RangedEnemy *enemy = new RangedEnemy({300, 400}, {20, 20});
  MeleeEnemy *menemy = new MeleeEnemy({500, 200}, {20, 20});
	enemy->health = 10;
  menemy->health = 10;

  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
  SetTargetFPS(TARGET_FPS);

  Camera2D cameraView = {0};
  cameraView.target = {player->position.x, player->position.y};
  cameraView.offset = {WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2};
  cameraView.zoom = 1.3f;

  Vector2 cameraPos = {player->position.x, player->position.y};

  std::cout << "CamEdges " << properties->camUpperLeft.x << " "
            << properties->camUpperLeft.y << " " << properties->camLowerRight.x
            << " " << properties->camLowerRight.y << std::endl;
  std::cout << "CamType " << properties->camType << std::endl;
  std::cout << "CamDrift " << properties->camDrift << std::endl;

  float accumulator = 0.0f;
  float delta = 0.0f;
  while (!WindowShouldClose())
  {
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

		enemy->Update(properties, level->obstacles);
    menemy->Update(properties, level->obstacles, player);

    float cameraPushX = 0.0f;
    float cameraPushY = 0.0f;
    float driftX = Clamp(
        player->position.x - (windowLeft + windowRight) / 2,
        -properties->camDrift, properties->camDrift);
    float driftY = Clamp(
        player->position.y - (windowTop + windowBot) / 2, -properties->camDrift,
        properties->camDrift);

    if ((player->position.x + player->halfSizes.x) > windowRight)
    {
      cameraPushX = (player->position.x + player->halfSizes.x) - windowRight;
      // std::cout << "CAM PUSHING RIGHT" << std::endl;
      cameraView.target.x += cameraPushX;
    }
    else if ((player->position.x - player->halfSizes.x) < windowLeft)
    {
      cameraPushX = (player->position.x - player->halfSizes.x) - windowLeft;
      // std::cout << "CAM PUSHING LEFT" << std::endl;
      cameraView.target.x += cameraPushX;
    }
    else
    {
      cameraView.target.x += driftX;
      // std::cout << "DRIFTING HORIZONTALLY" << std::endl;
    }
    if ((player->position.y + player->halfSizes.y) > windowBot)
    {
      cameraPushY = (player->position.y + player->halfSizes.y) - windowBot;
      // std::cout << "CAM PUSHING BOT" << std::endl;
      cameraView.target.y += cameraPushY;
    }
    else if ((player->position.y - player->halfSizes.y) < windowTop)
    {
      cameraPushY = (player->position.y - player->halfSizes.y) - windowTop;
      // std::cout << "CAM PUSHING TOP" << std::endl;
      cameraView.target.y += cameraPushY;
    }
    else
    {
      cameraView.target.y += driftY;
      // std::cout << "DRIFTING VERTICALLY" << std::endl;
    }

    accumulator += delta;
    while (accumulator >= TIMESTEP)
    {
      level->Update();
      accumulator -= TIMESTEP;
    }

    BeginDrawing();
    BeginMode2D(cameraView);
    ClearBackground(WHITE);

    level->Draw();
		enemy->Draw();
    menemy->Draw();

    // DrawRectangleLines(
    //     windowLeft, windowTop, windowRight - windowLeft, windowBot - windowTop,
    //     RED);
    
    EndDrawing();
  }

  CloseWindow();

  // Delete pointers
  delete player;
  for (Obstacle *o : level->obstacles)
  {
    delete o;
  }
  delete level;

  return 0;
}