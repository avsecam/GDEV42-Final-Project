#include <raylib.h>
#include <raymath.h>

#include <fstream>
#include <iostream>
#include <list>
#include <vector>

#include "headers/bezier.hpp"
#include "headers/enemies.hpp"
#include "headers/level.hpp"
#include "headers/properties.hpp"
#include "headers/uihandler.hpp"

const char *LEVEL_FILENAME("level.cfg");
const char *PROPERTIES_FILENAME("properties.cfg");

const float WINDOW_WIDTH(1280);
const float WINDOW_HEIGHT(720);
const char *WINDOW_TITLE("⚔ HAKENSLASH THE PLATFORMER ⚔");

const int TARGET_FPS(60);
const float TIMESTEP(1.0f / (float)TARGET_FPS);

const float START_TIME(30.0f); // in seconds
const float ATTACK_ANIMATION_LENGTH(0.15f);
const float SWING_COOLDOWN(0.5f);

int main()
{
  Properties *properties = LoadProperties(PROPERTIES_FILENAME, TARGET_FPS);
  Level *level = Level::LoadLevel(LEVEL_FILENAME);
  level->GeneratePaths();

  float attackAnimTimeLeft = ATTACK_ANIMATION_LENGTH;
  float swingCooldownTimeLeft = 0.0f;
  Player *player = level->player;
  PlayerWeapon *weapon = new PlayerWeapon(player->position, {40, 60});
  bool inAttackAnimation = false;
  bool canSwing = false;
  bool showWeaponHitbox = false;

  RangedEnemy *enemy = new RangedEnemy({300, 400}, {20, 20});
  MeleeEnemy *menemy = new MeleeEnemy({500, 200}, {20, 20});
  MeleeEnemy *menemy2 = new MeleeEnemy({500, 400}, {20, 20});
  MeleeEnemy *menemy3 = new MeleeEnemy({200, 500}, {20, 20});
  MeleeEnemy *menemy4 = new MeleeEnemy({600, 420}, {20, 20});
  MeleeEnemy *menemy5 = new MeleeEnemy({400, 120}, {20, 20});
  MeleeEnemy *menemy6 = new MeleeEnemy({300, 120}, {20, 20});
  MeleeEnemy *menemy7 = new MeleeEnemy({800, 280}, {20, 20});
  MeleeEnemy *menemy8 = new MeleeEnemy({200, 1000}, {20, 20});
  MeleeEnemy *menemy9 = new MeleeEnemy({800, 120}, {20, 20});

  level->rangedEnemies.push_back(new RangedEnemy({300, 400}, {20, 20}));
  level->rangedEnemies.push_back(new RangedEnemy({900, 400}, {20, 20}));

  Camera2D cameraView = {0};
  cameraView.target = {player->position.x, player->position.y};
  cameraView.offset = {WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2};
  cameraView.zoom = 1.3f;

  Vector2 cameraPos = {player->position.x, player->position.y};

  std::list<MeleeEnemy *> activeMeleeEnemies{menemy, menemy2, menemy3};
  std::list<MeleeEnemy *> inactiveMeleeEnemies{menemy4, menemy5, menemy6,
                                               menemy7, menemy8, menemy9};

  float timeLeft = START_TIME;
  float timeElapsed = 0.0f;

  float accumulator = 0.0f;
  float delta = 0.0f;
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
  SetTargetFPS(TARGET_FPS);
  while (!WindowShouldClose())
  {
    delta = GetFrameTime();

    float windowLeft = cameraView.target.x + properties->camUpperLeft.x;
    float windowRight = cameraView.target.x + properties->camLowerRight.x;
    float windowTop = cameraView.target.y + properties->camUpperLeft.y;
    float windowBot = cameraView.target.y + properties->camLowerRight.y;

    // Player Movement
    player->MoveHorizontal(properties);
    player->CollideHorizontal(level->obstacles, properties->gap);
    player->MoveVertical(properties);
    player->CollideVertical(level->obstacles, properties->gap);

    weapon->Update(player, level->bullets);

    // Attacking
    if (IsKeyPressed(KEY_J) && canSwing)
    {
      inAttackAnimation = true;
      for (auto const &i : activeMeleeEnemies)
      {
        if (weapon->IsIntersecting(i->GetCollider()))
        {
          i->kill();
          player->kills += 1;
          player->killsThreshold += 1;
          std::cout << "KILLS: " << player->kills << std::endl;
        }
      }

      canSwing = false;

      for (Bullet *b : level->bullets) {
        if (b->IsIntersecting(weapon->GetCollider())) {
          b->direction = {-b->direction.x, -b->direction.y};
        }
      }
    }
    // Enemy Movement
    for (auto const &i : activeMeleeEnemies)
    {
      i->Update(properties, level->obstacles, player);
    }

    if (player->killsThreshold == 10)
    {
      // Add 2 ranged enemies
      level->rangedEnemies.push_back(new RangedEnemy({300, 400}, {20, 20}));
      level->rangedEnemies.push_back(new RangedEnemy({900, 400}, {20, 20}));

      if (inactiveMeleeEnemies.size() > 0)
      {
        activeMeleeEnemies.push_back(inactiveMeleeEnemies.front());
        inactiveMeleeEnemies.pop_front();
        std::cout << "ADDED 1 ENEMY" << std::endl;
      }
      for (auto const &i : activeMeleeEnemies)
      {
        i->speedModifier += 0.025;
      }
      std::cout << "Added 0.025 speed" << std::endl;
      player->killsThreshold = 0;
    }

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

    if ((player->position.y + player->halfSizes.y) > windowBot) {
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

		// Clamp camera
		cameraView.target.x = Clamp(cameraView.target.x, 450, 750);
		cameraView.target.y = Clamp(cameraView.target.y, 300, 750);

    if (IsKeyPressed(KEY_Q)) {
      showWeaponHitbox = !showWeaponHitbox;
    }

    accumulator += delta;
    while (accumulator >= TIMESTEP)
    {
      // TIMER
      timeLeft -= accumulator;
      timeElapsed += accumulator;

      level->Update({0, 0, 1200, 1200}, TIMESTEP);
      for (size_t i = 0; i < level->bullets.size(); ++i) {
        Bullet *b = level->bullets[i];
        if (b->CollidePlayer(player))
        {
          player->health -= 1;
          level->bullets.erase(level->bullets.begin() + i);
          delete b;
        }
        if (b->IsOutsideLimits({0, 0, 1200, 1200})) {
          level->bullets.erase(level->bullets.begin() + i);
          delete b;
        }
      }

      for (size_t i = 0; i < level->rangedEnemies.size(); ++i)
      {
        RangedEnemy *r = level->rangedEnemies[i];
        if (rand() % 100 > 98)
        {
          level->bullets.push_back(r->Shoot(player));
        }
        r->Update(properties, level->obstacles);
        if (r->CollidePlayer(player))
        {
          player->health -= 1;
          level->rangedEnemies.erase(level->rangedEnemies.begin() + i);
          delete r;
        }
      }

      if (swingCooldownTimeLeft <= 0.0f) {
        canSwing = true;
      }
      else {
        swingCooldownTimeLeft -= TIMESTEP;
      }
      
      if (inAttackAnimation) {
        attackAnimTimeLeft -= TIMESTEP;
        if (attackAnimTimeLeft <= 0){
          inAttackAnimation = false;
          attackAnimTimeLeft = ATTACK_ANIMATION_LENGTH;
        }
      }

      accumulator -= TIMESTEP;
    }

    BeginDrawing();
    BeginMode2D(cameraView);
    ClearBackground(WHITE);

    level->Draw();

    for (RangedEnemy *r : level->rangedEnemies)
    {
      r->Draw();
    }
    if (inAttackAnimation)
    {
      weapon->Draw();
    }

    for (auto const &i : activeMeleeEnemies)
    {
      i->Draw();
    }
    // DrawRectangleLines(
    //     windowLeft, windowTop, windowRight - windowLeft, windowBot -
    //     windowTop, RED);

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