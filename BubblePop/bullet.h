#ifndef BULLET_H
#define BULLET_H

#include "iGraphics.h"
#include "obstacle.h"
#include "drone.h"
#include <cmath>

struct Bullet
{
	float x, y;
	float speed;
	int width, height;
	bool active;
};

#define MAX_BULLETS 20

Bullet bullets[MAX_BULLETS];

bool isShooting = false;
int bulletShootTimer = 0;

// BULLET AMMO SYSTEM (Level 2 & Level 3 only -- Level 1 has no gun)
//   Normal gameplay : each pickup gives +10 ammo.  Capacity: Level 2 = 20, Level 3 = 30.
//   Final Fight     : each pickup gives +30 ammo.  Capacity: 50.
const int LEVEL2_MAX_AMMO = 20;
const int LEVEL3_MAX_AMMO = 30;
const int FINAL_MAX_AMMO = 50;
const int NORMAL_AMMO_PICKUP = 10;
const int FINAL_AMMO_PICKUP = 30;

int currentAmmo = LEVEL2_MAX_AMMO;
int maxAmmo = LEVEL2_MAX_AMMO;

int getMaxAmmoFor(int level, bool inFinalFight)
{
	if (inFinalFight) return FINAL_MAX_AMMO;
	return (level == 3) ? LEVEL3_MAX_AMMO : LEVEL2_MAX_AMMO;
}

// BULLET COLLECTIBLE (Without External Image)
struct BulletItem {
	float x, y;
	int width, height;
	bool active;
};

BulletItem bulletItem;
float lastBulletReloadDistance = 500.0f;

void resetBullets(int level = 2)
{
	for (int i = 0; i < MAX_BULLETS; i++)
	{
		bullets[i].x = -100;
		bullets[i].y = -100;
		bullets[i].width = 24;
		bullets[i].height = 8;
		bullets[i].speed = 900.0f;
		bullets[i].active = false;
	}

	isShooting = false;
	bulletShootTimer = 0;
	maxAmmo = getMaxAmmoFor(level, false);
	currentAmmo = maxAmmo;          // a level starts with a full magazine

	bulletItem.width = 35;
	bulletItem.height = 35;
	bulletItem.x = 1600.0f;
	bulletItem.y = 250.0f;
	bulletItem.active = true;
	lastBulletReloadDistance = 500.0f;
}

void spawnBullet(float startX, float startY)
{
	if (currentAmmo <= 0) return; // Ammo limit check

	for (int i = 0; i < MAX_BULLETS; i++)
	{
		if (!bullets[i].active)
		{
			bullets[i].x = startX;
			bullets[i].y = startY;
			bullets[i].active = true;
			currentAmmo--; // Decrease ammo on fire
			break;
		}
	}
}

void updateBullets(bool inMenu, bool isWin, bool isLose, float playerX, float playerY, int currentLevel, float remainingDistance = 500.0f)
{
	if (inMenu || isWin || isLose) return;

	// Capacity depends on the level, and grows to 50 once the Final Fight starts.
	maxAmmo = getMaxAmmoFor(currentLevel, isFinalFight);
	if (currentAmmo > maxAmmo) currentAmmo = maxAmmo;

	// Gun mechanic exists in Level 2 AND Level 3.
	bool hasGun = (currentLevel == 2 || currentLevel == 3);

	// While the "FINAL ROUND" banner or the Drone Gauntlet (Phase 1) is up,
	// freeze auto-fire. Phase 1 is a pure dodge encounter (no offense), and
	// the banner shouldn't let the player waste ammo before enemies arrive.
	if (hasGun && isShooting && currentAmmo > 0 && !showFinalRoundText && !isDronePhase1)
	{
		bulletShootTimer++;
		if (bulletShootTimer % 8 == 0)
		{
			// Calibrated start position to emerge directly from the higher gun barrel
			spawnBullet(playerX + 148, playerY + 145);
		}
	}

	for (int i = 0; i < MAX_BULLETS; i++)
	{
		if (!bullets[i].active) continue;

		bullets[i].x += bullets[i].speed * 0.016f;

		if (bullets[i].x > 1300)
		{
			bullets[i].active = false;
			continue;
		}

		// ROCKET COLLISION (regular Level 2/3 Rocket hazard)
		if (rocket.active &&
			bullets[i].x < rocket.x + rocket.width &&
			bullets[i].x + bullets[i].width > rocket.x &&
			bullets[i].y < rocket.y + rocket.height &&
			bullets[i].y + bullets[i].height > rocket.y)
		{
			bullets[i].active = false;
			rocket.health--;

			if (rocket.health <= 0)
			{
				rocket.active = false;
			}
			continue;
		}

		// DRONE ROCKET COLLISION (Level 3 only: 100m Intrusion + Final Drone Fight)
		// 2 bullet hits destroys a droneRocket before it can one-shot the player.
		if (currentLevel == 3)
		{
			bool hitDroneRocket = false;

			for (int r = 0; r < MAX_DRONE_ROCKETS; r++)
			{
				if (!droneRockets[r].active) continue;

				if (bullets[i].x < droneRockets[r].x + droneRockets[r].width &&
					bullets[i].x + bullets[i].width > droneRockets[r].x &&
					bullets[i].y < droneRockets[r].y + droneRockets[r].height &&
					bullets[i].y + bullets[i].height > droneRockets[r].y)
				{
					bullets[i].active = false;
					droneRockets[r].health--;

					if (droneRockets[r].health <= 0)
					{
						droneRockets[r].active = false;
					}

					hitDroneRocket = true;
					break;
				}
			}

			if (hitDroneRocket) continue;

			// FINAL FIGHT DRONES: each takes FINAL_DRONE_HEALTH (30) bullet hits
			if (isFinalFight && finalFightActive)
			{
				bool hitDrone = false;
				float dl, dr, db, dt;

				if (drone1s[0].active)
				{
					getDrone1Hitbox(drone1s[0], dl, dr, db, dt);
					if (bullets[i].x < dr && bullets[i].x + bullets[i].width > dl &&
						bullets[i].y < dt && bullets[i].y + bullets[i].height > db)
					{
						bullets[i].active = false;
						drone1s[0].health--;
						if (drone1s[0].health <= 0)
						{
							drone1s[0].health = 0;
							drone1s[0].active = false;
						}
						hitDrone = true;
					}
				}

				if (!hitDrone && drone2.active)
				{
					getDrone2Hitbox(drone2, dl, dr, db, dt);
					if (bullets[i].x < dr && bullets[i].x + bullets[i].width > dl &&
						bullets[i].y < dt && bullets[i].y + bullets[i].height > db)
					{
						bullets[i].active = false;
						drone2.health--;
						if (drone2.health <= 0)
						{
							drone2.health = 0;
							drone2.active = false;
						}
						hitDrone = true;
					}
				}

				if (hitDrone) continue;
			}
		}

		// DESTROYABLE OBSTACLE COLLISION
		for (int obsIdx = 0; obsIdx < MAX_OBSTACLES; obsIdx++)
		{
			Obstacle& obs = obstacles[obsIdx];
			if (!obs.active) continue;

			if (obs.type == 1 && obs.health < 999)
			{
				float cx = obs.x + obs.width * 0.5f;
				float cy = obs.y + obs.height * 0.5f;
				const EllipseHitbox& e = obstacleEllipse[obs.type % 4];
				float rx = obs.width  * e.rx;
				float ry = obs.height * e.ry;

				float bLeft = bullets[i].x;
				float bRight = bullets[i].x + bullets[i].width;
				float bBottom = bullets[i].y;
				float bTop = bullets[i].y + bullets[i].height;

				if (checkEllipseBoxCollision(cx, cy, rx, ry, bLeft, bRight, bBottom, bTop))
				{
					bullets[i].active = false;
					obs.health--;

					if (obs.health <= 0)
					{
						obs.active = false;
					}
					break;
				}
			}
		}
	}

	// AMMO PICKUP SPAWN & COLLISION LOGIC (Level 2 & Level 3)
	// Normal run: a new pickup every 100m.
	// Final Fight (no distance left): a new pickup every FINAL_AMMO_INTERVAL (20) seconds.
	// Suppressed during the Drone Gauntlet (Phase 1) since the field is
	// otherwise fully cleared and shooting is disabled there anyway.
	if (hasGun && !isDronePhase1)
	{
		float speed = 220.0f;
		bulletItem.x -= speed * 0.016f;

		bool spawnPickup = false;

		if (isFinalFight)
		{
			if (!finalFightActive)
			{
				bulletItem.active = false;   // "FINAL ROUND" banner still up: clear any leftover pickup
			}
			else
			{
				finalAmmoTimer -= 0.016f;
				if (finalAmmoTimer <= 0.0f)
				{
					finalAmmoTimer += FINAL_AMMO_INTERVAL;
					spawnPickup = true;
				}
			}
		}
		else if (lastBulletReloadDistance - remainingDistance >= 100.0f)
		{
			lastBulletReloadDistance = remainingDistance;
			spawnPickup = true;
		}

		if (spawnPickup)
		{
			bulletItem.x = 1280.0f + (rand() % 150);
			bulletItem.y = (float)(150 + (rand() % 300));
			bulletItem.active = true;
		}

		float charLeft = playerX + 15;
		float charRight = playerX + 90;
		float charBottom = playerY;
		float charTop = playerY + 120;

		if (bulletItem.active &&
			charLeft < bulletItem.x + bulletItem.width &&
			charRight > bulletItem.x &&
			charBottom < bulletItem.y + bulletItem.height &&
			charTop > bulletItem.y)
		{
			// Pickup adds ammo (not a full reload): +10 normally, +30 in the Final Fight
			currentAmmo += isFinalFight ? FINAL_AMMO_PICKUP : NORMAL_AMMO_PICKUP;
			if (currentAmmo > maxAmmo) currentAmmo = maxAmmo;
			bulletItem.active = false;
		}
	}
}

// DRAW BULLETS
void drawBullets()
{
	for (int i = 0; i < MAX_BULLETS; i++)
	{
		if (!bullets[i].active) continue;

		int bx = (int)bullets[i].x;
		int by = (int)bullets[i].y;
		int bw = bullets[i].width;
		int bh = bullets[i].height;

		iSetColor(180, 130, 20);
		iFilledRectangle(bx, by, 4, bh);

		iSetColor(255, 200, 0);
		iFilledRectangle(bx + 4, by, 12, bh);

		iSetColor(255, 240, 150);
		iFilledRectangle(bx + 4, by + bh - 2, 12, 2);

		double tipX[3] = { (double)(bx + 16), (double)(bx + 16), (double)(bx + bw) };
		double tipY[3] = { (double)by, (double)(by + bh), (double)(by + (bh / 2.0)) };

		iSetColor(255, 215, 0);
		iFilledPolygon(tipX, tipY, 3);
	}
}

// DRAW CUSTOM BULLET PICKUP ITEM
void drawBulletPickup()
{
	if (!bulletItem.active) return;

	int ix = (int)bulletItem.x;
	int iy = (int)bulletItem.y;
	int cx = ix + 17;
	int cy = iy + 17;

	// White/Water Blue Shield Aura
	iSetColor(180, 230, 255);
	iFilledCircle(cx, cy, 22);

	iSetColor(255, 255, 255);
	iCircle(cx, cy, 22);
	iCircle(cx, cy, 20);

	// Drawn Golden Bullet Inside
	int bx = ix + 6;
	int by = iy + 13;

	iSetColor(180, 130, 20);
	iFilledRectangle(bx, by, 3, 10);

	iSetColor(255, 215, 0);
	iFilledRectangle(bx + 3, by, 12, 10);

	double tipX[3] = { (double)(bx + 15), (double)(bx + 15), (double)(bx + 23) };
	double tipY[3] = { (double)by, (double)(by + 10), (double)(by + 5) };

	iSetColor(255, 230, 80);
	iFilledPolygon(tipX, tipY, 3);
}

#endif // BULLET_H