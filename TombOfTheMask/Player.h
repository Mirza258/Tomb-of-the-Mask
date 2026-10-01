#pragma once

#include <functional>
#include <windows.h>

#include "Constants.h"

enum SurfaceState {
  SURFACE_FLOOR = 0,
  SURFACE_CEILING = 1,
  SURFACE_LEFT_WALL = 2,
  SURFACE_RIGHT_WALL = 3
};

struct TrailParticle {
  int x, y;  // Pixel position
  int alpha; // 0-255
  bool active;
  bool isHorizontal; // true for left/right movement, false for up/down
};

class Player {
public:
  // Trail system constants
  static const int MAX_TRAIL_PARTICLES = 100;
  static const int TRAIL_FADE_SPEED = 32;
  // Grid position
  int x;
  int y;

  // Pixel position
  int pixelX;
  int pixelY;

  // Movement state
  int targetX; // Next target (grid coords)
  int targetY;
  int moveDirX; // Direction of movement (-1, 0, or 1)
  int moveDirY;
  bool isMoving;
  int stepCount; // Steps taken in current movement cycle (0 to STEPS_PER_CYCLE)

  // Animation state
  int animFrame;
  int animCounter;

  // Death animation state
  bool isDying;
  int deathAnimFrame;
  int deathAnimCounter;

  // Facing and surface
  int facing;
  SurfaceState surface;

  // Callback for collision checks (set by Game)
  std::function<bool(int, int)> canMoveCallback;

  // Constructor
  Player(int startX = 1, int startY = 1);

  // Check if we can move to a position
  bool CanMoveTo(int checkX, int checkY) const;

  // Start sliding movement in direction (dx, dy)
  // Returns true if movement started, false if blocked
  bool StartMove(int dx, int dy);

  // Update movement animation each frame
  // Returns true if still moving, false if stopped
  bool UpdateMovement();

  // Continue movement to next tile (called after reaching a tile)
  // Returns true if movement continues, false if blocked
  bool ContinueMove();

  void UpdateAnimation();

  void DetermineSurfaceState();

  int GetSpriteRow() const;

  int GetSpriteColumn() const;

  void StartDeathAnimation();
  bool UpdateDeathAnimation(); // Returns true when animation is complete

  void Reset(int startX, int startY);

  void TrySwitchSurface(int dx, int dy);

  // ========== Trail System ==========

  void InitTrail();

  void UpdateTrail();

  void SpawnTrailParticle(int x, int y, bool isHorizontal);

  const TrailParticle *GetTrailParticles() const;

  void ResetTrail();

  TrailParticle trailParticles[MAX_TRAIL_PARTICLES];
};
