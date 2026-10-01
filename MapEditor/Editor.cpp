#include "Editor.hpp"
#include <queue>

bool hasLeaks(const nlohmann::json& map_json) {
    int width = map_json["width"].get<int>();
    int height = map_json["height"].get<int>();
    int startX = map_json["playerSpawn"]["x"].get<int>();
    int startY = map_json["playerSpawn"]["y"].get<int>();

    if (startX < 0 || startY < 0 || startX >= width || startY >= height) {
        return false;
    }

    std::vector<std::vector<bool>> visited(height, std::vector<bool>(width, false));

    std::queue<std::pair<int, int>> queue;
    queue.push(std::make_pair(startX, startY));
    visited[startY][startX] = true;

    const int dx[] = { 0, 0, -1, 1 };
    const int dy[] = { -1, 1, 0, 0 };

    while (!queue.empty()) {
        int x = queue.front().first;
        int y = queue.front().second;
        queue.pop();

        if (x == 0 || x == width - 1 || y == 0 || y == height - 1) {
            return true;
        }

        for (int dir = 0; dir < 4; dir++) {
            int nx = x + dx[dir];
            int ny = y + dy[dir];

            if (nx < 0 || nx >= width || ny < 0 || ny >= height) {
                continue;
            }

            if (visited[ny][nx]) {
                continue;
            }

            const auto& tileData = map_json["tiles"][ny][nx];
            bool isPassable = true;

            if (tileData.is_array()) {
                //check all tile in the array
                for (const auto& val : tileData) {
                    int tileId = val.get<int>();
                    if (tileId != 0 && tileId != 50 && tileId != 52 &&
                        tileId != 53 && tileId != 60) {
                        isPassable = false;
                        break;
                    }
                }
            }
            else {
                int tileId = tileData.get<int>();
                if (tileId != 0 && tileId != 50 && tileId != 52 &&
                    tileId != 53 && tileId != 60) {
                    isPassable = false;
                }
            }

            if (isPassable) {
                visited[ny][nx] = true;
                queue.push(std::make_pair(nx, ny));
            }
        }
    }
    return false;
}

void LoadSprites(HBITMAP& hTextureSheet, int number) {
    if (hTextureSheet != NULL) {
        DeleteObject(hTextureSheet);
        hTextureSheet = NULL;
    }

    std::wstring filename = L".\\assets\\mapobjects-sheet" + std::to_wstring(number) + L".bmp";

    hTextureSheet = (HBITMAP)LoadImage(NULL, filename.c_str(), IMAGE_BITMAP, 0, 0,
        LR_LOADFROMFILE);

    if (!hTextureSheet) {
        DWORD error = GetLastError();
        std::wstring errorMsg = L"Can't Find the Sprite! Error: " + std::to_wstring(error);
        MessageBoxW(NULL, errorMsg.c_str(), L"Sprite Loading Error", MB_OK | MB_ICONERROR);
    }
}

void CreateMapGrid(std::vector<std::vector<short>>& map_grid, unsigned int MAP_HEIGHT, unsigned int MAP_WIDTH) {
    map_grid.resize(MAP_HEIGHT);
    for (unsigned int y = 0; y < MAP_HEIGHT; y++) {
        map_grid[y].resize(MAP_WIDTH, UNSELECTED_EMPTY_CELL);
    }
}

void DeleteMapGrid(std::vector<std::vector<short>>& map_grid, nlohmann::json& map_json, unsigned int MAP_HEIGHT, unsigned int MAP_WIDTH) {
    map_grid.clear();

    //reset json
    map_json.clear();
    map_json["name"] = "";
    map_json["nextMap"] = "";
    map_json["width"] = MAP_WIDTH;
    map_json["height"] = MAP_HEIGHT;
    map_json["playerSpawn"]["x"] = -1;
    map_json["playerSpawn"]["y"] = -1;
    map_json["theme"] = 0;

    //create empty tiles
    map_json["tiles"] = nlohmann::json::array();
    for (unsigned int y = 0; y < MAP_HEIGHT; y++) {
        nlohmann::json row = nlohmann::json::array();
        for (unsigned int x = 0; x < MAP_WIDTH; x++) {
            row.push_back(0);
        }
        map_json["tiles"].push_back(row);
    }

    map_json["enemies"] = nlohmann::json::array();
}

bool tileContains(const nlohmann::json& tileData, int id) {
    for (const auto& val : tileData)
        if (val.get<int>() == id) return true;
    return false;
}

bool tileContains2(const nlohmann::json& tileData, int id0, int id1) {
    bool contains0 = false, contains1 = false;
    for (const auto& val : tileData)
        if (val.get<int>() == id0) contains0 = true;
        else if (val.get<int>() == id1) contains1 = true;
    return contains0 && contains1;
}

void addTexture(nlohmann::json& tileData, int id) {
    if (tileData.is_array()) {
        for (const auto& val : tileData) {
            if (val.get<int>() == id) return;
        }
        tileData.push_back(id);
    }
    else {
        int current = tileData.get<int>();
        if (current == 0) {
            tileData = id;
        }
        else {
            tileData = nlohmann::json::array({ current, id });
        }
    }
}

void autoAddEdges(nlohmann::json& map_json, std::vector<std::vector<short>>& map_grid, POINT& selectedPoint) {
    int width = map_json["width"].get<int>();
    int height = map_json["height"].get<int>();

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            auto& tileData = map_json["tiles"][y][x];
            if(tileData.is_array()) {
                if (tileContains2(tileData, 1, 4) && !tileContains(tileData, 9))
                    addTexture(tileData, 9);
                else if (tileContains2(tileData, 2, 4) && !tileContains(tileData, 10))
                    addTexture(tileData, 10);
                else if (tileContains2(tileData, 2, 3) && !tileContains(tileData, 11))
                    addTexture(tileData, 11);
                else if (tileContains2(tileData, 1, 3) && !tileContains(tileData, 12)) {
                    addTexture(tileData, 12);
                }
            }
        }
    }

    int temp_x, temp_y;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            auto& tileData = map_json["tiles"][y][x];

            if (tileContains(tileData, 1)) {
                temp_x = x - 1;
                temp_y = y - 1;

                if (temp_x >= 0 && temp_x < width && temp_y >= 0 && temp_y < height) {
                    auto& tileData0 = map_json["tiles"][temp_y][temp_x];
                    if (tileContains(tileData0, 4)) {
                        addTexture(map_json["tiles"][y][temp_x], 5);
                    }
                }

                temp_x = x + 1;

                if (temp_x >= 0 && temp_x < width && temp_y >= 0 && temp_y < height) {
                    auto& tileData0 = map_json["tiles"][temp_y][temp_x];
                    if (tileContains(tileData0, 3)) {
                        addTexture(map_json["tiles"][y][temp_x], 8);
                    }
                }
            }
            else if (tileContains(tileData, 2)) {
                temp_x = x - 1;
                temp_y = y + 1;

                if (temp_x >= 0 && temp_x < width && temp_y >= 0 && temp_y < height) {
                    auto& tileData0 = map_json["tiles"][temp_y][temp_x];
                    if (tileContains(tileData0, 4)) {
                        addTexture(map_json["tiles"][y][temp_x], 6);
                    }
                }

                temp_x = x + 1;

                if (temp_x >= 0 && temp_x < width && temp_y >= 0 && temp_y < height) {
                    auto& tileData0 = map_json["tiles"][temp_y][temp_x];
                    if (tileContains(tileData0, 3)) {
                        addTexture(map_json["tiles"][y][temp_x], 7);
                    }
                }
            }
            else if (tileContains(tileData, 20)) {
                temp_x = x - 1;
                temp_y = y + 1;

                if (temp_x >= 0 && temp_x < width && temp_y >= 0 && temp_y < height) {
                    auto& tileData0 = map_json["tiles"][temp_y][temp_x];
                    if (tileContains(tileData0, 23)) {
                        addTexture(map_json["tiles"][y][temp_x], 24);
                    }
                }

                temp_x = x + 1;

                if (temp_x >= 0 && temp_x < width && temp_y >= 0 && temp_y < height) {
                    auto& tileData0 = map_json["tiles"][temp_y][temp_x];
                    if (tileContains(tileData0, 22)) {
                        addTexture(map_json["tiles"][y][temp_x], 27);
                    }
                }
            }
            else if (tileContains(tileData, 21)) {
                temp_x = x - 1;
                temp_y = y + 1;

                if (temp_x >= 0 && temp_x < width && temp_y >= 0 && temp_y < height) {
                    auto& tileData0 = map_json["tiles"][temp_y][temp_x];
                    if (tileContains(tileData0, 22)) {
                        addTexture(map_json["tiles"][y][temp_x], 25);
                    }
                }

                temp_x = x + 1;

                if (temp_x >= 0 && temp_x < width && temp_y >= 0 && temp_y < height) {
                    auto& tileData0 = map_json["tiles"][temp_y][temp_x];
                    if (tileContains(tileData0, 23)) {
                        addTexture(map_json["tiles"][y][temp_x], 26);
                    }
                }
            }
        }
    }
}

bool backgroundSizeCheck(nlohmann::json& map_json, int xpos, int ypos) {
    int width = map_json["width"].get<int>();
    int height = map_json["height"].get<int>();

    if (ypos - 1 < 1 || ypos > height - 1)
        return false;

    if (xpos - 2 < 1 || xpos + 2 > width - 1)
        return false;

    for (int i = -2; i < 3; i++)
        if (map_json["tiles"][ypos - 1][xpos + i].is_array())
            return false;
        else if (map_json["tiles"][ypos - 1][xpos + i].get<int>() != 0)
            return false;

    return (map_json["tiles"][ypos][xpos - 1].get<int>() == 0 && map_json["tiles"][ypos][xpos + 1].get<int>() == 0);
}

bool fishSizeCheck(nlohmann::json& map_json, int xpos, int ypos) {
    int width = map_json["width"].get<int>();
    int height = map_json["height"].get<int>();

    if (ypos - 1 < 0 || ypos + 1 >= height)
        return false;

    if (xpos - 1 < 0 || xpos + 1 >= width)
        return false;

    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0)
                continue;

            auto& tileData = map_json["tiles"][ypos + dy][xpos + dx];
            if (tileData.is_array())
                return false;
            if (tileData.get<int>() != 0)
                return false;
        }
    }
    return true;
}