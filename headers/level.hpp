#ifndef LEVEL
#define LEVEL

#include <vector>

#include "entity.hpp"

struct Level {
  Player* player;
  std::vector<Entity*> obstacles;
  std::vector<MovingEntity*> movingObstacles;

  void Draw() {
    for (Entity* o : obstacles) {
      o->Draw();
    }
    player->Draw();
  }
};

Level* LoadLevel(const char filename[]) {
  Level* level = new Level;
  std::ifstream levelFile(filename);

  if (!levelFile) {
    std::cerr << "Unable to open level file.";
    exit(1);
  }

  Vector2 initialPlayerPosition;
  levelFile >> initialPlayerPosition.x >> initialPlayerPosition.y;
  Player* p = new Player(
    initialPlayerPosition, {PLAYER_WIDTH / 2, PLAYER_HEIGHT / 2}, PLAYER_COLOR
  );
  level->player = p;

  int obstacleCount;
	int movingObstacleCount;
  levelFile >> obstacleCount;
  levelFile >> movingObstacleCount;
  for (int i = 0; i < obstacleCount; ++i) {
    Vector2 oPosition;
    Vector2 oHalfSizes;
    levelFile >> oPosition.x >> oPosition.y;
    levelFile >> oHalfSizes.x >> oHalfSizes.y;
    Entity* o = new Entity(oPosition, oHalfSizes, OBSTACLE_COLOR);
    level->obstacles.push_back(o);
  }

	for (int i = 0; i < movingObstacleCount; ++i) {
    Vector2 oPosition;
    Vector2 oHalfSizes;
    levelFile >> oPosition.x >> oPosition.y;
    levelFile >> oHalfSizes.x >> oHalfSizes.y;
    MovingEntity* o = new MovingEntity(oPosition, oHalfSizes, OBSTACLE_COLOR);
    level->movingObstacles.push_back(o);
  }

  levelFile.close();

  return level;
}

#endif