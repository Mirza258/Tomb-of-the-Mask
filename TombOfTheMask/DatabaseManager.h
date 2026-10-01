#pragma once

#include "sqlite3.h"
#include <string>

class DatabaseManager {
public:
  DatabaseManager();
  ~DatabaseManager();

  bool Initialize(const std::string &dbPath = "game_progress.db");

  void Close();

  // ========== Level Progress ==========

  void MarkLevelCompleted(int levelNumber);

  bool IsLevelCompleted(int levelNumber);

  int GetHighestCompletedLevel();

  // ========== Coins ==========

  void AddCoins(int amount);

  int GetTotalCoins();

  void SetTotalCoins(int amount);

  // ========== Level Stats ==========

  void SaveLevelCoins(int levelNumber, int coins);

  int GetLevelBestCoins(int levelNumber);

  void SaveLevelStars(int levelNumber, int stars);

  int GetLevelBestStars(int levelNumber);

  bool IsLevelPerfected(int levelNumber);

  void MarkLevelPerfected(int levelNumber);

  // ========== Powerups ==========

  int GetShieldCount();

  void SetShieldCount(int count);

  int GetMagnetCount();

  void SetMagnetCount(int count);

  // ========== Debug ==========

  void ResetDatabase();

  void UnlockAllLevels(int maxLevel);

private:
  sqlite3 *db;
  bool isOpen;

  void CreateTables();

  bool ExecuteSQL(const char *sql);
};
