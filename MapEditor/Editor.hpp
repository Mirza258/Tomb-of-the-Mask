#pragma once

#include "framework.h"
#include "json.hpp"
#include <vector>

#define TILE_SIZE 32

#define UNSELECTD_CELL_X_OFFSET 0
#define SELECTD_CELL_X_OFFSET   1
#define UNSELECTD_CELL_Y_OFFSET 7
#define SELECTD_CELL_Y_OFFSET   7

#define UNSELECTED_EMPTY_CELL 0
#define SELECTED_EMPTY_CELL 1
#define UNSELECTED_NONEMPTY_CELL 2
#define SELECTED_NONEMPTY_CELL 3

#define TRANSPARENT_COLOR RGB(255, 0, 255)

#define ID_SELECT_CHECK   2000
#define ID_TEXTURE_CHECK  2001  
#define ID_ENTITY_CHECK   2002
#define ID_DELETE_CHECK   2003
#define ID_CLEAR_BTN      2004
#define ID_PLAYER_BTN     2005
#define ID_EXIT_BTN       2006
#define ID_BROWSE_TEXTURE 2007
#define ID_BROWSE_ENTITY  2008

#define TOOL_SELECT   0
#define TOOL_TEXTURE  1
#define TOOL_ENTITY   2
#define TOOL_DELETE   3

void LoadSprites(HBITMAP&, int);
void CreateMapGrid(std::vector<std::vector<short>>&, unsigned int, unsigned int);
void DeleteMapGrid(std::vector<std::vector<short>>&, nlohmann::json&, unsigned int, unsigned int);

bool hasLeaks(const nlohmann::json&);
void autoAddEdges(nlohmann::json&, std::vector<std::vector<short>>&, POINT&);
bool tileContains(const nlohmann::json&, int);
bool tileContains2(const nlohmann::json&, int, int);
void addTexture(nlohmann::json&, int);
bool backgroundSizeCheck(nlohmann::json&, int, int);
bool fishSizeCheck(nlohmann::json&, int, int);

enum class TileType { TEXTURE, ENTITY };

struct TileData {
    int sheetRow;
    int sheetCol;
    int tileId;
    TileType type;
    bool drawPicker;
};