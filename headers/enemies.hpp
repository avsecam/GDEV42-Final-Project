#ifndef ENEMIES
#define ENEMIES

#include "entity.hpp"

struct RangedEnemy : public Character
{
  Heading heading = Heading::LEFT;

  using Character::Character;

  void Update(
      const Properties *properties, const std::vector<Obstacle *> obstacles)
  {
    MoveHorizontal(properties);
    CollideHorizontal(obstacles, properties->gap);
    MoveVertical(properties);
    CollideVertical(obstacles, properties->gap);
  }

private:
  void MoveHorizontal(const Properties *properties)
  {
    if (heading == Heading::LEFT)
    {
      if (velocity.x > 0.0f)
      {
        velocity.x -= properties->hAccel * properties->hOpposite;
      }
      else
      {
        velocity.x -= properties->hAccel;
      }
      if (abs(velocity.x) >= properties->hVelMax)
      {
        velocity.x = -properties->hVelMax;
      }
    }
    else if (heading == Heading::RIGHT)
    {
      if (velocity.x < 0.0f)
      {
        velocity.x += properties->hAccel * properties->hOpposite;
      }
      else
      {
        velocity.x += properties->hAccel;
      }
      if (abs(velocity.x) >= properties->hVelMax)
      {
        velocity.x = properties->hVelMax;
      }
    }
    else
    {
      velocity.x *= properties->hCoeff; // Slow down on no input
    }

    // Minimum horizontal movement threshold
    if (abs(velocity.x) <= properties->hVelMin)
    {
      velocity.x = 0.0f;
    }
    position.x += velocity.x;
  }

  void MoveVertical(const Properties *properties)
  {
    HandleGravity(properties);
    LimitVerticalVelocity(properties);
    ApplyVerticalVelocity();
  }

  void CollideHorizontal(
      const std::vector<Obstacle *> obstacles, const float gap)
  {
    // Ledge check, don't fall!
    Obstacle *oLeft = nullptr;
    Obstacle *oRight = nullptr;
    for (Obstacle *o : obstacles)
    {
      if (o->IsIntersecting(GetBottomLeftCollider()))
      {
        oLeft = o;
        break;
      }
    }
    if (oLeft)
    {
      for (Obstacle *o : obstacles)
      {
        if (o->IsIntersecting(GetBottomRightCollider()))
        {
          oRight = o;
          break;
        }
      }
    }

    if (!oLeft || !oRight)
    {
      heading = heading == Heading::LEFT ? Heading::RIGHT : Heading::LEFT;
      return;
    }

    // Collide with walls
    for (Obstacle *o : obstacles)
    {
      Rectangle oCollider = o->GetCollider();
      if (IsIntersecting(oCollider))
      {
        // Move back
        if (o->type == ObstacleType::STATIC)
        {
          position.x = velocity.x > 0 ? oCollider.x - (halfSizes.x) - gap
                                      : (oCollider.x + oCollider.width) +
                                            (halfSizes.x) + gap;
        }
        else
        {
          position.x = velocity.x > 0 ? position.x - gap : position.x + gap;
        }

        heading = heading == Heading::LEFT ? Heading::RIGHT : Heading::LEFT;
        break;
      }
    }
  }

  void CollideVertical(
      const std::vector<Obstacle *> obstacles, const float gap)
  {
    for (Obstacle *o : obstacles)
    {
      Rectangle oCollider = o->GetCollider();
      if (IsIntersecting(oCollider))
      {
        // Move back
        if (o->type == ObstacleType::STATIC)
        {
          position.y = velocity.y > 0 ? oCollider.y - (halfSizes.y) - gap
                                      : (oCollider.y + oCollider.height) +
                                            (halfSizes.y) + gap;
        }
        else
        {
          position.y = oCollider.y - (halfSizes.y);
        }

        if (velocity.y >= 0)
        { // Grounded
          velocity.y = 0;
        }
        else
        { // Na-untog
          velocity.y = -velocity.y;
        }

        break;
      }
    }
  }

  Rectangle GetBottomLeftCollider()
  {
    return {
        this->position.x - this->halfSizes.x - 10,
        this->position.y + this->halfSizes.y, 10, 10};
  }

  Rectangle GetBottomRightCollider()
  {
    return {
        this->position.x + this->halfSizes.x,
        this->position.y + this->halfSizes.y, 10, 10};
  }
};

struct MeleeEnemy : public Character
{
  bool isMovingLeft = true;
  bool isMovingRight = false;
  bool isJumping = false;
  int jumpFrame = 0;
  int JUMP_CHANCE = 95;
  bool isFollowingPlayer = false;
  using Character::Character;

  void Update(
      const Properties *properties, const std::vector<Obstacle *> obstacles, Player *player)
  {
    findPlayer(player);
    checkIfJump();
    MoveHorizontal(properties);
    CollideHorizontal(obstacles, properties->gap);
    MoveVertical(properties);
    CollideVertical(obstacles, properties->gap);
    CollidePlayer(player);
  }

  void findPlayer(Player *p)
  {
    if (p->position.y < position.y+10 && p->position.y > position.y-10)
    {
      isFollowingPlayer = true;
      std::cout << "FOLLOWING" << std::endl;
      if (p->position.x > position.x)
      {
        isMovingLeft = false;
        isMovingRight = true;
      }
      else if (p->position.x < position.x)
      {
        isMovingLeft = true;
        isMovingRight = false;
      }
    }
    else{
      isFollowingPlayer = false;
      std::cout << "NOT FOLLOWING"<< std::endl;
    }
  }

  void checkIfJump()
  {
    int rng_num;
    if (isJumping == false && !isFollowingPlayer)
    {
      rng_num = rand() % 100;
      if (rng_num > JUMP_CHANCE)
      {
        isJumping = true;
        std::cout << "GONNA JUMP" << std::endl;
      }
    }
  }

  int randomizeMoveTimer()
  {
    int rng_num;
    rng_num = rand() % 200;
    return rng_num + 100;
  }

  void MoveHorizontal(const Properties *properties)
  {
    if (isMovingLeft)
    {
      if (velocity.x > 0.0f)
      {
        velocity.x -= properties->hAccel * properties->hOpposite;
      }
      else
      {
        velocity.x -= properties->hAccel;
      }
      if (abs(velocity.x) >= (properties->hVelMax * 0.8))
      {
        velocity.x = -(properties->hVelMax * 0.8);
      }
    }
    else if (isMovingRight)
    {
      if (velocity.x < 0.0f)
      {
        velocity.x += properties->hAccel * properties->hOpposite;
      }
      else
      {
        velocity.x += properties->hAccel;
      }
      if (abs(velocity.x) >= (properties->hVelMax * 0.8))
      {
        velocity.x = (properties->hVelMax * 0.8);
      }
    }
    else
    {
      velocity.x *= properties->hCoeff; // Slow down
    }

    position.x += velocity.x;
  }

  void MoveVertical(const Properties *properties)
  {
    if (isJumping && jumpFrame == 0)
    {
      velocity.y += properties->vAccel;
      ++jumpFrame;
    }
    else if (isJumping && velocity.y < 0)
    {
      if (jumpFrame < properties->vHold)
      {
        velocity.y += properties->vAccel * ((properties->vHold - jumpFrame) / properties->vHold);
        ++jumpFrame;
      }
      else
      {
        if (velocity.y < properties->vVelCut)
        {
          velocity.y = properties->vVelCut;
        }
      }
    }

    HandleGravity(properties);
    LimitVerticalVelocity(properties);
    ApplyVerticalVelocity();
  }

  void CollideHorizontal(
      const std::vector<Obstacle *> obstacles, const float gap)
  {

    // Collide with walls
    for (Obstacle *o : obstacles)
    {
      Rectangle oCollider = o->GetCollider();
      if (IsIntersecting(oCollider))
      {
        // Move back
        if (o->type == ObstacleType::STATIC)
        {
          position.x = velocity.x > 0 ? oCollider.x - (halfSizes.x) - gap
                                      : (oCollider.x + oCollider.width) +
                                            (halfSizes.x) + gap;
        }
        else
        {
          position.x = velocity.x > 0 ? position.x - gap : position.x + gap;
        }

        if (isMovingLeft)
        {
          isMovingRight = true;
          isMovingLeft = false;
        }
        else
        {
          isMovingLeft = true;
          isMovingRight = false;
        }
        break;
      }
    }
  }

  void CollideVertical(
      const std::vector<Obstacle *> obstacles, const float gap)
  {
    for (Obstacle *o : obstacles)
    {
      Rectangle oCollider = o->GetCollider();
      if (IsIntersecting(oCollider))
      {
        // Move back
        if (o->type == ObstacleType::STATIC)
        {
          position.y = velocity.y > 0 ? oCollider.y - (halfSizes.y) - gap
                                      : (oCollider.y + oCollider.height) +
                                            (halfSizes.y) + gap;
        }
        else
        {
          position.y = oCollider.y - (halfSizes.y);
        }

        if (velocity.y >= 0)
        { // Grounded
          velocity.y = 0;
          jumpFrame = 0;
          isJumping = false;
        }
        else
        { // Na-untog
          velocity.y = -velocity.y;
        }

        break;
      }
    }
  }

  void CollidePlayer(Player* p){
    Rectangle playerCollider = p->GetCollider();
    if (IsIntersecting(playerCollider))
    {
      p->health -= 1;
      std::cout<<p->health<<std::endl;
      //delete this;
    }
  }
};

#endif