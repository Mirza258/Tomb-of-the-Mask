#include "TombGame.h"
#include "Constants.h"
#include "Game.h"
#include "Renderer.h"
#include "framework.h"

#define MAX_LOADSTRING 100

// Level selection screen layout constants
#define LEVEL_BUTTON_WIDTH 100
#define LEVEL_BUTTON_HEIGHT 80
#define LEVEL_BUTTON_SPACING 50
#define LEVEL_GRID_START_Y 280
#define LEVEL_ROWS 3
#define NAV_BAR_HEIGHT 100

// Global Variables:
HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

// Game instance
Game game;

// Renderer instance
Renderer renderer;

// Forward declarations
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
void HandleLevelSelectDown(int mouseX, int mouseY, RECT &clientRect);
void HandleLevelSelectUp(int mouseX, int mouseY, RECT &clientRect);
void HandlePauseMenuDown(int mouseX, int mouseY, RECT &clientRect);
void HandlePauseMenuUp(int mouseX, int mouseY, RECT &clientRect);
void HandleGameOverMenuDown(int mouseX, int mouseY, RECT &clientRect);
void HandleGameOverMenuUp(int mouseX, int mouseY, RECT &clientRect);

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance, LPSTR lpCmdLine,
                     int nCmdShow) {
  (void)hPrevInstance;
  (void)lpCmdLine;

  // Initialize global strings
  LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
  LoadStringW(hInstance, IDC_TOMBGAME, szWindowClass, MAX_LOADSTRING);
  MyRegisterClass(hInstance);

  // Perform application initialization:
  if (!InitInstance(hInstance, nCmdShow)) {
    return FALSE;
  }

  HACCEL hAccelTable =
      LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TOMBGAME));

  MSG msg;

  // Main message loop:
  while (GetMessage(&msg, nullptr, 0, 0)) {
    if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }

  return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance) {
  WNDCLASSEXW wcex;

  wcex.cbSize = sizeof(WNDCLASSEX);

  wcex.style = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc = WndProc;
  wcex.cbClsExtra = 0;
  wcex.cbWndExtra = 0;
  wcex.hInstance = hInstance;
  wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TOMBGAME));
  wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH); // Black background
  wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_TOMBGAME);
  wcex.lpszClassName = szWindowClass;
  wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

  return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
  hInst = hInstance;

  int windowWidth = DEFAULT_WINDOW_WIDTH;
  int windowHeight = DEFAULT_WINDOW_HEIGHT;

  HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, 160,
                            90, windowWidth, windowHeight, nullptr, nullptr,
                            hInstance, nullptr);

  if (!hWnd) {
    return FALSE;
  }

  ShowWindow(hWnd, nCmdShow);
  UpdateWindow(hWnd);

  return TRUE;
}

// Handle mouse button down on level selection screen
void HandleLevelSelectDown(int mouseX, int mouseY, RECT &clientRect) {

  int navBarY = clientRect.bottom - NAV_BAR_HEIGHT;
  int buttonWidth = clientRect.right / 2;

  if (mouseY >= navBarY) {
    if (mouseX < buttonWidth) {
      game.selectedTab = 0;
    } else {
      game.selectedTab = 1;
    }
    return;
  }

  // Shop buy buttons
  if (game.selectedTab == 1) {
    RECT buyBtn = Renderer::GetShopBuyButtonRect(clientRect);
    if (Renderer::PointInRect(mouseX, mouseY, buyBtn)) {
      renderer.pressedShopBuy = true;
    }
    RECT buyMagnetBtn = Renderer::GetShopBuyMagnetButtonRect(clientRect);
    if (Renderer::PointInRect(mouseX, mouseY, buyMagnetBtn)) {
      renderer.pressedShopBuyMagnet = true;
    }
    return;
  }

  // Track which level button is pressed
  if (game.selectedTab != 0)
    return;

  int numLevels = (int)game.availableLevels.size();
  int numCols = (numLevels + LEVEL_ROWS - 1) / LEVEL_ROWS;
  int totalGridWidth = numCols * LEVEL_BUTTON_WIDTH +
                       (numCols > 1 ? (numCols - 1) * LEVEL_BUTTON_SPACING : 0);
  int gridStartX = (clientRect.right - totalGridWidth) / 2;

  renderer.pressedLevelButton = -1;
  for (int i = 0; i < numLevels; i++) {
    int levelNum = game.availableLevels[i];
    int col = i / LEVEL_ROWS;
    int rowInCol = i % LEVEL_ROWS;
    int row = (col % 2 == 0) ? rowInCol : (LEVEL_ROWS - 1 - rowInCol);

    int buttonX =
        gridStartX + col * (LEVEL_BUTTON_WIDTH + LEVEL_BUTTON_SPACING);
    int buttonY =
        LEVEL_GRID_START_Y + row * (LEVEL_BUTTON_HEIGHT + LEVEL_BUTTON_SPACING);

    RECT buttonRect = {buttonX, buttonY, buttonX + LEVEL_BUTTON_WIDTH,
                       buttonY + LEVEL_BUTTON_HEIGHT};

    if (Renderer::PointInRect(mouseX, mouseY, buttonRect)) {
      bool isUnlocked =
          (levelNum == 1) || game.db.IsLevelCompleted(levelNum - 1);
      if (isUnlocked) {
        renderer.pressedLevelButton = levelNum;
      }
      return;
    }
  }
}

// Handle mouse button up on level selection screen
void HandleLevelSelectUp(int mouseX, int mouseY, RECT &clientRect) {
  // Shop buy button (shield)
  if (renderer.pressedShopBuy) {
    renderer.pressedShopBuy = false;
    RECT buyBtn = Renderer::GetShopBuyButtonRect(clientRect);
    if (Renderer::PointInRect(mouseX, mouseY, buyBtn)) {
      int totalCoins = game.db.GetTotalCoins();
      if (totalCoins >= 200) {
        game.db.SetTotalCoins(totalCoins - 200);
        int shields = game.db.GetShieldCount();
        game.db.SetShieldCount(shields + 1);
      }
    }
    return;
  }

  // Shop buy button (magnet)
  if (renderer.pressedShopBuyMagnet) {
    renderer.pressedShopBuyMagnet = false;
    RECT buyMagnetBtn = Renderer::GetShopBuyMagnetButtonRect(clientRect);
    if (Renderer::PointInRect(mouseX, mouseY, buyMagnetBtn)) {
      int totalCoins = game.db.GetTotalCoins();
      if (totalCoins >= 50) {
        game.db.SetTotalCoins(totalCoins - 50);
        int magnets = game.db.GetMagnetCount();
        game.db.SetMagnetCount(magnets + 1);
      }
    }
    return;
  }

  if (renderer.pressedLevelButton == -1)
    return;

  int pressedLevel = renderer.pressedLevelButton;
  renderer.pressedLevelButton = -1;

  if (game.selectedTab != 0)
    return;

  int numLevels = (int)game.availableLevels.size();
  int numCols = (numLevels + LEVEL_ROWS - 1) / LEVEL_ROWS;
  int totalGridWidth = numCols * LEVEL_BUTTON_WIDTH +
                       (numCols > 1 ? (numCols - 1) * LEVEL_BUTTON_SPACING : 0);
  int gridStartX = (clientRect.right - totalGridWidth) / 2;

  for (int i = 0; i < numLevels; i++) {
    int levelNum = game.availableLevels[i];
    if (levelNum != pressedLevel)
      continue;

    int col = i / LEVEL_ROWS;
    int rowInCol = i % LEVEL_ROWS;
    int row = (col % 2 == 0) ? rowInCol : (LEVEL_ROWS - 1 - rowInCol);

    int buttonX =
        gridStartX + col * (LEVEL_BUTTON_WIDTH + LEVEL_BUTTON_SPACING);
    int buttonY =
        LEVEL_GRID_START_Y + row * (LEVEL_BUTTON_HEIGHT + LEVEL_BUTTON_SPACING);

    RECT buttonRect = {buttonX, buttonY, buttonX + LEVEL_BUTTON_WIDTH,
                       buttonY + LEVEL_BUTTON_HEIGHT};

    if (Renderer::PointInRect(mouseX, mouseY, buttonRect)) {
      game.SelectLevel(levelNum);
    }
    return;
  }
}

// Handle mouse button down on pause menu
void HandlePauseMenuDown(int mouseX, int mouseY, RECT &clientRect) {
  PauseMenuLayout layout = Renderer::GetPauseMenuLayout(clientRect);

  if (Renderer::PointInRect(mouseX, mouseY, layout.xCloseButton)) {
    renderer.pressedPauseButton = PAUSE_BUTTON_X_CLOSE;
  } else if (Renderer::PointInRect(mouseX, mouseY, layout.playButton)) {
    renderer.pressedPauseButton = PAUSE_BUTTON_PLAY;
  } else if (Renderer::PointInRect(mouseX, mouseY, layout.exitButton)) {
    renderer.pressedPauseButton = PAUSE_BUTTON_EXIT;
  } else {
    renderer.pressedPauseButton = PAUSE_BUTTON_NONE;
  }
}

// Handle mouse button up on pause menu
void HandlePauseMenuUp(int mouseX, int mouseY, RECT &clientRect) {
  if (renderer.pressedPauseButton == PAUSE_BUTTON_NONE)
    return;

  PauseMenuLayout layout = Renderer::GetPauseMenuLayout(clientRect);
  int buttonToActivate = renderer.pressedPauseButton;
  renderer.pressedPauseButton = PAUSE_BUTTON_NONE;

  if (buttonToActivate == PAUSE_BUTTON_X_CLOSE &&
      Renderer::PointInRect(mouseX, mouseY, layout.xCloseButton)) {
    game.Resume();
  } else if (buttonToActivate == PAUSE_BUTTON_PLAY &&
             Renderer::PointInRect(mouseX, mouseY, layout.playButton)) {
    game.Resume();
  } else if (buttonToActivate == PAUSE_BUTTON_EXIT &&
             Renderer::PointInRect(mouseX, mouseY, layout.exitButton)) {
    game.ReturnToMenu();
  }
}

// Handle mouse button down on game over menu
void HandleGameOverMenuDown(int mouseX, int mouseY, RECT &clientRect) {
  GameOverMenuLayout layout = Renderer::GetGameOverMenuLayout(clientRect);

  if (Renderer::PointInRect(mouseX, mouseY, layout.homeButton)) {
    renderer.pressedGameOverButton = GAMEOVER_BUTTON_HOME;
  } else if (Renderer::PointInRect(mouseX, mouseY, layout.replayButton)) {
    renderer.pressedGameOverButton = GAMEOVER_BUTTON_REPLAY;
  } else {
    renderer.pressedGameOverButton = GAMEOVER_BUTTON_NONE;
  }
}

// Handle mouse button up on game over menu
void HandleGameOverMenuUp(int mouseX, int mouseY, RECT &clientRect) {
  if (renderer.pressedGameOverButton == GAMEOVER_BUTTON_NONE)
    return;

  GameOverMenuLayout layout = Renderer::GetGameOverMenuLayout(clientRect);
  int buttonToActivate = renderer.pressedGameOverButton;
  renderer.pressedGameOverButton = GAMEOVER_BUTTON_NONE;

  if (buttonToActivate == GAMEOVER_BUTTON_HOME &&
      Renderer::PointInRect(mouseX, mouseY, layout.homeButton)) {
    game.ReturnToMenu();
  } else if (buttonToActivate == GAMEOVER_BUTTON_REPLAY &&
             Renderer::PointInRect(mouseX, mouseY, layout.replayButton)) {
    game.Reset();
  }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam,
                         LPARAM lParam) {

  switch (message) {
  case WM_CREATE:
    renderer.Init();
    game.Init();
    SetTimer(hWnd, TIMER_GAME, TIMER_INTERVAL_MS, NULL);
    break;

  case WM_TIMER:
    if (wParam == TIMER_GAME) {
      game.Update();
      InvalidateRect(hWnd, NULL, FALSE);
    }
    break;

  case WM_KEYDOWN:
    game.HandleInput(wParam);
    InvalidateRect(hWnd, NULL, FALSE);
    break;

  case WM_LBUTTONDOWN:
    if (game.state == STATE_LEVEL_SELECT) {
      int mouseX = LOWORD(lParam);
      int mouseY = HIWORD(lParam);
      RECT clientRect;
      GetClientRect(hWnd, &clientRect);
      HandleLevelSelectDown(mouseX, mouseY, clientRect);
      InvalidateRect(hWnd, NULL, FALSE);
    } else if (game.state == STATE_PAUSED) {
      int mouseX = LOWORD(lParam);
      int mouseY = HIWORD(lParam);
      RECT clientRect;
      GetClientRect(hWnd, &clientRect);
      HandlePauseMenuDown(mouseX, mouseY, clientRect);
      InvalidateRect(hWnd, NULL, FALSE);
    } else if (game.state == STATE_PLAYING) {
      int mouseX = LOWORD(lParam);
      int mouseY = HIWORD(lParam);
      RECT clientRect;
      GetClientRect(hWnd, &clientRect);
      RECT pauseBtn = Renderer::GetPlayingPauseButtonRect(clientRect);
      if (Renderer::PointInRect(mouseX, mouseY, pauseBtn)) {
        renderer.pressedPlayingPause = true;
      }
      InvalidateRect(hWnd, NULL, FALSE);
    } else if (game.state == STATE_LEVEL_COMPLETE) {
      int mouseX = LOWORD(lParam);
      int mouseY = HIWORD(lParam);
      RECT clientRect;
      GetClientRect(hWnd, &clientRect);
      RECT contBtn = Renderer::GetLevelCompleteContinueRect(clientRect);
      if (Renderer::PointInRect(mouseX, mouseY, contBtn)) {
        renderer.pressedContinueButton = true;
      }
      InvalidateRect(hWnd, NULL, FALSE);
    } else if (game.state == STATE_GAME_OVER) {
      int mouseX = LOWORD(lParam);
      int mouseY = HIWORD(lParam);
      RECT clientRect;
      GetClientRect(hWnd, &clientRect);
      HandleGameOverMenuDown(mouseX, mouseY, clientRect);
      InvalidateRect(hWnd, NULL, FALSE);
    }
    break;

  case WM_LBUTTONUP:
    if (game.state == STATE_LEVEL_SELECT) {
      int mouseX = LOWORD(lParam);
      int mouseY = HIWORD(lParam);
      RECT clientRect;
      GetClientRect(hWnd, &clientRect);
      HandleLevelSelectUp(mouseX, mouseY, clientRect);
      InvalidateRect(hWnd, NULL, FALSE);
    } else if (game.state == STATE_PAUSED) {
      int mouseX = LOWORD(lParam);
      int mouseY = HIWORD(lParam);
      RECT clientRect;
      GetClientRect(hWnd, &clientRect);
      HandlePauseMenuUp(mouseX, mouseY, clientRect);
      InvalidateRect(hWnd, NULL, FALSE);
    } else if (game.state == STATE_PLAYING && renderer.pressedPlayingPause) {
      int mouseX = LOWORD(lParam);
      int mouseY = HIWORD(lParam);
      RECT clientRect;
      GetClientRect(hWnd, &clientRect);
      RECT pauseBtn = Renderer::GetPlayingPauseButtonRect(clientRect);
      renderer.pressedPlayingPause = false;
      if (Renderer::PointInRect(mouseX, mouseY, pauseBtn)) {
        game.Pause();
      }
      InvalidateRect(hWnd, NULL, FALSE);
    } else if (game.state == STATE_LEVEL_COMPLETE &&
               renderer.pressedContinueButton) {
      int mouseX = LOWORD(lParam);
      int mouseY = HIWORD(lParam);
      RECT clientRect;
      GetClientRect(hWnd, &clientRect);
      RECT contBtn = Renderer::GetLevelCompleteContinueRect(clientRect);
      renderer.pressedContinueButton = false;
      if (Renderer::PointInRect(mouseX, mouseY, contBtn)) {
        game.ReturnToMenu();
      }
      InvalidateRect(hWnd, NULL, FALSE);
    } else if (game.state == STATE_GAME_OVER) {
      int mouseX = LOWORD(lParam);
      int mouseY = HIWORD(lParam);
      RECT clientRect;
      GetClientRect(hWnd, &clientRect);
      HandleGameOverMenuUp(mouseX, mouseY, clientRect);
      InvalidateRect(hWnd, NULL, FALSE);
    }
    break;

  case WM_COMMAND: {
    int wmId = LOWORD(wParam);
    switch (wmId) {
    case IDM_ABOUT:
      DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
      break;
    case IDM_EXIT:
      DestroyWindow(hWnd);
      break;
    case IDM_DEBUG_RESETDB:
      game.db.ResetDatabase();
      game.ReturnToMenu();
      InvalidateRect(hWnd, NULL, FALSE);
      break;
    case IDM_DEBUG_ADDCOINS:
      game.db.AddCoins(1000);
      InvalidateRect(hWnd, NULL, FALSE);
      break;
    case IDM_DEBUG_UNLOCKLEVELS:
      if (!game.availableLevels.empty()) {
        game.db.UnlockAllLevels(game.availableLevels.back());
      }
      InvalidateRect(hWnd, NULL, FALSE);
      break;
    default:
      return DefWindowProc(hWnd, message, wParam, lParam);
    }
  } break;

  case WM_PAINT: {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hWnd, &ps);
    renderer.Render(hdc, hWnd, game);
    EndPaint(hWnd, &ps);
  } break;

  case WM_DESTROY:
    KillTimer(hWnd, TIMER_GAME);
    game.Cleanup();
    renderer.Cleanup();
    PostQuitMessage(0);
    break;

  default:
    return DefWindowProc(hWnd, message, wParam, lParam);
  }
  return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
  (void)lParam;
  switch (message) {
  case WM_INITDIALOG:
    return (INT_PTR)TRUE;

  case WM_COMMAND:
    if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
      EndDialog(hDlg, LOWORD(wParam));
      return (INT_PTR)TRUE;
    }
    break;
  }
  return (INT_PTR)FALSE;
}
