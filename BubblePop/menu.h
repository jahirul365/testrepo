#ifndef MENU_H
#define MENU_H

#include "iGraphics.h"
#include "profile.h"

extern bool inMenu;
extern void stopMenuMusic();
extern void startLevel1();
extern void startLevel2();
extern void startLevel3();
int getBitmapTextWidth(const char* text, void* font);   // defined in iMain.cpp

static int menuBg[2] = { 0, 0 };
static int menuBgFrame = 0;
static int menuTitleImage = 0;

static int btnX = 60;
static int btnW = 280;
static int btnH = 70;
static int btnNewY = 490;
static int btnLoadY = 390;
static int btnAboutY = 290;
static int btnExitY = 190;

// small "Test" button (bottom-right corner): all levels unlocked, nothing is saved
static int testBtnX = 1160;
static int testBtnY = 25;
static int testBtnW = 100;
static int testBtnH = 42;

// menuSelected: 0 New Game, 1 Load Game, 2 About, 3 Exit, 4 Test
static int menuSelected = -1;
static bool usingKeyboard = false;
static bool aboutOpen = false;

// ---- NAME INPUT (New Game) ----
static bool inNameInput = false;
static char nameBuffer[MAX_NAME_LEN + 1] = "";
static int nameLen = 0;
static const char* nameError = "";

// ---- LOAD GAME LIST ----
static bool inLoadList = false;
static int loadSelected = -1;
static bool loadUsingKeyboard = false;

static int loadRowX = 320;
static int loadRowW = 640;
static int loadRowH = 44;
static int loadRowTopY = 520;   // y of row 0, each next row is loadRowStep lower
static int loadRowStep = 52;

// ---- LEVEL SELECT ----
static bool inLevelSelect = false;
static int levelSelected = -1;
static bool levelUsingKeyboard = false;

static int lvlBtnW = 300;
static int lvlBtnH = 80;
static int lvlBtnX = 490;
static int lvlBtnY[3] = { 430, 310, 190 };

static void loadMenuImages()
{
	menuBg[0] = iLoadImage("Images//menu_1.jpg");
	menuBg[1] = iLoadImage("Images//menu_2.jpg");
	menuTitleImage = iLoadImage("Images//menu_title.png");
	printf("Loaded menu background images\n");
}

static void advanceMenuFrame()
{
	menuBgFrame = (menuBgFrame + 1) % 2;
}

static bool insideButton(int mx, int my, int x, int y, int w, int h)
{
	return (mx >= x && mx <= x + w &&
		my >= y && my <= y + h);
}

static void drawCenteredText(int cx, int y, const char* text, void* font)
{
	iText(cx - getBitmapTextWidth(text, font) / 2, y, text, font);
}

static void drawMenuButton(int x, int y, int w, int h,
	const char* label, bool highlighted, bool disabled = false)
{
	if (disabled)
	{
		iSetColor(60, 60, 60);
		iFilledRectangle(x, y, w, h);
	}
	else if (highlighted)
	{
		iSetColor(220, 60, 60);
		iFilledRectangle(x, y, w, h);
	}

	if (disabled) iSetColor(120, 120, 120);
	else          iSetColor(255, 255, 255);
	iRectangle(x, y, w, h);

	if (disabled) iSetColor(120, 120, 120);
	else          iSetColor(255, 255, 255);
	iText(x + 20, y + h / 2 - 14, label, GLUT_BITMAP_TIMES_ROMAN_24);
}

static void drawAboutOverlay()
{
	iSetColor(0, 0, 0);
	iFilledRectangle(200, 100, 880, 520);

	iSetColor(220, 60, 60);
	iRectangle(200, 100, 880, 520);

	iSetColor(255, 80, 80);
	iText(340, 585, "HOW TO PLAY", GLUT_BITMAP_TIMES_ROMAN_24);

	iSetColor(255, 255, 255);

	iText(230, 545, "CONTROLS", GLUT_BITMAP_HELVETICA_18);
	iText(230, 515, "Hold Left Mouse / Space - Fly upward (jetpack)", GLUT_BITMAP_HELVETICA_12);
	iText(230, 495, "Release - Fall down", GLUT_BITMAP_HELVETICA_12);

	iText(230, 455, "OBJECTIVE", GLUT_BITMAP_HELVETICA_18);
	iText(230, 425, "Survive 700 m using your jetpack.", GLUT_BITMAP_HELVETICA_12);
	iText(230, 405, "Dodge obstacles and collect fuel packs to stay alive.", GLUT_BITMAP_HELVETICA_12);

	iText(230, 365, "TIPS", GLUT_BITMAP_HELVETICA_18);
	iText(230, 340, "- Fuel drains constantly - grab fuel packs quickly.", GLUT_BITMAP_HELVETICA_12);
	iText(230, 320, "- Missing 3 fuel packs in a row ends the run.", GLUT_BITMAP_HELVETICA_12);
	iText(230, 300, "- Speed increases in the final 300 m.", GLUT_BITMAP_HELVETICA_12);
	iText(230, 280, "- Collect coins for score.", GLUT_BITMAP_HELVETICA_12);

	iSetColor(180, 180, 180);
	iText(230, 150, "Press ESC or ENTER to close", GLUT_BITMAP_HELVETICA_12);
}

// -------------------------------------------------------
// SCREEN OPENERS
// -------------------------------------------------------
static void openNameInput()
{
	inNameInput = true;
	nameBuffer[0] = '\0';
	nameLen = 0;
	nameError = "";
}

static void openLoadList()
{
	inLoadList = true;
	loadSelected = -1;
	loadUsingKeyboard = false;
}

static void openLevelSelect()
{
	inLevelSelect = true;
	levelSelected = -1;
	levelUsingKeyboard = false;
}

// -------------------------------------------------------
// NAME INPUT SCREEN
// -------------------------------------------------------
static void drawNameInput()
{
	iShowImage(0, 0, 1280, 720, menuBg[menuBgFrame]);

	iSetColor(0, 0, 0);
	iFilledRectangle(340, 220, 600, 280);
	iSetColor(220, 60, 60);
	iRectangle(340, 220, 600, 280);

	iSetColor(255, 80, 80);
	drawCenteredText(640, 445, "ENTER PLAYER NAME", GLUT_BITMAP_TIMES_ROMAN_24);

	// text box
	iSetColor(30, 30, 30);
	iFilledRectangle(440, 350, 400, 50);
	iSetColor(255, 255, 255);
	iRectangle(440, 350, 400, 50);
	iText(452, 366, nameBuffer, GLUT_BITMAP_TIMES_ROMAN_24);

	// blinking cursor (menuBgFrame flips every 500 ms)
	if (menuBgFrame == 0)
	{
		int w = getBitmapTextWidth(nameBuffer, GLUT_BITMAP_TIMES_ROMAN_24);
		iFilledRectangle(452 + w + 2, 360, 2, 30);
	}

	iSetColor(255, 90, 90);
	drawCenteredText(640, 315, nameError, GLUT_BITMAP_HELVETICA_18);

	iSetColor(180, 180, 180);
	drawCenteredText(640, 250, "Press ENTER to continue   |   ESC to go back", GLUT_BITMAP_HELVETICA_12);
}

// Adds one typed key to the name box (backspace / printable characters).
static bool handleNameInputChar(unsigned char key)
{
	if (!inMenu || !inNameInput) return false;

	if (key == 8) // backspace
	{
		if (nameLen > 0) nameBuffer[--nameLen] = '\0';
		nameError = "";
		return true;
	}

	if (key >= 32 && key <= 126)
	{
		if (key == ' ' && nameLen == 0) return true;   // no leading spaces
		if (nameLen < MAX_NAME_LEN)
		{
			nameBuffer[nameLen++] = (char)key;
			nameBuffer[nameLen] = '\0';
		}
		nameError = "";
	}

	return true; // Enter / ESC are handled by the polling code in fixedUpdate()
}

// -------------------------------------------------------
// NAME TYPING -- two input paths, so it works whichever your iGraphics supports:
//   1) iKeyboard() callback      -> handleNameInputCallbackKey()
//   2) isKeyPressed() polling    -> pollNameInputKeys()  (same way ENTER / ESC already work)
// If both fire for the same key press, the second one is ignored (within 3 ticks).
// -------------------------------------------------------
static int inputTick = 0;
static int lastCallbackTick[256];
static int lastPollTick[256];
static bool prevTypeKeys[256];
static bool typeKeysInit = false;

static void ensureTypeKeysInit()
{
	if (typeKeysInit) return;
	for (int i = 0; i < 256; i++)
	{
		lastCallbackTick[i] = -100;
		lastPollTick[i] = -100;
		prevTypeKeys[i] = false;
	}
	typeKeysInit = true;
}

// Called from iKeyboard(). Returns true if the key belongs to the name box.
static bool handleNameInputCallbackKey(unsigned char key)
{
	if (!inMenu || !inNameInput) return false;
	ensureTypeKeysInit();

	if (inputTick - lastPollTick[key] > 3)
	{
		lastCallbackTick[key] = inputTick;
		handleNameInputChar(key);
	}
	return true;
}

// Called every 16 ms tick while in the menu (from fixedUpdate).
static void pollNameInputKeys()
{
	ensureTypeKeysInit();
	inputTick++;

	for (int k = 8; k <= 126; k++)
	{
		if (k != 8 && k < 32) continue;   // only backspace + printable characters

		bool cur = isKeyPressed((unsigned char)k);
		if (cur && !prevTypeKeys[k] && inNameInput && inputTick - lastCallbackTick[k] > 3)
		{
			lastPollTick[k] = inputTick;
			handleNameInputChar((unsigned char)k);
		}
		prevTypeKeys[k] = cur;
	}
}

static void confirmNameInput()
{
	char name[MAX_NAME_LEN + 1];
	strcpy_s(name, MAX_NAME_LEN + 1, nameBuffer);
	trimName(name);

	if (name[0] == '\0')
	{
		nameError = "Please type a name first.";
		return;
	}

	// same name (any capitalisation) = same player, keep their progress
	int idx = findProfileByName(name);
	if (idx < 0)
	{
		idx = addProfile(name);          // also writes savegame.txt
		if (idx < 0)
		{
			nameError = "Save list is full (10 players).";
			return;
		}
	}

	currentProfile = idx;
	testMode = false;
	inNameInput = false;
	openLevelSelect();
}

// -------------------------------------------------------
// LOAD GAME SCREEN
// -------------------------------------------------------
static void drawLoadList()
{
	iSetColor(0, 0, 0);
	iFilledRectangle(0, 0, 1280, 720);

	iSetColor(255, 255, 255);
	drawCenteredText(640, 610, "LOAD GAME", GLUT_BITMAP_TIMES_ROMAN_24);

	if (profileCount == 0)
	{
		iSetColor(200, 200, 200);
		drawCenteredText(640, 380, "No saved games yet.", GLUT_BITMAP_HELVETICA_18);
		drawCenteredText(640, 350, "Start a New Game to create one.", GLUT_BITMAP_HELVETICA_18);
	}

	// rows are listed by rank: profileOrder[0] is the highest total score
	for (int r = 0; r < profileCount; r++)
	{
		const Profile& pr = profiles[profileOrder[r]];
		int y = loadRowTopY - r * loadRowStep;

		char label[MAX_NAME_LEN + 8];
		sprintf_s(label, "%d. %s", r + 1, pr.name);
		drawMenuButton(loadRowX, y, loadRowW, loadRowH, label, loadSelected == r);

		char scoreStr[32];
		sprintf_s(scoreStr, "Score: %d", profileTotalScore(pr));
		iSetColor(255, 215, 0);
		iText(loadRowX + 280, y + loadRowH / 2 - 6, scoreStr, GLUT_BITMAP_HELVETICA_18);

		char info[40];
		sprintf_s(info, "Cleared: %d / %d", pr.cleared, TOTAL_LEVELS);
		iSetColor(255, 255, 255);
		iText(loadRowX + loadRowW - 150, y + loadRowH / 2 - 6, info, GLUT_BITMAP_HELVETICA_18);
	}

	iSetColor(150, 150, 150);
	drawCenteredText(640, 25, "Click a name (or use UP/DOWN + ENTER)   |   ESC to go back", GLUT_BITMAP_HELVETICA_12);
}

static void confirmLoadSelection()
{
	if (loadSelected < 0 || loadSelected >= profileCount) return;

	currentProfile = profileOrder[loadSelected];   // loadSelected is a rank in the sorted list
	testMode = false;
	inLoadList = false;
	openLevelSelect();
}

// -------------------------------------------------------
// LEVEL SELECT SCREEN
// -------------------------------------------------------
static void drawLevelSelect()
{
	iSetColor(0, 0, 0);
	iFilledRectangle(0, 0, 1280, 720);

	iSetColor(255, 255, 255);
	drawCenteredText(640, 610, "SELECT LEVEL", GLUT_BITMAP_TIMES_ROMAN_24);

	// who is playing
	if (testMode)
	{
		iSetColor(255, 200, 0);
		drawCenteredText(640, 570, "TEST MODE - all levels unlocked, progress is not saved", GLUT_BITMAP_HELVETICA_18);
	}
	else if (currentProfile >= 0 && currentProfile < profileCount)
	{
		char who[64];
		sprintf_s(who, "Player: %s", profiles[currentProfile].name);
		iSetColor(0, 210, 255);
		drawCenteredText(640, 570, who, GLUT_BITMAP_HELVETICA_18);
	}

	const char* unlockedLabels[3] = { "Level 1", "Level 2", "Level 3" };
	const char* lockedLabels[3] = { "Level 1  (Locked)", "Level 2  (Locked)", "Level 3  (Locked)" };

	for (int i = 0; i < 3; i++)
	{
		bool locked = !isLevelUnlocked(i);
		drawMenuButton(lvlBtnX, lvlBtnY[i], lvlBtnW, lvlBtnH,
			locked ? lockedLabels[i] : unlockedLabels[i], levelSelected == i, locked);
	}

	iSetColor(150, 150, 150);
	iText(490, 110, "Press ESC to go back", GLUT_BITMAP_HELVETICA_12);
}

static void confirmLevelSelection()
{
	if (levelSelected < 0 || !isLevelUnlocked(levelSelected)) return;

	inLevelSelect = false;
	stopMenuMusic();

	if (levelSelected == 0)      startLevel1();
	else if (levelSelected == 1) startLevel2();
	else if (levelSelected == 2) startLevel3();
}

// -------------------------------------------------------
// MAIN MENU
// -------------------------------------------------------
static void drawMenu()
{
	if (inNameInput)
	{
		drawNameInput();
		return;
	}

	if (inLoadList)
	{
		drawLoadList();
		return;
	}

	if (inLevelSelect)
	{
		drawLevelSelect();
		return;
	}

	iShowImage(0, 0, 1280, 720, menuBg[menuBgFrame]);

	iShowImage(btnX, btnNewY + 90, 280, 153, menuTitleImage);

	drawMenuButton(btnX, btnNewY, btnW, btnH, "New Game", menuSelected == 0);
	drawMenuButton(btnX, btnLoadY, btnW, btnH, "Load Game", menuSelected == 1);
	drawMenuButton(btnX, btnAboutY, btnW, btnH, "About Game", menuSelected == 2);
	drawMenuButton(btnX, btnExitY, btnW, btnH, "Exit", menuSelected == 3);

	drawMenuButton(testBtnX, testBtnY, testBtnW, testBtnH, "Test", menuSelected == 4);

	if (aboutOpen)
		drawAboutOverlay();
}

static void handleMenuMouseMove(int mx, int my)
{
	if (!inMenu) return;

	if (inNameInput) return;

	if (inLoadList)
	{
		loadUsingKeyboard = false;
		loadSelected = -1;
		for (int i = 0; i < profileCount; i++)
		{
			if (insideButton(mx, my, loadRowX, loadRowTopY - i * loadRowStep, loadRowW, loadRowH))
				loadSelected = i;
		}
		return;
	}

	if (inLevelSelect)
	{
		levelUsingKeyboard = false;
		levelSelected = -1;
		for (int i = 0; i < 3; i++)
		{
			if (isLevelUnlocked(i) && insideButton(mx, my, lvlBtnX, lvlBtnY[i], lvlBtnW, lvlBtnH))
				levelSelected = i;
		}
		return;
	}

	usingKeyboard = false;
	if (insideButton(mx, my, btnX, btnNewY, btnW, btnH)) menuSelected = 0;
	else if (insideButton(mx, my, btnX, btnLoadY, btnW, btnH)) menuSelected = 1;
	else if (insideButton(mx, my, btnX, btnAboutY, btnW, btnH)) menuSelected = 2;
	else if (insideButton(mx, my, btnX, btnExitY, btnW, btnH)) menuSelected = 3;
	else if (insideButton(mx, my, testBtnX, testBtnY, testBtnW, testBtnH)) menuSelected = 4;
	else                                                         menuSelected = -1;
}

static void startTestMode()
{
	testMode = true;
	currentProfile = -1;
	openLevelSelect();
}

static void confirmMenuSelection()
{
	switch (menuSelected)
	{
	case 0: openNameInput(); break;
	case 1: openLoadList(); break;
	case 2: aboutOpen = true; break;
	case 3: exit(0);
	case 4: startTestMode(); break;
	default: break;
	}
}

static bool handleMenuClick(int mx, int my)
{
	if (!inMenu) return false;

	if (inNameInput) return true;   // typing only; ENTER / ESC are handled by the keyboard code

	if (inLoadList)
	{
		for (int i = 0; i < profileCount; i++)
		{
			if (insideButton(mx, my, loadRowX, loadRowTopY - i * loadRowStep, loadRowW, loadRowH))
			{
				loadSelected = i;
				confirmLoadSelection();
				return true;
			}
		}
		return true;
	}

	if (inLevelSelect)
	{
		for (int i = 0; i < 3; i++)
		{
			if (insideButton(mx, my, lvlBtnX, lvlBtnY[i], lvlBtnW, lvlBtnH))
			{
				if (isLevelUnlocked(i))          // locked levels ignore clicks
				{
					levelSelected = i;
					confirmLevelSelection();
				}
				return true;
			}
		}
		return true;
	}

	if (aboutOpen) { aboutOpen = false; return true; }

	if (insideButton(mx, my, btnX, btnNewY, btnW, btnH)) { openNameInput(); return true; }
	if (insideButton(mx, my, btnX, btnLoadY, btnW, btnH)) { openLoadList(); return true; }
	if (insideButton(mx, my, btnX, btnAboutY, btnW, btnH)) { aboutOpen = true; return true; }
	if (insideButton(mx, my, btnX, btnExitY, btnW, btnH)) { exit(0); }
	if (insideButton(mx, my, testBtnX, testBtnY, testBtnW, testBtnH)) { startTestMode(); return true; }

	return false;
}

static void handleMenuKey(unsigned char key)
{
	if (!inMenu) return;

	if (inNameInput)
	{
		if (key == 13) confirmNameInput();
		else if (key == 27) inNameInput = false;
		return;
	}

	if (inLoadList)
	{
		if (key == 27) { inLoadList = false; menuSelected = 1; }
		if (key == 13) confirmLoadSelection();
		return;
	}

	if (inLevelSelect)
	{
		if (key == 27) { inLevelSelect = false; menuSelected = 0; }
		if (key == 13) confirmLevelSelection();
		return;
	}

	if (aboutOpen)
	{
		if (key == 13 || key == 27) aboutOpen = false;
		return;
	}

	if (key == 13)
		confirmMenuSelection();
}

static void handleMenuSpecialKey(int key)
{
	if (!inMenu) return;

	if (inNameInput) return;

	if (inLoadList)
	{
		if (profileCount == 0) return;

		if (key == GLUT_KEY_UP || key == GLUT_KEY_DOWN)
		{
			if (!loadUsingKeyboard)
			{
				loadUsingKeyboard = true;
				loadSelected = 0;
				return;
			}

			if (key == GLUT_KEY_UP)
				loadSelected = (loadSelected + profileCount - 1) % profileCount;
			if (key == GLUT_KEY_DOWN)
				loadSelected = (loadSelected + 1) % profileCount;
		}
		return;
	}

	if (inLevelSelect)
	{
		if (key == GLUT_KEY_UP || key == GLUT_KEY_DOWN)
		{
			int n = unlockedLevelCount();   // only unlocked levels can be selected

			if (!levelUsingKeyboard)
			{
				levelUsingKeyboard = true;
				levelSelected = 0;
				return;
			}

			if (key == GLUT_KEY_UP)
				levelSelected = (levelSelected + n - 1) % n;
			if (key == GLUT_KEY_DOWN)
				levelSelected = (levelSelected + 1) % n;
		}
		return;
	}

	if (aboutOpen) return;

	if (key == GLUT_KEY_UP || key == GLUT_KEY_DOWN)
	{
		if (!usingKeyboard)
		{
			usingKeyboard = true;
			menuSelected = 0;
			return;
		}

		if (key == GLUT_KEY_UP)
			menuSelected = (menuSelected + 3) % 4;
		if (key == GLUT_KEY_DOWN)
			menuSelected = (menuSelected + 1) % 4;
	}
}

#endif // MENU_H