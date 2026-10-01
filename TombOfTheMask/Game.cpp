#include "Game.h"
#include <algorithm>
#include <cstring>

constexpr int PufferBlock::PUFFER_SEQUENCE[];

Game::Game()
    : state(STATE_MENU), player(1, 1), camera{0, 0}, mapWidth(0), mapHeight(0),
      score(0), collectiblesCollected(0), totalCollectibles(0), starsCollected(0),
      totalStarsInLevel(0), perfectCompletionAwarded(false), shieldCount(0),
      shieldActive(false), shieldTimer(0), magnetCount(0), magnetActive(false),
      magnetTimer(0), deathDelayCounter(0), spawnX(1), spawnY(1),
      doorAnimFrame(0), doorAnimCounter(0), theme(0), selectedLevelIndex(-1),
      selectedTab(0), currentLevelNumber(0) {}

void Game::Init() {
  db.Initialize("game_progress.db");

  // Set default level path
  if (currentLevelPath.empty()) {
    currentLevelPath = "levels/level1.json";
  }
}

// Load a level from JSON file
bool Game::LoadLevel(const std::string &path) {
  CleanupMap();

  LevelData levelData;
  if (!LevelLoader::LoadLevel(path, levelData)) {
    std::string msg = "Failed to load level: " + path;
    MessageBoxA(NULL, msg.c_str(), "Level Load Error", MB_OK | MB_ICONWARNING);
    return false;
  }

  mapWidth = levelData.width;
  mapHeight = levelData.height;
  theme = levelData.theme;

  map = std::move(levelData.map);

  enemies = std::move(levelData.enemies);

  spawnX = levelData.playerSpawnX;
  spawnY = levelData.playerSpawnY;
  doorAnimFrame = 0;
  doorAnimCounter = 0;

  player.Reset(levelData.playerSpawnX, levelData.playerSpawnY);
  player.InitTrail();
  player.canMoveCallback = [this](int x, int y) { return CanMoveTo(x, y); };
  player.DetermineSurfaceState();

  // Initialize danger map (resize and clear)
  dangerMap.resize(mapHeight);
  for (int y = 0; y < mapHeight; y++) {
    dangerMap[y].assign(mapWidth, false);
  }

  // Set states
  currentLevelPath = path;
  state = STATE_PLAYING;
  score = 0;
  collectiblesCollected = 0;
  starsCollected = 0;
  perfectCompletionAwarded = false;
  shieldCount = db.GetShieldCount();
  shieldActive = false;
  shieldTimer = 0;
  magnetCount = db.GetMagnetCount();
  magnetActive = false;
  magnetTimer = 0;
  CountCollectiblesInLevel();

  return true;
}

void Game::ScanForLevels() {
  availableLevels.clear();
  WIN32_FIND_DATAA findData;
  HANDLE hFind = FindFirstFileA("levels/level*.json", &findData);
  if (hFind != INVALID_HANDLE_VALUE) {
    do {
      // Extract level number from filename
      std::string filename = findData.cFileName;
      size_t numStart = filename.find_first_of("0123456789");
      size_t numEnd = filename.find(".json");
      if (numStart != std::string::npos && numEnd != std::string::npos) {
        int levelNum = std::stoi(filename.substr(numStart, numEnd - numStart));
        availableLevels.push_back(levelNum);
      }
    } while (FindNextFileA(hFind, &findData));
    FindClose(hFind);
  }

  std::sort(availableLevels.begin(), availableLevels.end());
}

// Load and start level
void Game::SelectLevel(int levelNum) {
  std::string path = "levels/level" + std::to_string(levelNum) + ".json";
  if (LoadLevel(path)) {
    currentLevelNumber = levelNum;
    state = STATE_PLAYING;
  }
}

void Game::InitMap() {
  LoadLevel(currentLevelPath.empty() ? "levels/level1.json" : currentLevelPath);
}

void Game::CleanupMap() {
  for (int y = 0; y < (int)map.size(); y++) {
    for (int x = 0; x < (int)map[y].size(); x++) {
      delete map[y][x];
      map[y][x] = nullptr;
    }
  }
  map.clear();

  for (Enemy *enemy : enemies) {
    delete enemy;
  }
  enemies.clear();
}

void Game::Cleanup() {
  CleanupMap();
  db.Close();
}

bool Game::CanMoveTo(int x, int y) {
  if (x < 0 || x >= mapWidth || y < 0 || y >= mapHeight) {
    return false;
  }

  MapObject *obj = map[y][x];
  if (obj != nullptr && obj->IsSolid()) {
    return false;
  }

  return true;
}

void Game::UpdateEnemies() {
  for (Enemy *enemy : enemies) {
    enemy->Update();
  }

  // Remove arrow enemies that hit walls or went out of bounds
  for (int i = (int)enemies.size() - 1; i >= 0; i--) {
    if (enemies[i]->enemyType != ENEMY_ARROW)
      continue;

    int gx = enemies[i]->gridX;
    int gy = enemies[i]->gridY;
    bool shouldRemove = false;

    if (gx < 0 || gx >= mapWidth || gy < 0 || gy >= mapHeight) {
      shouldRemove = true;
    } else {
      MapObject *obj = map[gy][gx];
      if (obj != nullptr && obj->IsSolid()) {
        shouldRemove = true;
      }
    }

    if (shouldRemove) {
      delete enemies[i];
      enemies.erase(enemies.begin() + i);
    }
  }
}

void Game::SpawnTurretArrows() {
  for (int y = 0; y < mapHeight; y++) {
    for (int x = 0; x < mapWidth; x++) {
      MapObject *obj = map[y][x];
      if (obj == nullptr || obj->type != MapObjectType::Turret)
        continue;
      TurretBlock *turret = static_cast<TurretBlock *>(obj);
      if (!turret->shouldFire)
        continue;
      turret->shouldFire = false;

      int ax = x, ay = y;
      switch (turret->direction) {
      case 0:
        ay--;
        break; // Up
      case 1:
        ay++;
        break; // Down
      case 2:
        ax--;
        break; // Left
      case 3:
        ax++;
        break; // Right
      }

      // Check if spawn position is valid
      if (ax >= 0 && ax < mapWidth && ay >= 0 && ay < mapHeight) {
        MapObject *spawnTile = map[ay][ax];
        if (spawnTile == nullptr || !spawnTile->IsSolid()) {
          enemies.push_back(
              new ArrowEnemy((short)ax, (short)ay, turret->direction));
        }
      }
    }
  }
}

void Game::UpdateDangerMap() {
  for (int y = 0; y < mapHeight; y++) {
    std::fill(dangerMap[y].begin(), dangerMap[y].end(), false);
  }

  const int threshold = (int)(TILE_SIZE * 0.3f); // 30% overlap = ~10 pixels

  for (Enemy *enemy : enemies) {
    int gx = enemy->gridX;
    int gy = enemy->gridY;
    int ox = enemy->pixelX % TILE_SIZE; // offset into current tile (X)
    int oy = enemy->pixelY % TILE_SIZE; // offset into current tile (Y)

    // Current tile: dangerous if overlap >= threshold
    if (gx < mapWidth && gy < mapHeight) {
      if ((TILE_SIZE - ox) >= threshold && (TILE_SIZE - oy) >= threshold)
        dangerMap[gy][gx] = true;
    }

    // Next tile horizontally
    if (ox >= threshold && gx + 1 < mapWidth && gy < mapHeight) {
      dangerMap[gy][gx + 1] = true;
    }

    // Next tile vertically
    if (oy >= threshold && gx < mapWidth && gy + 1 < mapHeight) {
      dangerMap[gy + 1][gx] = true;
    }
  }

  // Mark tiles with puffer blocks and active spike walls as dangerous
  for (int y = 0; y < mapHeight; y++) {
    for (int x = 0; x < mapWidth; x++) {
      MapObject *obj = map[y][x];
      if (obj != nullptr) {
        if (obj->type == MapObjectType::Puffer) {
          PufferBlock *puffer = static_cast<PufferBlock *>(obj);
          if (puffer->IsPuffed()) {
            // Mark 3x3 area around puffer as dangerous
            for (int dy = -1; dy <= 1; dy++) {
              for (int dx = -1; dx <= 1; dx++) {
                int tx = x + dx;
                int ty = y + dy;
                if (tx >= 0 && tx < mapWidth && ty >= 0 && ty < mapHeight) {
                  dangerMap[ty][tx] = true;
                }
              }
            }
          }
        } else if (obj->type == MapObjectType::SpikeWall) {
          SpikeWall *spike = static_cast<SpikeWall *>(obj);
          for (const auto &trap : spike->traps) {
            if (trap.AreSpikesDangerous()) {
              int spikeX, spikeY;
              trap.GetSpikeRenderPos(spike->gridX, spike->gridY, spikeX,
                                     spikeY);
              if (spikeX >= 0 && spikeX < mapWidth && spikeY >= 0 &&
                  spikeY < mapHeight) {
                dangerMap[spikeY][spikeX] = true;
              }
            }
          }
        }
      }
    }
  }
}

bool Game::IsPlayerOnDanger() {
  int px = player.x;
  int py = player.y;
  if (px >= 0 && px < mapWidth && py >= 0 && py < mapHeight) {
    return dangerMap[py][px];
  }
  return false;
}

// Check if player hit a KillBlock (the tile that blocked them)
// This checks the tile in the direction the player was moving
bool Game::IsPlayerHittingKillBlock() {
  int checkX = player.x + player.moveDirX;
  int checkY = player.y + player.moveDirY;

  // If no movement direction stored, nothing to check
  if (player.moveDirX == 0 && player.moveDirY == 0) {
    return false;
  }

  SurfaceType hitSurface = SURFACE_TYPE_OTHER;
  if (player.moveDirY > 0)
    hitSurface = SURFACE_TYPE_FLOOR; // Moving down, hitting floor
  else if (player.moveDirY < 0)
    hitSurface = SURFACE_TYPE_CEILING; // Moving up, hitting ceiling
  else if (player.moveDirX > 0)
    hitSurface = SURFACE_TYPE_LEFT_WALL; // Moving right, hitting left wall
  else if (player.moveDirX < 0)
    hitSurface = SURFACE_TYPE_RIGHT_WALL; // Moving left, hitting right wall

  // Check if the blocking tile has a dangerous surface
  if (checkX >= 0 && checkX < mapWidth && checkY >= 0 && checkY < mapHeight) {
    MapObject *obj = map[checkY][checkX];
    if (obj != nullptr && obj->IsSurfaceDangerous(hitSurface)) {
      return true;
    }
  }
  return false;
}

void Game::CheckCollisions() {
  // Shield protects from all danger
  if (shieldActive)
    return;

  if (IsPlayerOnDanger()) {
    StartDeath();
    return;
  }

  if (!player.isMoving && IsPlayerHittingKillBlock()) {
    StartDeath();
    return;
  }
}

// Check if player is touching a KillBlock
// This handles the case where player switches to a deadly wall
bool Game::IsPlayerTouchingKillBlock() {
  int checkX = player.x;
  int checkY = player.y;
  SurfaceType checkSurface = SURFACE_TYPE_OTHER;

  switch (player.surface) {
  case SURFACE_FLOOR:
    checkY = player.y + 1;             // Tile below
    checkSurface = SURFACE_TYPE_FLOOR; // That tile's top surface (floor)
    break;
  case SURFACE_CEILING:
    checkY = player.y - 1;               // Tile above
    checkSurface = SURFACE_TYPE_CEILING; // That tile's bottom surface (ceiling)
    break;
  case SURFACE_LEFT_WALL:
    checkX = player.x - 1; // Tile to the left
    checkSurface =
        SURFACE_TYPE_RIGHT_WALL; // Player touches RIGHT side of that tile
    break;
  case SURFACE_RIGHT_WALL:
    checkX = player.x + 1; // Tile to the right
    checkSurface =
        SURFACE_TYPE_LEFT_WALL; // Player touches LEFT side of that tile
    break;
  }

  // Check if that tile has a dangerous surface
  if (checkX >= 0 && checkX < mapWidth && checkY >= 0 && checkY < mapHeight) {
    MapObject *obj = map[checkY][checkX];
    if (obj != nullptr && obj->IsSurfaceDangerous(checkSurface)) {
      return true;
    }
  }
  return false;
}

void Game::Update() {
  if (state == STATE_DYING) {
    // Update death animation
    if (player.UpdateDeathAnimation()) {
      // Animation done, count delay frames
      deathDelayCounter++;
      if (deathDelayCounter >= 15) {
        state = STATE_GAME_OVER;
        return;
      }
    }

    // Keep world alive during death
    doorAnimCounter++;
    if (doorAnimCounter >= 8) {
      doorAnimFrame = (doorAnimFrame + 1) % 4;
      doorAnimCounter = 0;
    }
    player.UpdateTrail();
    for (int y = 0; y < (int)map.size(); y++) {
      for (int x = 0; x < (int)map[y].size(); x++) {
        if (map[y][x] != nullptr) {
          map[y][x]->Update(player.x, player.y);
        }
      }
    }
    SpawnTurretArrows();
    UpdateEnemies();
    UpdateDangerMap();
    return;
  }

  if (state != STATE_PLAYING) {
    return;
  }

  if (shieldActive) {
    shieldTimer--;
    if (shieldTimer <= 0) {
      shieldActive = false;
      shieldTimer = 0;
    }
  }

  if (magnetActive) {
    magnetTimer--;
    if (magnetTimer <= 0) {
      magnetActive = false;
      magnetTimer = 0;
    } else {
      // Collect all collectibles, coins, and stars in 3x3 area around player
      for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
          int gx = player.x + dx;
          int gy = player.y + dy;
          CollectCollectibleAt(gx, gy);
          CollectCoinAt(gx, gy);
          CollectStarAt(gx, gy);
        }
      }
    }
  }

  player.UpdateAnimation();

  // Update door animation
  doorAnimCounter++;
  if (doorAnimCounter >= 8) {
    doorAnimFrame = (doorAnimFrame + 1) % 4;
    doorAnimCounter = 0;
  }

  player.UpdateTrail();

  for (int step = 0; step < STEPS_PER_CYCLE && player.isMoving; step++) {
    // Track old position for this step
    int oldTileX = player.x;
    int oldTileY = player.y;

    // Update movement
    player.UpdateMovement();

    // If we moved to a new tile, check for collectibles and dangers
    if (player.x != oldTileX || player.y != oldTileY) {
      // Spawn trail particle
      bool isHorizontalMove = (player.moveDirX != 0);
      player.SpawnTrailParticle(player.pixelX, player.pixelY, isHorizontalMove);

      CollectCollectibleAt(player.x, player.y);
      CollectCoinAt(player.x, player.y);
      CollectStarAt(player.x, player.y);

      CheckGoalAt(player.x, player.y);

      if (!shieldActive && CheckDangerAt(player.x, player.y)) {
        StartDeath();
        return;
      }
    }
  }

  // Collect items if standing still
  if (!player.isMoving) {
    CollectCollectibleAt(player.x, player.y);
    CollectCoinAt(player.x, player.y);
    CollectStarAt(player.x, player.y);
    CheckGoalAt(player.x, player.y);
  }

  // Update map objects
  for (int y = 0; y < (int)map.size(); y++) {
    for (int x = 0; x < (int)map[y].size(); x++) {
      if (map[y][x] != nullptr) {
        map[y][x]->Update(player.x, player.y);
      }
    }
  }

  SpawnTurretArrows();

  UpdateEnemies();
  UpdateDangerMap();

  CheckCollisions();
}

void Game::HandleInput(WPARAM key) {
  if (state == STATE_DYING) {
    return;
  }

  if (key == VK_ESCAPE) {
    TogglePause();
    return;
  }

  if (state == STATE_MENU) {
    if (key == VK_RETURN) {
      ScanForLevels();
      state = STATE_LEVEL_SELECT;
    }
    return;
  }

  if (state == STATE_LEVEL_SELECT) {
    return;
  }

  // Only handle movement when playing
  if (state != STATE_PLAYING) {
    if (state == STATE_LEVEL_COMPLETE) {
      if (key == VK_RETURN) {
        ReturnToMenu();
      }
    }
    return;
  }

  // Shield activation
  if (key == VK_SPACE) {
    ActivateShield();
    return;
  }

  // Magnet activation
  if (key == 'M') {
    ActivateMagnet();
    return;
  }

  // Movement input
  if (!player.isMoving) {
    int dx = 0;
    int dy = 0;

    switch (key) {
    case VK_LEFT:
      dx = -1;
      break;
    case VK_RIGHT:
      dx = 1;
      break;
    case VK_UP:
      dy = -1;
      break;
    case VK_DOWN:
      dy = 1;
      break;
    }

    if (dx != 0 || dy != 0) {
      if (!player.StartMove(dx, dy)) {
        player.TrySwitchSurface(dx, dy);
        // Check if player switched onto a KillBlock
        if (!shieldActive && IsPlayerTouchingKillBlock()) {
          StartDeath();
        }
      }
    }
  }
}

void Game::ActivateShield() {
  if (shieldActive || shieldCount <= 0)
    return;

  shieldCount--;
  db.SetShieldCount(shieldCount);
  shieldActive = true;
  shieldTimer = 180;
}

void Game::ActivateMagnet() {
  if (magnetActive || magnetCount <= 0)
    return;

  magnetCount--;
  db.SetMagnetCount(magnetCount);
  magnetActive = true;
  magnetTimer = 300;
}

void Game::Pause() {
  if (state == STATE_PLAYING) {
    state = STATE_PAUSED;
  }
}

void Game::Resume() {
  if (state == STATE_PAUSED) {
    state = STATE_PLAYING;
  }
}

void Game::TogglePause() {
  if (state == STATE_PLAYING) {
    state = STATE_PAUSED;
  } else if (state == STATE_PAUSED) {
    state = STATE_PLAYING;
  }
}

void Game::StartDeath() {
  if (state == STATE_DYING)
    return;
  state = STATE_DYING;
  player.StartDeathAnimation();
  deathDelayCounter = 0;
}

void Game::CompleteLevel() {
  if (state != STATE_PLAYING)
    return;

  db.AddCoins(score);

  db.MarkLevelCompleted(currentLevelNumber);

  db.SaveLevelCoins(currentLevelNumber, score);

  db.SaveLevelStars(currentLevelNumber, starsCollected);

  bool isPerfect = (collectiblesCollected >= totalCollectibles) &&
                   (starsCollected >= totalStarsInLevel);
  if (isPerfect && !db.IsLevelPerfected(currentLevelNumber)) {
    // First-time perfect completion: award 25 bonus coins
    db.AddCoins(25);
    db.MarkLevelPerfected(currentLevelNumber);
    perfectCompletionAwarded = true;
  }

  state = STATE_LEVEL_COMPLETE;
}

void Game::ReturnToMenu() {
  CleanupMap();
  player.ResetTrail();
  ScanForLevels();
  state = STATE_LEVEL_SELECT;
}

bool Game::CheckDangerAt(int gridX, int gridY) {
  if (gridX < 0 || gridX >= mapWidth || gridY < 0 || gridY >= mapHeight) {
    return false;
  }
  return dangerMap[gridY][gridX];
}

void Game::Reset() {
  CleanupMap();
  player.Reset(1, 1);
  player.ResetTrail();
  score = 0;
  collectiblesCollected = 0;
  starsCollected = 0;
  perfectCompletionAwarded = false;
  shieldCount = db.GetShieldCount();
  shieldActive = false;
  shieldTimer = 0;
  magnetCount = db.GetMagnetCount();
  magnetActive = false;
  magnetTimer = 0;
  deathDelayCounter = 0;
  doorAnimFrame = 0;
  doorAnimCounter = 0;
  InitMap();
  CountCollectiblesInLevel();
  state = STATE_PLAYING;
}

void Game::CollectCollectibleAt(int px, int py) {
  if (px < 0 || px >= mapWidth || py < 0 || py >= mapHeight) {
    return;
  }

  MapObject *obj = map[py][px];
  if (obj != nullptr) {
    if (obj->type == MapObjectType::Collectible) {
      Collectible *collectible = static_cast<Collectible *>(obj);
      if (!collectible->IsCollectible())
        return;
      collectible->Collect();
      collectiblesCollected++;
      delete map[py][px];
      map[py][px] = nullptr;
    }
  }
}

void Game::CollectCoinAt(int px, int py) {
  if (px < 0 || px >= mapWidth || py < 0 || py >= mapHeight) {
    return;
  }

  MapObject *obj = map[py][px];
  if (obj != nullptr) {
    if (obj->type == MapObjectType::Coin) {
      Coin *coin = static_cast<Coin *>(obj);
      if (!coin->IsCollectible())
        return;
      coin->Collect();
      score += 5; // Each coin is worth 5 currency
      delete map[py][px];
      map[py][px] = nullptr;
    }
  }
}

void Game::CollectStarAt(int px, int py) {
  if (px < 0 || px >= mapWidth || py < 0 || py >= mapHeight) {
    return;
  }

  MapObject *obj = map[py][px];
  if (obj != nullptr) {
    if (obj->type == MapObjectType::Star) {
      Star *star = static_cast<Star *>(obj);
      if (!star->IsCollectible())
        return;
      star->Collect();
      starsCollected++;
      delete map[py][px];
      map[py][px] = nullptr;
    }
  }
}

void Game::CountCollectiblesInLevel() {
  totalCollectibles = 0;
  totalStarsInLevel = 0;
  for (int y = 0; y < mapHeight; y++) {
    for (int x = 0; x < mapWidth; x++) {
      if (map[y][x] == nullptr)
        continue;
      if (map[y][x]->type == MapObjectType::Collectible) {
        totalCollectibles++;
      } else if (map[y][x]->type == MapObjectType::Star) {
        totalStarsInLevel++;
      }
    }
  }
}

void Game::CheckGoalAt(int gridX, int gridY) {
  if (gridX < 0 || gridX >= mapWidth || gridY < 0 || gridY >= mapHeight) {
    return;
  }

  MapObject *obj = map[gridY][gridX];
  if (obj != nullptr) {
    if (obj->type == MapObjectType::Goal) {
      CompleteLevel();
    }
  }
}
