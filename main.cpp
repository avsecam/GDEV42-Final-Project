#include <raylib.h>
#include <raymath.h>

#include <fstream>
#include <iostream>
#include <vector>

const float WINDOW_WIDTH(800);
const float WINDOW_HEIGHT(600);
const char* WINDOW_TITLE("HAKENSLASH THE PLATFORMER");

const int TARGET_FPS(60);

const float PLAYER_WIDTH(24);
const float PLAYER_HEIGHT(32);
const Color PLAYER_COLOR(BLUE);

const Color OBSTACLE_COLOR(GRAY);

float hAccel;  // per-second
float hCoeff;
float hOpposite;
float hAir;
float hVelMin;  // per-frame
float hVelMax;  // per-second
float gravity;  // per-second
float vAccel;   // per-second
float vHold;
float vSafe;
float vVelCut;  // per-second
float vVelMax;  // per-second
float gap;

int camType;
Vector2 camUpperLeft;
Vector2 camLowerRight;
Vector2 cam1UpperLeft;
Vector2 cam1LowerRight;
float camDrift; // per-frame

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

struct Player : Entity {
  Vector2 velocity = Vector2Zero();
  Vector2 acceleration = {hAccel, vAccel};
  float airControlFactor = 1.0f;
  bool isGrounded = false;
  int jumpFrame = 0;
  int framesAfterFallingOff = 0;

  using Entity::Entity;

  void MoveHorizontal() {
    // Moving through air
    if (abs(velocity.y) > 0.0f) {
      airControlFactor = hAir;
    } else {
      airControlFactor = 1.0f;
    }
    if (IsKeyDown(KEY_A)) {
      if (velocity.x > 0.0f) {
        velocity.x -= acceleration.x * hOpposite * airControlFactor;
      } else {
        velocity.x -= acceleration.x * airControlFactor;
      }
      if (abs(velocity.x) >= hVelMax) {
        velocity.x = -hVelMax;
      }
    } else if (IsKeyDown(KEY_D)) {
      if (velocity.x < 0.0f) {
        velocity.x += acceleration.x * hOpposite * airControlFactor;
      } else {
        velocity.x += acceleration.x * airControlFactor;
      }
      if (abs(velocity.x) >= hVelMax) {
        velocity.x = hVelMax;
      }
    } else {
      velocity.x *= hCoeff;  // Slow down on no input
    }

    // Minimum horizontal movement threshold
    if (abs(velocity.x) <= hVelMin) {
      velocity.x = 0.0f;
    }
    position.x += velocity.x;
  }

  void MoveVertical() {
    // Jump handling
    if (IsKeyPressed(KEY_SPACE) && jumpFrame <= 0 && framesAfterFallingOff <= vSafe) {
      velocity.y += vAccel;
      jumpFrame++;
    } else if (IsKeyDown(KEY_SPACE) && velocity.y < 0) {  // In jump
      if (jumpFrame < vHold) {
        velocity.y += vAccel * ((vHold - jumpFrame) / vHold);
        jumpFrame++;
      } else {
        if (velocity.y < vVelCut) {
          velocity.y = vVelCut;
        }
      }
    }
    if (IsKeyReleased(KEY_SPACE)) {
      if (velocity.y < vVelCut) {
        velocity.y = vVelCut;
      }
    }

    velocity.y += gravity;
    velocity.y = Clamp(velocity.y, -INT32_MAX, vVelMax);

    position.y += velocity.y;
  }

  void CollideHorizontal(const std::vector<Entity*> obstacles) {
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

  void CollideVertical(const std::vector<Entity*> obstacles) {
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

struct Level {
  Player* player;
  std::vector<Entity*> obstacles;

  void Draw() {
    for (Entity* o : obstacles) {
      o->Draw();
    }
    player->Draw();
  }
};

void LoadProperties();
Level* LoadLevel(char filename[]);

int main(int argc, char* argv[]) {
  LoadProperties();
  Level* level = LoadLevel(argv[1]);
  Player* player = level->player;
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
  SetTargetFPS(TARGET_FPS);

  Camera2D cameraView = { 0 };
  cameraView.target = { player->position.x , player->position.y };
  cameraView.offset = { WINDOW_WIDTH/2, WINDOW_HEIGHT/2};
  cameraView.zoom = 1.0f;

  Vector2 cameraPos = { player->position.x, player->position.y };

  std::cout << "CamEdges " << camUpperLeft.x << " " << camUpperLeft.y << " " << camLowerRight.x << " " << camLowerRight.y << std::endl;
  std::cout << "CamType " << camType << std::endl;
  std::cout << "CamDrift " << camDrift << std::endl;
  while (!WindowShouldClose()) {
    float windowLeft = cameraView.target.x + camUpperLeft.x;
    float windowRight = cameraView.target.x + camLowerRight.x;
    float windowTop = cameraView.target.y + camUpperLeft.y;
    float windowBot = cameraView.target.y + camLowerRight.y;
    // Move horizontally
    player->MoveHorizontal();

    // Check collision, assume from left or right
    player->CollideHorizontal(level->obstacles);

    // Move vertically
    player->MoveVertical();

    // Check top collision if moving upwards, bottom if downwards
    player->CollideVertical(level->obstacles);

    if(IsKeyPressed(KEY_E)) {
      if (camType >= 4) {
        camType = 0;
      } else {
        camType += 1;
      }
    }

    if(camType == 0) {
      cameraView.target = {player->position.x, player->position.y};
    } 
    else if(camType == 1) {
      cameraView.target = {player->position.x, player->position.y};
      if(player->position.x < cam1UpperLeft.x){
        cameraView.target.x = cam1UpperLeft.x;
      } 
      if(player->position.x > cam1LowerRight.x){
        cameraView.target.x = cam1LowerRight.x;
      }
      if(player->position.y < cam1UpperLeft.y){
        cameraView.target.y = cam1UpperLeft.y;
      }
      if(player->position.y > cam1LowerRight.y){
        cameraView.target.y = cam1LowerRight.y;
      }
    } 
    else if(camType == 2) {
      float cameraPushX = 0.0f;
      float cameraPushY = 0.0f;

      if((player->position.x + player->halfSizes.x) > windowRight){
        cameraPushX = (player->position.x + player->halfSizes.x) - windowRight;
        //std::cout << "CAM PUSHING RIGHT" << std::endl;
      } else if((player->position.x - player->halfSizes.x) < windowLeft){
        cameraPushX = (player->position.x - player->halfSizes.x) - windowLeft;
        //std::cout << "CAM PUSHING LEFT" << std::endl;
      } 
      if((player->position.y + player->halfSizes.y) > windowBot){
        cameraPushY = (player->position.y + player->halfSizes.y) - windowBot;
        //std::cout << "CAM PUSHING BOT" << std::endl;
      } else if((player->position.y - player->halfSizes.y) < windowTop){
        cameraPushY = (player->position.y - player->halfSizes.y) - windowTop;
        //std::cout << "CAM PUSHING TOP" << std::endl;
      }
      cameraView.target.x += cameraPushX;
      cameraView.target.y += cameraPushY;
    } 
    else if(camType == 3) {
      float cameraPushX = 0.0f;
      float cameraPushY = 0.0f;
      float driftX = Clamp(player->position.x - (windowLeft+windowRight)/2, -camDrift, camDrift);
      float driftY = Clamp(player->position.y - (windowTop+windowBot)/2, -camDrift, camDrift);

      if((player->position.x + player->halfSizes.x) > windowRight){
        cameraPushX = (player->position.x + player->halfSizes.x) - windowRight;
        //std::cout << "CAM PUSHING RIGHT" << std::endl;
        cameraView.target.x += cameraPushX;
      } else if((player->position.x - player->halfSizes.x) < windowLeft){
        cameraPushX = (player->position.x - player->halfSizes.x) - windowLeft;
        //std::cout << "CAM PUSHING LEFT" << std::endl;
        cameraView.target.x += cameraPushX;
      } else{
        cameraView.target.x += driftX;
        //std::cout << "DRIFTING HORIZONTALLY" << std::endl;
      } 
      if((player->position.y + player->halfSizes.y) > windowBot){
        cameraPushY = (player->position.y + player->halfSizes.y) - windowBot;
        //std::cout << "CAM PUSHING BOT" << std::endl;
        cameraView.target.y += cameraPushY;
      } else if((player->position.y - player->halfSizes.y) < windowTop){
        cameraPushY = (player->position.y - player->halfSizes.y) - windowTop;
        //std::cout << "CAM PUSHING TOP" << std::endl;
        cameraView.target.y += cameraPushY;
      } else {
        cameraView.target.y += driftY;
        //std::cout << "DRIFTING VERTICALLY" << std::endl;
      }
    }
    else if(camType == 4) {
      float cameraPushX = 0.0f;
      float cameraPushY = 0.0f;
      float driftY = Clamp(player->position.y - (windowTop+windowBot)/2, -camDrift, camDrift);

      if((player->position.x + player->halfSizes.x) > windowRight){
        cameraPushX = (player->position.x + player->halfSizes.x) - windowRight;
        //std::cout << "CAM PUSHING RIGHT" << std::endl;
      } else if((player->position.x - player->halfSizes.x) < windowLeft){
        cameraPushX = (player->position.x - player->halfSizes.x) - windowLeft;
        //std::cout << "CAM PUSHING LEFT" << std::endl;
      } 
      cameraView.target.x += cameraPushX;
      if(player->isGrounded) {
        if((player->position.y + player->halfSizes.y) > windowBot){
          cameraPushY = (player->position.y + player->halfSizes.y) - windowBot;
          //std::cout << "CAM PUSHING BOT" << std::endl;
          cameraView.target.y += cameraPushY;
        } else if((player->position.y - player->halfSizes.y) < windowTop){
          cameraPushY = (player->position.y - player->halfSizes.y) - windowTop;
          //std::cout << "CAM PUSHING TOP" << std::endl;
          cameraView.target.y += cameraPushY;
        } else {
          cameraView.target.y += driftY;
          //std::cout << "DRIFTING VERTICALLY" << std::endl;
        }
      }
    }
    
    BeginDrawing();
    BeginMode2D(cameraView);
    ClearBackground(WHITE);

    level->Draw();
    if(camType == 2 || camType == 3 || camType == 4){
      DrawRectangleLines(windowLeft, windowTop, windowRight-windowLeft, windowBot-windowTop, RED);
    }

    EndDrawing();
  }

  CloseWindow();

  // Delete pointers
  delete player;
  for (Entity* e : level->obstacles) {
    delete e;
  }
  delete level;

  return 0;
}

void LoadProperties() {
  std::ifstream propertiesFile("properties.cfg");

  if (!propertiesFile) {
    std::cerr << "Unable to open properties file.";
    exit(1);
  }

  std::string input;
  while (std::getline(propertiesFile, input)) {
    int splitter = input.find(" ");
    std::string inputProperty = input.substr(0, splitter - 0);
    std::cout << inputProperty << std::endl;

    if (inputProperty == "CAM_EDGES"){
      std::string edges = input.substr(splitter+1, input.length());
      splitter = edges.find(" ");
      camUpperLeft.x = stof(edges.substr(0, splitter));
      
      edges = edges.substr(splitter+1, edges.length());
      splitter = edges.find(" ");
      camUpperLeft.y = stof(edges.substr(0, splitter));
      
      edges = edges.substr(splitter+1, edges.length()); 
      splitter = edges.find(" ");
      camLowerRight.x = stof(edges.substr(0, splitter));

      edges = edges.substr(splitter+1, edges.length());
      camLowerRight.y = stof(edges);

      continue;
    }

    if (inputProperty == "CAM_1_EDGES"){
      std::string edges = input.substr(splitter+1, input.length());
      splitter = edges.find(" ");
      cam1UpperLeft.x = stof(edges.substr(0, splitter));
      
      edges = edges.substr(splitter+1, edges.length());
      splitter = edges.find(" ");
      cam1UpperLeft.y = stof(edges.substr(0, splitter));
      
      edges = edges.substr(splitter+1, edges.length()); 
      splitter = edges.find(" ");
      cam1LowerRight.x = stof(edges.substr(0, splitter));

      edges = edges.substr(splitter+1, edges.length());
      cam1LowerRight.y = stof(edges);

      continue;
    }
    
    float propertyValue = stof(input.substr(splitter, input.length()));

    if (inputProperty == "H_ACCEL") {
      hAccel = propertyValue;
    } else if (inputProperty == "H_COEFF") {
      hCoeff = propertyValue;
    } else if (inputProperty == "H_OPPOSITE") {
      hOpposite = propertyValue;
    } else if (inputProperty == "H_AIR") {
      hAir = propertyValue;
    } else if (inputProperty == "MIN_H_VEL") {
      hVelMin = propertyValue;
    } else if (inputProperty == "MAX_H_VEL") {
      hVelMax = propertyValue / TARGET_FPS;
    } else if (inputProperty == "GRAVITY") {
      gravity = propertyValue / TARGET_FPS;
    } else if (inputProperty == "V_ACCEL") {
      vAccel = propertyValue / TARGET_FPS;
    } else if (inputProperty == "V_HOLD") {
      vHold = propertyValue;
    } else if (inputProperty == "V_SAFE") {
      vSafe = propertyValue;
    } else if (inputProperty == "CUT_V_VEL") {
      vVelCut = propertyValue / TARGET_FPS;
    } else if (inputProperty == "MAX_V_VEL") {
      vVelMax = propertyValue / TARGET_FPS;
    } else if (inputProperty == "GAP") {
      gap = propertyValue;
    } else if (inputProperty == "CAM_TYPE") {
      camType = static_cast<int>(propertyValue);
    } else if (inputProperty == "CAM_DRIFT") {
      camDrift = propertyValue / TARGET_FPS;
    }
  }

  propertiesFile.close();
}

Level* LoadLevel(char filename[]) {
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
  levelFile >> obstacleCount;
  for (int i = 0; i < obstacleCount; ++i) {
    Vector2 oPosition;
    Vector2 oHalfSizes;
    levelFile >> oPosition.x >> oPosition.y;
    levelFile >> oHalfSizes.x >> oHalfSizes.y;
    Entity* o = new Entity(oPosition, oHalfSizes, OBSTACLE_COLOR);
    level->obstacles.push_back(o);
  }

  levelFile.close();

  return level;
}