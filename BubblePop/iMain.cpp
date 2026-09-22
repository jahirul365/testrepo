#include "iGraphics.h"

int images[4];
int currentFrame = 0;

int floorImage;
int characterImage;
int playerFrames[4];
int currentPlayerFrame = 0;
int characterJumpImage; //new......

// ---------------- FLOOR ----------------

float floorX = 0;
float floorSpeed = 200.0f;

int floorWidth = 1280;
int floorHeight = 124;


// character
float characterX = 200;
float characterY = 15;
float characterSpeed = 250.0f;

// jump
bool isJumping = false;
float jumpSpeed = 500.0f;  // new          .............
float gravity = 1000.0f;


// ---------------- BACKGROUND ANIMATION ----------------

void advanceFrame()
{
    currentFrame = (currentFrame + 1) % 4;
}

void updatePlayerAnimation()
{
	currentPlayerFrame++;

	if (currentPlayerFrame >= 4)
	{
		currentPlayerFrame = 0;
	}
}


// ---------------- FLOOR MOVEMENT ----------------

void updateFloor()
{
    floorX -= floorSpeed * 0.016f;

    if (floorX <= -floorWidth)
    {
        floorX += floorWidth;
    }
}



// jump movement function

void updateJump()
{
	if (isJumping)
	{
		// upper speed 
		characterY += 500.0f * 0.016f;

		if (characterY > 520)
		{
			characterY = 520;
		}
	}
	else
	{
		// down speed
		characterY -= 500.0f * 0.016f;

		if (characterY < 15)
		{
			characterY = 15;
		}
	}
}
// ---------------- DRAW ----------------

void iDraw()
{
    iClear();

    // Animated background
    iShowImage(0, 0, 1280, 720, images[currentFrame]);

    // First floor
    iShowImage(
        (int)floorX,
        0,
        floorWidth,
        floorHeight,
        floorImage
    );

    // Second floor
    iShowImage(
        (int)floorX + floorWidth,
        0,
        floorWidth,
        floorHeight,
        floorImage
    );

	// Character
	
	if (isJumping)
	{
		iShowImage((int)characterX, (int)characterY,
			180, 200, characterJumpImage);
	}
	else
	{
		iShowImage((int)characterX, (int)characterY,
			180, 200, playerFrames[currentPlayerFrame]);
	}
}


// ---------------- INPUT ----------------

void iMouseMove(int mx, int my) {}
void iPassiveMouseMove(int mx, int my) {}
void iMouse(int button, int state, int mx, int my)
{
	if (button == GLUT_LEFT_BUTTON)
	{
		if (state == GLUT_DOWN)
		{
			isJumping = true;
		}
		else if (state == GLUT_UP)
		{
			isJumping = false;
		}
	}
}
void fixedUpdate() {}




// ---------------- LOAD IMAGES ----------------

void loadImages()
{
	// Load 4 background frames
	for (int i = 0; i < 4; i++)
	{
		char path[100];

		sprintf_s(path, "Images//balloon%d.png", i + 1);

		images[i] = iLoadImage(path);

		printf("Loaded: %s\n", path);
	}

	// Load floor
	floorImage = iLoadImage("Images//floor.png");
	printf("Loaded: Images//floor.png\n");


	// ---------------- PLAYER RUNNING ANIMATION ----------------

	//playerFrames[0] = iLoadImage("Images//player.png");
	playerFrames[0] = iLoadImage("Images//player 2.png");
	playerFrames[1] = iLoadImage("Images//player 3.png");
	playerFrames[2] = iLoadImage("Images//player 4.png");
	playerFrames[3] = iLoadImage("Images//player 5.png");

	printf("Loaded player running frames\n");


	// ---------------- JETPACK ON ----------------

	characterJumpImage = iLoadImage("Images//player 1.png");
	printf("Loaded: Images//player 1.png\n");
}


// ---------------- MAIN ----------------

int main()
{
    // Background animation
    iSetTimer(83, advanceFrame);
	iSetTimer(83, updatePlayerAnimation);
    // Floor movement
    iSetTimer(16, updateFloor);
	
	iSetTimer(16, updateJump);
    // Game window
    iInitialize(1280, 720, "Animation");

    loadImages();

    iStart();

    return 0;
}