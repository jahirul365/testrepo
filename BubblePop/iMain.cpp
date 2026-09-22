#include "iGraphics.h"
#include "menu.h"
#include "profile.h"
#include "obstacle.h"
#include "character.h"
#include "drone.h"
#include "bullet.h"
#include <mmsystem.h>
#include <cstring>
#pragma comment(lib, "winmm.lib")

// -------------------------------------------------------
// GAME STATE
// -------------------------------------------------------
bool inMenu = true;
int currentLevel = 1;
bool isPaused = false;
bool showLevel2Intro = false;

float remainingDistance = 700.0f;
bool isWin = false;
bool isLose = false;
bool winRecorded = false;   // true once this win has been written to the save file

float distanceRate = 5.33f;

bool isBonusPhase = false;
float bonusDistance = 60.0f;

int score = 0;
float maxFuel = 100.0f;
float currentFuel = 100.0f;

bool DEBUG_HITBOXES = false;

// -------------------------------------------------------
// AUDIO
// -------------------------------------------------------
void startMenuMusic()
{
	mciSendString("open \"menu.mp3\" type mpegvideo alias menuMusic", NULL, 0, NULL);
	mciSendString("play menuMusic from 0", NULL, 0, NULL);
}

void restartMenuMusicIfLooping()
{
	if (inMenu) mciSendString("play menuMusic from 0", NULL, 0, NULL);
}

void resumeMenuMusic()
{
	mciSendString("play menuMusic from 0", NULL, 0, NULL);
}

void stopMenuMusic()
{
	mciSendString("stop menuMusic", NULL, 0, NULL);
}

int currentGroundSound = 0;

void loadGameplayAudio()
{
	mciSendString("open \"run.mp3\" type mpegvideo alias runMusic", NULL, 0, NULL);
	mciSendString("open \"jetpack.mp3\" type mpegvideo alias jetpackMusic", NULL, 0, NULL);
}

void startRunMusic()      { mciSendString("play runMusic from 0 repeat", NULL, 0, NULL); }
void stopRunMusic()       { mciSendString("stop runMusic", NULL, 0, NULL); }
void startJetpackMusic()  { mciSendString("play jetpackMusic from 0 repeat", NULL, 0, NULL); }
void stopJetpackMusic()   { mciSendString("stop jetpackMusic", NULL, 0, NULL); }

void updateCharacterAudio()
{
	if (isWin || isLose)
	{
		if (currentGroundSound != 0)
		{
			stopRunMusic();
			stopJetpackMusic();
			currentGroundSound = 0;
		}
		return;
	}

	int desired;
	if (isAirborne() && isJumping)      desired = 2;
	else if (!isAirborne())             desired = 1;
	else                                desired = 0;

	if (desired != currentGroundSound)
	{
		if (currentGroundSound == 1) stopRunMusic();
		if (currentGroundSound == 2) stopJetpackMusic();

		if (desired == 1) startRunMusic();
		if (desired == 2) startJetpackMusic();

		currentGroundSound = desired;
	}
}

// -------------------------------------------------------
// BACKGROUND & FLOOR
// -------------------------------------------------------
int images[4];
int level2Images[4];
int bonusBackgroundImage;
int level3BackgroundImage;
int currentFrame = 0;

void advanceFrame()
{
	currentFrame = (currentFrame + 1) % 4;
}

int floorImage;
float floorX = 0.0f;
float floorSpeed = 220.0f;
int floorWidth = 1280;
int floorHeight = 124;

// -------------------------------------------------------
// RESET GAME
// -------------------------------------------------------
void resetGame()
{
	if (currentLevel == 1)
	{
		remainingDistance = 700.0f;
		distanceRate = 5.33f;
	}
	else if (currentLevel == 2)
	{
		remainingDistance = 500.0f;
		distanceRate = 5.33f;
	}
	else if (currentLevel == 3)
	{
		remainingDistance = 400.0f;
		distanceRate = 4.00f;
	}

	isWin = false;
	isLose = false;
	winRecorded = false;
	isBonusPhase = false;
	bonusDistance = 60.0f;
	showLevel2Intro = false;

	score = 0;
	currentFuel = 100.0f;

	resetCharacter();
	resetObstacles(currentLevel);
	resetBullets(currentLevel);
	resetDronePhases();

	currentGroundSound = 0;
}

void startLevel1()
{
	currentLevel = 1;
	resetGame();
	inMenu = false;
}

void startLevel2()
{
	currentLevel = 2;
	resetGame();
	inMenu = false;
	showLevel2Intro = true;
}

void startLevel3()
{
	currentLevel = 3;
	resetGame();
	inMenu = false;
}

// -------------------------------------------------------
// UPDATE LOGIC (16 ms)
// -------------------------------------------------------
// Jetpack fuel burn. Same rate everywhere (normal run, 300m Drone Gauntlet, Final Fight).
void updateFuelDrain()
{
	if (isAirborne() && isJumping)
	{
		currentFuel -= 5.33f * 0.016f * 1.25f;
		if (currentFuel <= 0.0f)
		{
			currentFuel = 0.0f;
			isJumping = false;
		}
	}
}

void updateFloor()
{
	if (inMenu || isWin || isLose || isPaused || showLevel2Intro) return;

	floorX -= floorSpeed * 0.016f;
	if (floorX <= -floorWidth) floorX += floorWidth;

	float charLeft, charRight, charBottom, charTop;
	getCharacterAABB(charLeft, charRight, charBottom, charTop);

	if (isBonusPhase)
	{
		bonusDistance -= 5.33f * 0.016f;
		if (bonusDistance <= 0.0f)
		{
			bonusDistance = 0.0f;
			isBonusPhase = false;

			if (currentLevel == 3)
			{
				startFinalFight();
			}
			else
			{
				isWin = true;
			}
		}

		updateBonusCoins(floorSpeed, charLeft, charRight, charBottom, charTop, score);
		updateCharacterAudio();
		return;
	}

	if (isFinalFight)
	{
		updateFinalFight(isWin, isLose);

		// Fuel works in the Final Fight too (only once the drones are live, not during the banner).
		if (finalFightActive && !isWin && !isLose)
		{
			updateFuelDrain();
			updateDroneFuelPickup(floorSpeed, charLeft, charRight, charBottom, charTop, currentFuel, maxFuel);
		}

		updateBullets(inMenu, isWin, isLose, characterX, characterY, currentLevel, remainingDistance);
		updateCharacterAudio();
		return;
	}

	// ---------------------------------------------------
	// DRONE GAUNTLET 1 (300m remaining): takes over completely.
	// Obstacles/fuel/etc are cleared, distance is frozen, pure
	// dodge for 30 seconds. Bullets still update (so any bullet
	// already in flight can travel off screen) but auto-fire and
	// ammo pickups are suppressed inside bullet.h.
	// ---------------------------------------------------
	if (isDronePhase1)
	{
		updateDronePhase1(isLose, currentLevel);

		// Fuel works in the Gauntlet too. The isDronePhase1 check also skips the frame in
		// which the phase just ended (the normal field, incl. its own fuel pack, is back by then).
		if (isDronePhase1 && !showDrone1Warning && !isLose)
		{
			updateFuelDrain();
			updateDroneFuelPickup(floorSpeed, charLeft, charRight, charBottom, charTop, currentFuel, maxFuel);
		}

		updateBullets(inMenu, isWin, isLose, characterX, characterY, currentLevel, remainingDistance);
		updateCharacterAudio();
		return;
	}

	float distanceTraveled = distanceRate * 0.016f;

	if (remainingDistance > 0)
	{
		remainingDistance -= distanceTraveled;
		if (remainingDistance <= 0)
		{
			remainingDistance = 0;

			if (currentLevel == 2 || currentLevel == 3)
			{
				isBonusPhase = true;
				bonusDistance = 60.0f;
				initBonusCoins();

				// If the 100m Drone Intrusion hadn't finished its 30s yet,
				// end it cleanly so nothing lingers into the bonus round.
				if (isDronePhase2) endDronePhase2();
			}
			else
			{
				isWin = true;
			}
		}
	}

	// ---------------------------------------------------
	// DRONE PHASE TRIGGERS (Level 3 only, each fires exactly once)
	// ---------------------------------------------------
	if (currentLevel == 3)
	{
		if (!dronePhase1Triggered && remainingDistance <= 300.0f)
		{
			dronePhase1Triggered = true;
			startDronePhase1();
			updateCharacterAudio();
			return; // field is cleared this same frame; resume normal updates next frame
		}

		if (!dronePhase2Triggered && remainingDistance <= 100.0f)
		{
			dronePhase2Triggered = true;
			startDronePhase2();
		}
	}

	updateFuelDrain();

	float currentSpeed = floorSpeed;
	if (remainingDistance <= 300) currentSpeed += 100.0f;

	updateObstacles(currentSpeed, charLeft, charRight, charBottom, charTop, score, currentFuel, maxFuel, isLose, currentLevel, remainingDistance);
	updateBullets(inMenu, isWin, isLose, characterX, characterY, currentLevel, remainingDistance);

	// ---------------------------------------------------
	// DRONE INTRUSION 2 (100m remaining): layered on top of the
	// normal run above -- obstacles/fuel/coins keep working as usual.
	// ---------------------------------------------------
	if (isDronePhase2)
	{
		updateDronePhase2(isLose);
	}

	updateCharacterAudio();
}

void updateJump()
{
	if (isPaused || showLevel2Intro) return;
	updateCharacter(inMenu, isWin, isLose);
}

// -------------------------------------------------------
// TEXT WIDTH HELPER
// -------------------------------------------------------
int getBitmapTextWidth(const char* text, void* font)
{
	int width = 0;
	for (const char* c = text; *c != '\0'; c++)
	{
		width += glutBitmapWidth(font, *c);
	}
	return width;
}

// The Level 3 win screen carries the developer credits, so its box is taller
// and its RESTART/MENU buttons sit lower to make room. Shared by iDraw()
// (to draw them there) and iMouse() (so clicks are tested at the same spot).
bool isFinalWin()
{
	return isWin && currentLevel == 3;
}

int winLoseButtonY()
{
	return isFinalWin() ? 100 : 240;
}

// -------------------------------------------------------
// DRAW
// -------------------------------------------------------
void iDraw()
{
	iClear();

	if (inMenu)
	{
		drawMenu();
		return;
	}

	if (isPaused)
	{
		iSetColor(0, 0, 0);
		iFilledRectangle(0, 0, 1280, 720);

		iSetColor(255, 255, 255);

		const char* line1 = "GAME PAUSED";
		const char* line2 = "Press ENTER to resume";
		const char* line3 = "Press ESC again to go back to the level selection menu";

		void* font = GLUT_BITMAP_TIMES_ROMAN_24;
		int w1 = getBitmapTextWidth(line1, font);
		int w2 = getBitmapTextWidth(line2, font);
		int w3 = getBitmapTextWidth(line3, font);

		iText(640 - w1 / 2, 440, line1, font);
		iText(640 - w2 / 2, 380, line2, font);
		iText(640 - w3 / 2, 330, line3, font);
		return;
	}

	if (isBonusPhase)
	{
		iShowImage(0, 0, 1280, 720, bonusBackgroundImage);
	}
	else if (currentLevel == 1)
	{
		iShowImage(0, 0, 1280, 720, images[currentFrame]);
	}
	else if (currentLevel == 2)
	{
		iShowImage(0, 0, 1280, 720, level2Images[currentFrame]);
	}
	else if (currentLevel == 3)
	{
		iShowImage(0, 0, 1280, 720, level3BackgroundImage);
	}

	iShowImage((int)floorX, 0, floorWidth, floorHeight, floorImage);
	iShowImage((int)floorX + floorWidth, 0, floorWidth, floorHeight, floorImage);

	if (isBonusPhase)
	{
		drawBonusCoins();
	}
	else if (isFinalFight)
	{
		drawDroneFuelPickup();
		drawFinalDroneFight();
	}
	else if (isDronePhase1)
	{
		drawDroneFuelPickup();
		drawDrone1Squad();
		drawDroneBullets();
	}
	else
	{
		drawObstacles(currentLevel);

		// Drone Intrusion (100m) is layered on top of the normal obstacle field.
		if (isDronePhase2)
		{
			drawDrone2();
			drawDroneRockets();
		}
	}

	drawCharacter(currentLevel == 2 || currentLevel == 3);   // Level 1 has no gun

	if ((currentLevel == 2 || currentLevel == 3) && !isBonusPhase)
	{
		drawObstacleShields(currentLevel);
		drawBullets();
		drawBulletPickup();
	}

	if (DEBUG_HITBOXES && !isBonusPhase)
	{
		drawObstacleHitboxes(currentLevel);
		drawCharacterHitbox();
	}

	// -----------------------------------------------
	// HUD DISPLAY
	// -----------------------------------------------
	iSetColor(255, 255, 255);

	char distStr[50];
	if (isBonusPhase)
		sprintf_s(distStr, "BONUS: %d m", (int)bonusDistance);
	else if (isFinalFight)
		sprintf_s(distStr, "FINAL FIGHT");
	else if (isDronePhase1)
		sprintf_s(distStr, "DODGE: %d s", (int)dronePhase1Timer);
	else
		sprintf_s(distStr, "Distance: %d m", (int)remainingDistance);

	iText(1050, 670, distStr, GLUT_BITMAP_TIMES_ROMAN_24);

	char scoreStr[50];
	sprintf_s(scoreStr, "Score: %d", score);
	iText(1100, 45, scoreStr, GLUT_BITMAP_TIMES_ROMAN_24);

	// HEALTH BAR (Drone Gauntlet + Final Drone Fight only; sits just above the fuel bar)
	bool showHealthBar = isDronePhase1 || (isFinalFight && finalFightActive);
	if (showHealthBar)
	{
		drawHealthBar();
	}

	// FUEL BAR (always visible, including the drone encounters)
	iSetColor(100, 100, 100);
	iFilledRectangle(1050, 605, 180, 18);

	if (currentFuel > 40.0f) iSetColor(0, 230, 0);
	else                     iSetColor(255, 50, 50);
	iFilledRectangle(1050, 605, (int)((currentFuel / maxFuel) * 180), 18);

	iSetColor(255, 255, 255);
	iRectangle(1050, 605, 180, 18);
	iText(980, 608, "Fuel:", GLUT_BITMAP_HELVETICA_12);

	// AMMO HUD BOX (Level 2 & Level 3, hidden during the Drone Gauntlet since shooting is disabled there)
	if ((currentLevel == 2 || currentLevel == 3) && !isBonusPhase && !isDronePhase1)
	{
		iSetColor(20, 30, 50);
		iFilledRectangle(1050, 560, 180, 28);
		iSetColor(0, 200, 255);
		iRectangle(1050, 560, 180, 28);

		iSetColor(255, 215, 0);
		char ammoStr[50];
		sprintf_s(ammoStr, "AMMO: %d / %d", currentAmmo, maxAmmo);
		iText(1075, 568, ammoStr, GLUT_BITMAP_HELVETICA_18);
	}

	// -----------------------------------------------
	// WIN / LOSE OVERLAY
	// -----------------------------------------------
	if (isWin)
	{
		// Finishing Level 3 finishes the whole game, so that win screen is taller
		// (extends further down; the top edge stays where the normal box's is)
		// to fit the developer credits below the usual text.
		int boxY = isFinalWin() ? 40 : 160;
		int boxH = isFinalWin() ? 520 : 400;

		iSetColor(15, 35, 80);
		iFilledRectangle(340, boxY, 600, boxH);
		iSetColor(0, 210, 255);
		iRectangle(340, boxY, 600, boxH);

		iSetColor(255, 255, 255);
		if (currentLevel == 2)
			iText(440, 480, "500m COMPLETE! YOU WIN!", GLUT_BITMAP_TIMES_ROMAN_24);
		else if (currentLevel == 3)
			iText(400, 480, "DRONES DESTROYED! YOU WIN!", GLUT_BITMAP_TIMES_ROMAN_24);
		else
			iText(440, 480, "700m COMPLETE! EXCELLENT!", GLUT_BITMAP_TIMES_ROMAN_24);

		// progress feedback (not shown in Test mode, and not after the last level)
		if (!testMode && currentProfile >= 0 && currentLevel < TOTAL_LEVELS)
		{
			char unlockMsg[40];
			sprintf_s(unlockMsg, "Level %d unlocked!", currentLevel + 1);
			iSetColor(120, 255, 160);
			iText(640 - getBitmapTextWidth(unlockMsg, GLUT_BITMAP_HELVETICA_18) / 2, 400, unlockMsg, GLUT_BITMAP_HELVETICA_18);
		}

		// DEVELOPER CREDITS -- shown once, on the screen that finishes the whole game
		if (isFinalWin())
		{
			iSetColor(120, 220, 255);
			const char* creditsHeading = "This game was developed by";
			iText(640 - getBitmapTextWidth(creditsHeading, GLUT_BITMAP_HELVETICA_18) / 2, 420,
				creditsHeading, GLUT_BITMAP_HELVETICA_18);

			iSetColor(255, 255, 255);
			const char* dev1 = "Mohammad Jahirul Islam (AUST CSE)";
			const char* dev2 = "Fatmi Ahsan Shoeb (AUST CSE)";
			const char* dev3 = "Promit Biswas Deb (AUST CSE)";
			iText(640 - getBitmapTextWidth(dev1, GLUT_BITMAP_HELVETICA_18) / 2, 385, dev1, GLUT_BITMAP_HELVETICA_18);
			iText(640 - getBitmapTextWidth(dev2, GLUT_BITMAP_HELVETICA_18) / 2, 355, dev2, GLUT_BITMAP_HELVETICA_18);
			iText(640 - getBitmapTextWidth(dev3, GLUT_BITMAP_HELVETICA_18) / 2, 325, dev3, GLUT_BITMAP_HELVETICA_18);
		}

		int btnY = winLoseButtonY();

		iSetColor(0, 120, 200);
		iFilledRectangle(400, btnY, 200, 50);
		iSetColor(255, 255, 255);
		iText(460, btnY + 18, "RESTART", GLUT_BITMAP_HELVETICA_18);

		iSetColor(20, 60, 130);
		iFilledRectangle(680, btnY, 200, 50);
		iSetColor(255, 255, 255);
		iText(750, btnY + 18, "MENU", GLUT_BITMAP_HELVETICA_18);
	}

	if (isLose)
	{
		iSetColor(15, 35, 80);
		iFilledRectangle(340, 160, 600, 400);
		iSetColor(0, 210, 255);
		iRectangle(340, 160, 600, 400);

		iSetColor(255, 255, 255);
		char loseMsg[60];
		sprintf_s(loseMsg, "YOU LOSE! Distance Left: %d m", (int)remainingDistance);
		iText(420, 480, loseMsg, GLUT_BITMAP_TIMES_ROMAN_24);

		iSetColor(0, 120, 200);
		iFilledRectangle(400, 240, 200, 50);
		iSetColor(255, 255, 255);
		iText(460, 258, "RESTART", GLUT_BITMAP_HELVETICA_18);

		iSetColor(20, 60, 130);
		iFilledRectangle(680, 240, 200, 50);
		iSetColor(255, 255, 255);
		iText(750, 258, "MENU", GLUT_BITMAP_HELVETICA_18);
	}

	// -----------------------------------------------
	// LEVEL 2 INTRO OVERLAY
	// -----------------------------------------------
	if (showLevel2Intro)
	{
		iSetColor(15, 35, 80);
		iFilledRectangle(340, 160, 600, 400);
		iSetColor(0, 210, 255);
		iRectangle(340, 160, 600, 400);

		iSetColor(255, 255, 255);

		const char* heading = "LEVEL 2";
		const char* line1 = "In this level you have a gun and you can use it";
		const char* line2 = "to shoot down certain obstacles.";
		const char* line3 = "Use the left button of your mouse to shoot the gun.";

		int wHeading = getBitmapTextWidth(heading, GLUT_BITMAP_TIMES_ROMAN_24);
		int w1 = getBitmapTextWidth(line1, GLUT_BITMAP_HELVETICA_18);
		int w2 = getBitmapTextWidth(line2, GLUT_BITMAP_HELVETICA_18);
		int w3 = getBitmapTextWidth(line3, GLUT_BITMAP_HELVETICA_18);

		iText(640 - wHeading / 2, 490, heading, GLUT_BITMAP_TIMES_ROMAN_24);
		iText(640 - w1 / 2, 430, line1, GLUT_BITMAP_HELVETICA_18);
		iText(640 - w2 / 2, 400, line2, GLUT_BITMAP_HELVETICA_18);
		iText(640 - w3 / 2, 370, line3, GLUT_BITMAP_HELVETICA_18);

		iSetColor(0, 120, 200);
		iFilledRectangle(540, 240, 200, 50);
		iSetColor(255, 255, 255);
		iText(605, 258, "OKAY", GLUT_BITMAP_HELVETICA_18);
	}

	// -----------------------------------------------
	// LEVEL 3 "FINAL ROUND" BANNER
	// -----------------------------------------------
	if (showFinalRoundText)
	{
		iSetColor(0, 0, 0);
		iFilledRectangle(0, 0, 1280, 720);

		iSetColor(255, 40, 40);
		const char* bigText = "FINAL ROUND";
		void* bigFont = GLUT_BITMAP_TIMES_ROMAN_24;
		int wBig = getBitmapTextWidth(bigText, bigFont);
		iText(640 - wBig / 2, 390, bigText, bigFont);

		iSetColor(255, 255, 255);
		const char* subText = "Two drones inbound -- destroy them both!";
		int wSub = getBitmapTextWidth(subText, GLUT_BITMAP_HELVETICA_18);
		iText(640 - wSub / 2, 340, subText, GLUT_BITMAP_HELVETICA_18);
	}

	// -----------------------------------------------
	// DRONE PHASE WARNING BANNERS
	// -----------------------------------------------
	drawDrone1Warning();
	drawDrone2Warning();
}

// -------------------------------------------------------
// INPUT HANDLERS
// -------------------------------------------------------
void iMouseMove(int mx, int my) {}

void iPassiveMouseMove(int mx, int my)
{
	handleMenuMouseMove(mx, my);
}

void iKeyboard(unsigned char key)
{
	// typing the player name (New Game) takes every key
	if (handleNameInputCallbackKey(key)) return;

	if (key == 'h' || key == 'H') DEBUG_HITBOXES = !DEBUG_HITBOXES;
}

void iSpecialKeyboard(int key) {}

void iMouse(int button, int state, int mx, int my)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		if (inMenu)
		{
			if (handleMenuClick(mx, my)) return;
		}

		if (isWin || isLose)
		{
			int btnY = winLoseButtonY();   // matches wherever iDraw() put the buttons

			if (mx >= 400 && mx <= 600 && my >= btnY && my <= btnY + 50)
			{
				resetGame();
				return;
			}
			if (mx >= 680 && mx <= 880 && my >= btnY && my <= btnY + 50)
			{
				stopRunMusic();
				stopJetpackMusic();
				resetGame();
				inMenu = true;
				resumeMenuMusic();
				return;
			}
		}

		if (showLevel2Intro)
		{
			if (mx >= 540 && mx <= 740 && my >= 240 && my <= 290)
			{
				showLevel2Intro = false;
				return;
			}
			return;
		}

		if (!inMenu && !isWin && !isLose && !isPaused)
		{
			if (currentFuel > 0.0f)
			{
				isJumping = true;
			}

			// Gun works in Level 2 & Level 3, except during the Bonus Phase,
			// the "FINAL ROUND"/Drone banners, and the Drone Gauntlet (pure dodge, no offense).
			if ((currentLevel == 2 || currentLevel == 3) && !isBonusPhase && !showFinalRoundText && !isDronePhase1 && currentAmmo > 0)
			{
				isShooting = true;
				spawnBullet(characterX + 148, characterY + 145);
			}
		}
	}

	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
	{
		if (!inMenu && !isWin && !isLose)
		{
			isJumping = false;
			isShooting = false;
			bulletShootTimer = 0;
		}
	}
}

bool prevUpKey = false;
bool prevDownKey = false;
bool prevEnterKey = false;
bool prevEscKey = false;

void fixedUpdate()
{
	bool curUp = isSpecialKeyPressed(GLUT_KEY_UP);
	bool curDown = isSpecialKeyPressed(GLUT_KEY_DOWN);
	bool curEnter = isKeyPressed(13);
	bool curEsc = isKeyPressed(27);

	// Save level progress the moment a level is won (skipped automatically in Test mode)
	if (isWin && !winRecorded)
	{
		winRecorded = true;
		recordLevelClear(currentLevel, score);   // level progress + this run's coin score
	}

	if (inMenu)
	{
		pollNameInputKeys();   // typing in the New Game name box

		if (curUp    && !prevUpKey)    handleMenuSpecialKey(GLUT_KEY_UP);
		if (curDown  && !prevDownKey)  handleMenuSpecialKey(GLUT_KEY_DOWN);
		if (curEnter && !prevEnterKey) handleMenuKey(13);
		if (curEsc   && !prevEscKey)   handleMenuKey(27);

		prevUpKey = curUp; prevDownKey = curDown;
		prevEnterKey = curEnter; prevEscKey = curEsc;
		return;
	}

	if (showLevel2Intro)
	{
		prevUpKey = curUp; prevDownKey = curDown;
		prevEnterKey = curEnter; prevEscKey = curEsc;
		return;
	}

	if (curEsc && !prevEscKey)
	{
		if (isWin || isLose)
		{
			isPaused = false;
			stopRunMusic();
			stopJetpackMusic();
			inMenu = true;
			resumeMenuMusic();
		}
		else if (!isPaused)
		{
			isPaused = true;
		}
		else
		{
			isPaused = false;
			stopRunMusic();
			stopJetpackMusic();
			inMenu = true;
			inLevelSelect = true;
			levelSelected = -1;
			levelUsingKeyboard = false;
			resumeMenuMusic();
		}
	}

	if (curEnter && !prevEnterKey && isPaused && !isWin && !isLose)
	{
		isPaused = false;
	}

	prevUpKey = curUp; prevDownKey = curDown;
	prevEnterKey = curEnter; prevEscKey = curEsc;

	if (!isWin && !isLose && !isPaused)
	{
		bool curSpace = isKeyPressed(' ');
		if (curSpace && currentFuel > 0.0f)
		{
			isJumping = true;
		}
		else if (!curSpace)
		{
			isJumping = false;
		}
	}
}

// -------------------------------------------------------
// LOAD IMAGES
// -------------------------------------------------------
void loadImages()
{
	loadMenuImages();
	loadObstacleImages();
	loadCharacterImages();
	loadDroneImages();

	for (int i = 0; i < 4; i++)
	{
		char path[100];
		sprintf_s(path, "Images//level1_bg%d.png", i + 1);
		images[i] = iLoadImage(path);
	}

	for (int i = 0; i < 4; i++)
	{
		char path[100];
		sprintf_s(path, "Images//level2_bg%d.png", i + 1);
		level2Images[i] = iLoadImage(path);
	}

	bonusBackgroundImage = iLoadImage("Images//BonusBackground.png");
	level3BackgroundImage = iLoadImage("Images//BonusBackground3.png");
	floorImage = iLoadImage("Images//floor.png");
}

// -------------------------------------------------------
// MAIN
// -------------------------------------------------------
int main()
{
	loadProfiles();   // read savegame.txt (player names + progress)

	iSetTimer(500, advanceMenuFrame);
	iSetTimer(83, advanceFrame);
	iSetTimer(83, updatePlayerAnimation);
	iSetTimer(16, updateFloor);
	iSetTimer(16, updateJump);
	iSetTimer(16, fixedUpdate);
	iSetTimer(12000, restartMenuMusicIfLooping);

	iInitialize(1280, 720, "Escape Protocol");
	loadImages();
	loadGameplayAudio();
	startMenuMusic();
	resetGame();
	iStart();
	return 0;
}