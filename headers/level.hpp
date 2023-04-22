#ifndef LEVEL
#define LEVEL

#include <vector>

#include "bezier.hpp"
#include "entity.hpp"

struct Level {
  Player* player;
  std::vector<Obstacle*> obstacles;

  void Update() {
    for (Obstacle* o : obstacles) {
      if (o->type == ObstacleType::MOVING) {
        o->MoveAlongPath();
      }
    }
  }

  void Draw() {
		for (Obstacle* o : obstacles) {
			if (o->type == ObstacleType::MOVING) {
				o->path.Draw();
			}
		}
    for (Obstacle* o : obstacles) {
      o->Draw();
    }
    player->Draw();
  }

  void GeneratePaths() {
    for (Obstacle* o : obstacles) {
      if (o->type == ObstacleType::MOVING) {
        o->path.CalculateCurve();
      }
    }
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

  int staticObstacleCount;
  int movingObstacleCount;
  levelFile >> staticObstacleCount;
  levelFile >> movingObstacleCount;
  int highestControlPointCount = 0;
  for (int i = 0; i < staticObstacleCount; ++i) {
    Vector2 oPosition;
    Vector2 oHalfSizes;
    levelFile >> oPosition.x >> oPosition.y;
    levelFile >> oHalfSizes.x >> oHalfSizes.y;
    Obstacle* o = new Obstacle(ObstacleType::STATIC, oPosition, oHalfSizes);
    level->obstacles.push_back(o);
  }

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
      std::string errorMsg =
        std::string(
          "Invalid control point amount! Given curve order and control point amount: "
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

  levelFile.close();

  return level;
}

#endif