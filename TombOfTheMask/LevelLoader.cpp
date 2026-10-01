#include "LevelLoader.h"
#include "json.hpp"
#include <fstream>

using json = nlohmann::json;

static const short tileSpriteCoords[14][2] = {
    {-1, -1}, // 0: EMPTY (not drawn)
    {0, 0},   // 1: FLOOR
    {1, 0},   // 2: CEILING
    {2, 0},   // 3: WALL_LEFT
    {3, 0},   // 4: WALL_RIGHT
    {0, 1},   // 5: OUTER_CORNER_BL
    {1, 1},   // 6: OUTER_CORNER_TL
    {2, 1},   // 7: OUTER_CORNER_TR
    {3, 1},   // 8: OUTER_CORNER_BR
    {0, 2},   // 9: INNER_CORNER_TR
    {1, 2},   // 10: INNER_CORNER_BR
    {2, 2},   // 11: INNER_CORNER_BL
    {3, 2},   // 12: INNER_CORNER_TL
    {0, 3}    // 13: FULL_BLOCK (column = theme, row 3)
};

namespace LevelLoader {

SurfaceType GetSurfaceType(int tileId) {
  int normalizedId = (tileId >= 20 && tileId <= 34) ? (tileId - 19) : tileId;

  if (tileId >= 40 && tileId <= 43) {
    switch (tileId) {
    case 40:
      return SURFACE_TYPE_FLOOR;
    case 41:
      return SURFACE_TYPE_CEILING;
    case 42:
      return SURFACE_TYPE_LEFT_WALL;
    case 43:
      return SURFACE_TYPE_RIGHT_WALL;
    }
  }

  switch (normalizedId) {
  case 1:
    return SURFACE_TYPE_FLOOR;
  case 2:
    return SURFACE_TYPE_CEILING;
  case 3:
    return SURFACE_TYPE_LEFT_WALL;
  case 4:
    return SURFACE_TYPE_RIGHT_WALL;
  default:
    return SURFACE_TYPE_OTHER;
  }
}

void GetSpriteCoords(int tileId, short &outSrcX, short &outSrcY) {
  if (tileId >= 1 && tileId <= 13) {
    outSrcX = tileSpriteCoords[tileId][0];
    outSrcY = tileSpriteCoords[tileId][1];
  } else if (tileId >= 20 && tileId <= 31) {
    int spriteIndex = tileId - 19;
    outSrcX = tileSpriteCoords[spriteIndex][0];
    outSrcY = tileSpriteCoords[spriteIndex][1];
  } else if (tileId >= 40 && tileId <= 43) {
    // Col 0=floor, 1=ceiling, 2=left wall, 3=right wall
    outSrcX = (short)(tileId - 40);
    outSrcY = 5;
  } else {
    outSrcX = -1;
    outSrcY = -1;
  }
}

short GetSpikeAnimRow(int tileId) {
  switch (tileId) {
  case 40:
    return 7; // Floor spikes: row 8
  case 41:
    return 8; // Ceiling spikes: row 9
  case 43:
    return 9; // Right wall spikes: row 10
  case 42:
    return 10; // Left wall spikes: row 11
  default:
    return 7;
  }
}

bool LoadLevel(const std::string &filename, LevelData &outData) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    return false;
  }

  // Parse JSON
  json levelJson;
  try {
    file >> levelJson;
  } catch (const json::parse_error &) {
    return false;
  }

  // Read basic level info
  outData.name = levelJson.value("name", "Unnamed Level");
  outData.width = levelJson.value("width", 30);
  outData.height = levelJson.value("height", 20);

  outData.theme = levelJson.value("theme", 0);

  // Player spawn
  if (levelJson.contains("playerSpawn")) {
    outData.playerSpawnX = levelJson["playerSpawn"].value("x", 1);
    outData.playerSpawnY = levelJson["playerSpawn"].value("y", 1);
  }

  // Initialize map grid
  outData.map.resize(outData.height);
  for (int y = 0; y < outData.height; y++) {
    outData.map[y].resize(outData.width, nullptr);
  }

  // Parse tiles
  if (levelJson.contains("tiles") && levelJson["tiles"].is_array()) {
    const auto &tiles = levelJson["tiles"];
    for (int y = 0; y < (int)tiles.size() && y < outData.height; y++) {
      const auto &row = tiles[y];
      for (int x = 0; x < (int)row.size() && x < outData.width; x++) {
        const auto &cell = row[x];

        if (cell.is_number_integer()) {
          // Single tile ID
          int tileId = cell.get<int>();
          if (tileId == 0) {
            // Empty tile
            continue;
          } else if (tileId == 50) {
            // Collectible
            outData.map[y][x] =
                new Collectible((short)x, (short)y, 0, 0, (short)outData.theme);
          } else if (tileId == 51) {
            // Goal (exit tile)
            outData.map[y][x] =
                new Goal((short)x, (short)y, 0, 2, (short)outData.theme);
          } else if (tileId == 52) {
            // Coin
            outData.map[y][x] =
                new Coin((short)x, (short)y, 1, 1, (short)outData.theme);
          } else if (tileId == 53) {
            // Star
            outData.map[y][x] =
                new Star((short)x, (short)y, 1, 3, (short)outData.theme);
          } else if (tileId == 55) {
            // PufferFish
            outData.map[y][x] =
                new PufferBlock((short)x, (short)y, outData.theme);
          } else if (tileId >= 60 && tileId <= 63) {
            // Turret blockt
            // 60=up, 61=down, 62=left, 63=right
            outData.map[y][x] =
                new TurretBlock((short)x, (short)y, tileId - 60);
          } else if (tileId >= 20 && tileId <= 31) {
            // KillBlock sprite
            short sx, sy;
            GetSpriteCoords(tileId, sx, sy);
            SurfaceType surface = GetSurfaceType(tileId);
            outData.map[y][x] =
                new Block((short)x, (short)y, {Sprite(sx, sy, true, surface)});
          } else if (tileId >= 40 && tileId <= 43) {
            // SpikeWall - single direction
            short sx, sy;
            GetSpriteCoords(tileId, sx, sy);
            SurfaceType surface = GetSurfaceType(tileId);
            short spikeRow = GetSpikeAnimRow(tileId);
            outData.map[y][x] = new SpikeWall((short)x, (short)y,
                                              {Sprite(sx, sy, true, surface)},
                                              {SpikeTrap(surface, spikeRow)});
          } else if (tileId == 13) {
            // Full Block - solid on all sides, row 3, column = theme
            outData.map[y][x] = new Block(
                (short)x, (short)y,
                {Sprite((short)outData.theme, 3, false, SURFACE_TYPE_OTHER)});
          } else if (tileId >= 1 && tileId <= 12) {
            // Normal Block - apply theme offset (4 columns per theme)
            short sx, sy;
            GetSpriteCoords(tileId, sx, sy);
            sx += 4 * outData.theme;
            SurfaceType surface = GetSurfaceType(tileId);
            outData.map[y][x] =
                new Block((short)x, (short)y, {Sprite(sx, sy, false, surface)});
          }
        } else if (cell.is_array()) {
          // Multi-layer tile
          std::vector<Sprite> sprites;
          std::vector<SpikeTrap> spikeTraps;
          bool hasSpikeWall = false;

          for (const auto &id : cell) {
            if (id.is_number_integer()) {
              int tileId = id.get<int>();

              if (tileId >= 40 && tileId <= 43) {
                hasSpikeWall = true;
                short sx, sy;
                GetSpriteCoords(tileId, sx, sy);
                SurfaceType surface = GetSurfaceType(tileId);
                short spikeRow = GetSpikeAnimRow(tileId);
                sprites.push_back(Sprite(sx, sy, true, surface));
                spikeTraps.push_back(SpikeTrap(surface, spikeRow));
              } else if (tileId == 13) {
                // Full Block in multi-layer
                sprites.push_back(
                    Sprite((short)outData.theme, 3, false, SURFACE_TYPE_OTHER));
              } else {
                bool isDangerous = (tileId >= 20 && tileId <= 31);
                short sx, sy;
                GetSpriteCoords(tileId, sx, sy);
                SurfaceType surface = GetSurfaceType(tileId);
                // Apply theme offset to non-dangerous wall sprites
                if (!isDangerous && sx >= 0) {
                  sx += 4 * outData.theme;
                }
                if (sx >= 0 && sy >= 0) {
                  sprites.push_back(Sprite(sx, sy, isDangerous, surface));
                }
              }
            }
          }

          if (!sprites.empty()) {
            if (hasSpikeWall) {
              outData.map[y][x] =
                  new SpikeWall((short)x, (short)y, sprites, spikeTraps);
            } else {
              outData.map[y][x] = new Block((short)x, (short)y, sprites);
            }
          }
        }
      }
    }
  }

  // Parse enemies
  if (levelJson.contains("enemies") && levelJson["enemies"].is_array()) {
    for (const auto &enemyJson : levelJson["enemies"]) {
      std::string type = enemyJson.value("type", "bat");
      int ex = enemyJson.value("x", 0);
      int ey = enemyJson.value("y", 0);

      if (type == "bat") {
        std::string patrol = enemyJson.value("patrol", "horizontal");
        int range = enemyJson.value("range", 5);
        PatrolDirection dir =
            (patrol == "vertical") ? PATROL_VERTICAL : PATROL_HORIZONTAL;
        outData.enemies.push_back(
            new BatEnemy((short)ex, (short)ey, dir, range));
      }
    }
  }

  return true;
}

} // namespace LevelLoader
