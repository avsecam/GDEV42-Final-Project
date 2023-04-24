#ifndef PROPERTIES
#define PROPERTIES

#include <raylib.h>

#include <fstream>
#include <iostream>

struct Properties {
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
  float camDrift;  // per-frame
};

Properties* LoadProperties(const char filename[], const int targetFps) {
  std::ifstream propertiesFile(filename);
	Properties* properties = new Properties;

  if (!propertiesFile) {
    std::cerr << "Unable to open properties file.";
    exit(1);
  }

  std::string input;
  while (std::getline(propertiesFile, input)) {
    int splitter = input.find(" ");
    std::string inputProperty = input.substr(0, splitter - 0);

    if (inputProperty == "CAM_EDGES"){
      std::string edges = input.substr(splitter+1, input.length());
      splitter = edges.find(" ");
      properties->camUpperLeft.x = stof(edges.substr(0, splitter));
      
      edges = edges.substr(splitter+1, edges.length());
      splitter = edges.find(" ");
      properties->camUpperLeft.y = stof(edges.substr(0, splitter));
      
      edges = edges.substr(splitter+1, edges.length()); 
      splitter = edges.find(" ");
      properties->camLowerRight.x = stof(edges.substr(0, splitter));

      edges = edges.substr(splitter+1, edges.length());
      properties->camLowerRight.y = stof(edges);

      continue;
    }
    
    float propertyValue = stof(input.substr(splitter, input.length()));

    if (inputProperty == "H_ACCEL") {
      properties->hAccel = propertyValue;
    } else if (inputProperty == "H_COEFF") {
      properties->hCoeff = propertyValue;
    } else if (inputProperty == "H_OPPOSITE") {
      properties->hOpposite = propertyValue;
    } else if (inputProperty == "H_AIR") {
      properties->hAir = propertyValue;
    } else if (inputProperty == "MIN_H_VEL") {
      properties->hVelMin = propertyValue;
    } else if (inputProperty == "MAX_H_VEL") {
      properties->hVelMax = propertyValue / targetFps;
    } else if (inputProperty == "GRAVITY") {
      properties->gravity = propertyValue / targetFps;
    } else if (inputProperty == "V_ACCEL") {
      properties->vAccel = propertyValue / targetFps;
    } else if (inputProperty == "V_HOLD") {
      properties->vHold = propertyValue;
    } else if (inputProperty == "V_SAFE") {
      properties->vSafe = propertyValue;
    } else if (inputProperty == "CUT_V_VEL") {
      properties->vVelCut = propertyValue / targetFps;
    } else if (inputProperty == "MAX_V_VEL") {
      properties->vVelMax = propertyValue / targetFps;
    } else if (inputProperty == "GAP") {
      properties->gap = propertyValue;
    } else if (inputProperty == "CAM_DRIFT") {
      properties->camDrift = propertyValue / targetFps;
    }
  }

  propertiesFile.close();

	return properties;
}

#endif