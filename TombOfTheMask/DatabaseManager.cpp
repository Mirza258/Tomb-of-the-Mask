#include "DatabaseManager.h"

DatabaseManager::DatabaseManager() : db(nullptr), isOpen(false) {}

DatabaseManager::~DatabaseManager() { Close(); }

bool DatabaseManager::Initialize(const std::string &dbPath) {
  if (isOpen) {
    return true;
  }

  int result = sqlite3_open(dbPath.c_str(), &db);
  if (result != SQLITE_OK) {
    sqlite3_close(db);
    db = nullptr;
    return false;
  }

  isOpen = true;
  CreateTables();
  return true;
}

void DatabaseManager::Close() {
  if (db) {
    sqlite3_close(db);
    db = nullptr;
    isOpen = false;
  }
}

void DatabaseManager::CreateTables() {
  ExecuteSQL("CREATE TABLE IF NOT EXISTS player_progress ("
             "   id INTEGER PRIMARY KEY CHECK (id = 1),"
             "   total_coins INTEGER DEFAULT 0,"
             "   shield_count INTEGER DEFAULT 0,"
             "   magnet_count INTEGER DEFAULT 0"
             ");");

  ExecuteSQL(
      "INSERT OR IGNORE INTO player_progress (id, total_coins) VALUES (1, 0);");

  // Level completion
  ExecuteSQL("CREATE TABLE IF NOT EXISTS level_progress ("
             "   level_number INTEGER PRIMARY KEY,"
             "   completed INTEGER DEFAULT 0,"
             "   best_coins INTEGER DEFAULT 0,"
             "   best_stars INTEGER DEFAULT 0,"
             "   perfect_completed INTEGER DEFAULT 0"
             ");");
}

bool DatabaseManager::ExecuteSQL(const char *sql) {
  if (!isOpen || !db)
    return false;

  char *errMsg = nullptr;
  int result = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);

  if (errMsg) {
    sqlite3_free(errMsg);
  }

  return result == SQLITE_OK;
}

// ========== Level Progress ==========

void DatabaseManager::MarkLevelCompleted(int levelNumber) {
  if (!isOpen)
    return;

  const char *sql =
      "INSERT INTO level_progress (level_number, completed) VALUES (?, 1) "
      "ON CONFLICT(level_number) DO UPDATE SET completed = 1;";

  sqlite3_stmt *stmt;
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_int(stmt, 1, levelNumber);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
  }
}

bool DatabaseManager::IsLevelCompleted(int levelNumber) {
  if (!isOpen)
    return false;

  const char *sql =
      "SELECT completed FROM level_progress WHERE level_number = ?;";
  sqlite3_stmt *stmt;
  bool completed = false;

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_int(stmt, 1, levelNumber);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
      completed = sqlite3_column_int(stmt, 0) == 1;
    }
    sqlite3_finalize(stmt);
  }

  return completed;
}

int DatabaseManager::GetHighestCompletedLevel() {
  if (!isOpen)
    return 0;

  const char *sql =
      "SELECT MAX(level_number) FROM level_progress WHERE completed = 1;";
  sqlite3_stmt *stmt;
  int highest = 0;

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    if (sqlite3_step(stmt) == SQLITE_ROW) {
      highest = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
  }

  return highest;
}

// ========== Coins ==========

void DatabaseManager::AddCoins(int amount) {
  if (!isOpen || amount <= 0)
    return;

  const char *sql =
      "UPDATE player_progress SET total_coins = total_coins + ? WHERE id = 1;";
  sqlite3_stmt *stmt;

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_int(stmt, 1, amount);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
  }
}

int DatabaseManager::GetTotalCoins() {
  if (!isOpen)
    return 0;

  const char *sql = "SELECT total_coins FROM player_progress WHERE id = 1;";
  sqlite3_stmt *stmt;
  int coins = 0;

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    if (sqlite3_step(stmt) == SQLITE_ROW) {
      coins = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
  }

  return coins;
}

void DatabaseManager::SetTotalCoins(int amount) {
  if (!isOpen)
    return;

  const char *sql = "UPDATE player_progress SET total_coins = ? WHERE id = 1;";
  sqlite3_stmt *stmt;

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_int(stmt, 1, amount);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
  }
}

// ========== Level Stats ==========

void DatabaseManager::SaveLevelCoins(int levelNumber, int coins) {
  if (!isOpen)
    return;

  int currentBest = GetLevelBestCoins(levelNumber);
  if (coins <= currentBest)
    return;

  const char *sql =
      "INSERT INTO level_progress (level_number, best_coins) VALUES (?, ?) "
      "ON CONFLICT(level_number) DO UPDATE SET best_coins = ? WHERE best_coins "
      "< ?;";

  sqlite3_stmt *stmt;
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_int(stmt, 1, levelNumber);
    sqlite3_bind_int(stmt, 2, coins);
    sqlite3_bind_int(stmt, 3, coins);
    sqlite3_bind_int(stmt, 4, coins);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
  }
}

int DatabaseManager::GetLevelBestCoins(int levelNumber) {
  if (!isOpen)
    return 0;

  const char *sql =
      "SELECT best_coins FROM level_progress WHERE level_number = ?;";
  sqlite3_stmt *stmt;
  int coins = 0;

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_int(stmt, 1, levelNumber);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
      coins = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
  }

  return coins;
}

// ========== Stars ==========

void DatabaseManager::SaveLevelStars(int levelNumber, int stars) {
  if (!isOpen)
    return;

  int currentBest = GetLevelBestStars(levelNumber);
  if (stars <= currentBest)
    return;

  const char *sql =
      "INSERT INTO level_progress (level_number, best_stars) VALUES (?, ?) "
      "ON CONFLICT(level_number) DO UPDATE SET best_stars = ? WHERE best_stars "
      "< ?;";

  sqlite3_stmt *stmt;
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_int(stmt, 1, levelNumber);
    sqlite3_bind_int(stmt, 2, stars);
    sqlite3_bind_int(stmt, 3, stars);
    sqlite3_bind_int(stmt, 4, stars);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
  }
}

int DatabaseManager::GetLevelBestStars(int levelNumber) {
  if (!isOpen)
    return 0;

  const char *sql =
      "SELECT best_stars FROM level_progress WHERE level_number = ?;";
  sqlite3_stmt *stmt;
  int stars = 0;

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_int(stmt, 1, levelNumber);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
      stars = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
  }

  return stars;
}

// ========== Perfect Completion ==========

bool DatabaseManager::IsLevelPerfected(int levelNumber) {
  if (!isOpen)
    return false;

  const char *sql =
      "SELECT perfect_completed FROM level_progress WHERE level_number = ?;";
  sqlite3_stmt *stmt;
  bool perfected = false;

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_int(stmt, 1, levelNumber);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
      perfected = sqlite3_column_int(stmt, 0) == 1;
    }
    sqlite3_finalize(stmt);
  }

  return perfected;
}

void DatabaseManager::MarkLevelPerfected(int levelNumber) {
  if (!isOpen)
    return;

  const char *sql =
      "INSERT INTO level_progress (level_number, perfect_completed) VALUES (?, "
      "1) "
      "ON CONFLICT(level_number) DO UPDATE SET perfect_completed = 1;";

  sqlite3_stmt *stmt;
  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_int(stmt, 1, levelNumber);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
  }
}

// ========== Powerups ==========

int DatabaseManager::GetShieldCount() {
  if (!isOpen)
    return 0;

  const char *sql = "SELECT shield_count FROM player_progress WHERE id = 1;";
  sqlite3_stmt *stmt;
  int count = 0;

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    if (sqlite3_step(stmt) == SQLITE_ROW) {
      count = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
  }

  return count;
}

void DatabaseManager::SetShieldCount(int count) {
  if (!isOpen)
    return;

  const char *sql = "UPDATE player_progress SET shield_count = ? WHERE id = 1;";
  sqlite3_stmt *stmt;

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_int(stmt, 1, count);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
  }
}

int DatabaseManager::GetMagnetCount() {
  if (!isOpen)
    return 0;

  const char *sql = "SELECT magnet_count FROM player_progress WHERE id = 1;";
  sqlite3_stmt *stmt;
  int count = 0;

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    if (sqlite3_step(stmt) == SQLITE_ROW) {
      count = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
  }

  return count;
}

void DatabaseManager::SetMagnetCount(int count) {
  if (!isOpen)
    return;

  const char *sql = "UPDATE player_progress SET magnet_count = ? WHERE id = 1;";
  sqlite3_stmt *stmt;

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    sqlite3_bind_int(stmt, 1, count);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
  }
}

// ========== Debug ==========

void DatabaseManager::ResetDatabase() {
  if (!isOpen)
    return;

  ExecuteSQL("DROP TABLE IF EXISTS player_progress;");
  ExecuteSQL("DROP TABLE IF EXISTS level_progress;");
  CreateTables();
}

void DatabaseManager::UnlockAllLevels(int maxLevel) {
  if (!isOpen)
    return;

  for (int i = 1; i <= maxLevel; i++) {
    MarkLevelCompleted(i);
  }
}
