#ifndef ENTITY
#define ENTITY

#include <raylib.h>
#include <raymath.h>

#include <vector>

#include "properties.hpp"
#include "bezier.hpp"

const float PLAYER_WIDTH(24);
const float PLAYER_HEIGHT(32);
const Color PLAYER_COLOR(BLUE);

const Color OBSTACLE_COLOR(GRAY);

// All entity positions are assumed to be indicated by their centers, not
// upper-lefts

struct Entity {
  Vector2 position;
  Vector2 halfSizes;
  Color color;

  Entity(Vector2 _position, Vector2 _halfSizes, Color _color) {
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

struct MovingEntity : Entity {
	BezierCurve path;
	
  using Entity::Entity;
};

struct Player : Entity {
  Vector2 velocity = Vector2Zero();
  Vector2 acceleration;
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
        velocity.x -= acceleration.x * properties->hOpposite * airControlFactor;
      } else {
        velocity.x -= acceleration.x * airControlFactor;
      }
      if (abs(velocity.x) >= properties->hVelMax) {
        velocity.x = -properties->hVelMax;
      }
    } else if (IsKeyDown(KEY_D)) {
      if (velocity.x < 0.0f) {
        velocity.x += acceleration.x * properties->hOpposite * airControlFactor;
      } else {
        velocity.x += acceleration.x * airControlFactor;
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
      jumpFrame++;
    } else if (IsKeyDown(KEY_SPACE) && velocity.y < 0) {  // In jump
      if (jumpFrame < properties->vHold) {
        velocity.y += properties->vAccel * ((properties->vHold - jumpFrame) / properties->vHold);
        jumpFrame++;
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

    velocity.y += properties->gravity;
    velocity.y = Clamp(velocity.y, -INT32_MAX, properties->vVelMax);

    position.y += velocity.y;
  }

  void CollideHorizontal(const std::vector<Entity*> obstacles, const float gap) {
    for (Entity* o : obstacles) {
      Rectangle oCollider = o->GetCollider();
      if (IsIntersecting(oCollider)) {
        // Move back
        position.x = velocity.x > 0
                       ? oCollider.x - (halfSizes.x) - gap
                       : (oCollider.x + oCollider.width) + (halfSizes.x) + gap;

        velocity.x = 0;
        break;
      }
    }
  }

  void CollideVertical(const std::vector<Entity*> obstacles, const float gap) {
    bool isGroundedLastFrame = isGrounded;

    isGrounded = false;
    for (Entity* o : obstacles) {
      Rectangle oCollider = o->GetCollider();
      if (IsIntersecting(oCollider)) {
        // Move back
        position.y = velocity.y > 0
                       ? oCollider.y - (halfSizes.y) - gap
                       : (oCollider.y + oCollider.height) + (halfSizes.y) + gap;

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
      framesAfterFallingOff++;
    }
  }
};

#endif