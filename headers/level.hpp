#ifndef LEVEL
#define LEVEL

#include <vector>

#include "bezier.hpp"
#include "entity.hpp"
#include "enemies.hpp"

struct Level {
  Player* player;
  std::vector<Obstacle*> obstacles;
  std::vector<MeleeEnemy*> meleeEnemies;
  std::vector<RangedEnemy*> rangedEnemies;
	std::vector<Bullet*> bullets;

  std::vector<Vector2> itemSpawns;
  std::vector<Item*> items;

  void Update(Rectangle limits, const float timestep) {
    for (Obstacle* o : obstacles) {
      if (o->type == ObstacleType::MOVING) {
        o->MoveAlongPath();
      }
    }
		for (Bullet* b : bullets) {
			b->Update(timestep);
		}
  }

  void Draw() {
    for (Obstacle* o : obstacles) {
      o->Draw();
    }
		for (Vector2 itemSpawn : itemSpawns) {
			DrawCircleV(itemSpawn, 10, RED);
		}
		for (Bullet* b : bullets) {
			b->Draw();
		}
  }

  void GeneratePaths() {
    for (Obstacle* o : obstacles) {
      if (o->type == ObstacleType::MOVING) {
        o->path.CalculateCurve();
      }
    }
  }

  static Level* LoadLevel(const char filename[]) {
    Level* level = new Level;
    std::ifstream levelFile(filename);

    if (!levelFile) {
      std::cerr << "Unable to open level file.";
      exit(1);
    }

    Vector2 initialPlayerPosition;
    levelFile >> initialPlayerPosition.x >> initialPlayerPosition.y;
    Player* p = new Player(
      initialPlayerPosition, {PLAYER_WIDTH / 2, PLAYER_HEIGHT / 2}
    );
    level->player = p;

    int staticObstacleCount;
    levelFile >> staticObstacleCount;
    for (int i = 0; i < staticObstacleCount; ++i) {
      Vector2 oPosition;
      Vector2 oHalfSizes;
      levelFile >> oPosition.x >> oPosition.y;
      levelFile >> oHalfSizes.x >> oHalfSizes.y;
      Obstacle* o = new Obstacle(ObstacleType::STATIC, oPosition, oHalfSizes);
      level->obstacles.push_back(o);
    }

    int movingObstacleCount;
    int highestControlPointCount = 0;
    levelFile >> movingObstacleCount;
    for (int i = 0; i < movingObstacleCount; ++i) {
      Vector2 oHalfSizes;
      levelFile >> oHalfSizes.x >> oHalfSizes.y;

      int oCurveOrder;
      int oControlPointCount;
      float oNumberOfSteps;
      BezierCurve oPath;
      levelFile >> oCurveOrder >> oControlPointCount >> oNumberOfSteps;

      if (oCurveOrder <= 0) {
        std::string errorMsg =
          std::string(
            "Curve order must be a positive integer! Given curve order: "
          ) +
          std::to_string(oCurveOrder);
        throw std::invalid_argument(errorMsg);
      }

      if (!ValidateControlPointCount(oCurveOrder, oControlPointCount)) {
        std::string errorMsg = std::string(
                                 "Invalid control point amount! Given curve "
                                 "order and control point amount: "
                               ) +
                               std::to_string(oCurveOrder) +
                               std::to_string(oControlPointCount);
        throw std::invalid_argument(errorMsg);
      }

      if (oControlPointCount > highestControlPointCount) {
        highestControlPointCount = oControlPointCount;
      }

      for (int j = 0; j < oControlPointCount; ++j) {
        Vector2 controlPoint;
        levelFile >> controlPoint.x >> controlPoint.y;
        oPath.points.push_back(controlPoint);
      }

      oPath.numberOfSteps = oNumberOfSteps;

      Obstacle* o = new Obstacle(ObstacleType::MOVING, {0, 0}, oHalfSizes);
      o->color = MOVING_OBSTACLE_COLOR;
      o->path = oPath;
      level->obstacles.push_back(o);
    }

    if (highestControlPointCount > 0) {
      pascalsTriangle = GeneratePascalsTriangle(highestControlPointCount);
    }

    int itemSpawnCount;
    levelFile >> itemSpawnCount;
    for (int i = 0; i < itemSpawnCount; ++i) {
			Vector2 itemPosition;
			levelFile >> itemPosition.x >> itemPosition.y;
			level->itemSpawns.push_back(itemPosition);
		}
    
		levelFile.close();

    return level;
  }
};

#endif