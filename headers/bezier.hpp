#ifndef BEZIER
#define BEZIER

#include <raylib.h>
#include <raymath.h>

#include <cstdio>
#include <vector>

const std::vector<std::vector<int>> BASE_PASCALS_TRIANGLE = {{1}, {1, 1}};
std::vector<std::vector<int>> pascalsTriangle = BASE_PASCALS_TRIANGLE;

const int NUMBER_OF_STEPS = 50;

std::vector<std::vector<int>> GeneratePascalsTriangle(const int depth) {
  std::vector<std::vector<int>> newPascalsTriangle = BASE_PASCALS_TRIANGLE;

  for (size_t i = 2; i <= depth; ++i) {
    std::vector<int> row;
    row.push_back(1);
    for (size_t j = 1; j < i; ++j) {
      row.push_back(
        (newPascalsTriangle[i - 1][j - 1] + newPascalsTriangle[i - 1][j])
      );
    }
    row.push_back(1);
    newPascalsTriangle.push_back(row);
  }

  for (size_t i = 0; i <= depth; ++i) {
    for (size_t j = 0; j <= newPascalsTriangle[i].size() - 1; ++j) {
      printf("%d ", newPascalsTriangle[i][j]);
    }
    printf("\n");
  }

  return newPascalsTriangle;
}

bool ValidateControlPointCount(const int order, const int numberOfPoints) {
	return !(numberOfPoints <= order || (numberOfPoints - 1) % order != 0);
}

Vector2 GetPointInCurve(
  const std::vector<Vector2> points, const float distance,
  const std::vector<int> PTCoefficients
) {
  Vector2 outputPoint = {0, 0};
  int n = points.size() - 1;

  for (int i = 0; i < points.size(); ++i) {
    outputPoint.x += PTCoefficients[i] * points[i].x *
                     pow(1 - distance, n - i) * pow(distance, i);
    outputPoint.y += PTCoefficients[i] * points[i].y *
                     pow(1 - distance, n - i) * pow(distance, i);
  }

  return outputPoint;
}

struct BezierCurve {
  std::vector<Vector2> points;
  std::vector<Vector2> stepList;

  void Draw() {
    for (size_t i = 0; i < stepList.size() - 1; i++) {
      DrawLineEx(stepList[i], stepList[i + 1], 1, WHITE);
    }
  }

  void CalculateCurve() {
    if (!stepList.empty()) stepList.clear();
		
    for (int i = 0; i < NUMBER_OF_STEPS; ++i) {
      Vector2 stepPoint;

      stepPoint = GetPointInCurve(
        points, (float)i / NUMBER_OF_STEPS, pascalsTriangle[points.size() - 1]
      );
      stepList.push_back(stepPoint);
    }
    stepList.push_back(points[points.size() - 1]);
  }

  Vector2 GetStartPoint() { return stepList[0]; }

  Vector2 GetEndPoint() { return stepList[stepList.size() - 1]; }
};

#endif