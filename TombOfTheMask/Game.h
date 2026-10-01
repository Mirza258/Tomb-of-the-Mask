#pragma once

#include "DatabaseManager.h"
#include "Enemy.h"
#include "LevelLoader.h"
#include "MapObjects.h"
#include "Player.h"
#include <string>
#include <vector>
#include <windows.h>

struct Camera {
  int x; // Pixel offset X
  int y; // Pixel offset Y
};

enum GameState {
  STATE_MENU,
  STATE_LEVEL_SELECT,
  STATE_PLAYING,
  STATE_PAUSED,
  STATE_DYING,
  STATE_GAME_OVER,
  STATE_LEVEL_COMPLETE
};

class Game {
public:
  DatabaseManager db;

  GameState state;

  Player player;

  Camera camera;

  std::vector<std::vector<MapObject *>> map;

  int mapWidth;
  int mapHeight;

  std::vector<Enemy *> enemies;

  // Danger map - tracks tiles occupied by enemies
  std::vector<std::vector<bool>> dangerMap;

  // Score for current level
  int score;

  // Collectible tracking
  int collectiblesCollected;
  int totalCollectibles;

  // Star tracking
  int starsCollected;
  int totalStarsInLevel;

  // Whether perfect completion bonus was awarded this run
  bool perfectCompletionAwarded;

  // Shield power-up
  int shieldCount;
  bool shieldActive;
  int shieldTimer;

  // Magnet power-up
  int magnetCount;
  bool magnetActive;
  int magnetTimer;

  int deathDelayCounter; // Frames to wait after death anim before game over
                         // menu

  // Door animation at spawn point
  int spawnX;
  int spawnY;
  int doorAnimFrame;
  int doorAnimCounter;

  // Current level theme
  int theme;

  std::string currentLevelPath;

  // Level selection data
  std::vector<int> availableLevels;
  int selectedLevelIndex;
  int selectedTab; // 0 = Map, 1 = Shop
  int currentLevelNumber;

  Game();

  // Initialize game (load map, set up player...)
  void Init();

  // Load a level from JSON
  bool LoadLevel(const std::string &path);

  // Level selection
  void ScanForLevels();           // Scan levels folder for available levels
  void SelectLevel(int levelNum); // Load and start a specific level

  void Cleanup();

  // Main update loop
  void Update();

  void HandleInput(WPARAM key);

  // State
  void Pause();
  void Resume();
  void TogglePause();
  void StartDeath();
  void CompleteLevel();
  void ReturnToMenu();
  void Reset();

  // Shield power-up
  void ActivateShield();

  // Magnet power-up
  void ActivateMagnet();

  // Collision detection
  void CheckCollisions();
  bool IsPlayerOnDanger();
  bool IsPlayerHittingKillBlock();
  bool IsPlayerTouchingKillBlock();

  // Collision callback for Player movement
  bool CanMoveTo(int x, int y);

  void UpdateDangerMap();

  void CollectCollectibleAt(int gridX, int gridY);

  void CollectCoinAt(int gridX, int gridY);

  void CollectStarAt(int gridX, int gridY);

  void CountCollectiblesInLevel();

  void CheckGoalAt(int gridX, int gridY);

  bool CheckDangerAt(int gridX, int gridY);

  void InitMap();

  void CleanupMap();

  void UpdateEnemies();

  void SpawnTurretArrows();
};
