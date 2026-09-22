#ifndef CHARACTER_H
#define CHARACTER_H

#include "iGraphics.h"

// -------------------------------------------------------
// PLAYER STATE
// -------------------------------------------------------
float characterX = 200.0f;
float characterY = 15.0f;
bool  isJumping = false;   // true while space/LMB is held
bool  isShieldActive = false; // TRUE when invincibility shield is active

// -------------------------------------------------------
// PLAYER HITBOX INSETS
// -------------------------------------------------------
struct PlayerHitboxInset { float left, right, top, bottom; };

static const PlayerHitboxInset playerInsetRun = { 0.25f, 0.35f, 0.08f, 0.15f };
static const PlayerHitboxInset playerInsetGoUp = { 0.33f, 0.38f, 0.06f, 0.28f };
static const PlayerHitboxInset playerInsetGoDown = { 0.36f, 0.38f, 0.06f, 0.40f };

static const int PLAYER_W = 180;
static const int PLAYER_H = 200;
static const float GROUND_Y = 15.0f;

// -------------------------------------------------------
// ANIMATION FRAMES
// -------------------------------------------------------
int playerFrames[4];          // running frames (player 2–5)
int characterGoUpImage;       // goup.png   - ascending
int characterGoDownImage;     // godown.png - falling / descending
int currentPlayerFrame = 0;

void updatePlayerAnimation()
{
	currentPlayerFrame = (currentPlayerFrame + 1) % 4;
}

bool isAirborne()
{
	return characterY > GROUND_Y + 0.01f;
}

void resetCharacter()
{
	characterY = GROUND_Y;
	isJumping = false;
	isShieldActive = false;
	currentPlayerFrame = 0;
}

void updateCharacter(bool inMenu, bool isWin, bool isLose)
{
	if (inMenu || isWin || isLose) return;

	if (isJumping)
	{
		characterY += 500.0f * 0.016f;
		if (characterY > 520) characterY = 520;
	}
	else
	{
		characterY -= 500.0f * 0.016f;
		if (characterY < GROUND_Y) characterY = GROUND_Y;
	}
}

// -------------------------------------------------------
// PLAYER CIRCLE HITBOX
// -------------------------------------------------------
void getCharacterCircle(float& cx, float& cy, float& radius)
{
	cx = characterX + (PLAYER_W * 0.45f);
	cy = characterY + (PLAYER_H * 0.45f);
	radius = (PLAYER_W * 0.28f);
}

// -------------------------------------------------------
// PLAYER AABB (RECTANGLE)
// -------------------------------------------------------
void getCharacterAABB(float& left, float& right, float& bottom, float& top)
{
	PlayerHitboxInset in;

	if (isAirborne())
	{
		if (isJumping)
			in = playerInsetGoUp;
		else
			in = playerInsetGoDown;
	}
	else
	{
		in = playerInsetRun;
	}

	left = characterX + PLAYER_W * in.left;
	right = characterX + PLAYER_W * (1.0f - in.right);
	bottom = characterY + PLAYER_H * in.bottom;
	top = characterY + PLAYER_H * (1.0f - in.top);
}

// -------------------------------------------------------
// DRAW GUN ATTACHED TO PLAYER HAND
// -------------------------------------------------------
void drawGun()
{
	// Anchor gun position relative to player's front hand
	int gx = (int)characterX + 100;
	int gy = (int)characterY + 140;

	// 1. Grip / Handle
	iSetColor(50, 50, 60);
	iFilledRectangle(gx + 2, gy - 12, 8, 16);

	// 2. Main Gun Receiver
	iSetColor(70, 80, 95);
	iFilledRectangle(gx, gy, 28, 12);

	// 3. Gun Barrel
	iSetColor(140, 150, 165);
	iFilledRectangle(gx + 28, gy + 3, 16, 6);

	// 4. Muzzle Tip
	iSetColor(0, 210, 255);
	iFilledRectangle(gx + 44, gy + 3, 4, 6);

	// 5. Laser Sight / Scope Line
	iSetColor(0, 230, 255);
	iFilledRectangle(gx + 8, gy + 12, 18, 3);

	// 6. Trigger Guard
	iSetColor(120, 140, 160);
	iRectangle(gx + 8, gy - 6, 8, 6);
}

void drawCharacter(bool withGun = true)
{
	if (isAirborne())
	{
		if (isJumping)
			iShowImage((int)characterX, (int)characterY, PLAYER_W, PLAYER_H, characterGoUpImage);
		else
			iShowImage((int)characterX, (int)characterY, PLAYER_W, PLAYER_H, characterGoDownImage);
	}
	else
	{
		iShowImage((int)characterX, (int)characterY, PLAYER_W, PLAYER_H, playerFrames[currentPlayerFrame]);
	}

	// Draw weapon directly in front of the player (Level 2 & 3 only)
	if (withGun) drawGun();

	// -------------------------------------------------------
	// TRANSPARENT BLUE SHIELD CIRCLE AROUND PLAYER
	// -------------------------------------------------------
	if (isShieldActive)
	{
		// Adjusted offsets to perfectly center on the character model
		float cx = characterX + (PLAYER_W * 0.50f); // Centered horizontally
		float cy = characterY + (PLAYER_H * 0.50f); // Centered vertically
		int r = (int)(PLAYER_W * 0.58f);            // Slightly enlarged radius to cover full body

		// Layered semi-transparent concentric rings for cyan/blue shield effect
		iSetColor(100, 200, 255);
		iCircle((int)cx, (int)cy, r);
		iCircle((int)cx, (int)cy, r - 2);

		iSetColor(0, 160, 255);
		iCircle((int)cx, (int)cy, r + 2);
		iCircle((int)cx, (int)cy, r - 4);

		iSetColor(180, 240, 255);
		iCircle((int)cx, (int)cy, r + 4);
	}
}

void drawCharacterHitbox()
{
	float cx, cy, r;
	getCharacterCircle(cx, cy, r);
	iSetColor(0, 255, 0);
	iCircle((int)cx, (int)cy, (int)r);

	float cl, cr, cb, ct;
	getCharacterAABB(cl, cr, cb, ct);
	iSetColor(255, 0, 0);
	iRectangle((int)cl, (int)cb, (int)(cr - cl), (int)(ct - cb));
}

void loadCharacterImages()
{
	playerFrames[0] = iLoadImage("Images//player 2.png");
	playerFrames[1] = iLoadImage("Images//player 3.png");
	playerFrames[2] = iLoadImage("Images//player 4.png");
	playerFrames[3] = iLoadImage("Images//player 5.png");

	characterGoUpImage = iLoadImage("Images//goup.png");
	characterGoDownImage = iLoadImage("Images//godown.png");
}

#endif // CHARACTER_H