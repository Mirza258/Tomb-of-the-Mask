#include "Enemy.h"

// ==================== Enemy Base Class ====================

Enemy::Enemy(short x, short y, short sx, short sy)
    : enemyType(ENEMY_BAT), gridX(x), gridY(y), pixelX(x * TILE_SIZE),
      pixelY(y * TILE_SIZE), srcX(sx), srcY(sy), moveSpeed(2) {}

// ==================== BatEnemy ====================

BatEnemy::BatEnemy(short startX, short startY, PatrolDirection dir,
                   int patrolLength)
    : Enemy(startX, startY, 0, 3), patrolDir(dir),
      moveDir(patrolLength > 0 ? 1 : -1), animFrame(0), animCounter(0),
      waitTimer(0) {
  // Set patrol range
  if (dir == PATROL_HORIZONTAL) {
    patrolStart = (patrolLength > 0) ? startX : startX + patrolLength;
    patrolEnd = (patrolLength > 0) ? startX + patrolLength : startX;
  } else {
    patrolStart = (patrolLength > 0) ? startY : startY + patrolLength;
    patrolEnd = (patrolLength > 0) ? startY + patrolLength : startY;
  }

  UpdateSprite();
}

void BatEnemy::Update() {
  animCounter++;
  if (animCounter >= 4) {
    animFrame = (animFrame + 1) % 4;
    animCounter = 0;
  }

  // Pause at patrol end
  if (waitTimer > 0) {
    waitTimer--;
    UpdateSprite();
    return;
  }

  if (patrolDir == PATROL_HORIZONTAL) {
    pixelX += moveDir * moveSpeed;
    gridX = (short)(pixelX / TILE_SIZE);

    int patrolEndPixel = patrolEnd * TILE_SIZE;
    int patrolStartPixel = patrolStart * TILE_SIZE;

    if (moveDir > 0 && pixelX >= patrolEndPixel) {
      pixelX = patrolEndPixel;
      gridX = (short)patrolEnd;
      moveDir = -1;
      waitTimer = 30;
    } else if (moveDir < 0 && pixelX <= patrolStartPixel) {
      pixelX = patrolStartPixel;
      gridX = (short)patrolStart;
      moveDir = 1;
      waitTimer = 30;
    }
  } else { // PATROL_VERTICAL
    pixelY += moveDir * moveSpeed;
    gridY = (short)(pixelY / TILE_SIZE);

    int patrolEndPixel = patrolEnd * TILE_SIZE;
    int patrolStartPixel = patrolStart * TILE_SIZE;

    if (moveDir > 0 && pixelY >= patrolEndPixel) {
      pixelY = patrolEndPixel;
      gridY = (short)patrolEnd;
      moveDir = -1;
      waitTimer = 30;
    } else if (moveDir < 0 && pixelY <= patrolStartPixel) {
      pixelY = patrolStartPixel;
      gridY = (short)patrolStart;
      moveDir = 1;
      waitTimer = 30;
    }
  }

  UpdateSprite();
}

void BatEnemy::UpdateSprite() {
  srcX = (short)animFrame;

  // Row 3 = moving right
  // Row 4 = moving left
  if (patrolDir == PATROL_HORIZONTAL) {
    srcY = (moveDir > 0) ? 3 : 4;
  } else {
    srcY = 3; // Vertical always uses right row
  }
}

// ==================== ArrowEnemy ====================

ArrowEnemy::ArrowEnemy(short startX, short startY, int dir)
    : Enemy(startX, startY, (short)dir, 0), direction(dir), dirX(0), dirY(0),
      firstFrame(true) {
  enemyType = ENEMY_ARROW;
  moveSpeed = 8;

  switch (direction) {
  case 0:
    dirY = -1;
    break; // Up
  case 1:
    dirY = 1;
    break; // Down
  case 2:
    dirX = -1;
    break; // Left
  case 3:
    dirX = 1;
    break; // Right
  }
}

void ArrowEnemy::Update() {
  if (firstFrame) {
    firstFrame = false;
    return;
  }
  pixelX += dirX * moveSpeed;
  pixelY += dirY * moveSpeed;
  gridX = (short)(pixelX / TILE_SIZE);
  gridY = (short)(pixelY / TILE_SIZE);
}
