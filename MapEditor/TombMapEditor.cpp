// TombMapEditor.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "TombMapEditor.h"
#include "Editor.hpp"
#include "json.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <climits>
#include <utility>
#include <algorithm>
#include <vector>
#include <commdlg.h>

#ifndef min
#define min(a,b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

#define MAX_LOADSTRING 100

//dorward declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK SecondWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK TextureBrowserProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK EntityBrowserProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK BatDialogProc(HWND, UINT, WPARAM, LPARAM);

INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK NewMapDialogProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK SettingsDialogProc(HWND, UINT, WPARAM, LPARAM);

ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);

//window class names
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];
WCHAR szSecondClass[MAX_LOADSTRING] = L"klasa";
WCHAR szTextureClass[MAX_LOADSTRING] = L"TextureBrowserClass";
WCHAR szEntityClass[MAX_LOADSTRING] = L"EntityBrowserClass";

HINSTANCE hInst;
HWND hwndFirst = nullptr;
HWND hwndSecond = nullptr;
HWND hTextureBrowser = nullptr;
HWND hEntityBrowser = nullptr;

//map data
unsigned int MAP_WIDTH = 0, MAP_HEIGHT = 0;
std::vector<std::vector<short>> map_grid;
nlohmann::json map_json;

//tool and tile data
std::vector<TileData> textureTiles = {
    {0, 0, 1, TileType::TEXTURE, true},
    {0, 1, 2, TileType::TEXTURE},
    {0, 2, 3, TileType::TEXTURE},
    {0, 3, 4, TileType::TEXTURE},
    //Full wall^
    {1, 0, 5, TileType::TEXTURE, true},
    {1, 1, 6, TileType::TEXTURE},
    {1, 2, 7, TileType::TEXTURE},
    {1, 3, 8, TileType::TEXTURE},
    //Wall outer corner^
    {2, 0, 9, TileType::TEXTURE, true},
    {2, 1, 10, TileType::TEXTURE},
    {2, 2, 11, TileType::TEXTURE},
    {2, 3, 12, TileType::TEXTURE},
    //Wall inner corner^
    {8, 0, 13, TileType::TEXTURE, true},
    //Full block^
    {3, 0, 20, TileType::TEXTURE, true},
    {3, 1, 21, TileType::TEXTURE},
    {3, 2, 22, TileType::TEXTURE},
    {3, 3, 23, TileType::TEXTURE},
    //Spike full wall^
    {4, 0, 24, TileType::TEXTURE, true},
    {4, 1, 25, TileType::TEXTURE},
    {4, 2, 26, TileType::TEXTURE},
    {4, 3, 27, TileType::TEXTURE}
    //Spike wall outer corner^
};

std::vector<TileData> entityTiles = {
    {5, 0, 40, TileType::ENTITY, true},
    {5, 1, 41, TileType::ENTITY},
    {5, 2, 42, TileType::ENTITY},
    {5, 3, 43, TileType::ENTITY},
    //Expanding spike wall^
    {6, 0, 50, TileType::ENTITY, true}, //collectable
    {6, 1, 52, TileType::ENTITY, true}, //coin
    {6, 2, 51, TileType::ENTITY, true}, //exit
    {6, 3, 53, TileType::ENTITY, true}, //star
    {8, 1, 55, TileType::ENTITY, true}, //fish
    //--------------------------------//
    {9, 0, 60, TileType::ENTITY, true},
    {9, 1, 61, TileType::ENTITY},
    {9, 2, 62, TileType::ENTITY},
    {9, 3, 63, TileType::ENTITY},
    {7, 2, 70, TileType::ENTITY, true}, //bat
    {7, 3, 71, TileType::ENTITY, true}  //player
};

// Current state variables
unsigned int current_tool = 0;
unsigned int current_tile = 1;
unsigned int current_entity = 40;
unsigned int xPos, yPos;
unsigned int map_theme;
unsigned int number_of_stars = 0;

bool map_started = false;
bool valid_player = false;
bool multi_select = false;

//settings
bool show_gridlines = true;
bool enable_paintover = true;
bool show_toolbox = false;
bool show_texture_browser = false;
bool show_entity_browser = false;
bool saftey_check = true;

//Position variables
POINT mouse_position = { -1, -1 };
POINT mouse_position2 = { -1, -1 };
POINT mouse_position2Prev = { -1, -1 };

POINT texture_position = { 0, 0 };
POINT entity_position = { 0, 5 };
POINT current_offset = { 0, 0 };

//Resources
HBITMAP hTextureSheet = NULL;

HBITMAP hTextureSheet0 = NULL;
HBITMAP hTextureSheet1 = NULL;
HBITMAP hTextureSheet2 = NULL;
HBITMAP hTextureSheet3 = NULL;

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
    (void)hPrevInstance;
    (void)lpCmdLine;

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_TOMBMAPEDITOR, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TOMBMAPEDITOR));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TOMBMAPEDITOR));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_TOMBMAPEDITOR);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    RegisterClassExW(&wcex);

    // Register second window class
    wcex.lpfnWndProc = SecondWndProc;
    wcex.hbrBackground = CreateSolidBrush(RGB(240, 240, 245));
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = szSecondClass;
    RegisterClassExW(&wcex);

    // Register texture browser window class
    wcex.lpfnWndProc = TextureBrowserProc;
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName = szTextureClass;
    RegisterClassExW(&wcex);

    // Register entity browser window class
    wcex.lpfnWndProc = EntityBrowserProc;
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName = szEntityClass;
    RegisterClassExW(&wcex);

    return 1;
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance;

    //Calculate required window size for 24x16 tiles
    int clientWidth = 24 * TILE_SIZE;
    int clientHeight = 16 * TILE_SIZE;

    RECT rc = { 0, 0, clientWidth, clientHeight };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, TRUE);

    // Create main window
    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, //x, y position
        rc.right - rc.left, //width (adjusted)
        rc.bottom - rc.top,  //height (adjusted)
        nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    RECT mainRect;
    GetWindowRect(hWnd, &mainRect);

    HWND hSecondWnd = CreateWindowW(szSecondClass, L"Tile Palette",
        WS_OVERLAPPEDWINDOW,
        mainRect.right + 10,  
        mainRect.top, 
        350, 400,  
        nullptr, nullptr, hInstance, nullptr);

    hTextureBrowser = CreateWindowW(szTextureClass, L"Texture Browser",
        WS_OVERLAPPEDWINDOW,
        mainRect.right + 10,
        mainRect.top,
        400, 300,
        nullptr, nullptr, hInst, nullptr);

    hEntityBrowser = CreateWindowW(szEntityClass, L"Entity Browser",
        WS_OVERLAPPEDWINDOW,
        mainRect.right + 10,
        mainRect.top + 320,
        400, 300,
        nullptr, nullptr, hInst, nullptr);

    if (hSecondWnd)
        UpdateWindow(hSecondWnd);

    return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE: {
        InvalidateRect(hWnd, nullptr, false);

        LoadSprites(hTextureSheet0, 0);
        LoadSprites(hTextureSheet1, 1);
        LoadSprites(hTextureSheet2, 2);
        LoadSprites(hTextureSheet3, 3);

        hTextureSheet = hTextureSheet0;

        CreateMapGrid(map_grid, MAP_HEIGHT, MAP_WIDTH);

        hwndFirst = hWnd;
        break;
    }
    case WM_ERASEBKGND:
        return 1;

    case WM_KEYDOWN: {
        bool ctrlPressed = GetKeyState(VK_CONTROL) & 0x8000;

        switch (wParam) {
        case VK_UP:
            if (current_offset.y > 0) {
                current_offset.y--;
                InvalidateRect(hWnd, nullptr, false);
            }
            break;
        case VK_DOWN:
            if (MAP_HEIGHT > 16 && current_offset.y < (MAP_HEIGHT - 16)) {
                current_offset.y++;
                InvalidateRect(hWnd, nullptr, false);
            }
            break;
        case VK_LEFT:
            if (current_offset.x > 0) {
                current_offset.x--;
                InvalidateRect(hWnd, nullptr, false);
            }
            break;
        case VK_RIGHT:
            if (MAP_WIDTH > 24 && current_offset.x < (MAP_WIDTH - 24)) {
                current_offset.x++;
                InvalidateRect(hWnd, nullptr, false);
            }
            break;
        case VK_SHIFT:
            if (current_tool == TOOL_SELECT && mouse_position.x != -1 && (GetKeyState('E') & 0x8000)) {
                auto& tileData = map_json["tiles"][mouse_position.y][mouse_position.x];

                if (tileData.is_array()) {
                    int tileId = tileData[0].get<int>();

                    bool found = false;
                    for (auto& t : textureTiles) {
                        if (t.tileId == tileId) {
                            if (t.sheetRow < 3) {
                                MessageBox(NULL, L"This tile contains a regular wall.", L"Tile Information", MB_OK);
                            }
                            else if (t.sheetRow < 5) {
                                MessageBox(NULL, L"This tile contains a deadly wall.", L"Tile Information", MB_OK);
                            }
                            else if (t.sheetRow < 6) {
                                MessageBox(NULL, L"This tile contains an expanding spike wall.", L"Tile Information", MB_OK);
                            }
                            found = true;
                            break;
                        }
                    }

                    if (!found) {
                        for (auto& e : entityTiles) {
                            if (e.tileId == tileId) {
                                if (e.sheetRow == 6) {
                                    if (e.sheetCol == 0 || e.sheetCol == 1) {
                                        MessageBox(NULL, L"This tile contains a coin.", L"Tile Information", MB_OK);
                                    }
                                    else if (e.sheetCol == 2) {
                                        MessageBox(NULL, L"This tile contains an exit.", L"Tile Information", MB_OK);
                                    }
                                    else {
                                        MessageBox(NULL, L"This tile contains a star.", L"Tile Information", MB_OK);
                                    }
                                }
                                else if (e.sheetRow == 7) {
                                    if (e.sheetCol == 2) {
                                        for (auto& enemy : map_json["enemies"]) {
                                            if (enemy["x"] == mouse_position.x && enemy["y"] == mouse_position.y && enemy["type"] == "bat") {
                                                HWND hDialog = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_BAT_DIALOG), hWnd, BatDialogProc, (LPARAM)&enemy);
                                                ShowWindow(hDialog, SW_SHOW);
                                                break;
                                            }
                                        }
                                    }
                                    else if (e.sheetCol == 3) {
                                        MessageBox(NULL, L"This tile contains a player spawn.", L"Tile Information", MB_OK);
                                    }
                                }
                                break;
                            }
                        }
                    }
                }
                else {
                    //if it isn't an array
                    int tileId = tileData.get<int>();

                    if (tileId != 0) {
                        bool found = false;
                        for (auto& t : textureTiles) { //gore gleda teksture
                            if (t.tileId == tileId) {
                                if (t.sheetRow < 3) {
                                    MessageBox(NULL, L"This tile contains a regular wall.", L"Tile Information", MB_OK);
                                }
                                else if (t.sheetRow < 5) {
                                    MessageBox(NULL, L"This tile contains a deadly wall.", L"Tile Information", MB_OK);
                                }
                                else if (t.sheetRow < 6) {
                                    MessageBox(NULL, L"This tile contains an expanding spike wall.", L"Tile Information", MB_OK);
                                }
                                found = true;
                                break;
                            }
                        }

                        if (!found) {
                            for (auto& e : entityTiles) { //a ako ne nadje trazi entitete
                                if (e.tileId == tileId) {
                                    if (e.sheetRow == 6) {
                                        if (e.sheetCol == 0 || e.sheetCol == 1) {
                                            MessageBox(NULL, L"This tile contains a coin.", L"Tile Information", MB_OK);
                                        }
                                        else if (e.sheetCol == 2) {
                                            MessageBox(NULL, L"This tile contains an exit.", L"Tile Information", MB_OK);
                                        }
                                        else {
                                            MessageBox(NULL, L"This tile contains a star.", L"Tile Information", MB_OK);
                                        }
                                    }
                                    else if (e.sheetRow == 7) {
                                        if (e.sheetCol == 2) {
                                            for (auto& enemy : map_json["enemies"]) {
                                                if (enemy["x"] == mouse_position.x && enemy["y"] == mouse_position.y && enemy["type"] == "bat") {
                                                    HWND hDialog = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_BAT_DIALOG), hWnd, BatDialogProc, (LPARAM)&enemy);
                                                    ShowWindow(hDialog, SW_SHOW);
                                                    break;
                                                }
                                            }
                                        }
                                        else if (e.sheetCol == 3) {
                                            MessageBox(NULL, L"This tile contains a player spawn.", L"Tile Information", MB_OK);
                                        }
                                    }
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            break;
        case 'D':
            if (ctrlPressed && current_tool == TOOL_SELECT && (mouse_position.x != -1 || multi_select)) {
                if (multi_select && mouse_position2.x != -1 && mouse_position2.y != -1) {
                    int startX = min(mouse_position.x, mouse_position2.x);
                    int endX = max(mouse_position.x, mouse_position2.x);
                    int startY = min(mouse_position.y, mouse_position2.y);
                    int endY = max(mouse_position.y, mouse_position2.y);

                    for (int y = startY; y <= endY; y++) {
                        for (int x = startX; x <= endX; x++) { 
                            if (map_json["tiles"][y][x] == 70) { //ako naleti na sismisa
                                auto& enemies = map_json["enemies"];
                                for (auto it = enemies.begin(); it != enemies.end(); ) {
                                    if ((*it)["type"] == "bat" &&
                                        (*it)["x"] == x &&
                                        (*it)["y"] == y) {
                                        it = enemies.erase(it);
                                    }
                                    else {
                                        ++it;
                                    }
                                }
                            }

                            else if (map_json["tiles"][y][x] == 71) { //ako naleti na player-a
                                if (map_json["playerSpawn"]["x"] == x && map_json["playerSpawn"]["y"] == y) {
                                    map_json["playerSpawn"]["x"] = -1;
                                    map_json["playerSpawn"]["y"] = -1;
                                    valid_player = false;
                                }
                            }
                            // Handle star count
                            else if (map_json["tiles"][y][x] == 53) {//na zvjezdu
                                number_of_stars--;
                            }


                            map_json["tiles"][y][x] = 0;

                            // deselekcija u map_grid
                            if (map_grid[y][x] == SELECTED_EMPTY_CELL || map_grid[y][x] == SELECTED_NONEMPTY_CELL) {
                                map_grid[y][x] = (map_grid[y][x] == SELECTED_EMPTY_CELL) ? UNSELECTED_EMPTY_CELL : UNSELECTED_NONEMPTY_CELL;
                            }
                        }
                    }

                    // reseta sve pozicije miseva
                    mouse_position.x = -1;
                    mouse_position.y = -1;
                    mouse_position2.x = -1;
                    mouse_position2.y = -1;
                    mouse_position2Prev.x = -1;
                    mouse_position2Prev.y = -1;
                    multi_select = false;
                }
                else if (mouse_position.x != -1) {
                    //za brisanje ako je jedna selekcija
                    if (map_json["tiles"][mouse_position.y][mouse_position.x] == 70) {
                        auto& enemies = map_json["enemies"];
                        for (auto it = enemies.begin(); it != enemies.end(); ++it) {
                            if ((*it)["type"] == "bat" &&
                                (*it)["x"] == mouse_position.x &&
                                (*it)["y"] == mouse_position.y) {
                                enemies.erase(it);
                                break;
                            }
                        }
                    }
                    else if (map_json["tiles"][mouse_position.y][mouse_position.x] == 71) {
                        map_json["playerSpawn"]["x"] = -1;
                        map_json["playerSpawn"]["y"] = -1;
                        valid_player = false;
                    }

                    if (map_json["tiles"][mouse_position.y][mouse_position.x] == 53)
                        number_of_stars--;

                    map_json["tiles"][mouse_position.y][mouse_position.x] = 0;

                    // deselekcija
                    if (map_grid[mouse_position.y][mouse_position.x] == SELECTED_EMPTY_CELL ||
                        map_grid[mouse_position.y][mouse_position.x] == SELECTED_NONEMPTY_CELL) {
                        map_grid[mouse_position.y][mouse_position.x] = (map_grid[mouse_position.y][mouse_position.x] == SELECTED_EMPTY_CELL) ?
                            UNSELECTED_EMPTY_CELL : UNSELECTED_NONEMPTY_CELL;
                    }

                    // Reset selection state
                    mouse_position.x = -1;
                    mouse_position.y = -1;
                    mouse_position2.x = -1;
                    mouse_position2.y = -1;
                    mouse_position2Prev.x = -1;
                    mouse_position2Prev.y = -1;
                    multi_select = false;
                }

                InvalidateRect(hWnd, nullptr, false);
            }
            break;

        case 'R':
            if (ctrlPressed && !multi_select) {
                switch (current_tool) {
                case TOOL_ENTITY:
                    // cirkulise kroz 0-3 po kolonama
                    if (entity_position.y == 5) {
                        entity_position.x = (entity_position.x < 3) ? entity_position.x + 1 : 0;
                        current_entity = 40 + entity_position.x;
                        InvalidateRect(hwndSecond, nullptr, true);
                    }
                    else if (entity_position.y == 9) {
                        entity_position.x = (entity_position.x < 3) ? entity_position.x + 1 : 0;
                        current_entity = 60 + entity_position.x;
                        InvalidateRect(hwndSecond, nullptr, true);
                    }
                    break;

                case TOOL_TEXTURE:
                    if (texture_position.y == 0) { //Full wall (1-4)
                        texture_position.x = (texture_position.x < 3) ? texture_position.x + 1 : 0;
                        current_tile = 1 + texture_position.x;
                    }
                    else if (texture_position.y == 1) { //Wall outer corner (5-8)
                        texture_position.x = (texture_position.x < 3) ? texture_position.x + 1 : 0;
                        current_tile = 5 + texture_position.x;
                    }
                    else if (texture_position.y == 2) { //Wall inner corner (9-12)
                        texture_position.x = (texture_position.x < 3) ? texture_position.x + 1 : 0;
                        current_tile = 9 + texture_position.x;
                    }
                    else if (texture_position.y == 3) { //spike full wall (20-23)
                        texture_position.x = (texture_position.x < 3) ? texture_position.x + 1 : 0;
                        current_tile = 20 + texture_position.x;
                    }
                    else if (texture_position.y == 4) { //spike wall outer corner (24-27)
                        texture_position.x = (texture_position.x < 3) ? texture_position.x + 1 : 0;
                        current_tile = 24 + texture_position.x;
                    }

                    InvalidateRect(hwndSecond, nullptr, true);
                    break;

                case TOOL_SELECT:
                    if (mouse_position.x != -1) {
                        auto& tileData = map_json["tiles"][mouse_position.y][mouse_position.x];

                        if (!tileData.is_array()) {
                            int tileId = tileData.get<int>();

                            if (tileId >= 1 && tileId <= 4) { // Full wall
                                int newTile = (tileId - 1 + 1) % 4 + 1;
                                map_json["tiles"][mouse_position.y][mouse_position.x] = newTile;
                            }
                            else if (tileId >= 5 && tileId <= 8) { // Wall outer corner
                                int newTile = (tileId - 5 + 1) % 4 + 5;
                                map_json["tiles"][mouse_position.y][mouse_position.x] = newTile;
                            }
                            else if (tileId >= 9 && tileId <= 12) { // Wall inner corner
                                int newTile = (tileId - 9 + 1) % 4 + 9;
                                map_json["tiles"][mouse_position.y][mouse_position.x] = newTile;
                            }
                            else if (tileId >= 20 && tileId <= 23) { // Spike full wall
                                int newTile = (tileId - 20 + 1) % 4 + 20;
                                map_json["tiles"][mouse_position.y][mouse_position.x] = newTile;
                            }
                            else if (tileId >= 24 && tileId <= 27) { // Spike wall outer corner
                                int newTile = (tileId - 24 + 1) % 4 + 24;
                                map_json["tiles"][mouse_position.y][mouse_position.x] = newTile;
                            }
                            else if (tileId >= 40 && tileId <= 43) {
                                int newTile = (tileId - 40 + 1) % 4 + 40;
                                map_json["tiles"][mouse_position.y][mouse_position.x] = newTile;
                            }
                            else if (tileId >= 60 && tileId <= 63) {
                                int newTile = (tileId - 60 + 1) % 4 + 60;
                                map_json["tiles"][mouse_position.y][mouse_position.x] = newTile;
                            }
                        }
                    }
                    InvalidateRect(hWnd, nullptr, true);
                    break;
                }
            }
            break;
        }
        break;
    }

    case WM_COMMAND: {
        switch (LOWORD(wParam)) {
        case IDM_SAVE_MAP: {
            if (map_started == false) {
                MessageBoxA(hWnd, "There isn't a map to be saved.", "Error", MB_OK | MB_ICONERROR);
                break;
            }
            if (valid_player == false) {
                MessageBoxA(hWnd, "Player spawn is required.", "Error", MB_OK | MB_ICONERROR);
                break;
            }
            if (hasLeaks(map_json) && saftey_check) {
                MessageBoxA(hWnd, "This map contains leaks.", "Error", MB_OK | MB_ICONERROR);
                break;
            }

            OPENFILENAMEW ofn;
            wchar_t szFile[MAX_PATH] = L"level.json";

            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hWnd;
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = sizeof(szFile);
            ofn.lpstrFilter = L"JSON Files (*.json)\0*.json\0All Files (*.*)\0*.*\0";
            ofn.nFilterIndex = 1;
            ofn.lpstrDefExt = L"json";
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

            if (GetSaveFileNameW(&ofn)) {
                std::ofstream file(ofn.lpstrFile);
                if (file.is_open()) {
                    std::stringstream ss;
                    ss << "{\n";
                    ss << "  \"name\": \"" << map_json["name"].get<std::string>() << "\",\n";
                    ss << "  \"nextMap\": \"" << map_json["nextMap"].get<std::string>() << "\",\n";
                    ss << "  \"theme\": " << map_json["theme"].get<int>() << ",\n";
                    ss << "  \"width\": " << map_json["width"].get<int>() << ",\n";
                    ss << "  \"height\": " << map_json["height"].get<int>() << ",\n";
                    ss << "  \"playerSpawn\": " << map_json["playerSpawn"].dump() << ",\n";
                    ss << "  \"tiles\": [\n";

                    auto& tiles = map_json["tiles"];
                    for (size_t i = 0; i < tiles.size(); i++) {
                        ss << "    " << tiles[i].dump();
                        if (i < tiles.size() - 1) {
                            ss << ",";
                        }
                        ss << "\n";
                    }

                    ss << "  ],\n";
                    ss << "  \"enemies\": " << map_json["enemies"].dump() << "\n";
                    ss << "}";

                    file << ss.str();
                    file.close();
                }
            }
            break;
        }
        case IDM_LOAD_MAP: {
            int result;

            result = (map_started) ? MessageBox(hWnd, L"Do you want to save the current map?", L"Load Map", MB_YESNOCANCEL) : IDNO;

            if (result == IDCANCEL)
                break;

            if (result == IDYES) {
                // Trigger save
                if (map_started == false) {
                    MessageBoxA(hWnd, "There isn't a map to be saved.", "Error", MB_OK | MB_ICONERROR);
                    break;
                }

                if (valid_player == false) {
                    MessageBoxA(hWnd, "Player spawn is required.", "Error", MB_OK | MB_ICONERROR);
                    break;
                }

                if (hasLeaks(map_json) && saftey_check) {
                    MessageBoxA(hWnd, "This map contains leaks.", "Error", MB_OK | MB_ICONERROR);
                    break;
                }

                OPENFILENAMEW ofn;
                wchar_t szFile[MAX_PATH] = L"level.json";

                ZeroMemory(&ofn, sizeof(ofn));
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = hWnd;
                ofn.lpstrFile = szFile;
                ofn.nMaxFile = sizeof(szFile);
                ofn.lpstrFilter = L"JSON Files (*.json)\0*.json\0All Files (*.*)\0*.*\0";
                ofn.nFilterIndex = 1;
                ofn.lpstrDefExt = L"json";
                ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

                if (GetSaveFileNameW(&ofn)) {
                    std::ofstream file(ofn.lpstrFile);
                    if (file.is_open()) {
                        std::stringstream ss;
                        ss << "{\n";
                        ss << "  \"name\": \"" << map_json["name"].get<std::string>() << "\",\n";
                        ss << "  \"nextMap\": \"" << map_json["nextMap"].get<std::string>() << "\",\n";
                        ss << "  \"theme\": " << map_json["theme"].get<int>() << ",\n";
                        ss << "  \"width\": " << map_json["width"].get<int>() << ",\n";
                        ss << "  \"height\": " << map_json["height"].get<int>() << ",\n";
                        ss << "  \"playerSpawn\": " << map_json["playerSpawn"].dump() << ",\n";
                        ss << "  \"tiles\": [\n";

                        auto& tiles = map_json["tiles"];
                        for (size_t i = 0; i < tiles.size(); i++) {
                            ss << "    " << tiles[i].dump();
                            if (i < tiles.size() - 1) {
                                ss << ",";
                            }
                            ss << "\n";
                        }

                        ss << "  ],\n";
                        ss << "  \"enemies\": " << map_json["enemies"].dump() << "\n";
                        ss << "}";

                        file << ss.str();
                        file.close();
                    }
                }
            }

            //dialog za ucitavanje mape
            OPENFILENAMEW ofnLoad;
            wchar_t szLoadFile[MAX_PATH] = L"";

            ZeroMemory(&ofnLoad, sizeof(ofnLoad));
            ofnLoad.lStructSize = sizeof(ofnLoad);
            ofnLoad.hwndOwner = hWnd;
            ofnLoad.lpstrFile = szLoadFile;
            ofnLoad.nMaxFile = sizeof(szLoadFile);
            ofnLoad.lpstrFilter = L"JSON Files (*.json)\0*.json\0All Files (*.*)\0*.*\0";
            ofnLoad.nFilterIndex = 1;
            ofnLoad.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

            if (GetOpenFileNameW(&ofnLoad)) {
                std::ifstream file(szLoadFile);
                if (file.is_open()) {
                    try {
                        nlohmann::json loaded_json;
                        file >> loaded_json;
                        file.close();

                        //provjeri jel dobar format
                        if (!loaded_json.contains("name") || !loaded_json.contains("width") ||
                            !loaded_json.contains("height") || !loaded_json.contains("playerSpawn") ||
                            !loaded_json.contains("tiles") || !loaded_json.contains("enemies") || !loaded_json.contains("theme")) {
                            MessageBoxA(hWnd, "Invalid map file: Missing required fields.", "Error", MB_OK | MB_ICONERROR);
                            break;
                        }

                        //dimenzije i tema
                        int newWidth = loaded_json["width"].get<int>();
                        int newHeight = loaded_json["height"].get<int>();
                        map_theme = loaded_json["theme"].get<int>();

                        if (newWidth <= 0 || newHeight <= 0) {
                            MessageBoxA(hWnd, "Invalid map dimensions.", "Error", MB_OK | MB_ICONERROR);
                            break;
                        }

                        //ocisti staro
                        DeleteMapGrid(map_grid, map_json, MAP_HEIGHT, MAP_WIDTH);

                        MAP_WIDTH = newWidth;
                        MAP_HEIGHT = newHeight;

                        CreateMapGrid(map_grid, MAP_HEIGHT, MAP_WIDTH);

                        //prebaci vanjsi json u interni
                        map_json = loaded_json;
                        valid_player = false;

                        for (int y = 0; y < MAP_HEIGHT; y++) {
                            for (int x = 0; x < MAP_WIDTH; x++) {
                                map_grid[y][x] = UNSELECTED_EMPTY_CELL;

                                auto& tileData = map_json["tiles"][y][x];

                                if (tileData.is_array()) {
                                    //ako ima vise od jednog id-a u istom tile-u
                                    for (auto& val : tileData) {
                                        int tileId = val.get<int>();

                                        //trazi teksture
                                        bool found = false;
                                        for (auto& t : textureTiles) {
                                            if (t.tileId == tileId) {
                                                map_grid[y][x] = UNSELECTED_NONEMPTY_CELL;
                                                found = true;
                                                break;
                                            }
                                        }

                                        if (!found) {
                                            //trazie entitete
                                            for (auto& e : entityTiles) {
                                                if (e.tileId == tileId) {
                                                    map_grid[y][x] = UNSELECTED_NONEMPTY_CELL;

                                                    if (e.tileId == 71) {
                                                        valid_player = true;
                                                    }
                                                    break;
                                                }
                                            }
                                        }
                                    }
                                }
                                else {
                                    //ako je samo jedno
                                    int tileId = tileData.get<int>();

                                    if (tileId == 0) {
                                        map_grid[y][x] = UNSELECTED_EMPTY_CELL;
                                    }
                                    else {
                                        bool found = false;
                                        for (auto& t : textureTiles) {
                                            if (t.tileId == tileId) {
                                                map_grid[y][x] = UNSELECTED_NONEMPTY_CELL;
                                                found = true;
                                                break;
                                            }
                                        }

                                        if (!found) {
                                            for (auto& e : entityTiles) {
                                                if (e.tileId == tileId) {
                                                    map_grid[y][x] = UNSELECTED_NONEMPTY_CELL;

                                                    if (e.tileId == 71) {
                                                        valid_player = true;
                                                    }
                                                    break;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        //ocisti miseve
                        current_offset = { 0, 0 };
                        mouse_position = { -1, -1 };
                        mouse_position2 = { -1, -1 };
                        mouse_position2Prev = { -1, -1 };

                        //dodaj sismise
                        for (auto& enemy : map_json["enemies"]) {
                            if (enemy["type"] == "bat") {
                                int x = enemy["x"];
                                int y = enemy["y"];
                                bool batFound = false;
                                if (!batFound) {
                                    map_grid[y][x] = UNSELECTED_NONEMPTY_CELL;
                                }
                            }
                        }

                        //dodaj igraca
                        if (map_json["playerSpawn"]["x"] != 0 || map_json["playerSpawn"]["y"] != 0) {
                            int x = map_json["playerSpawn"]["x"];
                            int y = map_json["playerSpawn"]["y"];
                            bool spawnFound = false;
                            if (!spawnFound) {
                                map_grid[y][x] = UNSELECTED_NONEMPTY_CELL;
                                valid_player = true;
                            }
                        }

                        //odredi bitmapu za temu
                        switch (map_theme) {
                        case 0:
                            hTextureSheet = hTextureSheet0;
                            break;
                        case 1:
                            hTextureSheet = hTextureSheet1;
                            break;
                        case 2:
                            hTextureSheet = hTextureSheet2;
                            break;
                        case 3:
                            hTextureSheet = hTextureSheet3;
                            break;
                        }

                        InvalidateRect(hWnd, nullptr, TRUE);
                        UpdateWindow(hWnd);

                        map_started = true;
                        show_toolbox = true;

                        CheckMenuItem(GetMenu(hWnd), IDM_SHOW_TOOLBOX, MF_BYCOMMAND | MF_CHECKED);
                        ShowWindow(hwndSecond, SW_SHOW);

                        MessageBoxA(hWnd, "Map loaded successfully.", "Success", MB_OK);
                    }
                    catch (const std::exception& e) {
                        MessageBoxA(hWnd, ("Error parsing JSON: " + std::string(e.what())).c_str(), "Error", MB_OK | MB_ICONERROR);
                    }
                }
                else {
                    MessageBoxA(hWnd, "Could not open file.", "Error", MB_OK | MB_ICONERROR);
                }
            }

            break;
        }
        case IDM_NEW_MAP: {
            int result;
                
            result = (map_started) ? MessageBox(hWnd, L"Do you want to save the current map?", L"New Map", MB_YESNOCANCEL) : IDNO;

            if (result == IDCANCEL)
                break;

            if (result == IDYES) {
                if (map_started == false) {
                    MessageBoxA(hWnd, "There isn't a map to be saved.", "Error", MB_OK | MB_ICONERROR);
                    break;
                }

                if (valid_player == false) {
                    MessageBoxA(hWnd, "Player spawn is required.", "Error", MB_OK | MB_ICONERROR);
                    break;
                }

                if (hasLeaks(map_json) && saftey_check) {
                    MessageBoxA(hWnd, "This map contains leaks.", "Error", MB_OK | MB_ICONERROR);
                    break;
                }

                OPENFILENAMEW ofn;
                wchar_t szFile[MAX_PATH] = L"level.json";

                ZeroMemory(&ofn, sizeof(ofn));
                ofn.lStructSize = sizeof(ofn);
                ofn.hwndOwner = hWnd;
                ofn.lpstrFile = szFile;
                ofn.nMaxFile = sizeof(szFile);
                ofn.lpstrFilter = L"JSON Files (*.json)\0*.json\0All Files (*.*)\0*.*\0";
                ofn.nFilterIndex = 1;
                ofn.lpstrDefExt = L"json";
                ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

                if (GetSaveFileNameW(&ofn)) {
                    std::ofstream file(ofn.lpstrFile);
                    if (file.is_open()) {
                        std::stringstream ss;
                        ss << "{\n";
                        ss << "  \"name\": \"" << map_json["name"].get<std::string>() << "\",\n";
                        ss << "  \"nextMap\": \"" << map_json["nextMap"].get<std::string>() << "\",\n";
                        ss << "  \"theme\": " << map_json["theme"].get<int>() << ",\n";
                        ss << "  \"width\": " << map_json["width"].get<int>() << ",\n";
                        ss << "  \"height\": " << map_json["height"].get<int>() << ",\n";
                        ss << "  \"playerSpawn\": " << map_json["playerSpawn"].dump() << ",\n";
                        ss << "  \"tiles\": [\n";

                        auto& tiles = map_json["tiles"];
                        for (size_t i = 0; i < tiles.size(); i++) {
                            ss << "    " << tiles[i].dump();
                            if (i < tiles.size() - 1) {
                                ss << ",";
                            }
                            ss << "\n";
                        }

                        ss << "  ],\n";
                        ss << "  \"enemies\": " << map_json["enemies"].dump() << "\n";
                        ss << "}";

                        file << ss.str();
                        file.close();
                    }
                }
            }

            HWND hDialog = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_NEWMAP_DIALOG), hWnd, NewMapDialogProc, 0);
            ShowWindow(hDialog, SW_SHOW);
            break;
        }
        case IDM_OVERLAPPING: {
            HMENU hMenu = GetMenu(hWnd);
            UINT state = GetMenuState(hMenu, IDM_OVERLAPPING, MF_BYCOMMAND);
            if (state & MF_CHECKED) {
                CheckMenuItem(hMenu, IDM_OVERLAPPING, MF_BYCOMMAND | MF_UNCHECKED);
                enable_paintover = false;
            }
            else {
                CheckMenuItem(hMenu, IDM_OVERLAPPING, MF_BYCOMMAND | MF_CHECKED);
                enable_paintover = true;
            }
            break;
        }
        case IDM_SHOW_GRIDLINES: {
            HMENU hMenu = GetMenu(hWnd);
            UINT state = GetMenuState(hMenu, IDM_SHOW_GRIDLINES, MF_BYCOMMAND);
            if (state & MF_CHECKED) {
                CheckMenuItem(hMenu, IDM_SHOW_GRIDLINES, MF_BYCOMMAND | MF_UNCHECKED);
                show_gridlines = false;
            }
            else {
                CheckMenuItem(hMenu, IDM_SHOW_GRIDLINES, MF_BYCOMMAND | MF_CHECKED);
                show_gridlines = true;
            }
            InvalidateRect(hWnd, nullptr, false);
            break;
        }
        case IDM_SHOW_TOOLBOX: {
            if (map_started) {
                HMENU hMenu = GetMenu(hWnd);
                UINT state = GetMenuState(hMenu, IDM_SHOW_TOOLBOX, MF_BYCOMMAND);
                if (state & MF_CHECKED) {
                    CheckMenuItem(hMenu, IDM_SHOW_TOOLBOX, MF_BYCOMMAND | MF_UNCHECKED);
                    ShowWindow(hwndSecond, SW_HIDE);
                    show_toolbox = false;
                }
                else {
                    CheckMenuItem(hMenu, IDM_SHOW_TOOLBOX, MF_BYCOMMAND | MF_CHECKED);
                    ShowWindow(hwndSecond, SW_SHOW);
                    show_toolbox = true;
                }
            }
            break;
        }
        case IDM_CHECK: {
            HMENU hMenu = GetMenu(hWnd);
            UINT state = GetMenuState(hMenu, IDM_CHECK, MF_BYCOMMAND);
            if (state & MF_CHECKED) {
                CheckMenuItem(hMenu, IDM_CHECK, MF_BYCOMMAND | MF_UNCHECKED);
                saftey_check = false;
            }
            else {
                CheckMenuItem(hMenu, IDM_CHECK, MF_BYCOMMAND | MF_CHECKED);
                saftey_check = true;
            }
            InvalidateRect(hWnd, nullptr, false);
            break;
        }
        case IDM_CORNERS:
            if (map_started) {
                autoAddEdges(map_json, map_grid, mouse_position);
                InvalidateRect(hWnd, nullptr, true);
                MessageBoxA(hWnd, "Corners have been filled in.", "Task Completed", MB_OK);
            }
            break;
        case IDM_LEAKS:
            if (map_started && valid_player) {
                if(hasLeaks(map_json))
                    MessageBoxA(hWnd, "This map contains leaks.", "Error", MB_OK | MB_ICONERROR);
                else
                    MessageBoxA(hWnd, "This map does not contain leaks.", "Information", MB_OK);
            }
            break;
        case IDM_SETTINGS:
            if(map_started)
                DialogBox(hInst, MAKEINTRESOURCE(IDD_SETTINGS_DIALOG), hWnd, SettingsDialogProc);
            break;
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    }

    case WM_LBUTTONDOWN: {
        xPos = LOWORD(lParam) / TILE_SIZE + current_offset.x;
        yPos = HIWORD(lParam) / TILE_SIZE + current_offset.y;

        if (xPos < MAP_WIDTH && yPos < MAP_HEIGHT) {
            switch (current_tool) {
            case TOOL_SELECT:
            {
                bool shiftPressed = GetKeyState(VK_SHIFT) & 0x8000;

                if (!shiftPressed) {
                    // selekcija jedne kockice
                    if (multi_select) {
                        // deleketuj sve stare
                        if (mouse_position.x != -1 && mouse_position2Prev.x != -1 && mouse_position2Prev.y != -1) {
                            int oldStartX = min(mouse_position.x, mouse_position2Prev.x);
                            int oldEndX = max(mouse_position.x, mouse_position2Prev.x);
                            int oldStartY = min(mouse_position.y, mouse_position2Prev.y);
                            int oldEndY = max(mouse_position.y, mouse_position2Prev.y);

                            for (int y = oldStartY; y <= oldEndY; y++) {
                                for (int x = oldStartX; x <= oldEndX; x++) {
                                    if (map_grid[y][x] == SELECTED_EMPTY_CELL)
                                        map_grid[y][x] = UNSELECTED_EMPTY_CELL;
                                    else if (map_grid[y][x] == SELECTED_NONEMPTY_CELL)
                                        map_grid[y][x] = UNSELECTED_NONEMPTY_CELL;
                                }
                            }
                        }

                        // ocisti
                        multi_select = false;
                        mouse_position2.x = -1;
                        mouse_position2.y = -1;
                        mouse_position2Prev.x = -1;
                        mouse_position2Prev.y = -1;
                    }

                    // ako je samo jedna kockcika
                    if (mouse_position.x == -1) {
                        mouse_position.x = xPos;
                        mouse_position.y = yPos;

                        if (map_grid[yPos][xPos] == UNSELECTED_EMPTY_CELL)
                            map_grid[yPos][xPos] = SELECTED_EMPTY_CELL;
                        else if (map_grid[yPos][xPos] == UNSELECTED_NONEMPTY_CELL)
                            map_grid[yPos][xPos] = SELECTED_NONEMPTY_CELL;
                    }
                    else {
                        if (map_grid[mouse_position.y][mouse_position.x] == SELECTED_EMPTY_CELL)
                            map_grid[mouse_position.y][mouse_position.x] = UNSELECTED_EMPTY_CELL;
                        else if (map_grid[mouse_position.y][mouse_position.x] == SELECTED_NONEMPTY_CELL)
                            map_grid[mouse_position.y][mouse_position.x] = UNSELECTED_NONEMPTY_CELL;

                        if (xPos == mouse_position.x && yPos == mouse_position.y) {
                            mouse_position.x = -1;
                            mouse_position.y = -1;
                        }
                        else {
                            mouse_position.x = xPos;
                            mouse_position.y = yPos;

                            if (map_grid[yPos][xPos] == UNSELECTED_EMPTY_CELL)
                                map_grid[yPos][xPos] = SELECTED_EMPTY_CELL;
                            else if (map_grid[yPos][xPos] == UNSELECTED_NONEMPTY_CELL)
                                map_grid[yPos][xPos] = SELECTED_NONEMPTY_CELL;
                        }
                    }

                    InvalidateRect(hWnd, nullptr, true);
                }
                else {
                    // vise-selekcija ako se drzi shift
                    if (mouse_position.x == -1) {
                        // odredi pocetni coask
                        mouse_position.x = xPos;
                        mouse_position.y = yPos;

                        // selektuj
                        if (map_grid[yPos][xPos] == UNSELECTED_EMPTY_CELL)
                            map_grid[yPos][xPos] = SELECTED_EMPTY_CELL;
                        else if (map_grid[yPos][xPos] == UNSELECTED_NONEMPTY_CELL)
                            map_grid[yPos][xPos] = SELECTED_NONEMPTY_CELL;

                        multi_select = false;
                        mouse_position2.x = -1;
                        mouse_position2.y = -1;
                    }
                    else if (mouse_position2.x == -1 && mouse_position2.y == -1) {
                        // drugi klik oreduje drugi coask
                        mouse_position2.x = xPos;
                        mouse_position2.y = yPos;
                        multi_select = true;

                        // pravougaobik
                        int startX = min(mouse_position.x, xPos);
                        int endX = max(mouse_position.x, xPos);
                        int startY = min(mouse_position.y, yPos);
                        int endY = max(mouse_position.y, yPos);

                        for (int y = startY; y <= endY; y++) {
                            for (int x = startX; x <= endX; x++) {
                                if (map_grid[y][x] == UNSELECTED_EMPTY_CELL)
                                    map_grid[y][x] = SELECTED_EMPTY_CELL;
                                else if (map_grid[y][x] == UNSELECTED_NONEMPTY_CELL)
                                    map_grid[y][x] = SELECTED_NONEMPTY_CELL;
                            }
                        }

                        // i sacuvaj to
                        mouse_position2Prev.x = xPos;
                        mouse_position2Prev.y = yPos;
                    }
                    else if (mouse_position2.x != -1 && mouse_position2.y != -1) {
                        // update
                        // prvo deselektuj stari
                        int oldStartX = min(mouse_position.x, mouse_position2Prev.x);
                        int oldEndX = max(mouse_position.x, mouse_position2Prev.x);
                        int oldStartY = min(mouse_position.y, mouse_position2Prev.y);
                        int oldEndY = max(mouse_position.y, mouse_position2Prev.y);

                        for (int y = oldStartY; y <= oldEndY; y++) {
                            for (int x = oldStartX; x <= oldEndX; x++) {
                                if (map_grid[y][x] == SELECTED_EMPTY_CELL)
                                    map_grid[y][x] = UNSELECTED_EMPTY_CELL;
                                else if (map_grid[y][x] == SELECTED_NONEMPTY_CELL)
                                    map_grid[y][x] = UNSELECTED_NONEMPTY_CELL;
                            }
                        }

                        // stavi novi drugi coask
                        mouse_position2.x = xPos;
                        mouse_position2.y = yPos;

                        // selektriaj novi pravugaonik
                        int newStartX = min(mouse_position.x, xPos);
                        int newEndX = max(mouse_position.x, xPos);
                        int newStartY = min(mouse_position.y, yPos);
                        int newEndY = max(mouse_position.y, yPos);

                        for (int y = newStartY; y <= newEndY; y++) {
                            for (int x = newStartX; x <= newEndX; x++) {
                                if (map_grid[y][x] == UNSELECTED_EMPTY_CELL)
                                    map_grid[y][x] = SELECTED_EMPTY_CELL;
                                else if (map_grid[y][x] == UNSELECTED_NONEMPTY_CELL)
                                    map_grid[y][x] = SELECTED_NONEMPTY_CELL;
                            }
                        }

                        // i update-aj staro
                        mouse_position2Prev.x = xPos;
                        mouse_position2Prev.y = yPos;

                        multi_select = true;
                    }

                    InvalidateRect(hWnd, nullptr, true);
                }
                break;
            }

            case TOOL_TEXTURE:
                if (enable_paintover) {
                    auto& tileData = map_json["tiles"][yPos][xPos];

                    // ako je prazn
                    if (!tileData.is_array() && tileData.get<int>() == 0)
                        tileData = current_tile;
                    else if (!tileData.is_array()) { //ako nije prazno al nema vise
                        int existingTile = tileData.get<int>();
                        if (existingTile != current_tile)
                            tileData = nlohmann::json::array({ existingTile, current_tile });
                    }
                    else {
                        // ako ima vise vec
                        bool exists = false;
                        for (auto& val : tileData) {
                            if (val.get<int>() == current_tile) {
                                exists = true;
                                break;
                            }
                        }
                        if (!exists) {
                            tileData.push_back(current_tile);
                        }
                    }
                }
                else {
                    map_json["tiles"][yPos][xPos] = current_tile;
                }
                InvalidateRect(hWnd, nullptr, false);
                break;

            case TOOL_ENTITY:
                if (current_entity < 70) {
                    if (current_entity == 53) {
                        if (number_of_stars < 3 || !saftey_check) {
                            number_of_stars++;
                            map_json["tiles"][yPos][xPos] = current_entity;
                        }
                        else
                            MessageBoxA(hWnd, "A map can only contain up to three stars.", "Error", MB_OK | MB_ICONERROR);
                    }
                    else if (current_entity == 55) {
                        if(fishSizeCheck(map_json, xPos, yPos))
                            map_json["tiles"][yPos][xPos] = current_entity;
                        else
                            MessageBoxA(hWnd, "A fish needs free space around it.", "Error", MB_OK | MB_ICONERROR);
                    }
                    else if (current_entity >= 40 && current_entity <= 43) {
                        auto& tileData = map_json["tiles"][yPos][xPos];

                        if (tileData.is_array()) {
                            bool exists = false;
                            for (auto& val : tileData) {
                                if (val.get<int>() == current_entity) {
                                    exists = true;
                                    break;
                                }
                            }
                            if (!exists) {
                                tileData.push_back(current_entity);
                            }
                        }
                        else {
                            int existingValue = tileData.get<int>();
                            if (existingValue == 0)
                                tileData = current_entity;
                            else if (existingValue >= 40 && existingValue <= 43)
                                if (existingValue != current_entity)
                                    tileData = nlohmann::json::array({ existingValue, current_entity });
                            else
                                tileData = current_entity;
                        }
                    }
                    else
                        map_json["tiles"][yPos][xPos] = current_entity;
                }
                else {
                    switch (current_entity) {
                    case 70: {
                        map_json["tiles"][yPos][xPos] = 70;

                        nlohmann::json bat;
                        bat["type"] = "bat";
                        bat["x"] = xPos;
                        bat["y"] = yPos;
                        bat["patrol"] = "horizontal";
                        bat["range"] = 0;

                        map_json["enemies"].push_back(bat);
                        break;
                    }
                    case 71:
                        if (backgroundSizeCheck(map_json, xPos, yPos) || !saftey_check) {
                            if (map_json["playerSpawn"]["x"] != -1) {
                                int oldX = map_json["playerSpawn"]["x"];
                                int oldY = map_json["playerSpawn"]["y"];
                                map_json["tiles"][oldY][oldX] = 0;
                            }

                            map_json["tiles"][yPos][xPos] = 71;
                            map_json["playerSpawn"]["x"] = xPos;
                            map_json["playerSpawn"]["y"] = yPos;

                            valid_player = true;
                        }
                        else {
                            MessageBoxA(hWnd, "Player spawn requires at least 2x3 free space to be placed", "Error", MB_OK | MB_ICONERROR);
                        }
                        break;
                    }
                }
                InvalidateRect(hWnd, nullptr, false);
                break;
            }
        }
        break;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        RECT rc;
        GetClientRect(hWnd, &rc);
        int width = rc.right - rc.left;
        int height = rc.bottom - rc.top;

        HDC hdcMem = CreateCompatibleDC(hdc);
        HBITMAP hbmMem = CreateCompatibleBitmap(hdc, width, height);
        HGDIOBJ hbmOld = SelectObject(hdcMem, hbmMem);

        HDC hdcSprite = CreateCompatibleDC(hdc);
        SelectObject(hdcSprite, hTextureSheet);

        FillRect(hdcMem, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));

        for (unsigned int screenY = 0; screenY < 16; screenY++) {
            unsigned int worldY = current_offset.y + screenY;
            if (worldY >= MAP_HEIGHT) break;

            for (unsigned int screenX = 0; screenX < 24; screenX++) {
                unsigned int worldX = current_offset.x + screenX;
                if (worldX >= MAP_WIDTH) break;

                bool isSelected = (map_grid[worldY][worldX] == SELECTED_EMPTY_CELL ||
                    map_grid[worldY][worldX] == SELECTED_NONEMPTY_CELL);
                auto& tileData = map_json["tiles"][worldY][worldX];

                // nacrtaj kockice
                if (show_gridlines || isSelected) {
                    int spriteXOffset = isSelected ? SELECTD_CELL_X_OFFSET : UNSELECTD_CELL_X_OFFSET;
                    int spriteYOffset = isSelected ? SELECTD_CELL_Y_OFFSET : UNSELECTD_CELL_Y_OFFSET;

                    TransparentBlt(
                        hdcMem, screenX * TILE_SIZE, screenY * TILE_SIZE,
                        TILE_SIZE, TILE_SIZE, hdcSprite,
                        spriteXOffset * TILE_SIZE, spriteYOffset * TILE_SIZE,
                        TILE_SIZE, TILE_SIZE, TRANSPARENT_COLOR
                    );
                }

                // nacrtaj teksturu
                if (tileData.is_array()) {
                    for (auto& val : tileData) {
                        int tileId = val.get<int>();

                        bool found = false;
                        for (auto& t : textureTiles) {
                            if (t.tileId == tileId) {
                                TransparentBlt(
                                    hdcMem, screenX * TILE_SIZE, screenY * TILE_SIZE,
                                    TILE_SIZE, TILE_SIZE, hdcSprite,
                                    t.sheetCol * TILE_SIZE, t.sheetRow * TILE_SIZE,
                                    TILE_SIZE, TILE_SIZE, TRANSPARENT_COLOR
                                );
                                found = true;
                                break;
                            }
                        }

                        if (!found) {
                            for (auto& e : entityTiles) {
                                if (e.tileId == tileId) {
                                    TransparentBlt(
                                        hdcMem, screenX * TILE_SIZE, screenY * TILE_SIZE,
                                        TILE_SIZE, TILE_SIZE, hdcSprite,
                                        e.sheetCol * TILE_SIZE, e.sheetRow * TILE_SIZE,
                                        TILE_SIZE, TILE_SIZE, TRANSPARENT_COLOR
                                    );
                                    break;
                                }
                            }
                        }
                    }
                }
                else {
                    int tileId = tileData.get<int>();

                    if (tileId != 0) {
                        bool found = false;
                        for (auto& t : textureTiles) {
                            if (t.tileId == tileId) {
                                TransparentBlt(
                                    hdcMem, screenX * TILE_SIZE, screenY * TILE_SIZE,
                                    TILE_SIZE, TILE_SIZE, hdcSprite,
                                    t.sheetCol * TILE_SIZE, t.sheetRow * TILE_SIZE,
                                    TILE_SIZE, TILE_SIZE, TRANSPARENT_COLOR
                                );
                                found = true;
                                break;
                            }
                        }

                        if (!found) {
                            for (auto& e : entityTiles) {
                                if (e.tileId == tileId) {
                                    TransparentBlt(
                                        hdcMem, screenX * TILE_SIZE, screenY * TILE_SIZE,
                                        TILE_SIZE, TILE_SIZE, hdcSprite,
                                        e.sheetCol * TILE_SIZE, e.sheetRow * TILE_SIZE,
                                        TILE_SIZE, TILE_SIZE, TRANSPARENT_COLOR
                                    );
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }

        BitBlt(hdc, 0, 0, width, height, hdcMem, 0, 0, SRCCOPY);

        SelectObject(hdcMem, hbmOld);
        DeleteObject(hbmMem);
        DeleteDC(hdcMem);
        DeleteDC(hdcSprite);
        EndPaint(hWnd, &ps);
        break;
    }

    case WM_CLOSE: {
        int result = (map_started) ? MessageBox(hWnd, L"Do you want to save the map?", L"New Map", MB_YESNOCANCEL) : IDNO;

        if (result == IDCANCEL) {
            break;
        }

        if (result == IDYES) {
            // cuvanje
            OPENFILENAMEW ofn;
            wchar_t szFile[MAX_PATH] = L"level.json";

            if (map_started == false) {
                MessageBoxA(hWnd, "There isn't a map to be saved.", "Error", MB_OK | MB_ICONERROR);
                break;
            }

            if (valid_player == false) {
                MessageBoxA(hWnd, "Player spawn is required.", "Error", MB_OK | MB_ICONERROR);
                break;
            }

            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hWnd;
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = sizeof(szFile);
            ofn.lpstrFilter = L"JSON Files (*.json)\0*.json\0All Files (*.*)\0*.*\0";
            ofn.nFilterIndex = 1;
            ofn.lpstrDefExt = L"json";
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

            if (GetSaveFileNameW(&ofn)) {
                std::ofstream file(ofn.lpstrFile);
                if (file.is_open()) {
                    std::stringstream ss;
                    ss << "{\n";
                    ss << "  \"name\": \"" << map_json["name"].get<std::string>() << "\",\n";
                    ss << "  \"nextMap\": \"" << map_json["nextMap"].get<std::string>() << "\",\n";
                    ss << "  \"theme\": " << map_json["theme"].get<int>() << ",\n";
                    ss << "  \"width\": " << map_json["width"].get<int>() << ",\n";
                    ss << "  \"height\": " << map_json["height"].get<int>() << ",\n";
                    ss << "  \"playerSpawn\": " << map_json["playerSpawn"].dump() << ",\n";
                    ss << "  \"tiles\": [\n";

                    auto& tiles = map_json["tiles"];
                    for (size_t i = 0; i < tiles.size(); i++) {
                        ss << "    " << tiles[i].dump();
                        if (i < tiles.size() - 1) {
                            ss << ",";
                        }
                        ss << "\n";
                    }

                    ss << "  ],\n";
                    ss << "  \"enemies\": " << map_json["enemies"].dump() << "\n";
                    ss << "}";

                    file << ss.str();
                    file.close();
                }
            }
        }

        DeleteMapGrid(map_grid, map_json, MAP_HEIGHT, MAP_WIDTH);
        PostQuitMessage(0);
        break;
    }
    case WM_DESTROY: {      

        DeleteMapGrid(map_grid, map_json, MAP_HEIGHT, MAP_WIDTH);
        PostQuitMessage(0);
        break;
    }

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK SecondWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static HWND hSelectCheck = nullptr;
    static HWND hTextureCheck = nullptr;
    static HWND hEntityCheck = nullptr;
    static HWND hDeleteCheck = nullptr;
    static HWND hClearBtn = nullptr;
    static HWND hBrowseTexture = nullptr;
    static HWND hBrowseEntity = nullptr;
    static HWND hGroupBox = nullptr;
    static HWND hTextureLabel = nullptr;
    static HWND hEntityLabel = nullptr;

    static int groupStartX = 0;
    static int startY = 0;
    static int btnHeight = 25;
    static int spacing = 10;

    static int btnWidth = 120;
    static int groupWidth = btnWidth + 20;

    switch (message)
    {
    case WM_CREATE: {
        int padding = 15;
        int btnHeight = 25;
        int spacing = 10;
        int startY = padding;
        int groupStartX = padding;
        int groupStartY = startY;
        int groupHeight = 3 * (btnHeight + spacing) + 20;

        int rightColumnX = groupStartX + groupWidth + 30;

        hwndSecond = hWnd;

        hGroupBox = CreateWindow(L"BUTTON", L"Tools",
            WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
            groupStartX, groupStartY, groupWidth, groupHeight,
            hWnd, NULL, hInst, nullptr);

        hSelectCheck = CreateWindow(L"BUTTON", L"Select Tool",
            WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP,
            groupStartX + 10, groupStartY + 20, btnWidth, btnHeight,
            hWnd, (HMENU)ID_SELECT_CHECK, hInst, nullptr);

        hTextureCheck = CreateWindow(L"BUTTON", L"Texture Tool",
            WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
            groupStartX + 10, groupStartY + 20 + (btnHeight + spacing), btnWidth, btnHeight,
            hWnd, (HMENU)ID_TEXTURE_CHECK, hInst, nullptr);

        hEntityCheck = CreateWindow(L"BUTTON", L"Entity Tool",
            WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
            groupStartX + 10, groupStartY + 20 + 2 * (btnHeight + spacing), btnWidth, btnHeight,
            hWnd, (HMENU)ID_ENTITY_CHECK, hInst, nullptr);

        int buttonsStartY = groupStartY + groupHeight + 20;

        hClearBtn = CreateWindow(L"BUTTON", L"Clear Map",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_TEXT,
            groupStartX, buttonsStartY, btnWidth, btnHeight,
            hWnd, (HMENU)ID_CLEAR_BTN, hInst, nullptr);

        // teksture
        int textureStartY = startY;
        int textureBoxSize = 32;
        int sectionSpacing = 8;

        hTextureLabel = CreateWindow(L"STATIC", L"Current\nTexture:",
            WS_CHILD | WS_VISIBLE | SS_CENTER,
            rightColumnX, textureStartY, 80, 40,
            hWnd, NULL, hInst, nullptr);

        // za primjer odabrane teksture
        int textureBoxY = textureStartY + 45;
        int textureBoxX = rightColumnX;

        hBrowseTexture = CreateWindow(L"BUTTON", L"Browse",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_TEXT,
            rightColumnX, textureBoxY + textureBoxSize + sectionSpacing, 80, btnHeight,
            hWnd, (HMENU)ID_BROWSE_TEXTURE, hInst, nullptr);

        // za entitete
        int entityStartY = textureBoxY + textureBoxSize + sectionSpacing + btnHeight + 20;

        hEntityLabel = CreateWindow(L"STATIC", L"Current\nEntity:",
            WS_CHILD | WS_VISIBLE | SS_CENTER,
            rightColumnX, entityStartY, 80, 40,
            hWnd, NULL, hInst, nullptr);

        // primjer entiteta
        int entityBoxY = entityStartY + 45;
        int entityBoxX = rightColumnX;

        hBrowseEntity = CreateWindow(L"BUTTON", L"Browse",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_TEXT,
            rightColumnX, entityBoxY + textureBoxSize + sectionSpacing, 80, btnHeight,
            hWnd, (HMENU)ID_BROWSE_ENTITY, hInst, nullptr);

        SendMessage(hSelectCheck, BM_SETCHECK, BST_CHECKED, 0);
        current_tool = TOOL_SELECT;

        break;
    }

    case WM_KEYDOWN: {
        bool ctrlPressed = GetKeyState(VK_CONTROL) & 0x8000;

        if (wParam == 'R' && ctrlPressed) {
            switch (current_tool) {
            case TOOL_ENTITY:
                if (entity_position.y == 5) {
                    entity_position.x = (entity_position.x < 3) ? entity_position.x + 1 : 0;
                    current_entity = 40 + entity_position.x;
                    InvalidateRect(hWnd, nullptr, true);
                }
                else if (entity_position.y == 9) {
                    entity_position.x = (entity_position.x < 3) ? entity_position.x + 1 : 0;
                    current_entity = 60 + entity_position.x;
                    InvalidateRect(hwndSecond, nullptr, true);
                }
                break;

            case TOOL_TEXTURE:
                if (texture_position.y == 0) { // Full wall (1-4)
                    texture_position.x = (texture_position.x < 3) ? texture_position.x + 1 : 0;
                    current_tile = 1 + texture_position.x;
                }
                else if (texture_position.y == 1) { // Wall outer corner (5-8)
                    texture_position.x = (texture_position.x < 3) ? texture_position.x + 1 : 0;
                    current_tile = 5 + texture_position.x;
                }
                else if (texture_position.y == 2) { // Wall inner corner (9-12)
                    texture_position.x = (texture_position.x < 3) ? texture_position.x + 1 : 0;
                    current_tile = 9 + texture_position.x;
                }
                else if (texture_position.y == 3) { // Spike full wall (20-23)
                    texture_position.x = (texture_position.x < 3) ? texture_position.x + 1 : 0;
                    current_tile = 20 + texture_position.x;
                }
                else if (texture_position.y == 4) { // Spike wall outer corner (24-27)
                    texture_position.x = (texture_position.x < 3) ? texture_position.x + 1 : 0;
                    current_tile = 24 + texture_position.x;
                }

                InvalidateRect(hWnd, nullptr, true);
                break;
            }
        }
    }

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        RECT rc;
        GetClientRect(hWnd, &rc);
        int width = rc.right - rc.left;
        int height = rc.bottom - rc.top;

        HDC hdcMem = CreateCompatibleDC(hdc);
        HBITMAP hbmMem = CreateCompatibleBitmap(hdc, width, height);
        HGDIOBJ hbmOld = SelectObject(hdcMem, hbmMem);

        // popuni pozadinu
        HBRUSH hBgBrush = (HBRUSH)GetClassLongPtr(hWnd, GCLP_HBRBACKGROUND);
        FillRect(hdcMem, &rc, hBgBrush);

        int groupWidth = btnWidth + 20;

        // nacrtaj primjer teksture 
        int textureBoxX = groupStartX + groupWidth + 68;
        int textureBoxY = startY + 58;
        int entityBoxY = textureBoxY + 32 + 5 + btnHeight + 20 + 45;

        HDC hdcSprite = CreateCompatibleDC(hdc);
        SelectObject(hdcSprite, hTextureSheet);

        // nacrtaj texture_position teksturu
        TransparentBlt(
            hdcMem,
            textureBoxX, textureBoxY,
            TILE_SIZE, TILE_SIZE,
            hdcSprite,
            texture_position.x * TILE_SIZE,
            texture_position.y * TILE_SIZE,
            TILE_SIZE, TILE_SIZE,
            TRANSPARENT_COLOR
        );

        // isto za entitet
        TransparentBlt(
            hdcMem,
            textureBoxX, entityBoxY,
            TILE_SIZE, TILE_SIZE,
            hdcSprite,
            entity_position.x * TILE_SIZE,
            entity_position.y * TILE_SIZE,
            TILE_SIZE, TILE_SIZE,
            TRANSPARENT_COLOR
        );

        DeleteDC(hdcSprite);

        BitBlt(hdc, 0, 0, width, height, hdcMem, 0, 0, SRCCOPY);

        SelectObject(hdcMem, hbmOld);
        DeleteObject(hbmMem);
        DeleteDC(hdcMem);

        EndPaint(hWnd, &ps);
        break;
    }
    case WM_COMMAND:
    {
        switch (LOWORD(wParam)) {
        case ID_SELECT_CHECK:
            {
                current_tool = TOOL_SELECT;
                SendMessage(hSelectCheck, BM_SETCHECK, BST_CHECKED, 0);
                SendMessage(hTextureCheck, BM_SETCHECK, BST_UNCHECKED, 0);
                SendMessage(hEntityCheck, BM_SETCHECK, BST_UNCHECKED, 0);
                SendMessage(hDeleteCheck, BM_SETCHECK, BST_UNCHECKED, 0);
                InvalidateRect(hwndFirst, nullptr, false);
            }
            break;
        case ID_TEXTURE_CHECK:
            {
                current_tool = TOOL_TEXTURE;
                SendMessage(hSelectCheck, BM_SETCHECK, BST_UNCHECKED, 0);
                SendMessage(hTextureCheck, BM_SETCHECK, BST_CHECKED, 0);
                SendMessage(hEntityCheck, BM_SETCHECK, BST_UNCHECKED, 0);
                SendMessage(hDeleteCheck, BM_SETCHECK, BST_UNCHECKED, 0);

                if (mouse_position.x != -1) {
                    map_grid[mouse_position.y][mouse_position.x] = (map_grid[mouse_position.y][mouse_position.x] == SELECTED_NONEMPTY_CELL) ? UNSELECTED_NONEMPTY_CELL : UNSELECTED_EMPTY_CELL;
                    mouse_position.x = -1;
                    mouse_position.y = -1;
                }
                InvalidateRect(hwndFirst, nullptr, false);
            }
            break;
        case ID_ENTITY_CHECK:
            {
                current_tool = TOOL_ENTITY;
                SendMessage(hSelectCheck, BM_SETCHECK, BST_UNCHECKED, 0);
                SendMessage(hTextureCheck, BM_SETCHECK, BST_UNCHECKED, 0);
                SendMessage(hEntityCheck, BM_SETCHECK, BST_CHECKED, 0);
                SendMessage(hDeleteCheck, BM_SETCHECK, BST_UNCHECKED, 0);

                if (mouse_position.x != -1) {
                    map_grid[mouse_position.y][mouse_position.x] = (map_grid[mouse_position.y][mouse_position.x] == SELECTED_NONEMPTY_CELL) ? UNSELECTED_NONEMPTY_CELL : UNSELECTED_EMPTY_CELL;
                    mouse_position.x = -1;
                    mouse_position.y = -1;
                }
                InvalidateRect(hwndFirst, nullptr, false);
            }
            break;
        case ID_DELETE_CHECK:
            {
                current_tool = TOOL_DELETE;
                SendMessage(hSelectCheck, BM_SETCHECK, BST_UNCHECKED, 0);
                SendMessage(hTextureCheck, BM_SETCHECK, BST_UNCHECKED, 0);
                SendMessage(hEntityCheck, BM_SETCHECK, BST_UNCHECKED, 0);
                SendMessage(hDeleteCheck, BM_SETCHECK, BST_CHECKED, 0);

                if (mouse_position.x != -1) {
                    map_grid[mouse_position.y][mouse_position.x] = (map_grid[mouse_position.y][mouse_position.x] == SELECTED_NONEMPTY_CELL) ? UNSELECTED_NONEMPTY_CELL : UNSELECTED_EMPTY_CELL;
                    mouse_position.x = -1;
                    mouse_position.y = -1;
                }
                InvalidateRect(hwndFirst, nullptr, false);
            }
            break;
        case ID_CLEAR_BTN:
            {
                DeleteMapGrid(map_grid, map_json, MAP_HEIGHT, MAP_WIDTH);
                CreateMapGrid(map_grid, MAP_HEIGHT, MAP_WIDTH);
                InvalidateRect(hwndFirst, nullptr, false);
                mouse_position.x = -1;
                mouse_position.y = -1;
            }
            break;
        case ID_BROWSE_TEXTURE:
            {
                if (show_texture_browser) {
                    ShowWindow(hTextureBrowser, SW_HIDE);
                    show_texture_browser = false;
                }
                else {
                    ShowWindow(hTextureBrowser, SW_SHOW);
                    show_texture_browser = true;
                }
            }
            break;
        case ID_BROWSE_ENTITY:
            {
                if (show_entity_browser) {
                    ShowWindow(hEntityBrowser, SW_HIDE);
                    show_entity_browser = false;
                }
                else {
                    ShowWindow(hEntityBrowser, SW_SHOW);
                    show_entity_browser = true;
                }
            }
            break;
        }
        break;
    }
    case WM_CLOSE: {
        show_toolbox = false;

        HMENU hMenu = GetMenu(hwndFirst);
        CheckMenuItem(hMenu, IDM_SHOW_TOOLBOX, MF_BYCOMMAND | MF_UNCHECKED);

        ShowWindow(hwndSecond, SW_HIDE);
        return 0;
    }
    case WM_DESTROY: {
        show_toolbox = false;

        HMENU hMenu = GetMenu(hwndFirst);
        CheckMenuItem(hMenu, IDM_SHOW_TOOLBOX, MF_BYCOMMAND | MF_UNCHECKED);

        ShowWindow(hwndSecond, SW_HIDE);
        break;
    }

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK TextureBrowserProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static POINT selectedTile = { -1, -1 };
    static int padding = 10;
    static int tileDisplaySize = TILE_SIZE * 2;
    static int num_columns_drawn = 3;

    std::vector<TileData>& tiles = textureTiles;

    switch (message)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        if (hTextureSheet != NULL) {
            HDC hdcSprite = CreateCompatibleDC(hdc);
            SelectObject(hdcSprite, hTextureSheet);

            // filtriran vektor samo za true teksture
            std::vector<TileData> filteredTiles;
            for (const auto& tile : tiles) {
                if (tile.drawPicker) {
                    filteredTiles.push_back(tile);
                }
            }

            int totalTiles = filteredTiles.size();
            int displayRows = (totalTiles + num_columns_drawn - 1) / num_columns_drawn;

            for (int displayRow = 0; displayRow < displayRows; displayRow++) {
                for (int displayCol = 0; displayCol < num_columns_drawn; displayCol++) {
                    int tileIndex = displayRow * num_columns_drawn + displayCol;
                    if (tileIndex >= totalTiles) break;

                    TileData tile = filteredTiles[tileIndex];

                    int x = padding + displayCol * (tileDisplaySize + padding);
                    int y = padding + displayRow * (tileDisplaySize + padding);

                    TransparentBlt(
                        hdc,
                        x, y,
                        tileDisplaySize, tileDisplaySize,
                        hdcSprite,
                        tile.sheetCol * TILE_SIZE,
                        tile.sheetRow * TILE_SIZE,
                        TILE_SIZE, TILE_SIZE,
                        TRANSPARENT_COLOR
                    );
                    //nacrta okvir oko odabrane teksture
                    if (selectedTile.x == tile.sheetCol && selectedTile.y == tile.sheetRow) {
                        HPEN hPen = CreatePen(PS_SOLID, 2, RGB(128, 128, 128));
                        HGDIOBJ hOldPen = SelectObject(hdc, hPen);
                        HGDIOBJ hOldBrush = SelectObject(hdc, GetStockObject(NULL_BRUSH));

                        Rectangle(hdc, x - 2, y - 2, x + tileDisplaySize + 2, y + tileDisplaySize + 2);

                        SelectObject(hdc, hOldPen);
                        SelectObject(hdc, hOldBrush);
                        DeleteObject(hPen);
                    }
                }
            }

            DeleteDC(hdcSprite);
        }

        EndPaint(hWnd, &ps);
        break;
    }

    case WM_LBUTTONDOWN:
    {
        int clickX = LOWORD(lParam);
        int clickY = HIWORD(lParam);

        // i gore
        std::vector<TileData> filteredTiles;
        for (const auto& tile : tiles) {
            if (tile.drawPicker) {
                filteredTiles.push_back(tile);
            }
        }

        int totalTiles = filteredTiles.size();
        int displayRows = (totalTiles + num_columns_drawn - 1) / num_columns_drawn;

        for (int displayRow = 0; displayRow < displayRows; displayRow++) {
            for (int displayCol = 0; displayCol < num_columns_drawn; displayCol++) {
                int tileIndex = displayRow * num_columns_drawn + displayCol;
                if (tileIndex >= totalTiles) break;

                TileData tile = filteredTiles[tileIndex];

                int tileLeft = padding + displayCol * (tileDisplaySize + padding);
                int tileRight = tileLeft + tileDisplaySize;
                int tileTop = padding + displayRow * (tileDisplaySize + padding);
                int tileBottom = tileTop + tileDisplaySize;

                if (clickX >= tileLeft && clickX < tileRight &&
                    clickY >= tileTop && clickY < tileBottom) {

                    if (selectedTile.x == tile.sheetCol && selectedTile.y == tile.sheetRow) {
                        texture_position.x = tile.sheetCol;
                        texture_position.y = tile.sheetRow;
                        current_tile = tile.tileId;

                        InvalidateRect(hwndSecond, nullptr, true);
                        ShowWindow(hWnd, SW_HIDE);
                        show_texture_browser = false;
                    }
                    else {
                        selectedTile.x = tile.sheetCol;
                        selectedTile.y = tile.sheetRow;
                        InvalidateRect(hWnd, nullptr, true);
                    }
                    return 0;
                }
            }
        }
        break;
    }

    case WM_CLOSE:
        ShowWindow(hWnd, SW_HIDE);
        show_texture_browser = false;
        return 0;

    case WM_DESTROY:
        hTextureBrowser = nullptr;
        show_texture_browser = false;
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK EntityBrowserProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static POINT selectedTile = { -1, -1 };
    static int padding = 10;
    static int tileDisplaySize = TILE_SIZE * 2;
    static int num_columns_drawn = 4;

    std::vector<TileData>& tiles = entityTiles;

    switch (message)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        if (hTextureSheet != NULL) {
            HDC hdcSprite = CreateCompatibleDC(hdc);
            SelectObject(hdcSprite, hTextureSheet);

            // isto radi kao i za teksture samo za entitete
            std::vector<TileData> filteredTiles;
            for (const auto& tile : tiles) {
                if (tile.drawPicker) {
                    filteredTiles.push_back(tile);
                }
            }

            int totalTiles = filteredTiles.size();
            int displayRows = (totalTiles + num_columns_drawn - 1) / num_columns_drawn;

            for (int displayRow = 0; displayRow < displayRows; displayRow++) {
                for (int displayCol = 0; displayCol < num_columns_drawn; displayCol++) {
                    int tileIndex = displayRow * num_columns_drawn + displayCol;
                    if (tileIndex >= totalTiles) break;

                    TileData tile = filteredTiles[tileIndex];

                    int x = padding + displayCol * (tileDisplaySize + padding);
                    int y = padding + displayRow * (tileDisplaySize + padding);

                    TransparentBlt(
                        hdc,
                        x, y,
                        tileDisplaySize, tileDisplaySize,
                        hdcSprite,
                        tile.sheetCol * TILE_SIZE,
                        tile.sheetRow * TILE_SIZE,
                        TILE_SIZE, TILE_SIZE,
                        TRANSPARENT_COLOR
                    );

                    if (selectedTile.x == tile.sheetCol && selectedTile.y == tile.sheetRow) {
                        HPEN hPen = CreatePen(PS_SOLID, 2, RGB(128, 128, 128));
                        HGDIOBJ hOldPen = SelectObject(hdc, hPen);
                        HGDIOBJ hOldBrush = SelectObject(hdc, GetStockObject(NULL_BRUSH));

                        Rectangle(hdc, x - 2, y - 2, x + tileDisplaySize + 2, y + tileDisplaySize + 2);

                        SelectObject(hdc, hOldPen);
                        SelectObject(hdc, hOldBrush);
                        DeleteObject(hPen);
                    }
                }
            }

            DeleteDC(hdcSprite);
        }

        EndPaint(hWnd, &ps);
        break;
    }

    case WM_LBUTTONDOWN:
    {
        int clickX = LOWORD(lParam);
        int clickY = HIWORD(lParam);

        std::vector<TileData> filteredTiles;
        for (const auto& tile : tiles) {
            if (tile.drawPicker) {
                filteredTiles.push_back(tile);
            }
        }

        int totalTiles = filteredTiles.size();
        int displayRows = (totalTiles + num_columns_drawn - 1) / num_columns_drawn;

        for (int displayRow = 0; displayRow < displayRows; displayRow++) {
            for (int displayCol = 0; displayCol < num_columns_drawn; displayCol++) {
                int tileIndex = displayRow * num_columns_drawn + displayCol;
                if (tileIndex >= totalTiles) break;

                TileData tile = filteredTiles[tileIndex];

                int tileLeft = padding + displayCol * (tileDisplaySize + padding);
                int tileRight = tileLeft + tileDisplaySize;
                int tileTop = padding + displayRow * (tileDisplaySize + padding);
                int tileBottom = tileTop + tileDisplaySize;

                if (clickX >= tileLeft && clickX < tileRight &&
                    clickY >= tileTop && clickY < tileBottom) {

                    if (selectedTile.x == tile.sheetCol && selectedTile.y == tile.sheetRow) {
                        entity_position.x = tile.sheetCol;
                        entity_position.y = tile.sheetRow;
                        current_entity = tile.tileId;

                        ShowWindow(hWnd, SW_HIDE);
                        show_entity_browser = false;
                        InvalidateRect(hwndSecond, nullptr, true);
                    }
                    else {
                        selectedTile.x = tile.sheetCol;
                        selectedTile.y = tile.sheetRow;
                        InvalidateRect(hWnd, nullptr, true);
                    }
                    return 0;
                }
            }
        }
        break;
    }

    case WM_CLOSE:
        ShowWindow(hWnd, SW_HIDE);
        show_entity_browser = false;
        return 0;

    case WM_DESTROY:
        hEntityBrowser = nullptr;
        show_entity_browser = false;
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

INT_PTR CALLBACK BatDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static nlohmann::json* pEnemy = nullptr;

    switch (message)
    {
    case WM_INITDIALOG: {
        pEnemy = (nlohmann::json*)lParam;

        //ucitaj trenutacne vrijednosti i pripremi kombo box
        int range = (*pEnemy)["range"].get<int>();
        bool reverse = (range < 0);
        SetDlgItemInt(hDlg, IDC_RANGE_EDIT, abs(range), FALSE);
        CheckDlgButton(hDlg, IDC_REVERSE_CHECK, reverse ? BST_CHECKED : BST_UNCHECKED);

        HWND hCombo = GetDlgItem(hDlg, IDC_PATROL_COMBO);
        SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"horizontal");
        SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"vertical");

        //novu vrijednost
        std::string patrol = (*pEnemy)["patrol"].get<std::string>();
        if (patrol == "horizontal") {
            SendMessage(hCombo, CB_SETCURSEL, 0, 0);
        }
        else {
            SendMessage(hCombo, CB_SETCURSEL, 1, 0);
        }
    }
    return TRUE;

    case WM_COMMAND: {
        if (LOWORD(wParam) == IDOK_BAT) {
            //pokupi vrijednosti
            int range = GetDlgItemInt(hDlg, IDC_RANGE_EDIT, NULL, FALSE);
            bool reverse = (IsDlgButtonChecked(hDlg, IDC_REVERSE_CHECK) == BST_CHECKED);

            if (reverse) {
                range = -range;
            }

            HWND hCombo = GetDlgItem(hDlg, IDC_PATROL_COMBO);
            int sel = SendMessage(hCombo, CB_GETCURSEL, 0, 0);
            std::string patrol = (sel == 0) ? "horizontal" : "vertical";

            //postavi novo
            (*pEnemy)["range"] = range;
            (*pEnemy)["patrol"] = patrol;

            EndDialog(hDlg, IDOK);
            return TRUE;
        }
        else if (LOWORD(wParam) == IDCANCEL_BAT) {
            EndDialog(hDlg, IDCANCEL);
            return TRUE;
        }
    }
                   break;

    case WM_CLOSE:
        EndDialog(hDlg, IDCANCEL);
        return TRUE;
    }

    return FALSE;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

INT_PTR CALLBACK NewMapDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_INITDIALOG: {
        HWND hCombo = GetDlgItem(hDlg, IDC_THEME_COMBO);

        SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"1 - Default Pink");
        SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"2 - Default Purple");
        SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"3 - Rocky Cyan");
        SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"4 - Bricks Orange");

        SendMessage(hCombo, CB_SETCURSEL, 0, 0);

        return TRUE;
    }

    case WM_COMMAND: {
        int wmId = LOWORD(wParam);

        if (wmId == IDC_CREATE_BTN) {
            char nameBuffer[256] = "";
            char nextMapBuffer[256] = "";
            char widthBuffer[32] = "";
            char heightBuffer[32] = "";

            GetDlgItemTextA(hDlg, IDC_MAP_NAME_EDIT, nameBuffer, 256);
            GetDlgItemTextA(hDlg, IDC_NEXTMAP_EDIT, nextMapBuffer, 256);
            GetDlgItemTextA(hDlg, IDC_MAP_WIDTH_EDIT, widthBuffer, 32);
            GetDlgItemTextA(hDlg, IDC_MAP_HEIGHT_EDIT, heightBuffer, 32);

            HWND hCombo = GetDlgItem(hDlg, IDC_THEME_COMBO);
            map_theme = SendMessage(hCombo, CB_GETCURSEL, 0, 0);

            //
            if (strlen(nameBuffer) == 0 || strlen(widthBuffer) == 0 || strlen(heightBuffer) == 0) {
                MessageBoxA(hDlg, "Map Name, Width and Height are required.", "Error", MB_OK | MB_ICONERROR);
                return TRUE;
            }

            int width = atoi(widthBuffer);
            int height = atoi(heightBuffer);

            if (width <= 0 || height <= 0) {
                MessageBoxA(hDlg, "Width and Height must be positive numbers.", "Error", MB_OK | MB_ICONERROR);
                return TRUE;
            }

            DeleteMapGrid(map_grid, map_json, MAP_HEIGHT, MAP_WIDTH);

            MAP_WIDTH = width;
            MAP_HEIGHT = height;

            CreateMapGrid(map_grid, MAP_HEIGHT, MAP_WIDTH);

            // novi json
            map_json.clear();
            map_json["name"] = std::string(nameBuffer);

            if (strlen(nextMapBuffer) > 0) {
                map_json["nextMap"] = std::string(nextMapBuffer);
            }
            else {
                map_json["nextMap"] = "";
            }

            map_json["width"] = width;
            map_json["height"] = height;
            map_json["playerSpawn"]["x"] = -1;
            map_json["playerSpawn"]["y"] = -1;
            map_json["theme"] = map_theme;

            // kreiraj prazno
            map_json["tiles"] = nlohmann::json::array();
            for (int y = 0; y < height; y++) {
                nlohmann::json row = nlohmann::json::array();
                for (int x = 0; x < width; x++) {
                    row.push_back(0);
                }
                map_json["tiles"].push_back(row);
            }

            map_json["enemies"] = nlohmann::json::array();

            // rest
            current_offset = { 0, 0 };
            mouse_position = { (long)-1, (long)-1 };
            texture_position = { 0, 0 };
            entity_position = { 0, 5 };

            current_tile = 1;
            current_entity = 40;
            valid_player = false;

            hTextureSheet = nullptr;

            switch (map_theme) {
            case 0:
                hTextureSheet = hTextureSheet0;
                break;
            case 1:
                hTextureSheet = hTextureSheet1;
                break;
            case 2:
                hTextureSheet = hTextureSheet2;
                break;
            case 3:
                hTextureSheet = hTextureSheet3;
                break;
            }

            InvalidateRect(hwndFirst, nullptr, true);
            InvalidateRect(hwndSecond, nullptr, true);

            map_started = true;
            ShowWindow(hwndSecond, SW_SHOW);
            CheckMenuItem(GetMenu(hwndFirst), IDM_SHOW_TOOLBOX, MF_BYCOMMAND | MF_CHECKED);

            EndDialog(hDlg, IDOK);
            return TRUE;
        }
        else if (wmId == IDC_CANCEL_BTN) {
            DeleteMapGrid(map_grid, map_json, MAP_HEIGHT, MAP_WIDTH);

            MAP_WIDTH = 0;
            MAP_HEIGHT = 0;

            CreateMapGrid(map_grid, MAP_HEIGHT, MAP_WIDTH);

            // prazan json
            map_json.clear();
            map_json["name"] = "";
            map_json["nextMap"] = "";
            map_json["width"] = 0;
            map_json["height"] = 0;
            map_json["playerSpawn"]["x"] = -1;
            map_json["playerSpawn"]["y"] = -1;
            map_json["tiles"] = nlohmann::json::array();
            map_json["enemies"] = nlohmann::json::array();
            map_json["theme"] = 0;

            // Reset
            current_offset = { 0, 0 };
            mouse_position = { (long)-1, (long)-1 };
            texture_position = { 0, 0 };
            entity_position = { 0, 5 };

            current_tile = 1;
            current_entity = 40;
            valid_player = false;

            InvalidateRect(hwndFirst, nullptr, true);

            map_started = false;
            ShowWindow(hwndSecond, SW_HIDE);

            EndDialog(hDlg, IDCANCEL);
            return TRUE;
        }
        break;
    }

    case WM_CLOSE:
        EndDialog(hDlg, IDCANCEL);
        return TRUE;
    }
    return FALSE;
}

INT_PTR CALLBACK SettingsDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_INITDIALOG: {
        SetDlgItemTextA(hDlg, IDC_SETTINGS_NAME_EDIT, map_json["name"].get<std::string>().c_str());

        if (map_json.contains("nextMap") && !map_json["nextMap"].get<std::string>().empty()) {
            SetDlgItemTextA(hDlg, IDC_SETTINGS_NEXTMAP_EDIT, map_json["nextMap"].get<std::string>().c_str());
        }

        SetDlgItemInt(hDlg, IDC_SETTINGS_WIDTH_EDIT, MAP_WIDTH, FALSE);
        SetDlgItemInt(hDlg, IDC_SETTINGS_HEIGHT_EDIT, MAP_HEIGHT, FALSE);

        HWND hCombo = GetDlgItem(hDlg, IDC_SETTINGS_THEME_COMBO);
        SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"1 - Default Pink");
        SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"2 - Default Purple");
        SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"3 - Rocky Cyan");
        SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"4 - Bricks Orange");

        int currentTheme = map_json.contains("theme") ? map_json["theme"].get<int>() : 0;
        if (currentTheme >= 1 && currentTheme <= 4) {
            SendMessage(hCombo, CB_SETCURSEL, currentTheme, 0);
        }
        else
            SendMessage(hCombo, CB_SETCURSEL, 0, 0);

        return TRUE;
    }

    case WM_COMMAND: {
        int wmId = LOWORD(wParam);

        if (wmId == IDC_SETTINGS_APPLY) {
            char nameBuffer[256] = "";
            char nextMapBuffer[256] = "";
            char widthBuffer[32] = "";
            char heightBuffer[32] = "";

            GetDlgItemTextA(hDlg, IDC_SETTINGS_NAME_EDIT, nameBuffer, 256);
            GetDlgItemTextA(hDlg, IDC_SETTINGS_NEXTMAP_EDIT, nextMapBuffer, 256);
            GetDlgItemTextA(hDlg, IDC_SETTINGS_WIDTH_EDIT, widthBuffer, 32);
            GetDlgItemTextA(hDlg, IDC_SETTINGS_HEIGHT_EDIT, heightBuffer, 32);

            if (strlen(nameBuffer) == 0 || strlen(widthBuffer) == 0 || strlen(heightBuffer) == 0) {
                MessageBoxA(hDlg, "Map Name, Width and Height are required.", "Error", MB_OK | MB_ICONERROR);
                return TRUE;
            }

            int newWidth = atoi(widthBuffer);
            int newHeight = atoi(heightBuffer);

            if (newWidth <= 0 || newHeight <= 0) {
                MessageBoxA(hDlg, "Width and Height must be positive numbers.", "Error", MB_OK | MB_ICONERROR);
                return TRUE;
            }

            HWND hCombo = GetDlgItem(hDlg, IDC_SETTINGS_THEME_COMBO);
            int themeIndex = SendMessage(hCombo, CB_GETCURSEL, 0, 0);

            bool sizeChanged = (newWidth != MAP_WIDTH || newHeight != MAP_HEIGHT);
            bool sizeReduced = (newWidth < MAP_WIDTH || newHeight < MAP_HEIGHT);

            if (sizeChanged) {
                if (sizeReduced) {
                    int result = MessageBoxA(hDlg,
                        "Reducing map size may cause data loss in the cropped areas.\n\nDo you want to proceed?",
                        "Warning", MB_YESNO | MB_ICONWARNING);

                    if (result == IDNO) {
                        return TRUE;
                    }
                }

                MAP_WIDTH = newWidth;
                MAP_HEIGHT = newHeight;

                nlohmann::json newTiles = nlohmann::json::array();
                for (int y = 0; y < newHeight; y++) {
                    nlohmann::json row = nlohmann::json::array();
                    for (int x = 0; x < newWidth; x++) {
                        if (y < (int)map_json["tiles"].size() && x < (int)map_json["tiles"][y].size()) {
                            row.push_back(map_json["tiles"][y][x]);
                        }
                        else {
                            row.push_back(0);
                        }
                    }
                    newTiles.push_back(row);
                }
                map_json["tiles"] = newTiles;

                map_grid.clear();
                map_grid.resize(MAP_HEIGHT);
                for (unsigned int y = 0; y < MAP_HEIGHT; y++) {
                    map_grid[y].resize(MAP_WIDTH, UNSELECTED_EMPTY_CELL);
                    for (unsigned int x = 0; x < MAP_WIDTH; x++) {
                        auto& tileData = map_json["tiles"][y][x];
                        if (!tileData.is_array() && tileData.get<int>() != 0) {
                            map_grid[y][x] = UNSELECTED_NONEMPTY_CELL;
                        }
                        else if (tileData.is_array()) {
                            map_grid[y][x] = UNSELECTED_NONEMPTY_CELL;
                        }
                    }
                }

                map_json["width"] = MAP_WIDTH;
                map_json["height"] = MAP_HEIGHT;
            }

            map_json["name"] = std::string(nameBuffer);
            if (strlen(nextMapBuffer) > 0) {
                map_json["nextMap"] = std::string(nextMapBuffer);
            }
            else {
                map_json["nextMap"] = "";
            }

            if (themeIndex != map_theme) {
                map_json["theme"] = themeIndex;
                map_theme = themeIndex;
                switch (map_theme) {
                case 0:
                    hTextureSheet = hTextureSheet0;
                    break;
                case 1:
                    hTextureSheet = hTextureSheet1;
                    break;
                case 2:
                    hTextureSheet = hTextureSheet2;
                    break;
                case 3:
                    hTextureSheet = hTextureSheet3;
                    break;
                }
            }

            current_offset = { 0, 0 };
            mouse_position = { (long)-1, (long)-1 };

            InvalidateRect(hwndFirst, nullptr, true);
            InvalidateRect(hwndSecond, nullptr, true);

            EndDialog(hDlg, IDOK);
            return TRUE;
        }
        else if (wmId == IDC_SETTINGS_CANCEL || wmId == IDCANCEL) {
            EndDialog(hDlg, IDCANCEL);
            return TRUE;
        }
        break;
    }

    case WM_CLOSE:
        EndDialog(hDlg, IDCANCEL);
        return TRUE;
    }
    return FALSE;
}