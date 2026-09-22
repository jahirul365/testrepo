#ifndef DRONE_H
#define DRONE_H

#include "iGraphics.h"
#include "character.h"
#include "obstacle.h"
#include <cmath>
#include <cstdlib>

// Defined later in iMain.cpp; declared here so the drone banners can
// center their text the same way the existing menus/banners do.
int getBitmapTextWidth(const char* text, void* font);

// =========================================================
// SPRITE CONTENT BOUNDS
// The drone / bullet / rocket PNGs are 500x500 with lots of empty
// (transparent) padding, so drawing them at their hitbox size made
// the visible art tiny. These fractions (measured from the alpha
// channel, bottom-up) say where the real art sits inside the PNG so
// we can size things by what the player actually SEES.
// =========================================================
struct SpriteBounds { float x0, x1, y0, y1; };

static const SpriteBounds DRONE1_BOUNDS = { 0.162f, 0.832f, 0.192f, 0.770f };
static const SpriteBounds DRONE2_BOUNDS = { 0.098f, 0.832f, 0.192f, 0.770f };
static const SpriteBounds BULLET_BOUNDS = { 0.224f, 0.728f, 0.416f, 0.580f };  // droneBulletLeft.png (tip points LEFT)
static const SpriteBounds ROCKET_BOUNDS = { 0.128f, 0.930f, 0.318f, 0.686f };

// Draws an image so its VISIBLE art exactly fills the box (bx, by, bw, bh).
void drawSpriteFitted(int img, float bx, float by, float bw, float bh, const SpriteBounds& sb)
{
	float W = bw / (sb.x1 - sb.x0);
	float H = bh / (sb.y1 - sb.y0);
	iShowImage((int)(bx - sb.x0 * W), (int)(by - sb.y0 * H), (int)W, (int)H, img);
}

// =========================================================
// TUNING -- all the sizes / speeds you may want to tweak
// (drone sizes = PNG square size in px; bullet / rocket sizes =
//  size of the VISIBLE art in px, which is also their hitbox)
// =========================================================
// Drone bullets (Drone1). Same look in the Gauntlet and the Final Fight.
const int   DRONE_BULLET_W = 72;
const int   DRONE_BULLET_H = 24;

// --- Phase 1 (300m Gauntlet) ---
const int   P1_DRONE_SIZE = 165;      // was 120
const float P1_BULLET_SPEED = 1000.0f;  // was 800
const float P1_LOW_GUN_MIN_Y = 70.0f;    // lower drone's gun stays between this ...
const float P1_LOW_GUN_MAX_Y = 180.0f;   // ... and 25% of the 720px window
const float P1_LOW_PATROL_SPEED = 2.2f;     // was 1.3
// Upper drone: its gun sweeps from just above the lower drone's band all the
// way up to the ceiling, so the middle of the screen is covered too (no safe spot).
const float P1_HIGH_GUN_MIN_Y = 200.0f;
const float P1_HIGH_GUN_MAX_Y = 640.0f;
const float P1_HIGH_PATROL_SPEED = 1.5f;    // radians/sec of the sweep

// --- Phase 2 (100m Intrusion) ---
const int   P2_DRONE_SIZE = 170;           // was 130
const int   P2_ROCKET_W = 90;            // visible rocket, was ~56x11
const int   P2_ROCKET_H = 41;

// --- Final Fight ---
const int   FINAL_DRONE1_SIZE = 165;      // was 120
const int   FINAL_DRONE2_SIZE = 170;      // was 130
const float FINAL_BULLET_SPEED = 800.0f;
const int   FINAL_ROCKET_W = 78;       // slightly smaller than the Phase 2 rocket
const int   FINAL_ROCKET_H = 36;
const int   FINAL_DRONE_HEALTH = 30;       // player bullet hits needed per drone
const float FINAL_AMMO_INTERVAL = 10.0f;    // seconds between ammo pickups
// Final Fight Drone1 hugs the floor: its gun (and so its bullets) stays in this
// band, so a player who just keeps running on the ground gets hit.
const float FINAL_D1_GUN_MIN_Y = 75.0f;
const float FINAL_D1_GUN_MAX_Y = 180.0f;
const float FINAL_D1_PATROL_SPEED = 2.0f;

// =========================================================
// DRONE 1 -- bullet-shooting "dodge gauntlet" enemy.
// Used for the 300m Gauntlet (2 of them) and the Final Drone
// Fight (1 of them, reusing slot 0).
// =========================================================
struct Drone1
{
	float x, y;
	int width, height;
	bool active;

	float stopX;            // x position where it stops and starts patrolling
	float patrolCenterY;
	float patrolAmplitude;
	float patrolSpeed;      // radians/sec
	float patrolPhase;

	float fireTimer;
	float fireInterval;

	int   bulletW, bulletH; // size of the bullets it fires
	float bulletSpeed;

	bool uniformSweep;      // true = triangle-wave patrol (even coverage of its whole band), false = sine

	int health, maxHealth;  // only used in the Final Fight
};

#define MAX_DRONE1 2
Drone1 drone1s[MAX_DRONE1];
int drone1Image;

struct DroneBullet
{
	float x, y;
	int width, height;
	float speed;
	bool active;
};

#define MAX_DRONE_BULLETS 24
DroneBullet droneBullets[MAX_DRONE_BULLETS];
int droneBulletImage;

// =========================================================
// DRONE 2 -- rocket-shooting, one-hit-kill enemy. Its rockets
// can be shot down by 2 player bullet hits. Used for the 100m
// Intrusion (1 of them, layered on top of normal obstacles) and
// the Final Drone Fight (1 of them, reusing the same instance).
// =========================================================
struct Drone2
{
	float x, y;
	int width, height;
	bool active;

	float stopX;
	float patrolCenterY;
	float patrolAmplitude;
	float patrolSpeed;
	float patrolPhase;

	float fireTimer;
	float fireInterval;

	int rocketW, rocketH;   // size of the rockets it fires

	int health, maxHealth;  // only used in the Final Fight
};

Drone2 drone2;
int drone2Image;

struct DroneRocket
{
	float x, y;
	int width, height;
	float speed;
	int health;      // 2 bullet hits destroys it
	bool active;
};

#define MAX_DRONE_ROCKETS 6
DroneRocket droneRockets[MAX_DRONE_ROCKETS];
int droneRocketImage;

// Hitboxes of the drones' visible art (used by the player's bullets in the Final Fight)
void getDrone1Hitbox(const Drone1& d, float& l, float& r, float& b, float& t)
{
	l = d.x + DRONE1_BOUNDS.x0 * d.width;
	r = d.x + DRONE1_BOUNDS.x1 * d.width;
	b = d.y + DRONE1_BOUNDS.y0 * d.height;
	t = d.y + DRONE1_BOUNDS.y1 * d.height;
}

void getDrone2Hitbox(const Drone2& d, float& l, float& r, float& b, float& t)
{
	l = d.x + DRONE2_BOUNDS.x0 * d.width;
	r = d.x + DRONE2_BOUNDS.x1 * d.width;
	b = d.y + DRONE2_BOUNDS.y0 * d.height;
	t = d.y + DRONE2_BOUNDS.y1 * d.height;
}

// =========================================================
// PLAYER HEALTH -- only relevant while Drone1 fire is on screen
// (the 300m Gauntlet and the Final Drone Fight). Drawn in the
// exact same style/position as the fuel bar.
// =========================================================
float playerHealth = 100.0f;
float maxPlayerHealth = 100.0f;
float playerHitCooldown = 0.0f;

const float PLAYER_HIT_IFRAMES = 0.5f;      // brief invincibility after a drone1 hit
const float DRONE1_BULLET_DAMAGE = 20.0f;   // 5 clean hits = death

void resetPlayerHealth()
{
	playerHealth = maxPlayerHealth;
	playerHitCooldown = 0.0f;
}

// =========================================================
// PHASE STATE
// =========================================================
bool dronePhase1Triggered = false;   // fires once, at 300m remaining
bool dronePhase2Triggered = false;   // fires once, at 100m remaining

// --- Drone Gauntlet 1 (300m): takes over completely -------
bool isDronePhase1 = false;
bool showDrone1Warning = false;
float drone1WarningTimer = 0.0f;
const float DRONE1_WARNING_DURATION = 2.0f;
float dronePhase1Timer = 0.0f;
const float DRONE_PHASE1_DURATION = 30.0f;

// --- Drone Intrusion 2 (100m): layered on live gameplay ----
bool isDronePhase2 = false;
bool showDrone2Warning = false;
float drone2WarningTimer = 0.0f;
const float DRONE2_WARNING_DURATION = 2.0f;
float dronePhase2Timer = 0.0f;
const float DRONE_PHASE2_DURATION = 30.0f;

// --- Final Drone Fight (replaces the old Security shootout) -
bool isFinalFight = false;          // true for the whole encounter (banner + fight)
bool showFinalRoundText = false;    // true while the "FINAL ROUND" banner is up
float finalRoundTextTimer = 0.0f;
const float FINAL_ROUND_TEXT_DURATION = 2.5f;
bool finalFightActive = false;      // true once the drones are actually live
float finalAmmoTimer = 0.0f;        // counts down to the next ammo pickup (Final Fight only)

const float DRONE_ENTRY_SPEED = 260.0f; // px/sec, flying in from the right

// =========================================================
// RESET
// =========================================================
void resetFinalFight()
{
	isFinalFight = false;
	showFinalRoundText = false;
	finalFightActive = false;
	finalRoundTextTimer = 0.0f;
	finalAmmoTimer = 0.0f;
}

void resetDronePhases()
{
	dronePhase1Triggered = false;
	dronePhase2Triggered = false;

	isDronePhase1 = false;
	showDrone1Warning = false;
	drone1WarningTimer = 0.0f;
	dronePhase1Timer = 0.0f;

	isDronePhase2 = false;
	showDrone2Warning = false;
	drone2WarningTimer = 0.0f;
	dronePhase2Timer = 0.0f;

	for (int i = 0; i < MAX_DRONE1; i++) drone1s[i].active = false;
	drone2.active = false;
	for (int i = 0; i < MAX_DRONE_BULLETS; i++) droneBullets[i].active = false;
	for (int i = 0; i < MAX_DRONE_ROCKETS; i++) droneRockets[i].active = false;

	resetPlayerHealth();
	resetFinalFight();
}

// =========================================================
// SHARED HELPERS -- Drone1 (bullets) & Drone2 (rockets)
// =========================================================
// Patrol height offset in [-1, 1]. The sine version lingers near the ends of
// its band; the triangle version spends equal time everywhere, so every
// height in the band is covered evenly.
float drone1PatrolWave(const Drone1& d)
{
	float s = sinf(d.patrolPhase);
	if (!d.uniformSweep) return s;
	return (2.0f / 3.14159265f) * asinf(s);
}

void fireDrone1(Drone1& d)
{
	for (int i = 0; i < MAX_DRONE_BULLETS; i++)
	{
		if (!droneBullets[i].active)
		{
			droneBullets[i].width = d.bulletW;
			droneBullets[i].height = d.bulletH;
			droneBullets[i].speed = d.bulletSpeed;
			// emerges from the gun muzzle (left edge of the drone art)
			droneBullets[i].x = d.x + DRONE1_BOUNDS.x0 * d.width - d.bulletW * 0.5f;
			droneBullets[i].y = d.y + d.height * 0.5f - d.bulletH * 0.5f;
			droneBullets[i].active = true;
			break;
		}
	}
}

void updateDroneBullets(float charLeft, float charRight, float charBottom, float charTop, bool& isLose)
{
	if (playerHitCooldown > 0.0f) playerHitCooldown -= 0.016f;

	for (int i = 0; i < MAX_DRONE_BULLETS; i++)
	{
		if (!droneBullets[i].active) continue;

		droneBullets[i].x -= droneBullets[i].speed * 0.016f;

		if (droneBullets[i].x < -100)
		{
			droneBullets[i].active = false;
			continue;
		}

		if (charLeft < droneBullets[i].x + droneBullets[i].width &&
			charRight > droneBullets[i].x &&
			charBottom < droneBullets[i].y + droneBullets[i].height &&
			charTop > droneBullets[i].y)
		{
			droneBullets[i].active = false;

			if (!isShieldActive && playerHitCooldown <= 0.0f)
			{
				playerHealth -= DRONE1_BULLET_DAMAGE;
				playerHitCooldown = PLAYER_HIT_IFRAMES;

				if (playerHealth <= 0.0f)
				{
					playerHealth = 0.0f;
					isLose = true;
				}
			}
		}
	}
}

void updateDrone1Unit(Drone1& d)
{
	if (!d.active) return;

	if (d.x > d.stopX)
	{
		d.x -= DRONE_ENTRY_SPEED * 0.016f;
		if (d.x < d.stopX) d.x = d.stopX;
	}

	if (d.x <= d.stopX)
	{
		d.patrolPhase += d.patrolSpeed * 0.016f;
		d.y = d.patrolCenterY + d.patrolAmplitude * drone1PatrolWave(d);

		d.fireTimer -= 0.016f;
		if (d.fireTimer <= 0.0f)
		{
			fireDrone1(d);
			d.fireTimer = d.fireInterval + (rand() % 5) * 0.08f; // small jitter
		}
	}
}

void fireDroneRocket(float muzzleX, float gunY, float speed, int w, int h)
{
	for (int i = 0; i < MAX_DRONE_ROCKETS; i++)
	{
		if (!droneRockets[i].active)
		{
			droneRockets[i].x = muzzleX;
			droneRockets[i].y = gunY - h * 0.5f;
			droneRockets[i].width = w;
			droneRockets[i].height = h;
			droneRockets[i].speed = speed;
			droneRockets[i].health = 2; // 2 player bullet hits destroys it
			droneRockets[i].active = true;
			break;
		}
	}
}

void updateDroneRockets(float charLeft, float charRight, float charBottom, float charTop, bool& isLose)
{
	for (int i = 0; i < MAX_DRONE_ROCKETS; i++)
	{
		if (!droneRockets[i].active) continue;

		droneRockets[i].x -= droneRockets[i].speed * 0.016f;

		if (droneRockets[i].x < -120)
		{
			droneRockets[i].active = false;
			continue;
		}

		if (charLeft < droneRockets[i].x + droneRockets[i].width &&
			charRight > droneRockets[i].x &&
			charBottom < droneRockets[i].y + droneRockets[i].height &&
			charTop > droneRockets[i].y)
		{
			droneRockets[i].active = false;
			if (!isShieldActive) isLose = true; // one hit kill, unless a shield is active
		}
	}
}

void updateDrone2Unit(Drone2& d, bool allowFire)
{
	if (!d.active) return;

	if (d.x > d.stopX)
	{
		d.x -= DRONE_ENTRY_SPEED * 0.016f;
		if (d.x < d.stopX) d.x = d.stopX;
	}

	if (d.x <= d.stopX)
	{
		d.patrolPhase += d.patrolSpeed * 0.016f;
		d.y = d.patrolCenterY + d.patrolAmplitude * sinf(d.patrolPhase);

		if (allowFire)
		{
			d.fireTimer -= 0.016f;
			if (d.fireTimer <= 0.0f)
			{
				fireDroneRocket(d.x + DRONE2_BOUNDS.x0 * d.width + 10.0f,
					d.y + d.height * 0.5f, 480.0f, d.rocketW, d.rocketH);
				d.fireTimer = d.fireInterval + (rand() % 5) * 0.1f;
			}
		}
	}
}

// Shared setup so every drone is configured the same way.
void configureDrone1(Drone1& d, int size, float startX, float stopX,
	float centerY, float amplitude, float patrolSpeed, float phase,
	float fireTimer, float fireInterval, float bulletSpeed, bool uniformSweep = false)
{
	d.width = size;
	d.height = size;
	d.x = startX;
	d.stopX = stopX;
	d.patrolCenterY = centerY;
	d.patrolAmplitude = amplitude;
	d.patrolSpeed = patrolSpeed;
	d.patrolPhase = phase;
	d.uniformSweep = uniformSweep;
	d.y = centerY + amplitude * drone1PatrolWave(d);   // start where the patrol will pick up (no pop)
	d.fireTimer = fireTimer;
	d.fireInterval = fireInterval;
	d.bulletW = DRONE_BULLET_W;
	d.bulletH = DRONE_BULLET_H;
	d.bulletSpeed = bulletSpeed;
	d.maxHealth = FINAL_DRONE_HEALTH;
	d.health = FINAL_DRONE_HEALTH;
	d.active = false;
}

void configureDrone2(Drone2& d, int size, float startX, float stopX,
	float centerY, float amplitude, float patrolSpeed, float phase,
	float fireTimer, float fireInterval, int rocketW, int rocketH)
{
	d.width = size;
	d.height = size;
	d.x = startX;
	d.stopX = stopX;
	d.patrolCenterY = centerY;
	d.patrolAmplitude = amplitude;
	d.patrolSpeed = patrolSpeed;
	d.patrolPhase = phase;
	d.y = centerY + amplitude * sinf(phase);
	d.fireTimer = fireTimer;
	d.fireInterval = fireInterval;
	d.rocketW = rocketW;
	d.rocketH = rocketH;
	d.maxHealth = FINAL_DRONE_HEALTH;
	d.health = FINAL_DRONE_HEALTH;
	d.active = false;
}

// =========================================================
// DRONE GAUNTLET 1 (300m remaining) -- obstacles cleared,
// distance frozen, pure dodge challenge, 30 seconds.
// =========================================================
void startDronePhase1()
{
	isDronePhase1 = true;
	dronePhase1Timer = DRONE_PHASE1_DURATION;
	showDrone1Warning = true;
	drone1WarningTimer = DRONE1_WARNING_DURATION;

	resetPlayerHealth();
	clearFieldForDronePhase();

	// LOWER drone: hugs the floor and only moves within the bottom 25% of the
	// window (its gun -- and so its bullets -- stay between P1_LOW_GUN_MIN_Y and
	// P1_LOW_GUN_MAX_Y). A player who just keeps running on the ground gets hit,
	// so they have to fly (and fuel packs keep scrolling in to pay for it).
	float lowMinY = P1_LOW_GUN_MIN_Y - P1_DRONE_SIZE * 0.5f;   // PNG y of the lowest position
	float lowMaxY = P1_LOW_GUN_MAX_Y - P1_DRONE_SIZE * 0.5f;   // PNG y of the highest position
	configureDrone1(drone1s[0], P1_DRONE_SIZE, 1280.0f, 900.0f,
		(lowMinY + lowMaxY) * 0.5f, (lowMaxY - lowMinY) * 0.5f,
		P1_LOW_PATROL_SPEED, 0.0f, 1.0f, 1.1f, P1_BULLET_SPEED);
	drone1s[0].active = true;

	// UPPER drone: sweeps evenly from just above the lower drone's band up to the
	// ceiling, so the middle of the screen is covered and there is no blind spot.
	// It parks further right than the lower drone so the two never overlap on screen.
	float highMinY = P1_HIGH_GUN_MIN_Y - P1_DRONE_SIZE * 0.5f;
	float highMaxY = P1_HIGH_GUN_MAX_Y - P1_DRONE_SIZE * 0.5f;
	configureDrone1(drone1s[1], P1_DRONE_SIZE, 1420.0f, 1060.0f,
		(highMinY + highMaxY) * 0.5f, (highMaxY - highMinY) * 0.5f,
		P1_HIGH_PATROL_SPEED, 1.6f, 1.5f, 1.2f, P1_BULLET_SPEED, true);
	drone1s[1].active = true;

	resetDroneFuelPickup();   // fuel packs start scrolling in once the warning clears

	for (int i = 0; i < MAX_DRONE_BULLETS; i++) droneBullets[i].active = false;
}

void endDronePhase1(int currentLevel)
{
	isDronePhase1 = false;
	for (int i = 0; i < MAX_DRONE1; i++) drone1s[i].active = false;
	for (int i = 0; i < MAX_DRONE_BULLETS; i++) droneBullets[i].active = false;

	respawnFieldAfterDronePhase(currentLevel);
}

void updateDronePhase1(bool& isLose, int currentLevel)
{
	if (showDrone1Warning)
	{
		drone1WarningTimer -= 0.016f;
		if (drone1WarningTimer <= 0.0f) showDrone1Warning = false;
		return; // drones/bullets stay frozen until the warning clears
	}

	dronePhase1Timer -= 0.016f;
	if (dronePhase1Timer <= 0.0f)
	{
		endDronePhase1(currentLevel);
		return;
	}

	for (int i = 0; i < MAX_DRONE1; i++) updateDrone1Unit(drone1s[i]);

	float charLeft, charRight, charBottom, charTop;
	getCharacterAABB(charLeft, charRight, charBottom, charTop);
	updateDroneBullets(charLeft, charRight, charBottom, charTop, isLose);
}

// =========================================================
// DRONE INTRUSION 2 (100m remaining) -- obstacles/fuel/coins
// keep running exactly as normal; this is a hazard layered on
// top. Its rocket is a one-hit kill but can be shot down.
// =========================================================
void startDronePhase2()
{
	isDronePhase2 = true;
	dronePhase2Timer = DRONE_PHASE2_DURATION;
	showDrone2Warning = true;
	drone2WarningTimer = DRONE2_WARNING_DURATION;

	configureDrone2(drone2, P2_DRONE_SIZE, 1300.0f, 950.0f,
		350.0f, 220.0f, 0.6f, 0.0f,
		3.0f,    // grace period before its first shot
		2.8f,    // kept gentle since obstacles are also live
		P2_ROCKET_W, P2_ROCKET_H);
	drone2.y = 540.0f;          // flies in near the top, then settles into its patrol
	drone2.active = true;

	for (int i = 0; i < MAX_DRONE_ROCKETS; i++) droneRockets[i].active = false;
}

void endDronePhase2()
{
	isDronePhase2 = false;
	drone2.active = false;
	for (int i = 0; i < MAX_DRONE_ROCKETS; i++) droneRockets[i].active = false;
}

void updateDronePhase2(bool& isLose)
{
	if (showDrone2Warning)
	{
		drone2WarningTimer -= 0.016f;
		if (drone2WarningTimer <= 0.0f) showDrone2Warning = false;
	}

	dronePhase2Timer -= 0.016f;
	if (dronePhase2Timer <= 0.0f)
	{
		endDronePhase2();
		return;
	}

	float charLeft, charRight, charBottom, charTop;
	getCharacterAABB(charLeft, charRight, charBottom, charTop);
	updateDrone2Unit(drone2, !showDrone2Warning);
	updateDroneRockets(charLeft, charRight, charBottom, charTop, isLose);
}

// =========================================================
// FINAL DRONE FIGHT -- replaces the old 3-Security shootout.
// Same "FINAL ROUND" banner, then 1x Drone1 + 1x Drone2. Each has
// a health bar and needs FINAL_DRONE_HEALTH (30) player bullet
// hits. Destroy BOTH to win. There is no timer.
// Drone1 chip-damages health; Drone2's rockets are instant death
// unless shot down.
// =========================================================
void startFinalFight()
{
	isFinalFight = true;
	showFinalRoundText = true;
	finalRoundTextTimer = FINAL_ROUND_TEXT_DURATION;
	finalFightActive = false;
	finalAmmoTimer = FINAL_AMMO_INTERVAL;

	resetPlayerHealth();

	// The obstacle field is frozen (and invisible) during the final fight; deactivate it so
	// nothing invisible can swallow the player's bullets.
	clearFieldForDronePhase();
	resetDroneFuelPickup();   // fuel packs start scrolling in once the fight is live

	// Drone1 stays low (gun between FINAL_D1_GUN_MIN_Y and FINAL_D1_GUN_MAX_Y) so its
	// bullets reach a player who is simply running along the floor.
	float f1MinY = FINAL_D1_GUN_MIN_Y - FINAL_DRONE1_SIZE * 0.5f;
	float f1MaxY = FINAL_D1_GUN_MAX_Y - FINAL_DRONE1_SIZE * 0.5f;
	configureDrone1(drone1s[0], FINAL_DRONE1_SIZE, 1280.0f, 930.0f,
		(f1MinY + f1MaxY) * 0.5f, (f1MaxY - f1MinY) * 0.5f,
		FINAL_D1_PATROL_SPEED, 0.0f, 1.5f, 1.1f, FINAL_BULLET_SPEED, true);
	drone1s[0].active = false; // switched on once the banner ends

	drone1s[1].active = false; // only one Drone1 in the final fight

	configureDrone2(drone2, FINAL_DRONE2_SIZE, 1420.0f, 810.0f,
		350.0f, 220.0f, 0.6f, 0.8f, 2.5f,
		2.2f,    // a touch faster than the 100m Intrusion, nothing else on screen here
		FINAL_ROCKET_W, FINAL_ROCKET_H);
	drone2.active = false;

	for (int i = 0; i < MAX_DRONE_BULLETS; i++) droneBullets[i].active = false;
	for (int i = 0; i < MAX_DRONE_ROCKETS; i++) droneRockets[i].active = false;
}

void updateFinalFight(bool& currentIsWin, bool& currentIsLose)
{
	if (!isFinalFight) return;

	if (showFinalRoundText)
	{
		finalRoundTextTimer -= 0.016f;
		if (finalRoundTextTimer <= 0.0f)
		{
			finalRoundTextTimer = 0.0f;
			showFinalRoundText = false;
			finalFightActive = true;

			drone1s[0].active = true;
			drone2.active = true;
		}
		return; // drones stay off-screen while the banner is up
	}

	if (!finalFightActive) return;

	// WIN: both drones destroyed
	if (drone1s[0].health <= 0 && drone2.health <= 0)
	{
		for (int i = 0; i < MAX_DRONE_BULLETS; i++) droneBullets[i].active = false;
		for (int i = 0; i < MAX_DRONE_ROCKETS; i++) droneRockets[i].active = false;
		currentIsWin = true;
		return;
	}

	float charLeft, charRight, charBottom, charTop;
	getCharacterAABB(charLeft, charRight, charBottom, charTop);

	updateDrone1Unit(drone1s[0]);
	updateDroneBullets(charLeft, charRight, charBottom, charTop, currentIsLose);

	updateDrone2Unit(drone2, true);
	// rockets keep flying even after Drone2 is destroyed
	updateDroneRockets(charLeft, charRight, charBottom, charTop, currentIsLose);
}

// =========================================================
// DRAWING
// =========================================================
void drawDrone1Squad()
{
	for (int i = 0; i < MAX_DRONE1; i++)
	if (drone1s[i].active)
		iShowImage((int)drone1s[i].x, (int)drone1s[i].y, drone1s[i].width, drone1s[i].height, drone1Image);
}

void drawDroneBullets()
{
	for (int i = 0; i < MAX_DRONE_BULLETS; i++)
	if (droneBullets[i].active)
		drawSpriteFitted(droneBulletImage, droneBullets[i].x, droneBullets[i].y,
		(float)droneBullets[i].width, (float)droneBullets[i].height, BULLET_BOUNDS);
}

void drawDrone2()
{
	if (drone2.active)
		iShowImage((int)drone2.x, (int)drone2.y, drone2.width, drone2.height, drone2Image);
}

void drawDroneRockets()
{
	for (int i = 0; i < MAX_DRONE_ROCKETS; i++)
	if (droneRockets[i].active)
		drawSpriteFitted(droneRocketImage, droneRockets[i].x, droneRockets[i].y,
		(float)droneRockets[i].width, (float)droneRockets[i].height, ROCKET_BOUNDS);
}

// Small health bar floating above a drone (Final Fight)
void drawDroneHealthBar(float left, float right, float top, int health, int maxHealth)
{
	int w = (int)(right - left);
	int bx = (int)left;
	int by = (int)top + 8;
	if (by > 706) by = 706;      // keep it on screen when the drone is at the top

	float frac = (maxHealth > 0) ? (float)health / (float)maxHealth : 0.0f;
	if (frac < 0.0f) frac = 0.0f;
	if (frac > 1.0f) frac = 1.0f;

	iSetColor(60, 60, 60);
	iFilledRectangle(bx, by, w, 9);

	if (frac > 0.5f)        iSetColor(0, 230, 0);
	else if (frac > 0.25f)  iSetColor(255, 200, 0);
	else                    iSetColor(255, 50, 50);
	iFilledRectangle(bx, by, (int)(w * frac), 9);

	iSetColor(255, 255, 255);
	iRectangle(bx, by, w, 9);
}

void drawFinalDroneFight()
{
	float l, r, b, t;

	if (drone1s[0].active)
	{
		iShowImage((int)drone1s[0].x, (int)drone1s[0].y, drone1s[0].width, drone1s[0].height, drone1Image);
		getDrone1Hitbox(drone1s[0], l, r, b, t);
		drawDroneHealthBar(l, r, t, drone1s[0].health, drone1s[0].maxHealth);
	}

	if (drone2.active)
	{
		drawDrone2();
		getDrone2Hitbox(drone2, l, r, b, t);
		drawDroneHealthBar(l, r, t, drone2.health, drone2.maxHealth);
	}

	drawDroneBullets();
	drawDroneRockets();
}

void drawHealthBar()
{
	// sits directly above the fuel bar (fuel bar is at y = 605)
	iSetColor(100, 100, 100);
	iFilledRectangle(1050, 635, 180, 18);

	if (playerHealth > 40.0f) iSetColor(0, 230, 0);
	else                      iSetColor(255, 50, 50);
	iFilledRectangle(1050, 635, (int)((playerHealth / maxPlayerHealth) * 180), 18);

	iSetColor(255, 255, 255);
	iRectangle(1050, 635, 180, 18);
	iText(970, 638, "Health:", GLUT_BITMAP_HELVETICA_12);
}

void drawDrone1Warning()
{
	if (!showDrone1Warning) return;

	iSetColor(0, 0, 0);
	iFilledRectangle(0, 0, 1280, 720);

	iSetColor(255, 40, 40);
	const char* bigText = "DRONE STRIKE INCOMING";
	void* bigFont = GLUT_BITMAP_TIMES_ROMAN_24;
	int wBig = getBitmapTextWidth(bigText, bigFont);
	iText(640 - wBig / 2, 390, bigText, bigFont);

	iSetColor(255, 255, 255);
	const char* subText = "Two drones inbound -- dodge their fire!";
	int wSub = getBitmapTextWidth(subText, GLUT_BITMAP_HELVETICA_18);
	iText(640 - wSub / 2, 340, subText, GLUT_BITMAP_HELVETICA_18);
}

void drawDrone2Warning()
{
	if (!showDrone2Warning) return;

	iSetColor(255, 60, 60);
	const char* text = "WARNING: Rocket drone incoming -- shoot its rockets down!";
	int w = getBitmapTextWidth(text, GLUT_BITMAP_TIMES_ROMAN_24);
	iText(640 - w / 2, 650, text, GLUT_BITMAP_TIMES_ROMAN_24);
}

void loadDroneImages()
{
	drone1Image = iLoadImage("Images//drone1.png");
	drone2Image = iLoadImage("Images//drone2.png");
	droneBulletImage = iLoadImage("Images//droneBulletLeft.png");
	droneRocketImage = iLoadImage("Images//droneRocket.png");
}

#endif // DRONE_H