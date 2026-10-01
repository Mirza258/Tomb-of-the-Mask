#pragma once

#include <windows.h>

class Game;
class Enemy;

enum PauseMenuButton {
  PAUSE_BUTTON_NONE = 0,
  PAUSE_BUTTON_X_CLOSE = 1,
  PAUSE_BUTTON_PLAY = 2,
  PAUSE_BUTTON_EXIT = 3
};

struct PauseMenuLayout {
  RECT dialog;
  RECT xCloseButton;
  RECT playButton;
  RECT exitButton;
};

enum GameOverMenuButton {
  GAMEOVER_BUTTON_NONE = 0,
  GAMEOVER_BUTTON_HOME = 1,
  GAMEOVER_BUTTON_REPLAY = 2
};

struct GameOverMenuLayout {
  RECT dialog;
  RECT homeButton;
  RECT replayButton;
};

class Renderer {
public:
  int pressedPauseButton = PAUSE_BUTTON_NONE;

  int pressedGameOverButton = GAMEOVER_BUTTON_NONE;

  int pressedLevelButton = -1;

  bool pressedShopBuy = false;

  bool pressedShopBuyMagnet = false;

  bool pressedPlayingPause = false;

  bool pressedContinueButton = false;

  void Init();

  void Cleanup();

  void Render(HDC hdc, HWND hWnd, Game &game);

  // Layout helpers
  static PauseMenuLayout GetPauseMenuLayout(const RECT &clientRect);
  static RECT GetShopBuyButtonRect(const RECT &clientRect);
  static RECT GetShopBuyMagnetButtonRect(const RECT &clientRect);
  static RECT GetPlayingPauseButtonRect(const RECT &clientRect);
  static RECT GetLevelCompleteContinueRect(const RECT &clientRect);
  static GameOverMenuLayout GetGameOverMenuLayout(const RECT &clientRect);
  static bool PointInRect(int x, int y, const RECT &r);

private:
  // Sprite sheet
  HBITMAP hWallSheet = NULL;
  HBITMAP hCharSheet = NULL;
  HBITMAP hEnemySheet = NULL;
  HBITMAP hFishSheet = NULL;
  HBITMAP hUISheet = NULL;
  HBITMAP hTitleBmp = NULL;
  HBITMAP hMapTitleBmp = NULL;
  HBITMAP hDoorSheet = NULL;
  HBITMAP hTurretSheet = NULL;
  HBITMAP hSeparatorH = NULL;
  HBITMAP hCharDeathSheet = NULL;

  // Font handles
  HFONT hArcadeFont = NULL;
  HFONT hArcadeFontSmall = NULL;
  WCHAR szFontPath[MAX_PATH] = {};

  int frameCounter = 0;

  // Sprite drawing
  void DrawSprite(HDC hdc, HDC hdcSprite, HBITMAP hSheet, int srcX, int srcY,
                  int destX, int destY);
  void DrawTile(HDC hdc, HDC hdcSprite, int srcX, int srcY, int destX,
                int destY);
  void DrawKillTile(HDC hdc, HDC hdcSprite, int srcX, int srcY, int destX,
                    int destY);
  void DrawUISprite(HDC hdc, HDC hdcSprite, int srcX, int srcY, int destX,
                    int destY);
  void DrawTrailParticles(HDC hdc, const Game &game, int camX, int camY);

  // Button drawing
  void DrawButtonContent(HDC hdcBuffer, HDC hdcSprite, int x, int y, int width,
                         int height, const wchar_t *text, int iconSpriteX,
                         int iconSpriteY, int yOffset);
  void DrawButton(HDC hdcBuffer, HDC hdcSprite, int x, int y, int width,
                  int height, const wchar_t *text, int iconSpriteX,
                  int iconSpriteY, bool isPressed,
                  COLORREF color = RGB(255, 247, 0));

  void DrawTitle(HDC hdcBuffer, const RECT &clientRect, int y);

  void RenderMenu(HDC hdcBuffer, const RECT &clientRect);
  void RenderLevelSelect(HDC hdc, HDC hdcBuffer, RECT &clientRect, Game &game);
  void RenderShop(HDC hdcBuffer, HDC hdcSprite, const RECT &clientRect,
                  Game &game);
  void RenderPlaying(HDC hdc, HDC hdcBuffer, const RECT &clientRect,
                     Game &game);
};
