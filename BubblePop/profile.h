#ifndef PROFILE_H
#define PROFILE_H

#include <cstdio>
#include <cstring>
#include <cctype>

// =========================================================
// PLAYER PROFILES + LEVEL PROGRESS  (file handling)
//
// Everything is stored in a plain text file next to the .exe:
//
//      savegame.txt
//      ------------
//      <levels cleared>|<L1 score>|<L2 score>|<L3 score>|<player name>
//      2|41|37|0|Rahim
//      0|0|0|0|Karim
//
// "levels cleared" is the highest level the player has beaten
// (0 = none, 3 = all).  Level N is unlocked when cleared >= N-1.
//
// A level's score is the number of coins collected in the player's best
// winning run of that level. A profile's total score is the sum over the
// three levels; the Load Game list is sorted by it (highest on top).
//
// Old save files ("1 Rahim", no scores) are still read fine: their scores
// count as 0 and the file is upgraded the next time it is saved.
// =========================================================

#define MAX_PROFILES    10
#define MAX_NAME_LEN    14
#define TOTAL_LEVELS    3
#define SAVE_FILE_NAME  "savegame.txt"

struct Profile
{
	char name[MAX_NAME_LEN + 1];
	int  cleared;               // highest level cleared (0..TOTAL_LEVELS)
	int  levelScore[TOTAL_LEVELS];   // best coin count of a winning run, per level (0 = not cleared)
};

Profile profiles[MAX_PROFILES];
int  profileCount = 0;
int  profileOrder[MAX_PROFILES];   // profileOrder[0] = index of the top-scoring profile, and so on
int  currentProfile = -1;       // index into profiles[], -1 = nobody
bool testMode = false;          // Test button: everything unlocked, nothing saved

// -------------------------------------------------------
// small helpers
// -------------------------------------------------------
void trimName(char* s)
{
	int len = (int)strlen(s);
	while (len > 0 && (s[len - 1] == ' ' || s[len - 1] == '\r' || s[len - 1] == '\n'))
		s[--len] = '\0';

	int start = 0;
	while (s[start] == ' ') start++;
	if (start > 0) memmove(s, s + start, len - start + 1);
}

int profileTotalScore(const Profile& p)
{
	int total = 0;
	for (int i = 0; i < TOTAL_LEVELS; i++) total += p.levelScore[i];
	return total;
}

// Sorts profileOrder[] by total score, highest first. Equal scores keep their
// original (file) order. profiles[] itself is never reordered, so
// currentProfile stays a valid index.
void rebuildProfileOrder()
{
	for (int i = 0; i < profileCount; i++) profileOrder[i] = i;

	for (int i = 1; i < profileCount; i++)
	{
		int key = profileOrder[i];
		int keyScore = profileTotalScore(profiles[key]);

		int j = i - 1;
		while (j >= 0 && profileTotalScore(profiles[profileOrder[j]]) < keyScore)
		{
			profileOrder[j + 1] = profileOrder[j];
			j--;
		}
		profileOrder[j + 1] = key;
	}
}

int findProfileByName(const char* name)
{
	for (int i = 0; i < profileCount; i++)
	{
		if (_stricmp(profiles[i].name, name) == 0) return i;   // case-insensitive
	}
	return -1;
}

// -------------------------------------------------------
// SAVE  (rewrites the whole file, it is tiny)
// -------------------------------------------------------
void saveProfiles()
{
	FILE* f = NULL;
	if (fopen_s(&f, SAVE_FILE_NAME, "w") != 0 || f == NULL) return;

	for (int i = 0; i < profileCount; i++)
		fprintf(f, "%d|%d|%d|%d|%s\n", profiles[i].cleared,
		profiles[i].levelScore[0], profiles[i].levelScore[1], profiles[i].levelScore[2],
		profiles[i].name);

	fclose(f);
}

// reads a run of digits at p (moves p past them); returns 0 if there are none
static int readNumber(char*& p)
{
	int v = 0;
	while (isdigit((unsigned char)*p))
	{
		if (v < 100000000) v = v * 10 + (*p - '0');
		p++;
	}
	return v;
}

// -------------------------------------------------------
// LOAD  (called once at start-up; a missing file is fine)
// -------------------------------------------------------
void loadProfiles()
{
	profileCount = 0;

	FILE* f = NULL;
	if (fopen_s(&f, SAVE_FILE_NAME, "r") != 0 || f == NULL) return;

	char line[128];
	while (profileCount < MAX_PROFILES && fgets(line, sizeof(line), f))
	{
		char* p = line;
		while (*p == ' ') p++;
		if (!isdigit((unsigned char)*p)) continue;      // skip broken lines

		int cleared = readNumber(p);
		if (cleared > TOTAL_LEVELS) cleared = TOTAL_LEVELS;

		int scores[TOTAL_LEVELS] = { 0 };
		if (*p == '|')                                   // new format: cleared|s1|s2|s3|name
		{
			for (int i = 0; i < TOTAL_LEVELS; i++)
			{
				if (*p == '|') p++;
				scores[i] = readNumber(p);
			}
			if (*p == '|') p++;
		}
		else if (*p == ' ')                              // old format: "cleared name" (no scores)
		{
			p++;
		}

		char name[MAX_NAME_LEN + 1];
		int n = 0;
		while (*p && *p != '\n' && *p != '\r' && n < MAX_NAME_LEN) name[n++] = *p++;
		name[n] = '\0';
		trimName(name);
		if (name[0] == '\0') continue;

		strcpy_s(profiles[profileCount].name, MAX_NAME_LEN + 1, name);
		profiles[profileCount].cleared = cleared;
		for (int i = 0; i < TOTAL_LEVELS; i++) profiles[profileCount].levelScore[i] = scores[i];
		profileCount++;
	}

	fclose(f);
	rebuildProfileOrder();
}

// -------------------------------------------------------
// ADD A NEW PLAYER  (returns its index, or -1 if the list is full)
// -------------------------------------------------------
int addProfile(const char* name)
{
	if (profileCount >= MAX_PROFILES) return -1;

	strcpy_s(profiles[profileCount].name, MAX_NAME_LEN + 1, name);
	profiles[profileCount].cleared = 0;
	for (int i = 0; i < TOTAL_LEVELS; i++) profiles[profileCount].levelScore[i] = 0;
	profileCount++;

	rebuildProfileOrder();
	saveProfiles();
	return profileCount - 1;
}

// -------------------------------------------------------
// LEVEL UNLOCK RULES
// -------------------------------------------------------
int unlockedLevelCount()
{
	if (testMode) return TOTAL_LEVELS;

	int cleared = (currentProfile >= 0 && currentProfile < profileCount) ? profiles[currentProfile].cleared : 0;
	int count = cleared + 1;
	if (count > TOTAL_LEVELS) count = TOTAL_LEVELS;
	return count;
}

// levelIndex is 0-based (0 = Level 1)
bool isLevelUnlocked(int levelIndex)
{
	return levelIndex >= 0 && levelIndex < unlockedLevelCount();
}

// Call when a level is won. level is 1-based, levelScore = coins collected in that run.
// Progress is kept, and a level's score is only replaced by a better one.
// Does nothing in Test mode.
void recordLevelClear(int level, int levelScore)
{
	if (testMode) return;
	if (currentProfile < 0 || currentProfile >= profileCount) return;

	Profile& p = profiles[currentProfile];
	bool changed = false;

	if (p.cleared < level)
	{
		p.cleared = level;
		changed = true;
	}

	int idx = level - 1;
	if (idx >= 0 && idx < TOTAL_LEVELS && levelScore > p.levelScore[idx])
	{
		p.levelScore[idx] = levelScore;
		changed = true;
	}

	if (changed)
	{
		saveProfiles();
		rebuildProfileOrder();
	}
}

#endif // PROFILE_H