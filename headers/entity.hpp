#ifndef ENTITY
#define ENTITY

#include <raylib.h>
#include <raymath.h>

#include <vector>

#include "bezier.hpp"
#include "properties.hpp"

const float PLAYER_WIDTH(24);
const float PLAYER_HEIGHT(48);
const Color PLAYER_COLOR(BLUE);
const int MAX_PLAYER_HEALTH(10);

const Color MELEE_ENEMY_COLOR(RED);
const Color RANGED_ENEMY_COLOR(ORANGE);

const float BULLET_HALF_SIZE(5);
const float BULLET_SPEED(300.0f);
const Color BULLET_COLOR(MAGENTA);

const Color STATIC_OBSTACLE_COLOR(DARKPURPLE);
const Color MOVING_OBSTACLE_COLOR(PURPLE);

enum ObstacleType { STATIC, MOVING };
enum Heading { LEFT, RIGHT };

// All entity positions are assumed to be indicated by their centers, not
// upper-lefts

struct Entity {
  Vector2 position;
  Vector2 halfSizes;
  Color color;

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
    return CheckCollisionRecs(rec, GetCollider());
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
  Vector2 velocity;
  int health;

  Character(
    Vector2 _position, Vector2 _halfSizes, Color _color = MELEE_ENEMY_COLOR
  ) {
    this->position = _position;
    this->halfSizes = _halfSizes;
    this->color = _color;
    this->velocity = Vector2Zero();
  }

  virtual void MoveHorizontal(const Properties* properties) = 0;
  virtual void MoveVertical(const Properties* properties) = 0;
  virtual void CollideHorizontal(
    const std::vector<Obstacle*> obstacles, const float gap
  ) = 0;
  virtual void CollideVertical(
    const std::vector<Obstacle*> obstacles, const float gap
  ) = 0;

  void kill() {
    if (position.y < 400) {
      position.y = 600;
      position.x = rand() % 700 + 100;
    } else {
      position.y = 200;
      position.x = rand() % 700 + 100;
    }
  }

 protected:
  void HandleGravity(const Properties* properties) {
    velocity.y += properties->gravity;
  }

  void LimitVerticalVelocity(const Properties* properties) {
    velocity.y = Clamp(velocity.y, -INT32_MAX, properties->vVelMax);
  }

  void ApplyVerticalVelocity() { position.y += velocity.y; }
};

struct Player : public Character {
  float airControlFactor = 1.0f;
  bool isGrounded = false;
  int jumpFrame = 0;
  int framesAfterFallingOff = 0;
  int kills = 0;
  int killsThreshold = 0;
  std::string facingDirection = "right";

  Player(
    Vector2 _position, Vector2 _halfSizes, int _health = MAX_PLAYER_HEALTH,
    Color _color = PLAYER_COLOR
  )
      : Character(_position, _halfSizes, _color) {
    this->health = MAX_PLAYER_HEALTH;
  }

  void MoveHorizontal(const Properties* properties) {
    // Moving through air
    if (abs(velocity.y) > 0.0f) {
      airControlFactor = properties->hAir;
    } else {
      airControlFactor = 1.0f;
    }

    if (IsKeyDown(KEY_A)) {
      facingDirection = "left";
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
      facingDirection = "right";
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
    if (IsKeyPressed(KEY_SPACE) && jumpFrame <= 0 && framesAfterFallingOff <= properties->vSafe)
    {
      velocity.y = properties->vAccel;
      ++jumpFrame;
    } else if (IsKeyDown(KEY_SPACE) && velocity.y < 0) {  // In jump
      if (jumpFrame < properties->vHold) {
        velocity.y = properties->vAccel *
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

    HandleGravity(properties);
    LimitVerticalVelocity(properties);
    ApplyVerticalVelocity();
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

struct Bullet : public Entity {
  Vector2 direction;
  float speed;

  Bullet(
    Vector2 _position, Vector2 _direction, float _speed = BULLET_SPEED,
    Vector2 _halfSizes = {BULLET_HALF_SIZE, BULLET_HALF_SIZE},
    Color _color = BULLET_COLOR
  )
      : Entity(_position, _halfSizes, _color) {
    this->direction = _direction;
    this->speed = _speed;
  }

  void Draw() { DrawCircleV(position, halfSizes.x, color); }

  void Update(const float timestep) {
    position = Vector2Add(
      position, Vector2Scale(Vector2Normalize(direction), speed * timestep)
    );
  }

  bool CollidePlayer(Player* player) {
    return IsIntersecting(player->GetCollider());
  }

  bool IsOutsideLimits(const Rectangle limits) {
    return !CheckCollisionPointRec(position, limits);
  }
};

struct Item : public Entity {
  const float TIMER_ADD = 5.0f;

  using Entity::Entity;

  bool Update(Player* player, float timer) {
    if (IsIntersecting(player->GetCollider())) {
      player->health += 5;
      return true;
    }
    return false;
  }

  void Draw(Texture texture) {
    DrawCircleV(position, 15, GREEN);
    DrawTextureV(
      texture, Vector2Subtract(position, Vector2Scale(halfSizes, 0.5)), WHITE
    );
  }
};

struct PlayerWeapon : public Entity {
  // if(IsIntersecting(MeleeEnemy menemy))
  PlayerWeapon(Vector2 _position, Vector2 _halfSizes, Color _color = GREEN) {
    this->position = _position;
    this->halfSizes = _halfSizes;
    this->color = _color;
  }

  void Update(Player* player, std::vector<Bullet*> bullets) {
    if (player->facingDirection == "left") {
      position.x = player->position.x - 50;
    } else {
      position.x = player->position.x + 50;
    }
    position.y = player->position.y;
  }
};

#endif