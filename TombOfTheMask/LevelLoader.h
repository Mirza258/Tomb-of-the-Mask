#pragma once

#include "Enemy.h"
#include "MapObjects.h"
#include <string>
#include <vector>

struct LevelData {
  std::string name;
  int width;
  int height;
  int playerSpawnX;
  int playerSpawnY;
  int theme;

  // 2D grid of MapObject
  std::vector<std::vector<MapObject *>> map;

  // List of enemies
  std::vector<Enemy *> enemies;

  LevelData()
      : width(0), height(0), playerSpawnX(1), playerSpawnY(1), theme(0) {}
};

namespace LevelLoader {
// Returns true on success
bool LoadLevel(const std::string &filename, LevelData &outData);

void GetSpriteCoords(int tileId, short &outSrcX, short &outSrcY);
} // namespace LevelLoader
