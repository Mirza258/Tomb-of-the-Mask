#include "Renderer.h"
#include "Constants.h"
#include "Enemy.h"
#include "Game.h"
#include "MapObjects.h"

// Transparent color (magenta)
#define TRANSPARENT_COLOR RGB(255, 0, 255)

// Button constants
#define BUTTON_COLOR RGB(255, 247, 0)
#define BUTTON_TEXT_COLOR RGB(0, 0, 0)
#define BUTTON_SHADOW_INSET_1 10
#define BUTTON_SHADOW_INSET_2 20
#define BUTTON_ICON_OFFSET_Y 15
#define BUTTON_TEXT_OFFSET_Y 50
#define BUTTON_3D_OFFSET 4
#define BUTTON_PRESS_OFFSET 3
#define NAV_ICON_SIZE 32

// Level selection constants
#define LEVEL_BUTTON_WIDTH 100
#define LEVEL_BUTTON_HEIGHT 80
#define LEVEL_BUTTON_SPACING 50
#define LEVEL_GRID_START_Y 280
#define LEVEL_ROWS 3
#define NAV_BAR_HEIGHT 100

// ========== Initialization / Cleanup ==========

void Renderer::Init() {
  WCHAR exePath[MAX_PATH];
  GetModuleFileNameW(NULL, exePath, MAX_PATH);
  WCHAR *lastSlash = wcsrchr(exePath, L'\\');
  if (lastSlash)
    *lastSlash = L'\0';

  WCHAR wallPath[MAX_PATH], charPath[MAX_PATH], charDeathPath[MAX_PATH],
      enemyPath[MAX_PATH], fishPath[MAX_PATH], turretPath[MAX_PATH],
      uiPath[MAX_PATH], titlePath[MAX_PATH], mapTitlePath[MAX_PATH],
      doorPath[MAX_PATH], sepHPath[MAX_PATH];
  wsprintfW(wallPath, L"%s\\assets\\Walls-Sheet.bmp", exePath);
  wsprintfW(charPath, L"%s\\assets\\MainChar-Sheet.bmp", exePath);
  wsprintfW(charDeathPath, L"%s\\assets\\mainChar-death-Sheet.bmp", exePath);
  wsprintfW(enemyPath, L"%s\\assets\\enemies-Sheet.bmp", exePath);
  wsprintfW(fishPath, L"%s\\assets\\enemy-fish-Sheet.bmp", exePath);
  wsprintfW(turretPath, L"%s\\assets\\enemy-turret-Sheet.bmp", exePath);
  wsprintfW(uiPath, L"%s\\assets\\ui-Sheet.bmp", exePath);
  wsprintfW(titlePath, L"%s\\assets\\title.bmp", exePath);
  wsprintfW(mapTitlePath, L"%s\\assets\\map-title.bmp", exePath);
  wsprintfW(doorPath, L"%s\\assets\\doors-Sheet.bmp", exePath);
  wsprintfW(sepHPath, L"%s\\assets\\separator-horizonatl.bmp", exePath);

  hWallSheet =
      (HBITMAP)LoadImageW(NULL, wallPath, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
  hCharSheet =
      (HBITMAP)LoadImageW(NULL, charPath, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
  hCharDeathSheet = (HBITMAP)LoadImageW(NULL, charDeathPath, IMAGE_BITMAP, 0, 0,
                                        LR_LOADFROMFILE);
  hEnemySheet =
      (HBITMAP)LoadImageW(NULL, enemyPath, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
  hFishSheet =
      (HBITMAP)LoadImageW(NULL, fishPath, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
  hTurretSheet = (HBITMAP)LoadImageW(NULL, turretPath, IMAGE_BITMAP, 0, 0,
                                     LR_LOADFROMFILE);
  hUISheet =
      (HBITMAP)LoadImageW(NULL, uiPath, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
  hTitleBmp =
      (HBITMAP)LoadImageW(NULL, titlePath, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
  hMapTitleBmp = (HBITMAP)LoadImageW(NULL, mapTitlePath, IMAGE_BITMAP, 0, 0,
                                     LR_LOADFROMFILE);
  hDoorSheet =
      (HBITMAP)LoadImageW(NULL, doorPath, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
  hSeparatorH =
      (HBITMAP)LoadImageW(NULL, sepHPath, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

  if (!hWallSheet || !hCharSheet || !hEnemySheet) {
    WCHAR msg[512];
    wsprintfW(msg, L"Failed to load sprites!\nWall: %s\nEnemy: %s\n",
              hWallSheet ? L"YES" : L"NO", hEnemySheet ? L"YES" : L"NO");
    MessageBoxW(NULL, msg, L"Sprite Loading Error", MB_OK | MB_ICONERROR);
  }

  wsprintfW(szFontPath, L"%s\\assets\\font\\LowresPixel-Regular.otf", exePath);
  AddFontResourceExW(szFontPath, FR_PRIVATE, 0);
  hArcadeFont =
      CreateFontW(64, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                  OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
                  DEFAULT_PITCH | FF_DONTCARE, L"Lower Pixel");
  hArcadeFontSmall =
      CreateFontW(32, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                  OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
                  DEFAULT_PITCH | FF_DONTCARE, L"Lower Pixel");
}

void Renderer::Cleanup() {
  if (hWallSheet)
    DeleteObject(hWallSheet);
  if (hCharSheet)
    DeleteObject(hCharSheet);
  if (hCharDeathSheet)
    DeleteObject(hCharDeathSheet);
  if (hEnemySheet)
    DeleteObject(hEnemySheet);
  if (hFishSheet)
    DeleteObject(hFishSheet);
  if (hTurretSheet)
    DeleteObject(hTurretSheet);
  if (hUISheet)
    DeleteObject(hUISheet);
  if (hTitleBmp)
    DeleteObject(hTitleBmp);
  if (hMapTitleBmp)
    DeleteObject(hMapTitleBmp);
  if (hDoorSheet)
    DeleteObject(hDoorSheet);
  if (hSeparatorH)
    DeleteObject(hSeparatorH);
  if (hArcadeFont)
    DeleteObject(hArcadeFont);
  if (hArcadeFontSmall)
    DeleteObject(hArcadeFontSmall);
  RemoveFontResourceExW(szFontPath, FR_PRIVATE, 0);
}

// ========== Layout Helpers ==========

PauseMenuLayout Renderer::GetPauseMenuLayout(const RECT &clientRect) {
  PauseMenuLayout layout;
  int dialogWidth = 400;
  int dialogHeight = 300;
  int dialogX = (clientRect.right - dialogWidth) / 2;
  int dialogY = (clientRect.bottom - dialogHeight) / 2;
  layout.dialog = {dialogX, dialogY, dialogX + dialogWidth,
                   dialogY + dialogHeight};

  // X close button - top right of header
  int xButtonSize = 32;
  int xButtonX = dialogX + dialogWidth - xButtonSize - 10;
  int xButtonY = dialogY + 10;
  layout.xCloseButton = {xButtonX, xButtonY, xButtonX + xButtonSize,
                         xButtonY + xButtonSize};

  // Buttons below separator: EXIT left, RESUME right
  int buttonWidth = 120;
  int buttonHeight = 80;
  int buttonSpacing = 20;
  int totalButtonWidth = buttonWidth * 2 + buttonSpacing;
  int buttonsStartX = dialogX + (dialogWidth - totalButtonWidth) / 2;
  int buttonsY = dialogY + 185;
  layout.exitButton = {buttonsStartX, buttonsY, buttonsStartX + buttonWidth,
                       buttonsY + buttonHeight};
  int playButtonX = buttonsStartX + buttonWidth + buttonSpacing;
  layout.playButton = {playButtonX, buttonsY, playButtonX + buttonWidth,
                       buttonsY + buttonHeight};
  return layout;
}

RECT Renderer::GetLevelCompleteContinueRect(const RECT &clientRect) {
  int dlgW = 400;
  int dlgH = 420;
  int dlgX = (clientRect.right - dlgW) / 2;
  int dlgY = (clientRect.bottom - dlgH) / 2;
  int btnW = 200;
  int btnH = 60;
  int btnX = dlgX + (dlgW - btnW) / 2;
  int btnY = dlgY + dlgH - btnH - 20;
  return {btnX, btnY, btnX + btnW, btnY + btnH};
}

RECT Renderer::GetPlayingPauseButtonRect(const RECT &clientRect) {
  int btnSize = 60;
  int btnX = clientRect.right - btnSize - 20;
  int btnY = (HUD_STRIPE_TOP_H - btnSize) / 2;
  return {btnX, btnY, btnX + btnSize, btnY + btnSize};
}

GameOverMenuLayout Renderer::GetGameOverMenuLayout(const RECT &clientRect) {
  GameOverMenuLayout layout;
  int dialogWidth = 400;
  int dialogHeight = 300;
  int dialogX = (clientRect.right - dialogWidth) / 2;
  int dialogY = (clientRect.bottom - dialogHeight) / 2;
  layout.dialog = {dialogX, dialogY, dialogX + dialogWidth,
                   dialogY + dialogHeight};

  // Buttons below separator: HOME left, REPLAY right
  int buttonWidth = 120;
  int buttonHeight = 80;
  int buttonSpacing = 20;
  int totalButtonWidth = buttonWidth * 2 + buttonSpacing;
  int buttonsStartX = dialogX + (dialogWidth - totalButtonWidth) / 2;
  int buttonsY = dialogY + 185;
  layout.homeButton = {buttonsStartX, buttonsY, buttonsStartX + buttonWidth,
                       buttonsY + buttonHeight};
  int replayButtonX = buttonsStartX + buttonWidth + buttonSpacing;
  layout.replayButton = {replayButtonX, buttonsY, replayButtonX + buttonWidth,
                         buttonsY + buttonHeight};
  return layout;
}

bool Renderer::PointInRect(int x, int y, const RECT &r) {
  return x >= r.left && x < r.right && y >= r.top && y < r.bottom;
}

// ========== Sprite Drawing ==========

void Renderer::DrawSprite(HDC hdc, HDC hdcSprite, HBITMAP hSheet, int srcX,
                          int srcY, int destX, int destY) {
  if (srcX < 0 || srcY < 0 || !hSheet)
    return;
  HBITMAP oldBmp = (HBITMAP)SelectObject(hdcSprite, hSheet);
  TransparentBlt(hdc, destX, destY, TILE_SIZE, TILE_SIZE, hdcSprite,
                 srcX * TILE_SIZE, srcY * TILE_SIZE, TILE_SIZE, TILE_SIZE,
                 TRANSPARENT_COLOR);
  SelectObject(hdcSprite, oldBmp);
}

void Renderer::DrawTile(HDC hdc, HDC hdcSprite, int srcX, int srcY, int destX,
                        int destY) {
  DrawSprite(hdc, hdcSprite, hWallSheet, srcX, srcY, destX, destY);
}

void Renderer::DrawKillTile(HDC hdc, HDC hdcSprite, int srcX, int srcY,
                            int destX, int destY) {
  DrawSprite(hdc, hdcSprite, hEnemySheet, srcX, srcY, destX, destY);
}

void Renderer::DrawUISprite(HDC hdc, HDC hdcSprite, int srcX, int srcY,
                            int destX, int destY) {
  DrawSprite(hdc, hdcSprite, hUISheet, srcX, srcY, destX, destY);
}

void Renderer::DrawTrailParticles(HDC hdc, const Game &game, int camX,
                                  int camY) {
  const TrailParticle *trails = game.player.GetTrailParticles();
  for (int i = 0; i < Player::MAX_TRAIL_PARTICLES; i++) {
    if (trails[i].active) {
      int screenX = trails[i].x - camX;
      int screenY = trails[i].y - camY;

      int intensity = trails[i].alpha;
      COLORREF trailColor = RGB(intensity, intensity, 0);
      HBRUSH hBrush = CreateSolidBrush(trailColor);

      RECT trailRect;
      if (trails[i].isHorizontal) {
        trailRect = {screenX, screenY + 12, screenX + TILE_SIZE,
                     screenY + TILE_SIZE - 12};
      } else {
        trailRect = {screenX + 12, screenY, screenX + TILE_SIZE - 12,
                     screenY + TILE_SIZE};
      }

      FillRect(hdc, &trailRect, hBrush);
      DeleteObject(hBrush);
    }
  }
}

// ========== Button Drawing ==========

void Renderer::DrawButtonContent(HDC hdcBuffer, HDC hdcSprite, int x, int y,
                                 int width, int height, const wchar_t *text,
                                 int iconSpriteX, int iconSpriteY,
                                 int yOffset) {
  bool hasIcon = (iconSpriteX >= 0);
  bool hasText = (text && text[0]);

  if (hasIcon && hUISheet) {
    HBITMAP oldBmp = (HBITMAP)SelectObject(hdcSprite, hUISheet);
    int iconX = x + (width - NAV_ICON_SIZE) / 2;
    int iconY;
    if (hasText) {
      iconY = y + BUTTON_ICON_OFFSET_Y + yOffset;
    } else {
      iconY = y + (height - NAV_ICON_SIZE) / 2 + yOffset;
    }
    TransparentBlt(hdcBuffer, iconX, iconY, NAV_ICON_SIZE, NAV_ICON_SIZE,
                   hdcSprite, iconSpriteX * TILE_SIZE, iconSpriteY * TILE_SIZE,
                   TILE_SIZE, TILE_SIZE, TRANSPARENT_COLOR);
    SelectObject(hdcSprite, oldBmp);
  }

  if (hasText) {
    SetBkMode(hdcBuffer, TRANSPARENT);
    SetTextColor(hdcBuffer, BUTTON_TEXT_COLOR);
    if (hasIcon) {
      HFONT oldFont = (HFONT)SelectObject(hdcBuffer, hArcadeFontSmall);
      RECT textRect = {x, y + BUTTON_TEXT_OFFSET_Y + yOffset, x + width,
                       y + height + yOffset};
      DrawTextW(hdcBuffer, text, -1, &textRect, DT_CENTER | DT_TOP);
      SelectObject(hdcBuffer, oldFont);
    } else {
      HFONT oldFont = (HFONT)SelectObject(hdcBuffer, hArcadeFont);
      RECT textRect = {x, y + yOffset, x + width, y + height + yOffset};
      DrawTextW(hdcBuffer, text, -1, &textRect,
                DT_CENTER | DT_VCENTER | DT_SINGLELINE);
      SelectObject(hdcBuffer, oldFont);
    }
  }
}

void Renderer::DrawButton(HDC hdcBuffer, HDC hdcSprite, int x, int y, int width,
                          int height, const wchar_t *text, int iconSpriteX,
                          int iconSpriteY, bool isPressed, COLORREF color) {
  HBRUSH yellowBrush = CreateSolidBrush(color);
  HBRUSH blackBrush = CreateSolidBrush(RGB(0, 0, 0));

  RECT buttonRect = {x, y, x + width, y + height};
  FillRect(hdcBuffer, &buttonRect, yellowBrush);

  if (isPressed) {
    RECT borderTop = {x - 2, y - 2, x + width + 2, y};
    FillRect(hdcBuffer, &borderTop, blackBrush);
    RECT borderBottom = {x - 2, y + height, x + width + 2, y + height + 2};
    FillRect(hdcBuffer, &borderBottom, blackBrush);
    RECT borderLeft = {x - 2, y, x, y + height};
    FillRect(hdcBuffer, &borderLeft, blackBrush);
    RECT borderRight = {x + width, y, x + width + 2, y + height};
    FillRect(hdcBuffer, &borderRight, blackBrush);
  } else {
    RECT shadowTop1 = {x - 2, y - 2, x + width + 2, y};
    FillRect(hdcBuffer, &shadowTop1, blackBrush);
    RECT top1 = {x + 2, y - 2, x + width - 2, y};
    FillRect(hdcBuffer, &top1, yellowBrush);
    RECT shadowTop2 = {x + 2, y - 4, x + width - 2, y - 2};
    FillRect(hdcBuffer, &shadowTop2, blackBrush);
    RECT top2 = {x + 6, y - 4, x + width - 6, y - 2};
    FillRect(hdcBuffer, &top2, yellowBrush);
    RECT shadowTop3 = {x + 6, y - 6, x + width - 6, y - 4};
    FillRect(hdcBuffer, &shadowTop3, blackBrush);
    RECT shadow1 = {x - 2, y + height, x + width + 2, y + height + 2};
    FillRect(hdcBuffer, &shadow1, blackBrush);
    RECT shadow2 = {x + 2, y + height - 2, x + width - 2, y + height};
    FillRect(hdcBuffer, &shadow2, blackBrush);
    RECT shadow3 = {x + 6, y + height - 4, x + width - 6, y + height - 2};
    FillRect(hdcBuffer, &shadow3, blackBrush);
    RECT leftOutline = {x - 2, y, x, y + height};
    FillRect(hdcBuffer, &leftOutline, blackBrush);
    RECT rightOutline = {x + width, y, x + width + 2, y + height};
    FillRect(hdcBuffer, &rightOutline, blackBrush);
  }

  int contentOffset = isPressed ? BUTTON_PRESS_OFFSET : 0;
  DrawButtonContent(hdcBuffer, hdcSprite, x, y, width, height, text,
                    iconSpriteX, iconSpriteY, contentOffset);

  DeleteObject(yellowBrush);
  DeleteObject(blackBrush);
}

// ========== Title Drawing ==========

void Renderer::DrawTitle(HDC hdcBuffer, const RECT &clientRect, int y) {
  if (!hTitleBmp)
    return;
  HDC hdcTitle = CreateCompatibleDC(hdcBuffer);
  HBITMAP oldBmp = (HBITMAP)SelectObject(hdcTitle, hTitleBmp);
  int titleW = 360;
  int titleH = 80;
  int titleX = (clientRect.right - titleW) / 2;
  TransparentBlt(hdcBuffer, titleX, y, titleW, titleH, hdcTitle, 0, 0, titleW,
                 titleH, TRANSPARENT_COLOR);
  SelectObject(hdcTitle, oldBmp);
  DeleteDC(hdcTitle);
}

// ========== State-Specific Rendering ==========

void Renderer::RenderMenu(HDC hdcBuffer, const RECT &clientRect) {
  HBRUSH blackBrush = CreateSolidBrush(RGB(0, 0, 0));
  FillRect(hdcBuffer, &clientRect, blackBrush);
  DeleteObject(blackBrush);

  DrawTitle(hdcBuffer, clientRect, 50);

  // Flash "PRESS ENTER TO PLAY"
  COLORREF flashColor;
  if ((frameCounter / 30) % 2 == 0)
    flashColor = RGB(255, 247, 0); // yellow
  else
    flashColor = RGB(212, 0, 255); // pink

  SetBkMode(hdcBuffer, TRANSPARENT);
  SetTextColor(hdcBuffer, flashColor);
  HFONT oldFont = (HFONT)SelectObject(hdcBuffer, hArcadeFont);
  DrawTextW(hdcBuffer, L"PRESS ENTER TO PLAY", -1, (LPRECT)&clientRect,
            DT_CENTER | DT_VCENTER | DT_SINGLELINE);
  SelectObject(hdcBuffer, oldFont);
}

// Calculate button position from level index.
// Parni top-to-bottom , neparni bottom-to-top
static void GetLevelButtonPos(int i, int gridStartX, int &buttonX,
                              int &buttonY) {
  int col = i / LEVEL_ROWS;
  int rowInCol = i % LEVEL_ROWS;
  int row = (col % 2 == 0) ? rowInCol : (LEVEL_ROWS - 1 - rowInCol);
  buttonX = gridStartX + col * (LEVEL_BUTTON_WIDTH + LEVEL_BUTTON_SPACING);
  buttonY =
      LEVEL_GRID_START_Y + row * (LEVEL_BUTTON_HEIGHT + LEVEL_BUTTON_SPACING);
}

void Renderer::RenderLevelSelect(HDC hdc, HDC hdcBuffer, RECT &clientRect,
                                 Game &game) {
  HBRUSH blackBrush = CreateSolidBrush(RGB(0, 0, 0));
  FillRect(hdcBuffer, &clientRect, blackBrush);
  DeleteObject(blackBrush);

  DrawTitle(hdcBuffer, clientRect, 10);

  HDC hdcSprite = CreateCompatibleDC(hdc);

  // Draw coin icon and number
  if (hUISheet) {
    int coinIconSize = 64;
    int coinX = 20;
    int coinY = 20;

    HBITMAP oldBmp = (HBITMAP)SelectObject(hdcSprite, hUISheet);
    TransparentBlt(hdcBuffer, coinX, coinY, coinIconSize, coinIconSize,
                   hdcSprite, 0 * TILE_SIZE, 6 * TILE_SIZE, TILE_SIZE,
                   TILE_SIZE, TRANSPARENT_COLOR);
    SelectObject(hdcSprite, oldBmp);

    int totalCoins = game.db.GetTotalCoins();
    WCHAR coinsText[32];
    wsprintfW(coinsText, L"%d", totalCoins);

    SetBkMode(hdcBuffer, TRANSPARENT);
    SetTextColor(hdcBuffer, RGB(255, 247, 0));
    HFONT oldCoinFont = (HFONT)SelectObject(hdcBuffer, hArcadeFontSmall);
    TextOutW(hdcBuffer, coinX + coinIconSize, coinY + 15, coinsText,
             (int)wcslen(coinsText));
    SelectObject(hdcBuffer, oldCoinFont);
  }

  // Draw level grid when Map tab is selected
  if (game.selectedTab == 0) {
    if (hMapTitleBmp) {
      HDC hdcMapTitle = CreateCompatibleDC(hdcBuffer);
      HBITMAP oldMapTitleBmp = (HBITMAP)SelectObject(hdcMapTitle, hMapTitleBmp);

      BITMAP mapTitleInfo = {};
      GetObject(hMapTitleBmp, sizeof(BITMAP), &mapTitleInfo);
      int mapTitleX = (clientRect.right - mapTitleInfo.bmWidth) / 2;
      int mapTitleY = 150;

      TransparentBlt(hdcBuffer, mapTitleX, mapTitleY, mapTitleInfo.bmWidth,
                     mapTitleInfo.bmHeight, hdcMapTitle, 0, 0,
                     mapTitleInfo.bmWidth, mapTitleInfo.bmHeight,
                     TRANSPARENT_COLOR);

      SelectObject(hdcMapTitle, oldMapTitleBmp);
      DeleteDC(hdcMapTitle);
    }

    int numLevels = (int)game.availableLevels.size();
    int numCols = (numLevels + LEVEL_ROWS - 1) / LEVEL_ROWS;
    int totalGridWidth =
        numCols * LEVEL_BUTTON_WIDTH +
        (numCols > 1 ? (numCols - 1) * LEVEL_BUTTON_SPACING : 0);
    int gridStartX = (clientRect.right - totalGridWidth) / 2;

    // Draw yellow stripes connecting consecutive level buttons
    {
      HPEN yellowPen = CreatePen(PS_SOLID, 6, RGB(255, 247, 0));
      HPEN oldPen = (HPEN)SelectObject(hdcBuffer, yellowPen);
      for (int i = 0; i + 1 < numLevels; i++) {
        int x1, y1, x2, y2;
        GetLevelButtonPos(i, gridStartX, x1, y1);
        GetLevelButtonPos(i + 1, gridStartX, x2, y2);
        int cx1 = x1 + LEVEL_BUTTON_WIDTH / 2;
        int cy1 = y1 + LEVEL_BUTTON_HEIGHT / 2;
        int cx2 = x2 + LEVEL_BUTTON_WIDTH / 2;
        int cy2 = y2 + LEVEL_BUTTON_HEIGHT / 2;
        MoveToEx(hdcBuffer, cx1, cy1, NULL);
        LineTo(hdcBuffer, cx2, cy2);
      }
      SelectObject(hdcBuffer, oldPen);
      DeleteObject(yellowPen);
    }

    // Draw level buttons
    for (int i = 0; i < numLevels; i++) {
      int levelNum = game.availableLevels[i];
      int buttonX, buttonY;
      GetLevelButtonPos(i, gridStartX, buttonX, buttonY);

      bool isUnlocked =
          (levelNum == 1) || game.db.IsLevelCompleted(levelNum - 1);

      if (isUnlocked) {
        WCHAR levelText[8];
        wsprintfW(levelText, L"%d", levelNum);
        bool levelPressed = (pressedLevelButton == levelNum);
        DrawButton(hdcBuffer, hdcSprite, buttonX, buttonY, LEVEL_BUTTON_WIDTH,
                   LEVEL_BUTTON_HEIGHT, levelText, -1, -1, levelPressed);

        // Draw black dot on top-right if bonus not yet claimed
        if (!game.db.IsLevelPerfected(levelNum)) {
          int dotSize = 12;
          int dotMargin = 6;
          int dotX = buttonX + LEVEL_BUTTON_WIDTH - dotSize - dotMargin;
          int dotY = buttonY + dotMargin;
          HBRUSH dotBrush = CreateSolidBrush(RGB(0, 0, 0));
          HBRUSH oldBrush = (HBRUSH)SelectObject(hdcBuffer, dotBrush);
          HPEN nullPen =
              (HPEN)SelectObject(hdcBuffer, GetStockObject(NULL_PEN));
          Ellipse(hdcBuffer, dotX, dotY, dotX + dotSize, dotY + dotSize);
          SelectObject(hdcBuffer, nullPen);
          SelectObject(hdcBuffer, oldBrush);
          DeleteObject(dotBrush);
        }
      } else {
        DrawButton(hdcBuffer, hdcSprite, buttonX, buttonY, LEVEL_BUTTON_WIDTH,
                   LEVEL_BUTTON_HEIGHT, L"", 2, 4, false, RGB(212, 0, 255));
      }
    }

    // Draw stars below level buttons
    if (hUISheet) {
      int starSize = 24;
      int starSpacing = 6;
      int totalStarWidth = 3 * starSize + 2 * starSpacing;

      for (int i = 0; i < numLevels; i++) {
        int levelNum = game.availableLevels[i];
        bool isUnlocked =
            (levelNum == 1) || game.db.IsLevelCompleted(levelNum - 1);
        if (!isUnlocked)
          continue;

        int buttonX, buttonY;
        GetLevelButtonPos(i, gridStartX, buttonX, buttonY);

        int bestStars = game.db.GetLevelBestStars(levelNum);
        int starStartX = buttonX + (LEVEL_BUTTON_WIDTH - totalStarWidth) / 2;
        int starY = buttonY + LEVEL_BUTTON_HEIGHT - 8;

        HBITMAP oldBmp = (HBITMAP)SelectObject(hdcSprite, hUISheet);
        for (int s = 0; s < 3; s++) {
          int sx = starStartX + s * (starSize + starSpacing);
          int srcCol = (s < bestStars) ? 0 : 2;
          TransparentBlt(hdcBuffer, sx, starY, starSize, starSize, hdcSprite,
                         srcCol * TILE_SIZE, 3 * TILE_SIZE, TILE_SIZE,
                         TILE_SIZE, TRANSPARENT_COLOR);
        }
        SelectObject(hdcSprite, oldBmp);
      }
    }
  }

  // Draw shop if selected
  if (game.selectedTab == 1) {
    RenderShop(hdcBuffer, hdcSprite, clientRect, game);
  }

  // Draw bottom navigation bar
  int navBarY = clientRect.bottom - NAV_BAR_HEIGHT;
  int buttonWidth = clientRect.right / 2 - 9;

  bool mapSelected = (game.selectedTab == 0);
  DrawButton(hdcBuffer, hdcSprite, 6, navBarY, buttonWidth, NAV_BAR_HEIGHT,
             L"Map", 0, 4, mapSelected);

  bool shopSelected = (game.selectedTab == 1);
  DrawButton(hdcBuffer, hdcSprite, buttonWidth + 12, navBarY, buttonWidth,
             NAV_BAR_HEIGHT, L"Shop", 1, 4, shopSelected);

  DeleteDC(hdcSprite);
}

// ========== Shop Layout ==========

#define SHOP_HEIGHT 400
#define SHOP_CARD_WIDTH 300
#define SHOP_BUY_BTN_H 60

// Draw a vertical separator
static void DrawShopSeparator(HDC hdc, int x, int sepTop, int sepBottom) {
  HBRUSH black = CreateSolidBrush(RGB(0, 0, 0));
  int dotSize = 5;
  int hd = dotSize / 2;

  // 4 dot
  int dotOffsets[] = {hd, dotSize + 6 + hd, dotSize * 2 + 6 + 2 + hd,
                      dotSize * 3 + 6 + 2 + 2 + hd};
  for (int i = 0; i < 4; i++) {
    int off = dotOffsets[i];
    int ty = sepTop + off;
    int by = sepBottom - off;
    RECT dt = {x - hd, ty - hd, x + hd + 1, ty + hd + 1};
    RECT db = {x - hd, by - hd, x + hd + 1, by + hd + 1};
    FillRect(hdc, &dt, black);
    FillRect(hdc, &db, black);
  }

  // Solid line
  int lineTop = sepTop + dotOffsets[3] + hd + 1;
  int lineBot = sepBottom - dotOffsets[3] - hd;
  RECT solidLine = {x - 1, lineTop, x + 2, lineBot};
  FillRect(hdc, &solidLine, black);

  DeleteObject(black);
}

static void GetShopCardPositions(const RECT &clientRect, int &leftCardX,
                                 int &rightCardX) {
  int cardW = SHOP_CARD_WIDTH;
  int gap = 40;
  int totalW = cardW * 2 + gap;
  leftCardX = (clientRect.right - totalW) / 2;
  rightCardX = leftCardX + cardW + gap;
}

RECT Renderer::GetShopBuyButtonRect(const RECT &clientRect) {
  int navBarY = clientRect.bottom - NAV_BAR_HEIGHT;
  int panelTop = navBarY - 5 - SHOP_HEIGHT;

  int cardW = SHOP_CARD_WIDTH;
  int leftCardX, rightCardX;
  GetShopCardPositions(clientRect, leftCardX, rightCardX);
  int btnW = (int)(cardW * 0.9);
  int btnX = leftCardX + (cardW - btnW) / 2;

  // Vertical offsets
  int cy = panelTop + 20;
  cy += 84; // title + gap
  cy += 84; // icon + gap
  cy += 52; // count + gap
  cy += 90; // description + gap

  return {btnX, cy, btnX + btnW, cy + SHOP_BUY_BTN_H};
}

RECT Renderer::GetShopBuyMagnetButtonRect(const RECT &clientRect) {
  int navBarY = clientRect.bottom - NAV_BAR_HEIGHT;
  int panelTop = navBarY - 5 - SHOP_HEIGHT;

  int cardW = SHOP_CARD_WIDTH;
  int leftCardX, rightCardX;
  GetShopCardPositions(clientRect, leftCardX, rightCardX);
  int btnW = (int)(cardW * 0.9);
  int btnX = rightCardX + (cardW - btnW) / 2;

  // Vertical offsets
  int cy = panelTop + 20;
  cy += 84; // title + gap
  cy += 84; // icon + gap
  cy += 52; // count + gap
  cy += 90; // description + gap

  return {btnX, cy, btnX + btnW, cy + SHOP_BUY_BTN_H};
}

void Renderer::RenderShop(HDC hdcBuffer, HDC hdcSprite, const RECT &clientRect,
                          Game &game) {
  // Position panel
  int navBarY = clientRect.bottom - NAV_BAR_HEIGHT;
  int panelBottom = navBarY - 5;
  int panelTop = panelBottom - SHOP_HEIGHT;

  // Yellow background
  HBRUSH yellowBrush = CreateSolidBrush(BUTTON_COLOR);
  RECT panelRect = {0, panelTop, clientRect.right, panelBottom};
  FillRect(hdcBuffer, &panelRect, yellowBrush);
  DeleteObject(yellowBrush);

  // Two cards
  int cardW = SHOP_CARD_WIDTH;
  int leftCardX, rightCardX;
  GetShopCardPositions(clientRect, leftCardX, rightCardX);

  // Separators
  int sepTop = panelTop + 20;
  int sepBottom = panelTop + 20 + 84 + 84 + 52 + 90 + SHOP_BUY_BTN_H + 5;

  // Description font
  HFONT hDescFont =
      CreateFontW(20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                  OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
                  DEFAULT_PITCH | FF_DONTCARE, L"Lower Pixel");

  // ====== SHIELD CARD ======
  DrawShopSeparator(hdcBuffer, leftCardX, sepTop, sepBottom);
  DrawShopSeparator(hdcBuffer, leftCardX + cardW, sepTop, sepBottom);

  {
    int cy = panelTop + 20;

    SetBkMode(hdcBuffer, TRANSPARENT);
    SetTextColor(hdcBuffer, RGB(0, 0, 0));
    HFONT oldFont = (HFONT)SelectObject(hdcBuffer, hArcadeFont);
    RECT titleRect = {leftCardX, cy, leftCardX + cardW, cy + 64};
    DrawTextW(hdcBuffer, L"SHIELD", -1, &titleRect,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    cy += 84;

    int iconSize = 64;
    int iconX = leftCardX + (cardW - iconSize) / 2;
    if (hUISheet) {
      HBITMAP oldBmp = (HBITMAP)SelectObject(hdcSprite, hUISheet);
      TransparentBlt(hdcBuffer, iconX, cy, iconSize, iconSize, hdcSprite,
                     1 * TILE_SIZE, 6 * TILE_SIZE, TILE_SIZE, TILE_SIZE,
                     TRANSPARENT_COLOR);
      SelectObject(hdcSprite, oldBmp);
    }
    cy += iconSize + 20;

    int shieldCount = game.db.GetShieldCount();
    SelectObject(hdcBuffer, hArcadeFontSmall);
    SetTextColor(hdcBuffer, RGB(0, 0, 0));
    WCHAR countText[16];
    wsprintfW(countText, L"x%d", shieldCount);
    RECT countRect = {leftCardX, cy, leftCardX + cardW, cy + 32};
    DrawTextW(hdcBuffer, countText, -1, &countRect,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    cy += 52;

    SelectObject(hdcBuffer, hDescFont);
    SetTextColor(hdcBuffer, RGB(0, 0, 0));
    RECT descRect = {leftCardX + 10, cy, leftCardX + cardW - 10, cy + 70};
    DrawTextW(hdcBuffer,
              L"PROTECT YOURSELF FROM\r\nCRASHING FOR SHORT TIME\r\n"
              L"ACTIVATE WITH SPACEBAR",
              -1, &descRect, DT_CENTER | DT_WORDBREAK);
    cy += 90;

    int btnW = (int)(cardW * 0.9);
    int btnX = leftCardX + (cardW - btnW) / 2;
    DrawButton(hdcBuffer, hdcSprite, btnX, cy, btnW, SHOP_BUY_BTN_H, L"", -1,
               -1, pressedShopBuy);
    int textOffset = pressedShopBuy ? BUTTON_PRESS_OFFSET : 0;
    SelectObject(hdcBuffer, hArcadeFontSmall);
    SetTextColor(hdcBuffer, BUTTON_TEXT_COLOR);
    RECT btnTextRect = {btnX, cy + textOffset, btnX + btnW,
                        cy + SHOP_BUY_BTN_H + textOffset};
    DrawTextW(hdcBuffer, L"BUY 200", -1, &btnTextRect,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    SelectObject(hdcBuffer, oldFont);
  }

  // ====== MAGNET CARD ======
  DrawShopSeparator(hdcBuffer, rightCardX, sepTop, sepBottom);
  DrawShopSeparator(hdcBuffer, rightCardX + cardW, sepTop, sepBottom);

  {
    int cy = panelTop + 20;

    SetBkMode(hdcBuffer, TRANSPARENT);
    SetTextColor(hdcBuffer, RGB(0, 0, 0));
    HFONT oldFont = (HFONT)SelectObject(hdcBuffer, hArcadeFont);
    RECT titleRect = {rightCardX, cy, rightCardX + cardW, cy + 64};
    DrawTextW(hdcBuffer, L"MAGNET", -1, &titleRect,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    cy += 84;

    int iconSize = 64;
    int iconX = rightCardX + (cardW - iconSize) / 2;
    if (hUISheet) {
      HBITMAP oldBmp = (HBITMAP)SelectObject(hdcSprite, hUISheet);
      TransparentBlt(hdcBuffer, iconX, cy, iconSize, iconSize, hdcSprite,
                     2 * TILE_SIZE, 6 * TILE_SIZE, TILE_SIZE, TILE_SIZE,
                     TRANSPARENT_COLOR);
      SelectObject(hdcSprite, oldBmp);
    }
    cy += iconSize + 20;

    int magnetCount = game.db.GetMagnetCount();
    SelectObject(hdcBuffer, hArcadeFontSmall);
    SetTextColor(hdcBuffer, RGB(0, 0, 0));
    WCHAR countText[16];
    wsprintfW(countText, L"x%d", magnetCount);
    RECT countRect = {rightCardX, cy, rightCardX + cardW, cy + 32};
    DrawTextW(hdcBuffer, countText, -1, &countRect,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    cy += 52;

    SelectObject(hdcBuffer, hDescFont);
    SetTextColor(hdcBuffer, RGB(0, 0, 0));
    RECT descRect = {rightCardX + 10, cy, rightCardX + cardW - 10, cy + 70};
    DrawTextW(hdcBuffer,
              L"PICKS UP COLLECTIBLES\r\nIN 3X3 AREA AROUND YOU\r\n"
              L"ACTIVATE WITH M",
              -1, &descRect, DT_CENTER | DT_WORDBREAK);
    cy += 90;

    int btnW = (int)(cardW * 0.9);
    int btnX = rightCardX + (cardW - btnW) / 2;
    DrawButton(hdcBuffer, hdcSprite, btnX, cy, btnW, SHOP_BUY_BTN_H, L"", -1,
               -1, pressedShopBuyMagnet);
    int textOffset = pressedShopBuyMagnet ? BUTTON_PRESS_OFFSET : 0;
    SelectObject(hdcBuffer, hArcadeFontSmall);
    SetTextColor(hdcBuffer, BUTTON_TEXT_COLOR);
    RECT btnTextRect = {btnX, cy + textOffset, btnX + btnW,
                        cy + SHOP_BUY_BTN_H + textOffset};
    DrawTextW(hdcBuffer, L"BUY 50", -1, &btnTextRect,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    SelectObject(hdcBuffer, oldFont);
  }

  DeleteObject(hDescFont);
}

void Renderer::RenderPlaying(HDC hdc, HDC hdcBuffer, const RECT &clientRect,
                             Game &game) {
  // background
  HBRUSH bgBrush = CreateSolidBrush(RGB(0, 0, 0));
  FillRect(hdcBuffer, &clientRect, bgBrush);
  DeleteObject(bgBrush);

  HDC hdcSprite = CreateCompatibleDC(hdc);

  // Update camera to center player
  int visibleW = clientRect.right - HUD_STRIPE_LEFT_W - HUD_STRIPE_RIGHT_W;
  int visibleH = clientRect.bottom - HUD_STRIPE_TOP_H;
  int centerScreenX = HUD_STRIPE_LEFT_W + visibleW / 2;
  int centerScreenY = HUD_STRIPE_TOP_H + visibleH / 2;
  game.camera.x = game.player.pixelX - centerScreenX + TILE_SIZE / 2;
  game.camera.y = game.player.pixelY - centerScreenY + TILE_SIZE / 2;

  // Draw map tiles
  int startTileX = game.camera.x / TILE_SIZE;
  int startTileY = game.camera.y / TILE_SIZE;
  int endTileX = startTileX + VIEWPORT_WIDTH + 2;
  int endTileY = startTileY + VIEWPORT_HEIGHT + 2;

  for (int y = startTileY; y < endTileY; y++) {
    for (int x = startTileX; x < endTileX; x++) {
      if (x < 0 || x >= game.mapWidth || y < 0 || y >= game.mapHeight)
        continue;
      MapObject *obj = game.map[y][x];
      if (obj == nullptr)
        continue;

      int destX = x * TILE_SIZE - game.camera.x;
      int destY = y * TILE_SIZE - game.camera.y;

      // Turret block
      if (obj->type == MapObjectType::Turret) {
        TurretBlock *turret = static_cast<TurretBlock *>(obj);
        DrawSprite(hdcBuffer, hdcSprite, hTurretSheet, turret->srcX,
                   turret->srcY, destX, destY);
        continue;
      }

      // Puffer block (puffed 3x3 drawn later )
      if (obj->type == MapObjectType::Puffer) {
        PufferBlock *puffer = static_cast<PufferBlock *>(obj);
        if (!puffer->IsPuffed()) {
          DrawSprite(hdcBuffer, hdcSprite, hFishSheet, puffer->srcX,
                     puffer->srcY, destX, destY);
        }
        continue;
      }

      // UI sprite sheet objects
      if (obj->type == MapObjectType::Collectible ||
          obj->type == MapObjectType::Coin ||
          obj->type == MapObjectType::Star ||
          obj->type == MapObjectType::Goal) {
        if (!obj->sprites.empty())
          DrawUISprite(hdcBuffer, hdcSprite, obj->sprites[0].srcX,
                       obj->sprites[0].srcY, destX, destY);
      } else {
        // Wall/block tiles
        for (const auto &sprite : obj->sprites) {
          if (sprite.srcY != 2) {
            if (sprite.isDangerous)
              DrawKillTile(hdcBuffer, hdcSprite, sprite.srcX, sprite.srcY,
                           destX, destY);
            else
              DrawTile(hdcBuffer, hdcSprite, sprite.srcX, sprite.srcY, destX,
                       destY);
          }
        }
        for (const auto &sprite : obj->sprites) {
          if (sprite.srcY == 2) {
            if (sprite.isDangerous)
              DrawKillTile(hdcBuffer, hdcSprite, sprite.srcX, sprite.srcY,
                           destX, destY);
            else
              DrawTile(hdcBuffer, hdcSprite, sprite.srcX, sprite.srcY, destX,
                       destY);
          }
        }
      }
    }
  }

  // Draw puffed pufferfish
  for (int y = startTileY; y < endTileY; y++) {
    for (int x = startTileX; x < endTileX; x++) {
      if (x < 0 || x >= game.mapWidth || y < 0 || y >= game.mapHeight)
        continue;
      MapObject *obj = game.map[y][x];
      if (obj == nullptr || obj->type != MapObjectType::Puffer)
        continue;
      PufferBlock *puffer = static_cast<PufferBlock *>(obj);
      if (!puffer->IsPuffed())
        continue;
      static const int offsets[9][2] = {{-1, -1}, {0, -1}, {1, -1},
                                        {-1, 0},  {0, 0},  {1, 0},
                                        {-1, 1},  {0, 1},  {1, 1}};
      for (int i = 0; i < 9; i++) {
        int dX = (x + offsets[i][0]) * TILE_SIZE - game.camera.x;
        int dY = (y + offsets[i][1]) * TILE_SIZE - game.camera.y;
        DrawSprite(hdcBuffer, hdcSprite, hFishSheet, puffer->srcX, i, dX, dY);
      }
    }
  }

  // Draw spike animations
  for (int y = startTileY; y < endTileY; y++) {
    for (int x = startTileX; x < endTileX; x++) {
      if (x < 0 || x >= game.mapWidth || y < 0 || y >= game.mapHeight)
        continue;
      MapObject *obj = game.map[y][x];
      if (obj == nullptr)
        continue;
      if (obj->type == MapObjectType::SpikeWall) {
        SpikeWall *spike = static_cast<SpikeWall *>(obj);
        for (const auto &trap : spike->traps) {
          if (trap.HasActiveSpikes()) {
            int spikeX, spikeY;
            trap.GetSpikeRenderPos(spike->gridX, spike->gridY, spikeX, spikeY);
            int dX = spikeX * TILE_SIZE - game.camera.x;
            int dY = spikeY * TILE_SIZE - game.camera.y;
            DrawKillTile(hdcBuffer, hdcSprite, trap.spikeAnimFrame,
                         trap.spikeRow, dX, dY);
          }
        }
      }
    }
  }

  // Draw door at spawn point
  if (hDoorSheet) {
    int doorW = 96;
    int doorH = 64;

    int doorDestX =
        game.spawnX * TILE_SIZE - (doorW - TILE_SIZE) / 2 - game.camera.x;
    int doorDestY =
        game.spawnY * TILE_SIZE - (doorH - TILE_SIZE) - game.camera.y;
    HBITMAP oldBmp = (HBITMAP)SelectObject(hdcSprite, hDoorSheet);
    TransparentBlt(hdcBuffer, doorDestX, doorDestY, doorW, doorH, hdcSprite, 0,
                   game.doorAnimFrame * doorH, doorW, doorH, TRANSPARENT_COLOR);
    SelectObject(hdcSprite, oldBmp);
  }

  DrawTrailParticles(hdcBuffer, game, game.camera.x, game.camera.y);

  // Draw enemies
  for (Enemy *enemy : game.enemies) {
    if (!enemy)
      continue;
    if (enemy->enemyType == ENEMY_ARROW) {
      DrawSprite(hdcBuffer, hdcSprite, hTurretSheet, enemy->srcX, enemy->srcY,
                 enemy->pixelX - game.camera.x, enemy->pixelY - game.camera.y);
    } else {
      DrawSprite(hdcBuffer, hdcSprite, hEnemySheet, enemy->srcX, enemy->srcY,
                 enemy->pixelX - game.camera.x, enemy->pixelY - game.camera.y);
    }
  }

  // Draw shield effect
  if (game.shieldActive) {
    bool blinking = game.shieldTimer <= 60 && (game.shieldTimer / 10) % 2 == 0;
    if (!blinking) {
      int px = game.player.pixelX - game.camera.x;
      int py = game.player.pixelY - game.camera.y;
      int pad = 4;
      // zig-zag outline
      int pulse = 150 + (game.shieldTimer % 30) * 3;
      if (pulse > 255)
        pulse = 255;
      HPEN shieldPen = CreatePen(PS_SOLID, 2, RGB(0, pulse, pulse));
      HPEN oldPen = (HPEN)SelectObject(hdcBuffer, shieldPen);

      int l = px - pad;
      int t = py - pad;
      int r = px + TILE_SIZE + pad;
      int b = py + TILE_SIZE + pad;
      int step = 5;

      // Left side zig-zag
      MoveToEx(hdcBuffer, l, t, NULL);
      for (int y = t; y < b; y += step * 2) {
        LineTo(hdcBuffer, l, y + step);
        LineTo(hdcBuffer, l + step, y + step);
        LineTo(hdcBuffer, l + step, y + step * 2);
        LineTo(hdcBuffer, l, y + step * 2);
      }

      // Right side zig-zag
      MoveToEx(hdcBuffer, r, t, NULL);
      for (int y = t; y < b; y += step * 2) {
        LineTo(hdcBuffer, r, y + step);
        LineTo(hdcBuffer, r - step, y + step);
        LineTo(hdcBuffer, r - step, y + step * 2);
        LineTo(hdcBuffer, r, y + step * 2);
      }

      // Top side zig-zag
      MoveToEx(hdcBuffer, l, t, NULL);
      for (int x = l; x < r; x += step * 2) {
        LineTo(hdcBuffer, x + step, t);
        LineTo(hdcBuffer, x + step, t + step);
        LineTo(hdcBuffer, x + step * 2, t + step);
        LineTo(hdcBuffer, x + step * 2, t);
      }

      // Bottom side zig-zag
      MoveToEx(hdcBuffer, l, b, NULL);
      for (int x = l; x < r; x += step * 2) {
        LineTo(hdcBuffer, x + step, b);
        LineTo(hdcBuffer, x + step, b - step);
        LineTo(hdcBuffer, x + step * 2, b - step);
        LineTo(hdcBuffer, x + step * 2, b);
      }

      SelectObject(hdcBuffer, oldPen);
      DeleteObject(shieldPen);
    }
  }

  // Draw player
  if (game.player.isDying && hCharDeathSheet) {
    // Death animation
    int frameW = 224;
    int frameH = 96;
    int srcX = 0;
    int srcY = game.player.deathAnimFrame * frameH;
    int destX = game.player.pixelX - game.camera.x - (frameW - TILE_SIZE) / 2;
    int destY = game.player.pixelY - game.camera.y - (frameH - TILE_SIZE) / 2;
    HBITMAP oldBmp = (HBITMAP)SelectObject(hdcSprite, hCharDeathSheet);
    TransparentBlt(hdcBuffer, destX, destY, frameW, frameH, hdcSprite, srcX,
                   srcY, frameW, frameH, TRANSPARENT_COLOR);
    SelectObject(hdcSprite, oldBmp);
  } else {
    DrawSprite(hdcBuffer, hdcSprite, hCharSheet, game.player.GetSpriteColumn(),
               game.player.GetSpriteRow(), game.player.pixelX - game.camera.x,
               game.player.pixelY - game.camera.y);
  }

  // === Draw black stripes ===
  {
    HBRUSH stripeBrush = CreateSolidBrush(RGB(0, 0, 0));
    RECT topStripe = {0, 0, clientRect.right, HUD_STRIPE_TOP_H};
    FillRect(hdcBuffer, &topStripe, stripeBrush);
    RECT leftStripe = {0, 0, HUD_STRIPE_LEFT_W, clientRect.bottom};
    FillRect(hdcBuffer, &leftStripe, stripeBrush);
    RECT rightStripe = {clientRect.right - HUD_STRIPE_RIGHT_W, 0,
                        clientRect.right, clientRect.bottom};
    FillRect(hdcBuffer, &rightStripe, stripeBrush);
    DeleteObject(stripeBrush);

    // fade edge
    int fadeY = HUD_STRIPE_TOP_H;
    for (int x = HUD_STRIPE_LEFT_W; x < clientRect.right - HUD_STRIPE_RIGHT_W;
         x += 2) {
      SetPixel(hdcBuffer, x, fadeY, RGB(0, 0, 0));
      SetPixel(hdcBuffer, x, fadeY + 2, RGB(0, 0, 0));
      SetPixel(hdcBuffer, x, fadeY + 4, RGB(0, 0, 0));
      SetPixel(hdcBuffer, x, fadeY + 6, RGB(0, 0, 0));
      SetPixel(hdcBuffer, x, fadeY + 8, RGB(0, 0, 0));
      SetPixel(hdcBuffer, x, fadeY + 10, RGB(0, 0, 0));
    }

    for (int x = HUD_STRIPE_LEFT_W + 1;
         x < clientRect.right - HUD_STRIPE_RIGHT_W; x += 2) {
      SetPixel(hdcBuffer, x, fadeY + 1, RGB(0, 0, 0));
      SetPixel(hdcBuffer, x, fadeY + 3, RGB(0, 0, 0));
      SetPixel(hdcBuffer, x, fadeY + 5, RGB(0, 0, 0));
      SetPixel(hdcBuffer, x, fadeY + 7, RGB(0, 0, 0));
      SetPixel(hdcBuffer, x, fadeY + 9, RGB(0, 0, 0));
      SetPixel(hdcBuffer, x, fadeY + 11, RGB(0, 0, 0));
    }
  }

  // === Draw HUD ===
  if (game.state == STATE_PLAYING || game.state == STATE_PAUSED ||
      game.state == STATE_DYING) {
    // Money display
    if (hUISheet) {
      int coinIconSize = 64;
      int coinX = 20;
      int coinY = 18;
      HBITMAP oldBmp = (HBITMAP)SelectObject(hdcSprite, hUISheet);
      TransparentBlt(hdcBuffer, coinX, coinY, coinIconSize, coinIconSize,
                     hdcSprite, 0 * TILE_SIZE, 6 * TILE_SIZE, TILE_SIZE,
                     TILE_SIZE, TRANSPARENT_COLOR);
      SelectObject(hdcSprite, oldBmp);

      int totalCoins = game.db.GetTotalCoins() + game.score;
      WCHAR coinsText[32];
      wsprintfW(coinsText, L"%d", totalCoins);
      SetBkMode(hdcBuffer, TRANSPARENT);
      SetTextColor(hdcBuffer, RGB(255, 247, 0));
      HFONT oldCoinFont = (HFONT)SelectObject(hdcBuffer, hArcadeFontSmall);
      TextOutW(hdcBuffer, coinX + coinIconSize, coinY + 15, coinsText,
               (int)wcslen(coinsText));
      SelectObject(hdcBuffer, oldCoinFont);
    }

    // Stars display
    if (hUISheet) {
      int starSize = 48;
      int starSpacing = 10;
      int totalStarWidth = 3 * starSize + 2 * starSpacing;
      int starStartX = (clientRect.right - totalStarWidth) / 2;
      int starY = (HUD_STRIPE_TOP_H - starSize) / 2;

      HBITMAP oldBmp = (HBITMAP)SelectObject(hdcSprite, hUISheet);
      for (int s = 0; s < 3; s++) {
        int sx = starStartX + s * (starSize + starSpacing);
        int srcCol = (s < game.starsCollected) ? 0 : 2;
        TransparentBlt(hdcBuffer, sx, starY, starSize, starSize, hdcSprite,
                       srcCol * TILE_SIZE, 3 * TILE_SIZE, TILE_SIZE, TILE_SIZE,
                       TRANSPARENT_COLOR);
      }
      SelectObject(hdcSprite, oldBmp);
    }

    // Pause button
    {
      RECT pauseBtn = GetPlayingPauseButtonRect(clientRect);
      int btnW = pauseBtn.right - pauseBtn.left;
      int btnH = pauseBtn.bottom - pauseBtn.top;
      DrawButton(hdcBuffer, hdcSprite, pauseBtn.left, pauseBtn.top, btnW, btnH,
                 L"", 3, 5, pressedPlayingPause);
    }

    // Shield icon and count
    if (hUISheet) {
      int shieldIconSize = 64;
      int shieldX = (HUD_STRIPE_LEFT_W - shieldIconSize) / 2;
      int shieldY = clientRect.bottom - shieldIconSize - 18;

      HBITMAP oldBmp = (HBITMAP)SelectObject(hdcSprite, hUISheet);
      TransparentBlt(hdcBuffer, shieldX, shieldY, shieldIconSize,
                     shieldIconSize, hdcSprite, 1 * TILE_SIZE, 6 * TILE_SIZE,
                     TILE_SIZE, TILE_SIZE, TRANSPARENT_COLOR);
      SelectObject(hdcSprite, oldBmp);

      WCHAR shieldText[16];
      wsprintfW(shieldText, L"x%d", game.shieldCount);
      SetBkMode(hdcBuffer, TRANSPARENT);
      SetTextColor(hdcBuffer, RGB(255, 247, 0));
      HFONT oldFont = (HFONT)SelectObject(hdcBuffer, hArcadeFontSmall);
      TextOutW(hdcBuffer, shieldX + shieldIconSize + 5, shieldY + 15,
               shieldText, (int)wcslen(shieldText));
      SelectObject(hdcBuffer, oldFont);
    }

    // Magnet icon and count
    if (hUISheet) {
      int magnetIconSize = 64;
      int magnetX = clientRect.right - HUD_STRIPE_RIGHT_W +
                    (HUD_STRIPE_RIGHT_W - magnetIconSize) / 2;
      int magnetY = clientRect.bottom - magnetIconSize - 18;

      HBITMAP oldBmp = (HBITMAP)SelectObject(hdcSprite, hUISheet);
      TransparentBlt(hdcBuffer, magnetX, magnetY, magnetIconSize,
                     magnetIconSize, hdcSprite, 2 * TILE_SIZE, 6 * TILE_SIZE,
                     TILE_SIZE, TILE_SIZE, TRANSPARENT_COLOR);
      SelectObject(hdcSprite, oldBmp);

      WCHAR magnetText[16];
      wsprintfW(magnetText, L"x%d", game.magnetCount);
      SetBkMode(hdcBuffer, TRANSPARENT);
      SetTextColor(hdcBuffer, RGB(255, 247, 0));
      HFONT oldFont = (HFONT)SelectObject(hdcBuffer, hArcadeFontSmall);
      TextOutW(hdcBuffer, magnetX + magnetIconSize + 5, magnetY + 15,
               magnetText, (int)wcslen(magnetText));
      SelectObject(hdcBuffer, oldFont);
    }

    // Shield countdown timer
    if (game.shieldActive) {
      int secs = (game.shieldTimer + 59) / 60;
      WCHAR timerText[8];
      wsprintfW(timerText, L"%d", secs);
      SetBkMode(hdcBuffer, TRANSPARENT);
      SetTextColor(hdcBuffer, RGB(255, 247, 0));
      HFONT oldFont = (HFONT)SelectObject(hdcBuffer, hArcadeFont);
      RECT timerRect = {0, HUD_STRIPE_TOP_H, HUD_STRIPE_LEFT_W,
                        clientRect.bottom - 100};
      DrawTextW(hdcBuffer, timerText, -1, &timerRect,
                DT_CENTER | DT_VCENTER | DT_SINGLELINE);
      SelectObject(hdcBuffer, oldFont);
    }

    // Magnet countdown timer
    if (game.magnetActive) {
      int secs = (game.magnetTimer + 59) / 60;
      WCHAR timerText[8];
      wsprintfW(timerText, L"%d", secs);
      SetBkMode(hdcBuffer, TRANSPARENT);
      SetTextColor(hdcBuffer, RGB(255, 247, 0));
      HFONT oldFont = (HFONT)SelectObject(hdcBuffer, hArcadeFont);
      RECT timerRect = {clientRect.right - HUD_STRIPE_RIGHT_W, HUD_STRIPE_TOP_H,
                        clientRect.right, clientRect.bottom - 100};
      DrawTextW(hdcBuffer, timerText, -1, &timerRect,
                DT_CENTER | DT_VCENTER | DT_SINGLELINE);
      SelectObject(hdcBuffer, oldFont);
    }
  }

  // overlays
  if (game.state == STATE_PAUSED) {
    PauseMenuLayout layout = GetPauseMenuLayout(clientRect);
    int dlgW = layout.dialog.right - layout.dialog.left;

    HBRUSH yellowBrush = CreateSolidBrush(BUTTON_COLOR);
    FillRect(hdcBuffer, &layout.dialog, yellowBrush);
    DeleteObject(yellowBrush);

    // "LEVEL N"
    SetBkMode(hdcBuffer, TRANSPARENT);
    SetTextColor(hdcBuffer, RGB(0, 0, 0));
    HFONT oldFont = (HFONT)SelectObject(hdcBuffer, hArcadeFontSmall);
    WCHAR levelText[32];
    wsprintfW(levelText, L"LEVEL %d", game.currentLevelNumber);
    RECT titleRect = {layout.dialog.left, layout.dialog.top + 10,
                      layout.dialog.right, layout.dialog.top + 60};
    DrawTextW(hdcBuffer, levelText, -1, &titleRect,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(hdcBuffer, oldFont);

    // X close button
    if (hUISheet) {
      HBITMAP oldBmp = (HBITMAP)SelectObject(hdcSprite, hUISheet);
      int xbW = layout.xCloseButton.right - layout.xCloseButton.left;
      TransparentBlt(hdcBuffer, layout.xCloseButton.left,
                     layout.xCloseButton.top, xbW, xbW, hdcSprite, 2 * 32,
                     5 * 32, 32, 32, TRANSPARENT_COLOR);
      SelectObject(hdcSprite, oldBmp);
    }

    // Separator
    if (hSeparatorH) {
      int sepSrcW = 360;
      int sepSrcH = 4;
      int sepX = layout.dialog.left + (dlgW - sepSrcW) / 2;
      int sepY = layout.dialog.top + 65;
      HBITMAP oldBmp = (HBITMAP)SelectObject(hdcSprite, hSeparatorH);
      TransparentBlt(hdcBuffer, sepX, sepY, sepSrcW, sepSrcH, hdcSprite, 0, 0,
                     sepSrcW, sepSrcH, TRANSPARENT_COLOR);
      SelectObject(hdcSprite, oldBmp);
    }

    // "PAUSE"
    {
      HFONT oldPauseFont = (HFONT)SelectObject(hdcBuffer, hArcadeFont);
      SetTextColor(hdcBuffer, RGB(0, 0, 0));
      RECT pauseTextRect = {layout.dialog.left, layout.dialog.top + 75,
                            layout.dialog.right, layout.dialog.top + 175};
      DrawTextW(hdcBuffer, L"PAUSE", -1, &pauseTextRect,
                DT_CENTER | DT_VCENTER | DT_SINGLELINE);
      SelectObject(hdcBuffer, oldPauseFont);
    }

    // EXIT button and RESUME button
    int bw = layout.exitButton.right - layout.exitButton.left;
    int bh = layout.exitButton.bottom - layout.exitButton.top;
    bool exitPressed = (pressedPauseButton == PAUSE_BUTTON_EXIT);
    DrawButton(hdcBuffer, hdcSprite, layout.exitButton.left,
               layout.exitButton.top, bw, bh, L"", -1, -1, exitPressed);
    int pw = layout.playButton.right - layout.playButton.left;
    int ph = layout.playButton.bottom - layout.playButton.top;
    bool playPressed = (pressedPauseButton == PAUSE_BUTTON_PLAY);
    DrawButton(hdcBuffer, hdcSprite, layout.playButton.left,
               layout.playButton.top, pw, ph, L"", -1, -1, playPressed);

    // Icons
    if (hUISheet) {
      int iconSize = 64;
      HBITMAP oldBmp = (HBITMAP)SelectObject(hdcSprite, hUISheet);
      // Exit icon (srcX=1, srcY=5)
      int eiX = layout.exitButton.left + (bw - iconSize) / 2;
      int eiY = layout.exitButton.top + (bh - iconSize) / 2 +
                (exitPressed ? BUTTON_PRESS_OFFSET : 0);
      TransparentBlt(hdcBuffer, eiX, eiY, iconSize, iconSize, hdcSprite,
                     1 * TILE_SIZE, 5 * TILE_SIZE, TILE_SIZE, TILE_SIZE,
                     TRANSPARENT_COLOR);
      // Resume icon (srcX=0, srcY=5)
      int piX = layout.playButton.left + (pw - iconSize) / 2;
      int piY = layout.playButton.top + (ph - iconSize) / 2 +
                (playPressed ? BUTTON_PRESS_OFFSET : 0);
      TransparentBlt(hdcBuffer, piX, piY, iconSize, iconSize, hdcSprite,
                     0 * TILE_SIZE, 5 * TILE_SIZE, TILE_SIZE, TILE_SIZE,
                     TRANSPARENT_COLOR);
      SelectObject(hdcSprite, oldBmp);
    }
  } else if (game.state == STATE_GAME_OVER) {
    GameOverMenuLayout layout = GetGameOverMenuLayout(clientRect);
    int dlgW = layout.dialog.right - layout.dialog.left;

    HBRUSH yellowBrush = CreateSolidBrush(BUTTON_COLOR);
    FillRect(hdcBuffer, &layout.dialog, yellowBrush);
    DeleteObject(yellowBrush);

    // "LEVEL N"
    SetBkMode(hdcBuffer, TRANSPARENT);
    SetTextColor(hdcBuffer, RGB(0, 0, 0));
    HFONT oldFont = (HFONT)SelectObject(hdcBuffer, hArcadeFontSmall);
    WCHAR levelText[32];
    wsprintfW(levelText, L"LEVEL %d", game.currentLevelNumber);
    RECT titleRect = {layout.dialog.left, layout.dialog.top + 10,
                      layout.dialog.right, layout.dialog.top + 60};
    DrawTextW(hdcBuffer, levelText, -1, &titleRect,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(hdcBuffer, oldFont);

    // Separator
    if (hSeparatorH) {
      int sepSrcW = 360;
      int sepSrcH = 4;
      int sepX = layout.dialog.left + (dlgW - sepSrcW) / 2;
      int sepY = layout.dialog.top + 65;
      HBITMAP oldBmp = (HBITMAP)SelectObject(hdcSprite, hSeparatorH);
      TransparentBlt(hdcBuffer, sepX, sepY, sepSrcW, sepSrcH, hdcSprite, 0, 0,
                     sepSrcW, sepSrcH, TRANSPARENT_COLOR);
      SelectObject(hdcSprite, oldBmp);
    }

    // "DEFEAT"
    {
      HFONT oldDefeatFont = (HFONT)SelectObject(hdcBuffer, hArcadeFont);
      SetTextColor(hdcBuffer, RGB(0, 0, 0));
      RECT defeatTextRect = {layout.dialog.left, layout.dialog.top + 75,
                             layout.dialog.right, layout.dialog.top + 175};
      DrawTextW(hdcBuffer, L"DEFEAT", -1, &defeatTextRect,
                DT_CENTER | DT_VCENTER | DT_SINGLELINE);
      SelectObject(hdcBuffer, oldDefeatFont);
    }

    // HOME button and REPLAY button
    int bw = layout.homeButton.right - layout.homeButton.left;
    int bh = layout.homeButton.bottom - layout.homeButton.top;
    bool homePressed = (pressedGameOverButton == GAMEOVER_BUTTON_HOME);
    DrawButton(hdcBuffer, hdcSprite, layout.homeButton.left,
               layout.homeButton.top, bw, bh, L"", -1, -1, homePressed);
    int rw = layout.replayButton.right - layout.replayButton.left;
    int rh = layout.replayButton.bottom - layout.replayButton.top;
    bool replayPressed = (pressedGameOverButton == GAMEOVER_BUTTON_REPLAY);
    DrawButton(hdcBuffer, hdcSprite, layout.replayButton.left,
               layout.replayButton.top, rw, rh, L"", -1, -1, replayPressed);

    //  icons
    if (hUISheet) {
      int iconSize = 64;
      HBITMAP oldBmp = (HBITMAP)SelectObject(hdcSprite, hUISheet);
      // Home icon (srcX=1, srcY=5)
      int hiX = layout.homeButton.left + (bw - iconSize) / 2;
      int hiY = layout.homeButton.top + (bh - iconSize) / 2 +
                (homePressed ? BUTTON_PRESS_OFFSET : 0);
      TransparentBlt(hdcBuffer, hiX, hiY, iconSize, iconSize, hdcSprite,
                     1 * TILE_SIZE, 5 * TILE_SIZE, TILE_SIZE, TILE_SIZE,
                     TRANSPARENT_COLOR);
      // Replay icon (srcX=4, srcY=5)
      int riX = layout.replayButton.left + (rw - iconSize) / 2;
      int riY = layout.replayButton.top + (rh - iconSize) / 2 +
                (replayPressed ? BUTTON_PRESS_OFFSET : 0);
      TransparentBlt(hdcBuffer, riX, riY, iconSize, iconSize, hdcSprite,
                     4 * TILE_SIZE, 5 * TILE_SIZE, TILE_SIZE, TILE_SIZE,
                     TRANSPARENT_COLOR);
      SelectObject(hdcSprite, oldBmp);
    }
  } else if (game.state == STATE_LEVEL_COMPLETE) {
    int dlgW = 400;
    int dlgH = 420;
    int dlgX = (clientRect.right - dlgW) / 2;
    int dlgY = (clientRect.bottom - dlgH) / 2;
    RECT dlgRect = {dlgX, dlgY, dlgX + dlgW, dlgY + dlgH};

    HBRUSH yellowBrush = CreateSolidBrush(BUTTON_COLOR);
    FillRect(hdcBuffer, &dlgRect, yellowBrush);
    DeleteObject(yellowBrush);

    SetBkMode(hdcBuffer, TRANSPARENT);
    SetTextColor(hdcBuffer, RGB(0, 0, 0));
    int cy = dlgY + 15;

    // "LEVEL N"
    HFONT oldFont = (HFONT)SelectObject(hdcBuffer, hArcadeFontSmall);
    WCHAR levelText[32];
    wsprintfW(levelText, L"LEVEL %d", game.currentLevelNumber);
    RECT levelRect = {dlgX, cy, dlgX + dlgW, cy + 32};
    DrawTextW(hdcBuffer, levelText, -1, &levelRect,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    cy += 35;

    // "CLEAR"
    HFONT oldLargeFont = (HFONT)SelectObject(hdcBuffer, hArcadeFont);
    RECT clearRect = {dlgX, cy, dlgX + dlgW, cy + 64};
    DrawTextW(hdcBuffer, L"CLEAR", -1, &clearRect,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(hdcBuffer, oldLargeFont);
    cy += 70;

    // Separator
    if (hSeparatorH) {
      int sepSrcW = 360;
      int sepSrcH = 4;
      int sepX = dlgX + (dlgW - sepSrcW) / 2;
      HBITMAP oldBmp = (HBITMAP)SelectObject(hdcSprite, hSeparatorH);
      TransparentBlt(hdcBuffer, sepX, cy, sepSrcW, sepSrcH, hdcSprite, 0, 0,
                     sepSrcW, sepSrcH, TRANSPARENT_COLOR);
      SelectObject(hdcSprite, oldBmp);
    }
    cy += 25;

    // Stars display
    if (hUISheet) {
      int starSize = 48;
      int starSpacing = 10;
      int totalStarW = 3 * starSize + 2 * starSpacing;
      int starStartX = dlgX + (dlgW - totalStarW) / 2;
      HBITMAP oldBmp = (HBITMAP)SelectObject(hdcSprite, hUISheet);
      for (int s = 0; s < 3; s++) {
        int sx = starStartX + s * (starSize + starSpacing);
        int srcCol = (s < game.starsCollected) ? 0 : 2;
        TransparentBlt(hdcBuffer, sx, cy, starSize, starSize, hdcSprite,
                       srcCol * TILE_SIZE, 3 * TILE_SIZE, TILE_SIZE, TILE_SIZE,
                       TRANSPARENT_COLOR);
      }
      SelectObject(hdcSprite, oldBmp);
    }
    cy += 70;

    // Separator
    if (hSeparatorH) {
      int sepSrcW = 360;
      int sepSrcH = 4;
      int sepX = dlgX + (dlgW - sepSrcW) / 2;
      HBITMAP oldBmp = (HBITMAP)SelectObject(hdcSprite, hSeparatorH);
      TransparentBlt(hdcBuffer, sepX, cy, sepSrcW, sepSrcH, hdcSprite, 0, 0,
                     sepSrcW, sepSrcH, TRANSPARENT_COLOR);
      SelectObject(hdcSprite, oldBmp);
    }
    cy += 15;

    // "DOTS COLLECTED"
    SelectObject(hdcBuffer, hArcadeFontSmall);
    RECT dotsLabelRect = {dlgX, cy, dlgX + dlgW, cy + 32};
    DrawTextW(hdcBuffer, L"DOTS COLLECTED", -1, &dotsLabelRect,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    cy += 35;

    // "X/Y"
    WCHAR dotsText[32];
    wsprintfW(dotsText, L"%d/%d", game.collectiblesCollected,
              game.totalCollectibles);
    RECT dotsCountRect = {dlgX, cy, dlgX + dlgW, cy + 32};
    DrawTextW(hdcBuffer, dotsText, -1, &dotsCountRect,
              DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    cy += 35;

    // "!!! BONUS !!!"
    if (game.perfectCompletionAwarded) {
      RECT bonusRect = {dlgX, cy, dlgX + dlgW, cy + 32};
      DrawTextW(hdcBuffer, L"!!! BONUS 25 COINS !!!", -1, &bonusRect,
                DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    }
    cy += 35;

    // CONTINUE button
    RECT contBtn = GetLevelCompleteContinueRect(clientRect);
    int cbW = contBtn.right - contBtn.left;
    int cbH = contBtn.bottom - contBtn.top;
    DrawButton(hdcBuffer, hdcSprite, contBtn.left, contBtn.top, cbW, cbH, L"",
               -1, -1, pressedContinueButton);
    {
      SetBkMode(hdcBuffer, TRANSPARENT);
      SetTextColor(hdcBuffer, BUTTON_TEXT_COLOR);
      HFONT oldBtnFont = (HFONT)SelectObject(hdcBuffer, hArcadeFontSmall);
      int btnYOffset = pressedContinueButton ? BUTTON_PRESS_OFFSET : 0;
      RECT btnTextRect = {contBtn.left, contBtn.top + btnYOffset, contBtn.right,
                          contBtn.bottom + btnYOffset};
      DrawTextW(hdcBuffer, L"CONTINUE", -1, &btnTextRect,
                DT_CENTER | DT_VCENTER | DT_SINGLELINE);
      SelectObject(hdcBuffer, oldBtnFont);
    }

    SelectObject(hdcBuffer, oldFont);
  }

  DeleteDC(hdcSprite);
}

// ========== Main Render ==========

void Renderer::Render(HDC hdc, HWND hWnd, Game &game) {
  frameCounter++;

  RECT clientRect;
  GetClientRect(hWnd, &clientRect);

  HDC hdcBuffer = CreateCompatibleDC(hdc);
  HBITMAP hBufferBmp =
      CreateCompatibleBitmap(hdc, clientRect.right, clientRect.bottom);
  HBITMAP oldBufferBmp = (HBITMAP)SelectObject(hdcBuffer, hBufferBmp);

  if (game.state == STATE_MENU) {
    RenderMenu(hdcBuffer, clientRect);
  } else if (game.state == STATE_LEVEL_SELECT) {
    RenderLevelSelect(hdc, hdcBuffer, clientRect, game);
  } else {
    RenderPlaying(hdc, hdcBuffer, clientRect, game);
  }

  BitBlt(hdc, 0, 0, clientRect.right, clientRect.bottom, hdcBuffer, 0, 0,
         SRCCOPY);

  SelectObject(hdcBuffer, oldBufferBmp);
  DeleteObject(hBufferBmp);
  DeleteDC(hdcBuffer);
}
