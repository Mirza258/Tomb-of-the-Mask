#pragma once

#include <vector>
#include <windows.h>

#include "Constants.h"

enum SurfaceType {
  SURFACE_TYPE_FLOOR = 0,
  SURFACE_TYPE_CEILING = 1,
  SURFACE_TYPE_LEFT_WALL = 2,
  SURFACE_TYPE_RIGHT_WALL = 3,
  SURFACE_TYPE_OTHER = 4 // corners, doubles, etc.
};

enum class MapObjectType {
  Block,
  Collectible,
  Coin,
  Goal,
  Star,
  SpikeWall,
  Turret,
  Puffer
};

struct Sprite {
  short srcX, srcY;        // Sprite sheet coordinates
  bool isDangerous;        // true = dangerous, use enemy sheet
  SurfaceType surfaceType; // Which surface this sprite represents

  Sprite(short x, short y, bool dangerous = false,
         SurfaceType surface = SURFACE_TYPE_OTHER)
      : srcX(x), srcY(y), isDangerous(dangerous), surfaceType(surface) {}
};

struct MapObject {
  MapObjectType type;
  short gridX, gridY; // Grid position
  std::vector<Sprite> sprites;

  MapObject(MapObjectType t, short x, short y) : type(t), gridX(x), gridY(y) {}
  virtual ~MapObject() = default;

  virtual void Update(int playerX = 0, int playerY = 0) {}
  virtual bool IsSolid() const { return false; }

  virtual bool IsSurfaceDangerous(SurfaceType surface) const { return false; }
};

// Block - all wall types
struct Block : public MapObject {
  Block(short x, short y, const std::vector<Sprite> &layerSprites)
      : MapObject(MapObjectType::Block, x, y) {
    sprites = layerSprites;
  }

  bool IsSolid() const override { return true; }

  bool IsSurfaceDangerous(SurfaceType surface) const override {
    for (const auto &s : sprites) {
      if (s.isDangerous && s.surfaceType == surface) {
        return true;
      }
    }
    return false;
  }
};

struct Collectible : public MapObject {
  bool collected;
  short animFrame;   // Current animation frame (0-3)
  short animCounter; // Frame counter for animation timing
  short theme;       // Theme offset for animation

  Collectible(short x, short y, short sx = 0, short sy = 0, short thm = 0)
      : MapObject(MapObjectType::Collectible, x, y), collected(false),
        animFrame(0), animCounter(0), theme(thm) {
    sprites.push_back(Sprite(sx, sy, false, SURFACE_TYPE_OTHER));
  }

  void Update(int playerX = 0, int playerY = 0) override {
    animCounter++;
    if (animCounter >= 5) {
      animFrame = (animFrame + 1) % 4;
      // Frames 0-1 stay the same, frames 2-3 shift by 2*theme
      sprites[0].srcX = animFrame < 2 ? animFrame : animFrame + 2 * theme;
      animCounter = 0;
    }
  }

  bool IsSolid() const override { return false; }
  bool IsCollectible() const { return !collected; }
  void Collect() { collected = true; }
};

struct Coin : public MapObject {
  bool collected;
  short animFrame;   // Current animation frame (0-3)
  short animCounter; // Frame counter for animation timing
  short theme;       // Theme offset for animation

  Coin(short x, short y, short sx = 1, short sy = 1, short thm = 0)
      : MapObject(MapObjectType::Coin, x, y), collected(false), animFrame(0),
        animCounter(0), theme(thm) {
    sprites.push_back(Sprite(sx, sy, false, SURFACE_TYPE_OTHER));
  }

  void Update(int playerX = 0, int playerY = 0) override {
    animCounter++;
    if (animCounter >= 5) {
      animFrame = (animFrame + 1) % 4;
      // Frames 0-1 stay the same, frames 2-3 shift by 2*theme
      sprites[0].srcX = animFrame < 2 ? animFrame : animFrame + 2 * theme;
      animCounter = 0;
    }
  }

  bool IsSolid() const override { return false; }
  bool IsCollectible() const { return !collected; }
  void Collect() { collected = true; }
};

struct Goal : public MapObject {
  short animFrame;   // Current animation frame (0-3)
  short animCounter; // Frame counter for animation timing
  short theme;       // Theme offset for animation

  Goal(short x, short y, short sx = 0, short sy = 2, short thm = 0)
      : MapObject(MapObjectType::Goal, x, y), animFrame(0), animCounter(0),
        theme(thm) {
    sprites.push_back(Sprite(sx, sy, false, SURFACE_TYPE_OTHER));
  }

  void Update(int playerX = 0, int playerY = 0) override {
    animCounter++;
    if (animCounter >= 5) {
      animFrame = (animFrame + 1) % 4;
      // Frames 0-1 stay the same, frames 2-3 shift by 2*theme
      sprites[0].srcX = animFrame < 2 ? animFrame : animFrame + 2 * theme;
      animCounter = 0;
    }
  }

  bool IsSolid() const override { return false; }
  bool IsGoal() const { return true; }
};

struct Star : public MapObject {
  bool collected;
  short animFrame;   // Current animation frame (0-3)
  short animCounter; // Frame counter for animation timing
  short theme;       // Theme offset for animation

  Star(short x, short y, short sx = 1, short sy = 3, short thm = 0)
      : MapObject(MapObjectType::Star, x, y), collected(false), animFrame(0),
        animCounter(0), theme(thm) {
    sprites.push_back(Sprite(sx, sy, false, SURFACE_TYPE_OTHER));
  }

  void Update(int playerX = 0, int playerY = 0) override {
    animCounter++;
    if (animCounter >= 5) {
      animFrame = (animFrame + 1) % 4;
      // Frames 0-1 stay the same, frames 2-3 shift by 2*theme
      sprites[0].srcX = animFrame < 2 ? animFrame : animFrame + 2 * theme;
      animCounter = 0;
    }
  }

  bool IsSolid() const override { return false; }
  bool IsCollectible() const { return !collected; }
  void Collect() { collected = true; }
};

enum SpikeState {
  SPIKE_IDLE,
  SPIKE_DELAY,
  SPIKE_EXTENDING,
  SPIKE_HOLD,
  SPIKE_RETRACTING
};

struct SpikeTrap {
  SurfaceType direction;
  short spikeRow;
  SpikeState state;
  int timer;
  short spikeAnimFrame;

  static const int DELAY_FRAMES = 30;
  static const int EXTEND_FRAMES = 12;
  static const int HOLD_FRAMES = 40;
  static const int RETRACT_FRAMES = 12;

  SpikeTrap(SurfaceType dir, short row)
      : direction(dir), spikeRow(row), state(SPIKE_IDLE), timer(0),
        spikeAnimFrame(0) {}

  bool IsPlayerAdjacent(int gridX, int gridY, int playerX, int playerY) const {
    switch (direction) {
    case SURFACE_TYPE_FLOOR:
      return playerX == gridX && playerY == gridY - 1;
    case SURFACE_TYPE_CEILING:
      return playerX == gridX && playerY == gridY + 1;
    case SURFACE_TYPE_LEFT_WALL:
      return playerX == gridX - 1 && playerY == gridY;
    case SURFACE_TYPE_RIGHT_WALL:
      return playerX == gridX + 1 && playerY == gridY;
    default:
      return false;
    }
  }

  void Update(int gridX, int gridY, int playerX, int playerY) {
    switch (state) {
    case SPIKE_IDLE:
      if (IsPlayerAdjacent(gridX, gridY, playerX, playerY)) {
        state = SPIKE_DELAY;
        timer = 0;
      }
      break;
    case SPIKE_DELAY:
      timer++;
      if (timer >= DELAY_FRAMES) {
        state = SPIKE_EXTENDING;
        timer = 0;
        spikeAnimFrame = 0;
      }
      break;
    case SPIKE_EXTENDING:
      timer++;
      if (timer >= EXTEND_FRAMES / 3) {
        timer = 0;
        spikeAnimFrame++;
        if (spikeAnimFrame >= 3) {
          spikeAnimFrame = 2;
          state = SPIKE_HOLD;
          timer = 0;
        }
      }
      break;
    case SPIKE_HOLD:
      timer++;
      if (timer >= HOLD_FRAMES) {
        state = SPIKE_RETRACTING;
        timer = 0;
        spikeAnimFrame = 2;
      }
      break;
    case SPIKE_RETRACTING:
      timer++;
      if (timer >= RETRACT_FRAMES / 3) {
        timer = 0;
        spikeAnimFrame--;
        if (spikeAnimFrame < 0) {
          spikeAnimFrame = 0;
          state = SPIKE_IDLE;
          timer = 0;
        }
      }
      break;
    }
  }

  void GetSpikeRenderPos(int gridX, int gridY, int &outX, int &outY) const {
    outX = gridX;
    outY = gridY;
    switch (direction) {
    case SURFACE_TYPE_FLOOR:
      outY = gridY - 1;
      break;
    case SURFACE_TYPE_CEILING:
      outY = gridY + 1;
      break;
    case SURFACE_TYPE_LEFT_WALL:
      outX = gridX - 1;
      break;
    case SURFACE_TYPE_RIGHT_WALL:
      outX = gridX + 1;
      break;
    default:
      break;
    }
  }

  bool HasActiveSpikes() const { return state != SPIKE_IDLE; }

  bool AreSpikesDangerous() const {
    return state == SPIKE_EXTENDING || state == SPIKE_HOLD ||
           state == SPIKE_RETRACTING;
  }
};

struct SpikeWall : public MapObject {
  std::vector<SpikeTrap> traps;

  SpikeWall(short x, short y, const std::vector<Sprite> &wallSprites,
            const std::vector<SpikeTrap> &spikeTraps)
      : MapObject(MapObjectType::SpikeWall, x, y), traps(spikeTraps) {
    sprites = wallSprites;
  }

  bool IsSolid() const override { return true; }

  bool IsSurfaceDangerous(SurfaceType surface) const override { return false; }

  void Update(int playerX, int playerY) override {
    for (auto &trap : traps) {
      trap.Update(gridX, gridY, playerX, playerY);
    }
  }

  bool HasActiveSpikes() const {
    for (const auto &trap : traps) {
      if (trap.HasActiveSpikes())
        return true;
    }
    return false;
  }

  bool AreSpikesdangerous() const {
    for (const auto &trap : traps) {
      if (trap.AreSpikesDangerous())
        return true;
    }
    return false;
  }
};

//   Row 0: Arrow sprites (col 0=up, 1=down, 2=left, 3=right)
//   Row 1: Idle turret (same columns)
//   Rows 2-4: Shooting animation (3 frames )
struct TurretBlock : public MapObject {
  int direction; // 0=up, 1=down, 2=left, 3=right
  int fireTimer;
  int fireInterval;
  bool isShooting;
  int animFrame;
  int animCounter;
  bool shouldFire; // Flag : spawn arrow

  // Sprite coordinates for rendering
  short srcX, srcY;

  static const int FIRE_INTERVAL = 80;
  static const int ANIM_SPEED = 4;

  TurretBlock(short x, short y, int dir)
      : MapObject(MapObjectType::Turret, x, y), direction(dir),
        fireInterval(FIRE_INTERVAL), isShooting(false), animFrame(0),
        animCounter(0), shouldFire(false), srcX((short)dir), srcY(1) {
    // Svi pucaju u isto vrijeme
    fireTimer = 0;
  }

  bool IsSolid() const override { return true; }

  void Update(int playerX = 0, int playerY = 0) override {
    if (isShooting) {
      animCounter++;
      if (animCounter >= ANIM_SPEED) {
        animCounter = 0;
        animFrame++;
        if (animFrame >= 3) {
          shouldFire = true;
          isShooting = false;
          animFrame = 0;
          srcY = 1; // Back to idle
        } else {
          srcY = (short)(2 + animFrame);
        }
      }
    } else {
      fireTimer--;
      if (fireTimer <= 0) {
        isShooting = true;
        animFrame = 0;
        animCounter = 0;
        srcY = 2; // First shooting frame
        fireTimer = fireInterval;
      }
    }
  }
};

enum PufferState { PUFFER_BLOCK, PUFFER_EXPANDED };

//   Rows 9+theme: Block animation (4 frames)
//   Rows 0-8: Puffer tiles (3x3 grid)
struct PufferBlock : public MapObject {
  PufferState pufferState;
  int animFrame;
  int animCounter;
  int theme;             // Theme offset
  int blockCycleCount;   // How many full block animation cycles completed
  int pufferAnimIndex;   // Current position in puffer animation sequence
  int pufferAnimCounter; // Frame counter for puffer animation speed

  // Sprite coordinates
  short srcX, srcY;

  static constexpr int PUFFER_SEQUENCE[] = {0, 1, 2, 3, 2, 3, 2, 3, 2, 3, 1, 0};
  static const int PUFFER_SEQUENCE_LENGTH = 12;

  PufferBlock(short x, short y, int thm)
      : MapObject(MapObjectType::Puffer, x, y), pufferState(PUFFER_BLOCK),
        animFrame(0), animCounter(0), theme(thm), blockCycleCount(0),
        pufferAnimIndex(0), pufferAnimCounter(0), srcX(0),
        srcY((short)(9 + thm)) {}

  bool IsSolid() const override { return pufferState == PUFFER_BLOCK; }

  bool IsPuffed() const { return pufferState == PUFFER_EXPANDED; }

  void Update(int playerX = 0, int playerY = 0) override {
    if (pufferState == PUFFER_BLOCK) {
      animCounter++;
      if (animCounter >= 5) {
        animFrame = (animFrame + 1) % 4;
        animCounter = 0;
        if (animFrame == 0) {
          blockCycleCount++;
          if (blockCycleCount >= 2) {
            pufferState = PUFFER_EXPANDED;
            blockCycleCount = 0;
            pufferAnimIndex = 0;
            pufferAnimCounter = 0;
          }
        }
      }
      srcX = (short)animFrame;
      srcY = (short)(9 + theme);
    } else {
      srcX = (short)PUFFER_SEQUENCE[pufferAnimIndex];
      pufferAnimCounter++;
      if (pufferAnimCounter >= 5) {
        pufferAnimIndex++;
        pufferAnimCounter = 0;
        if (pufferAnimIndex >= PUFFER_SEQUENCE_LENGTH) {
          pufferState = PUFFER_BLOCK;
          pufferAnimIndex = 0;
          animFrame = 0;
          animCounter = 0;
        }
      }
    }
  }
};
