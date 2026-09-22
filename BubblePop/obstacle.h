#ifndef OBSTACLE_H
#define OBSTACLE_H

#include "iGraphics.h"
#include "character.h"
#include <cmath>
#include <cstdlib>

// -------------------------------------------------------
// OBSTACLE STRUCT
// -------------------------------------------------------
struct Obstacle
{
	float x, y;
	int width, height;
	int type;      // 0 to 3 (Level 1, Level 2 & Level 3)
	int health;
	bool active;
};

#define MAX_OBSTACLES 6

Obstacle obstacles[MAX_OBSTACLES];

int obstacleImagesLvl1[4];
int obstacleImagesLvl2[4]; // shared visually by Level 2 AND Level 3
int nextObstacleType = 0;

// -------------------------------------------------------
// ROCKET (Level 2 & Level 3)
// -------------------------------------------------------
struct Rocket
{
	float x, y;
	int width, height;
	float speed;
	int health;
	bool active;
};

Rocket rocket;
int rocketImage;
float lastRocketDistance = 500.0f;

// -------------------------------------------------------
// COLLECTIBLES & SHIELD SYSTEM
// -------------------------------------------------------
struct Collectible
{
	float x, y;
	int width, height;
	bool active;
};

#define MAX_REGULAR_COINS 8

Collectible coinItems[MAX_REGULAR_COINS];
Collectible fuelItem;
Collectible shieldItem;

int coinImage;
int fuelImage;
int shieldImage;

float lastShieldSpawnDistance = 700.0f;
float shieldActiveDistanceRemaining = 0.0f;

// -------------------------------------------------------
// MAGNET POWER-UP (Level 3 ONLY)
// Spawns every 100m. On pickup, auto-collects every coin & fuel item
// on screen for MAGNET_DURATION seconds (no need to touch them).
// -------------------------------------------------------
Collectible magnetItem;
int magnetImage;

float lastMagnetSpawnDistance = 400.0f;
bool isMagnetActive = false;
float magnetTimeRemaining = 0.0f;
const float MAGNET_DURATION = 5.0f; // seconds

// -------------------------------------------------------
// BONUS COIN STREAM SYSTEM (Level 2 & Level 3 bonus phase)
// -------------------------------------------------------
#define MAX_BONUS_COINS 36

struct BonusCoin {
	float x, y;
	bool active;
};

BonusCoin bonusCoins[MAX_BONUS_COINS];

void initBonusCoins() {
	for (int i = 0; i < MAX_BONUS_COINS; i++) {
		bonusCoins[i].x = 1280.0f + (i % 12) * 45.0f;

		int tier = i / 12;
		if (tier == 0)      bonusCoins[i].y = 200.0f;
		else if (tier == 1) bonusCoins[i].y = 320.0f;
		else                bonusCoins[i].y = 440.0f;

		bonusCoins[i].active = true;
	}
}

void updateBonusCoins(float speed, float charLeft, float charRight, float charBottom, float charTop, int& score) {
	for (int i = 0; i < MAX_BONUS_COINS; i++) {
		bonusCoins[i].x -= speed * 0.016f;

		if (bonusCoins[i].x < -50.0f) {
			bonusCoins[i].x = 1280.0f + (rand() % 200);
			bonusCoins[i].active = true;
		}

		if (bonusCoins[i].active &&
			charLeft < bonusCoins[i].x + 40 &&
			charRight > bonusCoins[i].x &&
			charBottom < bonusCoins[i].y + 40 &&
			charTop > bonusCoins[i].y)
		{
			score += 1;
			bonusCoins[i].active = false;
		}
	}
}

void drawBonusCoins() {
	for (int i = 0; i < MAX_BONUS_COINS; i++) {
		if (bonusCoins[i].active) {
			iShowImage((int)bonusCoins[i].x, (int)bonusCoins[i].y, 40, 40, coinImage);
		}
	}
}

// -------------------------------------------------------
// HITBOX INSETS (Level 1)
// -------------------------------------------------------
struct HitboxInset
{
	float left, right, top, bottom;
};

#define MAX_SUBBOXES 4

int obstacleBoxCount[4] = { 1, 1, 1, 1 };

HitboxInset obstacleInsets[4][MAX_SUBBOXES] =
{
	{ { 0.137f, 0.149f, 0.069f, 0.074f } },
	{ { 0.050f, 0.050f, 0.050f, 0.050f } },
	{ { 0.050f, 0.050f, 0.050f, 0.050f } },
	{ { 0.066f, 0.033f, 0.140f, 0.721f } }
};

// -------------------------------------------------------
// LEVEL 2 / LEVEL 3 HITBOX SHAPES (shared)
// -------------------------------------------------------
struct EllipseHitbox { float rx, ry; };

EllipseHitbox obstacleEllipse[4] =
{
	{ 0.23f, 0.35f },  // type0 - Obstacle11
	{ 0.24f, 0.37f },  // type1 - Obstacle12
	{ 0.22f, 0.39f },  // type2 - Obstacle13
	{ 0.24f, 0.23f },  // type3 - Obstacle14
};

int getRandomCollectibleY()
{
	int positions[] = { 150, 220, 300, 380, 450 };
	return positions[rand() % 5];
}

void applyObstacleType(Obstacle& obs, int currentLevel = 1)
{
	obs.active = true;

	// NOTE: sizes below already default to the "Level 2 style" dimensions
	// for any level that isn't Level 1 (i.e. Level 2 AND Level 3 share them).
	if (obs.type == 0)
	{
		obs.width = (currentLevel == 1) ? 280 : 350;
		obs.height = (currentLevel == 1) ? 144 : 180;
		obs.health = 999;
		int positions[] = { 124, 220, 320 };
		obs.y = (float)positions[rand() % 3];
	}
	else if (obs.type == 1)
	{
		obs.width = (currentLevel == 1) ? 256 : 320;
		obs.height = (currentLevel == 1) ? 144 : 180;
		// Destructible-by-bullets obstacle: Level 2 AND Level 3 both use it.
		obs.health = (currentLevel == 2 || currentLevel == 3) ? 5 : 999;
		int positions[] = { 124, 230, 380, 480 };
		obs.y = (float)positions[rand() % 4];
	}
	else if (obs.type == 2)
	{
		obs.width = (currentLevel == 1) ? 288 : 360;
		obs.height = (currentLevel == 1) ? 144 : 180;
		obs.health = 999;
		int positions[] = { 124, 220, 320, 400 };
		obs.y = (float)positions[rand() % 4];
	}
	else if (obs.type == 3)
	{
		obs.width = (currentLevel == 1) ? 224 : 280;
		obs.height = (currentLevel == 1) ? 160 : 200;
		obs.health = 999;
		int positions[] = { 124, 200, 280, 350 };
		obs.y = (float)positions[rand() % 4];
	}
}

void resetRocket()
{
	rocket.width = 120;
	rocket.height = 45;
	rocket.x = 1300;
	rocket.y = 200;
	rocket.speed = 550.0f;
	rocket.health = 4;
	rocket.active = false;
	lastRocketDistance = 500.0f;
}

void resetObstacles(int currentLevel = 1)
{
	nextObstacleType = 0;

	for (int i = 0; i < MAX_OBSTACLES; i++)
	{
		obstacles[i].x = 1280.0f + (i * 550.0f);
		obstacles[i].type = nextObstacleType;
		nextObstacleType = (nextObstacleType + 1) % 4;
		applyObstacleType(obstacles[i], currentLevel);
	}

	for (int i = 0; i < MAX_REGULAR_COINS; i++)
	{
		coinItems[i].width = 40;
		coinItems[i].height = 40;
		coinItems[i].x = 1280.0f + (i * 220.0f);
		coinItems[i].y = (float)getRandomCollectibleY();
		coinItems[i].active = true;
	}

	fuelItem.width = 50;
	fuelItem.height = 50;
	fuelItem.x = obstacles[1].x + 250 + (rand() % 200);
	fuelItem.y = (float)getRandomCollectibleY();
	fuelItem.active = true;

	// SHIELD ITEM RESET (Same dimensions as fuel item: 50x50)
	shieldItem.width = 50;
	shieldItem.height = 50;
	shieldItem.x = -200.0f;
	shieldItem.y = (float)getRandomCollectibleY();
	shieldItem.active = false;

	// Level 3 uses 400m, same 100m-interval spawn cadence as Level 2's 500m.
	if (currentLevel == 1)      lastShieldSpawnDistance = 700.0f;
	else if (currentLevel == 2) lastShieldSpawnDistance = 500.0f;
	else if (currentLevel == 3) lastShieldSpawnDistance = 400.0f;

	shieldActiveDistanceRemaining = 0.0f;
	isShieldActive = false;

	// MAGNET ITEM RESET (Level 3 only)
	magnetItem.width = 50;
	magnetItem.height = 50;
	magnetItem.x = -200.0f;
	magnetItem.y = (float)getRandomCollectibleY();
	magnetItem.active = false;

	lastMagnetSpawnDistance = 400.0f; // only ever used when currentLevel == 3
	isMagnetActive = false;
	magnetTimeRemaining = 0.0f;

	resetRocket();
}

// -------------------------------------------------------
// DRONE-PHASE FIELD HELPERS
// Used by the 300m Drone Gauntlet (see drone.h) to clear the
// obstacle field for its duration, then bring everything back
// (freshly placed off-screen) once the gauntlet ends. Kept here
// since they operate purely on obstacle.h's own data.
// -------------------------------------------------------
void clearFieldForDronePhase()
{
	for (int i = 0; i < MAX_OBSTACLES; i++) obstacles[i].active = false;
	for (int i = 0; i < MAX_REGULAR_COINS; i++) coinItems[i].active = false;
	fuelItem.active = false;
	shieldItem.active = false;
	magnetItem.active = false;
	rocket.active = false;
}

void respawnFieldAfterDronePhase(int currentLevel)
{
	for (int i = 0; i < MAX_OBSTACLES; i++)
	{
		obstacles[i].x = 1280.0f + (i * 550.0f);
		obstacles[i].type = nextObstacleType;
		nextObstacleType = (nextObstacleType + 1) % 4;
		applyObstacleType(obstacles[i], currentLevel);
	}

	for (int i = 0; i < MAX_REGULAR_COINS; i++)
	{
		coinItems[i].x = 1280.0f + (i * 220.0f);
		coinItems[i].y = (float)getRandomCollectibleY();
		coinItems[i].active = true;
	}

	fuelItem.x = obstacles[1].x + 250 + (rand() % 200);
	fuelItem.y = (float)getRandomCollectibleY();
	fuelItem.active = true;

	shieldItem.active = false;  // resumes on its own 100m cadence
	magnetItem.active = false;  // resumes on its own 100m cadence
	rocket.active = false;
}

// -------------------------------------------------------
// FUEL PICKUPS DURING THE DRONE ENCOUNTERS
// The normal obstacle field is switched off in the 300m Drone
// Gauntlet and the Final Fight, so the regular fuel spawner does
// not run there. This re-uses fuelItem: a fresh pack scrolls in
// every DRONE_FUEL_INTERVAL seconds (the first one after
// DRONE_FUEL_FIRST_DELAY) and gives the same +35 fuel as usual.
// -------------------------------------------------------
const float DRONE_FUEL_INTERVAL = 5.5f;      // seconds between fuel packs
const float DRONE_FUEL_FIRST_DELAY = 2.0f;   // first pack shows up quickly
float droneFuelTimer = 0.0f;

void resetDroneFuelPickup()
{
	droneFuelTimer = DRONE_FUEL_FIRST_DELAY;
	fuelItem.active = false;
	fuelItem.x = -200.0f;
}

void updateDroneFuelPickup(float speed, float charLeft, float charRight, float charBottom, float charTop,
	float& currentFuel, float maxFuel)
{
	droneFuelTimer -= 0.016f;
	if (droneFuelTimer <= 0.0f)
	{
		droneFuelTimer += DRONE_FUEL_INTERVAL;

		if (!fuelItem.active)   // an uncollected pack is simply left to scroll by
		{
			fuelItem.x = 1280.0f + (rand() % 100);
			fuelItem.y = (float)getRandomCollectibleY();
			fuelItem.active = true;
		}
	}

	if (!fuelItem.active) return;

	fuelItem.x -= speed * 0.016f;
	if (fuelItem.x < -60.0f)
	{
		fuelItem.active = false;
		return;
	}

	if (charLeft < fuelItem.x + fuelItem.width && charRight > fuelItem.x &&
		charBottom < fuelItem.y + fuelItem.height && charTop > fuelItem.y)
	{
		currentFuel += 35.0f;
		if (currentFuel > maxFuel) currentFuel = maxFuel;
		fuelItem.active = false;
	}
}

void drawDroneFuelPickup()
{
	if (fuelItem.active)
		iShowImage((int)fuelItem.x, (int)fuelItem.y, fuelItem.width, fuelItem.height, fuelImage);
}

bool checkCircleBoxCollision(float circleX, float circleY, float radius, float rectLeft, float rectRight, float rectBottom, float rectTop)
{
	float closestX = circleX;
	if (closestX < rectLeft) closestX = rectLeft;
	if (closestX > rectRight) closestX = rectRight;

	float closestY = circleY;
	if (closestY < rectBottom) closestY = rectBottom;
	if (closestY > rectTop) closestY = rectTop;

	float dx = circleX - closestX;
	float dy = circleY - closestY;

	return (dx * dx + dy * dy) <= (radius * radius);
}

bool checkEllipseBoxCollision(float ecx, float ecy, float rx, float ry,
	float rectLeft, float rectRight, float rectBottom, float rectTop)
{
	if (rx <= 0.0f) rx = 0.0001f;
	if (ry <= 0.0f) ry = 0.0001f;

	float sLeft = (rectLeft - ecx) / rx;
	float sRight = (rectRight - ecx) / rx;
	float sBottom = (rectBottom - ecy) / ry;
	float sTop = (rectTop - ecy) / ry;

	float closestX = 0.0f;
	if (closestX < sLeft)   closestX = sLeft;
	if (closestX > sRight)  closestX = sRight;

	float closestY = 0.0f;
	if (closestY < sBottom) closestY = sBottom;
	if (closestY > sTop)    closestY = sTop;

	return (closestX * closestX + closestY * closestY) <= 1.0f;
}

bool checkObstacleCollision(const Obstacle& obs, float charLeft, float charRight, float charBottom, float charTop, int currentLevel = 1)
{
	if (!obs.active) return false;

	// Level 2 AND Level 3 use the ellipse-shaped hitbox.
	if (currentLevel == 2 || currentLevel == 3)
	{
		float centerX = obs.x + obs.width * 0.5f;
		float centerY = obs.y + obs.height * 0.5f;

		const EllipseHitbox& e = obstacleEllipse[obs.type % 4];
		float rx = obs.width  * e.rx;
		float ry = obs.height * e.ry;

		return checkEllipseBoxCollision(centerX, centerY, rx, ry, charLeft, charRight, charBottom, charTop);
	}

	// Level 1 uses the inset box hitbox.
	float ox = obs.x;
	float oy = obs.y;
	float ow = (float)obs.width;
	float oh = (float)obs.height;

	int count = obstacleBoxCount[obs.type];

	for (int b = 0; b < count; b++)
	{
		const HitboxInset& in = obstacleInsets[obs.type][b];

		float obsLeft = ox + ow * in.left;
		float obsRight = ox + ow * (1.0f - in.right);
		float obsBottom = oy + oh * in.bottom;
		float obsTop = oy + oh * in.top;

		if (charLeft < obsRight && charRight > obsLeft && charBottom < obsTop && charTop > obsBottom)
		{
			return true;
		}
	}

	return false;
}

void updateRocket(float remainingDistance, float charLeft, float charRight, float charBottom, float charTop, bool& isLose, int currentLevel)
{
	// Rocket hazard exists in Level 2 AND Level 3.
	if (currentLevel != 2 && currentLevel != 3) return;

	if (!rocket.active && (lastRocketDistance - remainingDistance >= 50.0f))
	{
		rocket.x = 1280.0f;
		rocket.y = (float)(130 + (rand() % 400));
		rocket.health = 4;
		rocket.active = true;
		lastRocketDistance = remainingDistance;
	}

	if (rocket.active)
	{
		rocket.x -= rocket.speed * 0.016f;

		if (rocket.x < -150) rocket.active = false;

		if (charLeft < rocket.x + rocket.width &&
			charRight > rocket.x &&
			charBottom < rocket.y + rocket.height &&
			charTop > rocket.y)
		{
			if (!isShieldActive)
			{
				isLose = true;
			}
		}
	}
}

void updateObstacles(
	float currentSpeed, float charLeft, float charRight, float charBottom, float charTop,
	int& score, float& currentFuel, float maxFuel, bool& isLose,
	int currentLevel = 1, float remainingDistance = 500.0f)
{
	// -------------------------------------------------------
	// SHIELD DURATION & SPAWN SYSTEM (Every 100m, lasts 20m)
	// -------------------------------------------------------
	float metersTraveled = 5.33f * 0.016f;

	if (isShieldActive)
	{
		shieldActiveDistanceRemaining -= metersTraveled;
		if (shieldActiveDistanceRemaining <= 0.0f)
		{
			shieldActiveDistanceRemaining = 0.0f;
			isShieldActive = false; // Deactivate shield after 20 meters
		}
	}

	// Level 1 has no shield power-up (Level 2 & Level 3 only)
	if (currentLevel != 1 && lastShieldSpawnDistance - remainingDistance >= 100.0f)
	{
		shieldItem.x = 1280.0f + (rand() % 150);
		shieldItem.y = (float)getRandomCollectibleY();
		shieldItem.active = true;
		lastShieldSpawnDistance = remainingDistance;
	}

	// -------------------------------------------------------
	// MAGNET POWER-UP: SPAWN (every 100m), PICKUP & COUNTDOWN
	// Level 3 ONLY.
	// -------------------------------------------------------
	if (currentLevel == 3)
	{
		if (lastMagnetSpawnDistance - remainingDistance >= 100.0f)
		{
			magnetItem.x = 1280.0f + (rand() % 150);
			magnetItem.y = (float)getRandomCollectibleY();
			magnetItem.active = true;
			lastMagnetSpawnDistance = remainingDistance;
		}

		magnetItem.x -= currentSpeed * 0.016f;

		if (magnetItem.active &&
			charLeft < magnetItem.x + magnetItem.width &&
			charRight > magnetItem.x &&
			charBottom < magnetItem.y + magnetItem.height &&
			charTop > magnetItem.y)
		{
			isMagnetActive = true;
			magnetTimeRemaining = MAGNET_DURATION;
			magnetItem.active = false;
		}

		if (isMagnetActive)
		{
			magnetTimeRemaining -= 0.016f; // fixed 16ms tick
			if (magnetTimeRemaining <= 0.0f)
			{
				magnetTimeRemaining = 0.0f;
				isMagnetActive = false;
			}
		}
	}

	// OBSTACLES MOVEMENT AND COLLISION
	for (int i = 0; i < MAX_OBSTACLES; i++)
	{
		obstacles[i].x -= currentSpeed * 0.016f;

		if (obstacles[i].x < -350)
		{
			int gap = 420 + (rand() % 150);
			int prevIndex = (i == 0) ? MAX_OBSTACLES - 1 : i - 1;

			obstacles[i].x = obstacles[prevIndex].x + gap;
			if (obstacles[i].x < 1280) obstacles[i].x = 1280.0f + gap;

			obstacles[i].type = nextObstacleType;
			nextObstacleType = (nextObstacleType + 1) % 4;
			applyObstacleType(obstacles[i], currentLevel);
		}

		if (obstacles[i].active && checkObstacleCollision(obstacles[i], charLeft, charRight, charBottom, charTop, currentLevel))
		{
			if (!isShieldActive)
			{
				// Non-lethal "drains half the fuel bar" obstacle: shared by
				// Level 2 AND Level 3.
				if ((currentLevel == 2 || currentLevel == 3) && obstacles[i].type == 3)
				{
					// Obstacle14.png: non-lethal hit - drains half the fuel bar instead of ending the run.
					currentFuel -= maxFuel * 0.5f;
					if (currentFuel < 0.0f) currentFuel = 0.0f;

					obstacles[i].active = false; // consume this obstacle so it can't hit repeatedly while overlapping
				}
				else
				{
					isLose = true;
				}
			}
		}
	}

	// COINS
	// Magnet pull target: center of the character's hitbox.
	float charCenterX = (charLeft + charRight) * 0.5f;
	float charCenterY = (charBottom + charTop) * 0.5f;
	const float MAGNET_PULL_SPEED = 1100.0f; // px/sec -- how fast coins/fuel fly toward the player
	const float MAGNET_COLLECT_RADIUS = 35.0f; // how close counts as "collected"

	for (int i = 0; i < MAX_REGULAR_COINS; i++)
	{
		if (isMagnetActive && coinItems[i].active)
		{
			// Visibly fly toward the character instead of just scrolling left.
			float targetX = charCenterX - coinItems[i].width * 0.5f;
			float targetY = charCenterY - coinItems[i].height * 0.5f;

			float dx = targetX - coinItems[i].x;
			float dy = targetY - coinItems[i].y;
			float dist = sqrtf(dx * dx + dy * dy);

			if (dist > 1.0f)
			{
				coinItems[i].x += (dx / dist) * MAGNET_PULL_SPEED * 0.016f;
				coinItems[i].y += (dy / dist) * MAGNET_PULL_SPEED * 0.016f;
			}

			if (dist < MAGNET_COLLECT_RADIUS)
			{
				score += 1;
				coinItems[i].active = false;
			}
		}
		else
		{
			coinItems[i].x -= currentSpeed * 0.016f;
			if (coinItems[i].x < -50)
			{
				coinItems[i].x = 1280.0f + (rand() % 300);
				coinItems[i].y = (float)getRandomCollectibleY();
				coinItems[i].active = true;
			}

			bool touchingCoin = (charLeft < coinItems[i].x + coinItems[i].width && charRight > coinItems[i].x &&
				charBottom < coinItems[i].y + coinItems[i].height && charTop > coinItems[i].y);

			if (coinItems[i].active && touchingCoin)
			{
				score += 1;
				coinItems[i].active = false;
			}
		}
	}

	// FUEL ITEM
	if (isMagnetActive && fuelItem.active)
	{
		// Visibly fly toward the character instead of just scrolling left.
		float targetX = charCenterX - fuelItem.width * 0.5f;
		float targetY = charCenterY - fuelItem.height * 0.5f;

		float dx = targetX - fuelItem.x;
		float dy = targetY - fuelItem.y;
		float dist = sqrtf(dx * dx + dy * dy);

		if (dist > 1.0f)
		{
			fuelItem.x += (dx / dist) * MAGNET_PULL_SPEED * 0.016f;
			fuelItem.y += (dy / dist) * MAGNET_PULL_SPEED * 0.016f;
		}

		if (dist < MAGNET_COLLECT_RADIUS)
		{
			currentFuel += 35.0f;
			if (currentFuel > maxFuel) currentFuel = maxFuel;
			fuelItem.active = false;
		}
	}
	else
	{
		fuelItem.x -= currentSpeed * 0.016f;
		if (fuelItem.x < -50)
		{
			fuelItem.x = obstacles[2].x + 250 + (rand() % 250);
			fuelItem.y = (float)getRandomCollectibleY();
			fuelItem.active = true;
		}

		bool touchingFuel = (charLeft < fuelItem.x + fuelItem.width && charRight > fuelItem.x &&
			charBottom < fuelItem.y + fuelItem.height && charTop > fuelItem.y);

		if (fuelItem.active && touchingFuel)
		{
			currentFuel += 35.0f;
			if (currentFuel > maxFuel) currentFuel = maxFuel;
			fuelItem.active = false;
		}
	}

	// SHIELD ITEM PICKUP MOVEMENT & COLLISION
	shieldItem.x -= currentSpeed * 0.016f;

	if (shieldItem.active &&
		charLeft < shieldItem.x + shieldItem.width &&
		charRight > shieldItem.x &&
		charBottom < shieldItem.y + shieldItem.height &&
		charTop > shieldItem.y)
	{
		isShieldActive = true;
		shieldActiveDistanceRemaining = 20.0f; // Active for 20 meters
		shieldItem.active = false;
	}

	updateRocket(remainingDistance, charLeft, charRight, charBottom, charTop, isLose, currentLevel);
}

void drawObstacles(int currentLevel = 1)
{
	for (int i = 0; i < MAX_OBSTACLES; i++)
	{
		if (!obstacles[i].active) continue;

		// Level 2 and Level 3 share the same obstacle artwork.
		int imgToDraw;
		if (currentLevel == 2 || currentLevel == 3) imgToDraw = obstacleImagesLvl2[obstacles[i].type % 4];
		else                                         imgToDraw = obstacleImagesLvl1[obstacles[i].type % 4];

		iShowImage((int)obstacles[i].x, (int)obstacles[i].y, obstacles[i].width, obstacles[i].height, imgToDraw);
	}

	for (int i = 0; i < MAX_REGULAR_COINS; i++)
	{
		if (coinItems[i].active)
		{
			iShowImage((int)coinItems[i].x, (int)coinItems[i].y, coinItems[i].width, coinItems[i].height, coinImage);
		}
	}

	if (fuelItem.active) iShowImage((int)fuelItem.x, (int)fuelItem.y, fuelItem.width, fuelItem.height, fuelImage);

	// DRAW SHIELD PNG ITEM
	if (shieldItem.active)
	{
		iShowImage((int)shieldItem.x, (int)shieldItem.y, shieldItem.width, shieldItem.height, shieldImage);
	}

	// DRAW MAGNET PNG ITEM (Level 3 only -- inactive/off-screen on other levels)
	if (magnetItem.active)
	{
		iShowImage((int)magnetItem.x, (int)magnetItem.y, magnetItem.width, magnetItem.height, magnetImage);
	}

	// Rocket hazard rendering: Level 2 AND Level 3.
	if ((currentLevel == 2 || currentLevel == 3) && rocket.active)
	{
		iShowImage((int)rocket.x, (int)rocket.y, rocket.width, rocket.height, rocketImage);
	}
}

// -------------------------------------------------------
// DEBUG HITBOX OVERLAY
// -------------------------------------------------------
void drawEllipseOutline(float cx, float cy, float rx, float ry, int segments = 24)
{
	float prevX = cx + rx, prevY = cy;
	for (int i = 1; i <= segments; i++)
	{
		float theta = (2.0f * 3.14159265f * i) / segments;
		float x = cx + rx * cosf(theta);
		float y = cy + ry * sinf(theta);
		iLine((int)prevX, (int)prevY, (int)x, (int)y);
		prevX = x; prevY = y;
	}
}

void drawObstacleHitboxes(int currentLevel = 1)
{
	iSetColor(255, 0, 0);

	for (int i = 0; i < MAX_OBSTACLES; i++)
	{
		Obstacle& o = obstacles[i];
		if (!o.active) continue;

		if (currentLevel == 2 || currentLevel == 3)
		{
			float cx = o.x + o.width * 0.5f;
			float cy = o.y + o.height * 0.5f;
			const EllipseHitbox& e = obstacleEllipse[o.type % 4];
			drawEllipseOutline(cx, cy, o.width * e.rx, o.height * e.ry);
		}
		else
		{
			int count = obstacleBoxCount[o.type];
			for (int b = 0; b < count; b++)
			{
				const HitboxInset& in = obstacleInsets[o.type][b];
				float obsLeft = o.x + o.width * in.left;
				float obsRight = o.x + o.width * (1.0f - in.right);
				float obsBottom = o.y + o.height * in.bottom;
				float obsTop = o.y + o.height * (1.0f - in.top);
				iRectangle((int)obsLeft, (int)obsBottom, (int)(obsRight - obsLeft), (int)(obsTop - obsBottom));
			}
		}
	}

	if (rocket.active) iRectangle((int)rocket.x, (int)rocket.y, rocket.width, rocket.height);
}

// -------------------------------------------------------
// SHIELD VISUAL (destructible obstacle glow ring)
// -------------------------------------------------------
void drawObstacleShields(int currentLevel = 1)
{
	// Shown for Level 2 AND Level 3.
	if (currentLevel != 2 && currentLevel != 3) return;

	for (int i = 0; i < MAX_OBSTACLES; i++)
	{
		Obstacle& o = obstacles[i];
		if (!o.active) continue;
		if (o.health >= 999) continue;

		float cx = o.x + o.width * 0.5f;
		float cy = o.y + o.height * 0.5f;
		const EllipseHitbox& e = obstacleEllipse[o.type % 4];
		float rx = o.width * e.rx;
		float ry = o.height * e.ry;

		iSetColor(120, 200, 255);
		drawEllipseOutline(cx, cy, rx, ry, 32);
		drawEllipseOutline(cx, cy, rx - 3.0f, ry - 3.0f, 32);

		iSetColor(200, 235, 255);
		drawEllipseOutline(cx, cy, rx + 2.0f, ry + 2.0f, 32);
	}
}

void loadObstacleImages()
{
	for (int i = 0; i < 4; i++)
	{
		char path[100];
		sprintf_s(path, "Images//obstacle %d.png", i + 1);
		obstacleImagesLvl1[i] = iLoadImage(path);
	}

	// Shared by Level 2 AND Level 3
	for (int i = 0; i < 4; i++)
	{
		char path[100];
		sprintf_s(path, "Images//Obstacle%d.png", 11 + i);
		obstacleImagesLvl2[i] = iLoadImage(path);
	}

	coinImage = iLoadImage("Images//coin.png");
	fuelImage = iLoadImage("Images//fuel.png");
	shieldImage = iLoadImage("Images//shield.png"); // Loads Images/shield.png
	rocketImage = iLoadImage("Images//Rocket.png");
	magnetImage = iLoadImage("Images//Magnet.png"); // Level 3 magnet power-up icon
}

#endif // OBSTACLE_H