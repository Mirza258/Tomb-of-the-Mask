#include "Player.h"

Player::Player(int startX, int startY)
    : x(startX), y(startY), pixelX(startX * TILE_SIZE),
      pixelY(startY * TILE_SIZE), targetX(startX), targetY(startY), moveDirX(0),
      moveDirY(0), isMoving(false), stepCount(0), animFrame(0), animCounter(0),
      isDying(false), deathAnimFrame(0), deathAnimCounter(0), facing(1),
      surface(SURFACE_FLOOR) {}

bool Player::CanMoveTo(int checkX, int checkY) const {
  if (canMoveCallback) {
    return canMoveCallback(checkX, checkY);
  }
  return false;
}

bool Player::StartMove(int dx, int dy) {
  if (isMoving)
    return false;
  if (dx == 0 && dy == 0)
    return false;

  // Check if we can move to the next tile
  int nextX = x + dx;
  int nextY = y + dy;

  if (!CanMoveTo(nextX, nextY)) {
    return false; // Blocked
  }

  // Set target to next tile
  targetX = nextX;
  targetY = nextY;
  moveDirX = dx;
  moveDirY = dy;
  isMoving = true;
  stepCount = 0; // Start new movement cycle

  // Update facing direction for horizontal movement
  if (dx != 0) {
    facing = dx; // 1 for right, -1 for left
  }
  return true;
}

bool Player::UpdateMovement() {
  if (!isMoving)
    return false;

  int targetPixelX = targetX * TILE_SIZE;
  int targetPixelY = targetY * TILE_SIZE;

  if (pixelX < targetPixelX) {
    pixelX += MOVE_SPEED;
    if (pixelX > targetPixelX)
      pixelX = targetPixelX;
  } else if (pixelX > targetPixelX) {
    pixelX -= MOVE_SPEED;
    if (pixelX < targetPixelX)
      pixelX = targetPixelX;
  }

  if (pixelY < targetPixelY) {
    pixelY += MOVE_SPEED;
    if (pixelY > targetPixelY)
      pixelY = targetPixelY;
  } else if (pixelY > targetPixelY) {
    pixelY -= MOVE_SPEED;
    if (pixelY < targetPixelY)
      pixelY = targetPixelY;
  }

  // Update grid position
  x = pixelX / TILE_SIZE;
  y = pixelY / TILE_SIZE;

  // Check if reached current tile target
  if (pixelX == targetPixelX && pixelY == targetPixelY) {
    stepCount++;

    // Try to continue movement
    if (ContinueMove()) {
      return true; // Still moving
    }

    // Movement stopped
    isMoving = false;
    DetermineSurfaceState();
    return false;
  }

  return true;
}

bool Player::ContinueMove() {
  // Check if we can move to the next tile
  int nextX = x + moveDirX;
  int nextY = y + moveDirY;

  if (!CanMoveTo(nextX, nextY)) {
    return false; // Blocked
  }

  // Continue to next tile
  targetX = nextX;
  targetY = nextY;
  return true;
}

void Player::UpdateAnimation() {
  animCounter++;
  if (animCounter >= 6) {
    animFrame = (animFrame + 1) % 4;
    animCounter = 0;
  }
}

void Player::TrySwitchSurface(int dx, int dy) {
  if (CanMoveTo(x + dx, y + dy)) {
    return; // Not blocked
  }

  SurfaceState newSurface = surface;

  // Determine which wall they are pressing into
  if (dx > 0) {
    newSurface = SURFACE_RIGHT_WALL;
    facing = 1;
  } else if (dx < 0) {
    newSurface = SURFACE_LEFT_WALL;
    facing = 1;
  } else if (dy > 0) {
    newSurface = SURFACE_FLOOR;
  } else if (dy < 0) {
    newSurface = SURFACE_CEILING;
  }

  surface = newSurface;
}

void Player::DetermineSurfaceState() {
  // Check which wall the player is touching based on movement direction
  if (moveDirX != 0) {
    // Horizontal movement
    if (moveDirX > 0 && !CanMoveTo(x + 1, y)) {
      surface = SURFACE_RIGHT_WALL;
      return;
    }
    if (moveDirX < 0 && !CanMoveTo(x - 1, y)) {
      surface = SURFACE_LEFT_WALL;
      return;
    }
  } else if (moveDirY != 0) {
    // Vertical movement
    if (moveDirY > 0 && !CanMoveTo(x, y + 1)) {
      surface = SURFACE_FLOOR;
      return;
    }
    if (moveDirY < 0 && !CanMoveTo(x, y - 1)) {
      surface = SURFACE_CEILING;
      return;
    }
  }

  // Default to floor
  surface = SURFACE_FLOOR;
}

int Player::GetSpriteRow() const {
  if (isMoving) {
    return 8;
  }

  // Idle animation
  switch (surface) {
  case SURFACE_FLOOR:
    return (facing == 1) ? 0 : 1;
  case SURFACE_CEILING:
    return (facing == 1) ? 2 : 3;
  case SURFACE_LEFT_WALL:
    return (facing == 1) ? 4 : 5;
  case SURFACE_RIGHT_WALL:
    return (facing == 1) ? 6 : 7;
  default:
    return 0;
  }
}

int Player::GetSpriteColumn() const {
  if (isMoving) {
    // Column 0: horizontal, Column 1: vertical
    return (moveDirX != 0) ? 0 : 1;
  }
  return animFrame;
}

void Player::StartDeathAnimation() {
  isDying = true;
  isMoving = false;
  deathAnimFrame = 0;
  deathAnimCounter = 0;
}

bool Player::UpdateDeathAnimation() {
  if (!isDying)
    return true;

  deathAnimCounter++;
  if (deathAnimCounter >= 3) {
    deathAnimFrame++;
    deathAnimCounter = 0;
    if (deathAnimFrame >= 7) {
      deathAnimFrame = 6; // Hold on last frame
      return true;        // Animation complete
    }
  }
  return false;
}

void Player::Reset(int startX, int startY) {
  x = startX;
  y = startY;
  pixelX = startX * TILE_SIZE;
  pixelY = startY * TILE_SIZE;
  targetX = startX;
  targetY = startY;
  moveDirX = 0;
  moveDirY = 0;
  isMoving = false;
  stepCount = 0;
  animFrame = 0;
  animCounter = 0;
  facing = 1;
  surface = SURFACE_FLOOR;
  isDying = false;
  deathAnimFrame = 0;
  deathAnimCounter = 0;
  ResetTrail();
}

// ========== Trail System ==========

void Player::InitTrail() {
  for (int i = 0; i < MAX_TRAIL_PARTICLES; i++) {
    trailParticles[i].active = false;
  }
}

void Player::UpdateTrail() {
  for (int i = 0; i < MAX_TRAIL_PARTICLES; i++) {
    if (trailParticles[i].active) {
      trailParticles[i].alpha -= TRAIL_FADE_SPEED;
      if (trailParticles[i].alpha <= 0) {
        trailParticles[i].active = false;
      }
    }
  }
}

void Player::SpawnTrailParticle(int x, int y, bool isHorizontal) {
  for (int i = 0; i < MAX_TRAIL_PARTICLES; i++) {
    if (!trailParticles[i].active) {
      trailParticles[i].x = x;
      trailParticles[i].y = y;
      trailParticles[i].alpha = 255;
      trailParticles[i].active = true;
      trailParticles[i].isHorizontal = isHorizontal;
      return;
    }
  }
}

const TrailParticle *Player::GetTrailParticles() const {
  return trailParticles;
}

void Player::ResetTrail() {
  for (int i = 0; i < MAX_TRAIL_PARTICLES; i++) {
    trailParticles[i].active = false;
  }
}
