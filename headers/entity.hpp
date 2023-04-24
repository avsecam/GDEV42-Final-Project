#ifndef ENTITY
#define ENTITY

#include <raylib.h>
#include <raymath.h>

#include <vector>

#include "bezier.hpp"
#include "properties.hpp"

const float PLAYER_WIDTH(24);
const float PLAYER_HEIGHT(32);
const Color PLAYER_COLOR(BLUE);

const Color STATIC_OBSTACLE_COLOR(GRAY);
const Color MOVING_OBSTACLE_COLOR(DARKGRAY);

enum ObstacleType { STATIC, MOVING };
enum ItemType { TIME, HEALTH };

// All entity positions are assumed to be indicated by their centers, not
// upper-lefts

struct Entity {
  Vector2 position;
  Vector2 halfSizes;
  Color color;
  int health;

  Entity() = default;

  Entity(
    Vector2 _position, Vector2 _halfSizes, Color _color = STATIC_OBSTACLE_COLOR
  ) {
    this->position = _position;
    this->halfSizes = _halfSizes;
    this->color = _color;
  }

  void Draw() { DrawRectangleRec(GetCollider(), color); }

  Rectangle GetCollider() {
    return {
      position.x - halfSizes.x,
      position.y - halfSizes.y,
      halfSizes.x * 2,
      halfSizes.y * 2,
    };
  }

  bool IsIntersecting(Rectangle rec) {
    return CheckCollisionRecs(GetCollider(), rec);
  }
};

struct Obstacle : public Entity {
  ObstacleType type;

  BezierCurve path;
  bool isMovingForward = true;
  int progress = 0;  // on what step of the curve is the obstacle in

  Obstacle(
    ObstacleType _type, Vector2 _position, Vector2 _halfSizes,
    Color _color = STATIC_OBSTACLE_COLOR
  ) {
    this->type = _type;
    this->position = _position;
    this->halfSizes = _halfSizes;
    this->color = _color;
  }

  void MoveAlongPath() {
    if (isMovingForward) {
      if (progress < path.stepList.size() - 1) {
        ++progress;
        position = path.stepList[progress];
        if (progress >= path.stepList.size() - 1) {
          isMovingForward = false;
        }
      }
    } else {
      if (progress > 1) {
        --progress;
        position = path.stepList[progress];
        if (progress <= 1) {
          isMovingForward = true;
        }
      }
    }
  }
};

struct Character : public Entity {
  Vector2 velocity = Vector2Zero();

	using Entity::Entity;

	void MoveVertical(const Properties* properties) {
		// gravity handling and velocity limiting
    velocity.y += properties->gravity;
    velocity.y = Clamp(velocity.y, -INT32_MAX, properties->vVelMax);
		
    position.y += velocity.y;
	}
};

struct Player : public Character {
  Vector2 velocity = Vector2Zero();
  float airControlFactor = 1.0f;
  bool isGrounded = false;
  int jumpFrame = 0;
  int framesAfterFallingOff = 0;

  using Entity::Entity;

  void MoveHorizontal(const Properties* properties) {
    // Moving through air
    if (abs(velocity.y) > 0.0f) {
      airControlFactor = properties->hAir;
    } else {
      airControlFactor = 1.0f;
    }

    if (IsKeyDown(KEY_A)) {
      if (velocity.x > 0.0f) {
        velocity.x -=
          properties->hAccel * properties->hOpposite * airControlFactor;
      } else {
        velocity.x -= properties->hAccel * airControlFactor;
      }
      if (abs(velocity.x) >= properties->hVelMax) {
        velocity.x = -properties->hVelMax;
      }
    } else if (IsKeyDown(KEY_D)) {
      if (velocity.x < 0.0f) {
        velocity.x +=
          properties->hAccel * properties->hOpposite * airControlFactor;
      } else {
        velocity.x += properties->hAccel * airControlFactor;
      }
      if (abs(velocity.x) >= properties->hVelMax) {
        velocity.x = properties->hVelMax;
      }
    } else {
      velocity.x *= properties->hCoeff;  // Slow down on no input
    }

    // Minimum horizontal movement threshold
    if (abs(velocity.x) <= properties->hVelMin) {
      velocity.x = 0.0f;
    }
    position.x += velocity.x;
  }

  void MoveVertical(const Properties* properties) {
    // Jump handling
    if (IsKeyPressed(KEY_SPACE) && jumpFrame <= 0 && framesAfterFallingOff <= properties->vSafe) {
      velocity.y += properties->vAccel;
      ++jumpFrame;
    } else if (IsKeyDown(KEY_SPACE) && velocity.y < 0) {  // In jump
      if (jumpFrame < properties->vHold) {
        velocity.y += properties->vAccel *
                      ((properties->vHold - jumpFrame) / properties->vHold);
        ++jumpFrame;
      } else {
        if (velocity.y < properties->vVelCut) {
          velocity.y = properties->vVelCut;
        }
      }
    }
    if (IsKeyReleased(KEY_SPACE)) {
      if (velocity.y < properties->vVelCut) {
        velocity.y = properties->vVelCut;
      }
    }

    Character::MoveVertical(properties);
  }

  void CollideHorizontal(
    const std::vector<Obstacle*> obstacles, const float gap
  ) {
    for (Obstacle* o : obstacles) {
      Rectangle oCollider = o->GetCollider();
      if (IsIntersecting(oCollider)) {
        // Move back
        if (o->type == ObstacleType::STATIC) {
          position.x = velocity.x > 0 ? oCollider.x - (halfSizes.x) - gap
                                      : (oCollider.x + oCollider.width) +
                                          (halfSizes.x) + gap;
        } else {
          position.x = velocity.x > 0 ? position.x - gap : position.x + gap;
        }

        velocity.x = 0;
        break;
      }
    }
  }

  void CollideVertical(
    const std::vector<Obstacle*> obstacles, const float gap
  ) {
    bool isGroundedLastFrame = isGrounded;

    isGrounded = false;
    for (Obstacle* o : obstacles) {
      Rectangle oCollider = o->GetCollider();
      if (IsIntersecting(oCollider)) {
        // Move back
        if (o->type == ObstacleType::STATIC) {
          position.y = velocity.y > 0 ? oCollider.y - (halfSizes.y) - gap
                                      : (oCollider.y + oCollider.height) +
                                          (halfSizes.y) + gap;
        } else {
          position.y = oCollider.y - (halfSizes.y);
        }

        if (velocity.y >= 0) {  // Grounded
          jumpFrame = 0;
          framesAfterFallingOff = 0;
          velocity.y = 0;
          isGrounded = true;
        } else {  // Na-untog
          velocity.y = -velocity.y;
        }

        break;
      }
    }

    // Left a platform
    if ((isGroundedLastFrame && !isGrounded) || (!isGrounded && framesAfterFallingOff > 0)) {
      ++framesAfterFallingOff;
    }
  }
};

struct Enemy : public Entity {

	void Draw() {
		Entity::Draw();
		DrawCircleV(GetBottomLeft(), 5, RED);
		DrawCircleV(GetBottomRight(), 5, RED);
	}

	void MoveLeft() {
	}

	private:
	Vector2 GetBottomLeft() {
		return {
			this->position.x - this->halfSizes.x,
			this->position.y + this->halfSizes.y
		};
	}

	Vector2 GetBottomRight() {
		return {
			this->position.x + this->halfSizes.x,
			this->position.y + this->halfSizes.y
		};
	}
};

struct Item : public Entity {
  ItemType type;

  using Entity::Entity;
};

#endif