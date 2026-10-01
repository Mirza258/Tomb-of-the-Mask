#pragma once

#include <windows.h>

#include "Constants.h"

enum EnemyType { ENEMY_BAT, ENEMY_ARROW };

class Enemy {
public:
  EnemyType enemyType;

  // Grid position
  short gridX, gridY;

  // Pixel position
  int pixelX, pixelY;

  // Sprite sheet
  short srcX, srcY;

  // Movement speed
  int moveSpeed;

  Enemy(short x, short y, short sx = 0, short sy = 0);
  virtual ~Enemy() = default;

  virtual void Update() = 0;

  virtual bool IsDangerous() const { return true; }

  virtual bool BlocksMovementAt(int x, int y) const { return false; }

  short GetSrcX() const { return srcX; }
  short GetSrcY() const { return srcY; }
};

enum PatrolDirection { PATROL_HORIZONTAL, PATROL_VERTICAL };

// Sprite sheet: Row 5 (index 4) = moving right, Row 6 (index 5) = moving left
class BatEnemy : public Enemy {
public:
  // Patrol parameters
  PatrolDirection patrolDir;
  int patrolStart; // Start position (grid coordinate)
  int patrolEnd;   // End position (grid coordinate)
  int moveDir;     // Current movement direction: 1 or -1

  int animFrame;
  int animCounter;
  int waitTimer; // Frames to pause at patrol ends

  // patrolLength =  (positive = right/down, negative = left/up)
  BatEnemy(short startX, short startY, PatrolDirection dir, int patrolLength);

  void Update() override;

  void UpdateSprite();
};

class ArrowEnemy : public Enemy {
public:
  int direction; // 0=up, 1=down, 2=left, 3=right
  int dirX, dirY;
  bool firstFrame;

  ArrowEnemy(short startX, short startY, int dir);

  void Update() override;

  bool IsDangerous() const override { return true; }
};
