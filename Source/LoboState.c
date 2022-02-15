/********************************************************************************/
/*
	File name: LoboState.c
	Project name: Neon 'n' Slash
	Author: Victor Soto, Jack O'Brien, Austin Sherrod, Aditya Ravichandran, Kalby Jang

	All content © 2018 DigiPen (USA) Corporation, all rights reserved.
*/
/********************************************************************************/
#include "AEEngine.h"
#include "LoboState.h"
#include "Trace.h"
#include "FiniteStateMachine.h"
#include "LoboMovement.h"
#include "BossCheckHealth.h"
#include "Bullet.h"
#include "BulletMovement.h"
#include "BulletPatterns.h"
#include "BossAttack.h"
#include "AOE.h"
#include "Movement.h"
#include "LoboBehavior.h"
#include "Stats.h"
#include "Sprite.h"
#include "Timer.h"
#include "PauseMenu.h"
#include "Animation.h"
#include "SpriteSource.h"
#include "Mesh.h"
#include "AnimationFrame.h"
#include "AnimationTypes.h"
#include "BossTornado.h"
#include "Bomb.h"

AnimationSequencePtr AwakenAnimation;
AnimationSequencePtr IdleAnimation;
AnimationSequencePtr MeleeAnimation;
AnimationSequencePtr ShootAnimation;
AnimationSequencePtr AOEAnimation;
AnimationSequencePtr DeathAnimation;
AnimationSequencePtr AfterDeathAnimation;

static RGBAColors LoboColor = {0};

static TornadoPtr currentTornado = NULL; /* The tornado that the boss is going to launch */

static bombPtr currentBomb = NULL;

static AnimationFrame LoboAwaken[] = { {46, 0.13f}, {47, 0.13f }, {48, 0.13f }, {49, 0.13f },
                                       {50, 0.13f }, {51, 0.13f }, {52, 0.13f }, {53, 0.08f },
                                       {54, 0.08f }, {55, 0.08f }, {56, 0.08f }, {57, 0.08f },
                                       {58, 0.13f } };

static AnimationFrame LoboIdle[] = { {38, 0.17f}, {39, 0.17f}, {40, 0.17f}, {41, 0.17f} };

static AnimationFrame LoboMelee[] = { {19, 0.02f}, {20, 0.03f}, {21, 0.03f}, {22, 0.05f},              /* Start Up */
									  {23, 0.06f}, {24, 0.06f}, {25, 0.06f}, {26, 0.09f}, {27, 0.12f}, /* Start Up */
									  {28, 0.02f}, {29, 0.05f}, {30, 0.06f}, {31, 0.02f}, /* The slap itself */
                                      {32, 0.01f}, {33, 0.02f}, {34, 0.01f},   /* Recovery */
                                      {35, 0.02f}, {36, 0.01f} }; /* Recovery */

static AnimationFrame LoboAOE[] = { {0, 0.03f}, {1, 0.04f}, {2, 0.04f}, {3, 0.06f}, {4, 0.07f}, // Wind up
									{5, 0.08f}, {6, 0.08f},
									{7, 0.17f}, {8, 0.13f}, // Hold before dropping 
									{9, 0.08f}, // Drop begins
									{10, 0.05f}, {11, 0.04f}, {12, 0.08f}, // The drop itself
									{13, 0.08f}, // Post drop
									{14, 0.08f}, {15, 0.08f}, {16, 0.08f}, {17, 0.08f}, {18, 0.08f } }; // Recovery

static AnimationFrame LoboShoot[] = { {42, 0.1f}, {43, 0.1f}, {43, 0.1f}, {44, 0.1f}, {45, 0.1f} };

#define deathTimer 0.1f

static AnimationFrame LoboDeath[] = { { 0, deathTimer },{ 1, deathTimer },{ 2, deathTimer },{ 3, deathTimer },{ 5, deathTimer },
                                      { 6, deathTimer },{ 7, deathTimer },{ 8, deathTimer },{ 9, deathTimer },{ 10, deathTimer },
                                      { 11, deathTimer },{ 12, deathTimer },{ 13, deathTimer },{ 14, deathTimer },{ 15, deathTimer },
                                      { 16, deathTimer },{ 17, deathTimer },{ 18, deathTimer },{ 19, deathTimer },{ 20, deathTimer },
                                      { 21, deathTimer },{ 22, deathTimer },{ 23, deathTimer },{ 24, deathTimer },{ 25, deathTimer },
                                      { 26, deathTimer },{ 27, deathTimer },{ 28, deathTimer },{ 29, deathTimer },{ 30, deathTimer },
                                      { 31, deathTimer },{ 32, deathTimer } };

static AnimationFrame LoboAfterDeath[] = { { 33 , deathTimer }, { 34 , deathTimer }, { 35 , deathTimer }, { 36 , deathTimer } };

/* Loads all the assets used for Lobo like meshes and textures */
void LoboStateLoad(BossPtr *Lobo)
{
	AEVec2 size = { 100.0f, 100.0f };

	TraceMessage("Lobo: Load");

	// Load the boss texture from assets.
	AEGfxTexturePtr LoboTexture = AEGfxTextureLoad("Assets\\Boss_all.png");
	// Create the bosses mesh with apporoproate u and v values. These are currently based off
	// of the 8x8 Boss_all spritesheet.
	AEGfxVertexList *LoboMesh = CreateQuadMesh(size.x * 2, size.y * 2, (1.0f / 8.0f), (1.0f / 8.0f));
	// Make the sprite sheet fo Lobo, currently based off of the 8x8 Boss_all sheet.
	SpriteSourcePtr LoboSheet = SpriteSourceCreate(8, 8, LoboTexture);
	// Make the Boss's sprite object
	SpritePtr LoboSprite = SpriteCreate("Lobo Sprite");
	// Make the Lobo's aniamtion object with the new sprite
	AnimationPtr LoboAnimation = AnimationCreate(LoboSprite);
	
	// Set the sprite's mesh
	SpriteSetMesh(LoboSprite, LoboMesh);
	// Set the sprite's spritesource
	SpriteSetSpriteSource(LoboSprite, LoboSheet);
	
	// Make the animation sequences for the various animations
	AwakenAnimation = AnimationSequenceCreate(13, LoboAwaken, 0);
	IdleAnimation = AnimationSequenceCreate(4, LoboIdle, 1);
	MeleeAnimation = AnimationSequenceCreate(18, LoboMelee, 0);
	ShootAnimation = AnimationSequenceCreate(4, LoboShoot, 1);
	AOEAnimation = AnimationSequenceCreate(19, LoboAOE, 0);
	DeathAnimation = AnimationSequenceCreate(33, LoboDeath, 0);
	AfterDeathAnimation = AnimationSequenceCreate(4, LoboAfterDeath, 1);
	
	//AnimationPlay(LoboAnimation, 58, 0.17f, 1);

	*Lobo = BossCreate(LoboMesh, LoboTexture, LoboSprite, LoboAnimation, LoboSheet, size); /* Creates the struct for Lobo */

	BulletLoad();

	TornadoLoad();

	LoboBehaviorLoad();
}

/* Initializes the Lobo struct */
void LoboStateInit(BossPtr Lobo)
{
	TraceMessage("Lobo: Init");

	/* Sets Lobo's health, speed, and position */
	switch (StatsGetCurrentLevel())
	{
		case 0:
			BossSetHealth(Lobo, 800);
			BossSetSpeed(Lobo, 2.0f);
			LoboColor.R = 0.0f;
			LoboColor.G = 0.5f;
			LoboColor.B = 0.0f;
			LoboColor.A = 0.4f;
			break;

		case 1:
			BossSetHealth(Lobo, 1200);
			BossSetSpeed(Lobo, 3.0f);
			LoboColor.R = 0.5f;
			LoboColor.G = 0.0f;
			LoboColor.B = 0.0f;
			LoboColor.A = 0.2f;
			break;

		case 2:
			BossSetHealth(Lobo, 2000);
			BossSetSpeed(Lobo, 5.0f);
			LoboColor.R = 0.0f;
			LoboColor.G = 0.3f;
			LoboColor.B = 0.7f;
			LoboColor.A = 0.4f;
			break;

		case 3:
			BossSetHealth(Lobo, 2500);
			BossSetSpeed(Lobo, 7.0f);
			LoboColor.R = 0.0f;
			LoboColor.G = 0.0f;
			LoboColor.B = 0.0f;
			LoboColor.A = 0.0f;
			break;

		default:
			BossSetHealth(Lobo, 100);
			BossSetSpeed(Lobo, 0.5f);
			LoboColor.R = 0.2f;
			LoboColor.G = 0.2f;
			LoboColor.B = 0.2f;
			LoboColor.A = 0.2f;
			break;
	}

	SpriteSetBlendColor(BossGetSprite(Lobo), LoboColor.R, LoboColor.G, LoboColor.B, LoboColor.A);

	BossSetPosX(Lobo, StatsGetBossInitialPos().x);
	BossSetPosY(Lobo, StatsGetBossInitialPos().y);
	BossSetDirection(Lobo, UP);

	// Creating and setting Tornado
	currentTornado = TornadoCreate(NULL);

   bombLoad(&currentBomb);

	LoboBehaviorInit();
}

/* Updates lobo's actions: Move, attack */
void LoboStateUpdate(Player_Object* Player, BossPtr Lobo, f64 dt)
{
	TraceMessage("Lobo: Update");

	if (GetpauseCheck())
	{
		BulletDraw();
		
		TornadoDraw(currentTornado);
		
		SpriteDraw(BossGetSprite(Lobo), BossGetPos(Lobo));
		
		bombDraw(currentBomb);
		
		return;
	}

	LoboBehaviorUpdate(Player, Lobo, dt);

	// If there is no other animation playing, change the animation to idle.
	if (AnimationIsDone(BossGetAnimation(Lobo)) && !(AnimationIsLooping(BossGetAnimation(Lobo))))
	{
		LoboPlayIdle(Lobo);  
	}
	
	// Update the animation for the boss.
	AnimationUpdate(BossGetAnimation(Lobo), (float)dt);

	if (BossGetAttacked(Lobo))
	{
		SpriteSetBlendColor(BossGetSprite(Lobo), 1, 1, 1, 0.8f);
		TimerStart(0.2f, "BOSSHIT");
	}

	if (TimerIsDone("BOSSHIT") == 1)
	{
		SpriteSetBlendColor(BossGetSprite(Lobo), LoboColor.R, LoboColor.G, LoboColor.B, LoboColor.A);
	}

	if (TimerIsDone("INVINCIBLE") == TimerDone)
	{
		//SpriteSetBlendColor(Get_Player_Sprite(Player), LoboColor.R, LoboColor.G, LoboColor.B, LoboColor.A);
    SpriteSetBlendColor(Get_Player_Sprite(Player), 1.0f, 1.0f, 1.0f, 0.0f);
		EndTimer("ON");
		EndTimer("OFF");
	}
	else if(TimerIsDone("INVINCIBLE") == TimerNotDone)
	{
		if (TimerIsDone("ON") == TimerDone)
		{
			SpriteSetBlendColor(Get_Player_Sprite(Player), 1.0f, 1.0f, 1.0f, 0.0f);
			TimerStart(0.1f, "OFF");
		}
		if (TimerIsDone("OFF") == TimerDone)
		{
			SpriteSetBlendColor(Get_Player_Sprite(Player), 0.2f, 0.9f, 0.9f, 0.8f);
			TimerStart(0.1f, "ON");

		}
	}

	BulletDraw();

	TornadoDraw(currentTornado);

	SpriteDraw(BossGetSprite(Lobo), BossGetPos(Lobo));

	bombDraw(currentBomb);
}

/* Sets every member of the lobo struct to 0 */
 void LoboStateShutdown(BossPtr Lobo)
{
	TraceMessage("Lobo: Shutdown");

  // Free Tornado
  TornadoFree(&currentTornado);

  // Free the bomb
  bombFree(&currentBomb);

	/* Sets Lobo's health, speed and position to 0 */
	BossSetHealth(Lobo, 0);
	BossSetSpeed(Lobo, 0.0);
	BossSetPosX(Lobo, 0.0);
	BossSetPosY(Lobo, 0.0);

	LoboBehaviorShutdown();

	SpriteSetBlendColor(BossGetSprite(Lobo), 1.0f, 1.0f, 1.0f, 0.0f);
}

/* Unloads Textures and destroys meshes */
void LoboStateUnload(BossPtr *Lobo)
{
	TraceMessage("Lobo: Unload");

	AnimationSequenceFree(&AwakenAnimation);
	AnimationSequenceFree(&IdleAnimation);
	AnimationSequenceFree(&MeleeAnimation);
	AnimationSequenceFree(&ShootAnimation);
	AnimationSequenceFree(&AOEAnimation);
	AnimationSequenceFree(&DeathAnimation);
	AnimationSequenceFree(&AfterDeathAnimation);

	// Unload Tornado
	TornadoUnload();

	BulletUnload();
	LoboBehaviorUnload();

	/* Frees Lobo (go home boy, you're free now :'( ) */
	BossFree(Lobo);
}

// All of these functions play their corresponding animations

void LoboPlayAwaken(BossPtr Lobo)
{
  SpriteSetFlip(BossGetSprite(Lobo), 0);
  AnimationPlaySequence(BossGetAnimation(Lobo), AwakenAnimation);
  BossSetAnimationType(Lobo, Awaken);
}

void LoboPlayIdle(BossPtr Lobo)
{
  SpriteSetFlip(BossGetSprite(Lobo), 0);
  AnimationPlaySequence(BossGetAnimation(Lobo), IdleAnimation);
  BossSetAnimationType(Lobo, Idle);
}

void LoboPlayMeleeLeft(BossPtr Lobo)
{
  SpriteSetFlip(BossGetSprite(Lobo), 1);
  AnimationPlaySequence(BossGetAnimation(Lobo), MeleeAnimation);
  BossSetAnimationType(Lobo, MeleeLeft);
}

void LoboPlayMeleeRight(BossPtr Lobo)
{
  SpriteSetFlip(BossGetSprite(Lobo), 0);
  AnimationPlaySequence(BossGetAnimation(Lobo), MeleeAnimation);
  BossSetAnimationType(Lobo, MeleeRight);
}

void LoboPlayShoot(BossPtr Lobo)
{
  SpriteSetFlip(BossGetSprite(Lobo), 0);
  AnimationPlaySequence(BossGetAnimation(Lobo), ShootAnimation);
  BossSetAnimationType(Lobo, Shoot);
}

void LoboPlayAOE(BossPtr Lobo)
{
  SpriteSetFlip(BossGetSprite(Lobo), 0);
  AnimationPlaySequence(BossGetAnimation(Lobo), AOEAnimation);
  BossSetAnimationType(Lobo, GetOffMe);
}

void LoboPlayDeath(BossPtr Lobo)
{
  AEGfxTexturePtr deathSheet = AEGfxTextureLoad("Assets\\Boss_Death.png");
  SpriteSourcePtr deathSource = SpriteSourceCreate(8, 8, deathSheet);

  SpriteSetSpriteSource(BossGetSprite(Lobo), deathSource);

  SpriteSetFlip(BossGetSprite(Lobo), 0);
  AnimationPlaySequence(BossGetAnimation(Lobo), DeathAnimation);
  BossSetAnimationType(Lobo, Dead);
}

void LoboPlayAfterDeath(BossPtr Lobo)
{
	AEGfxTexturePtr deathSheet = AEGfxTextureLoad("Assets\\Boss_Death.png");
	SpriteSourcePtr deathSource = SpriteSourceCreate(8, 8, deathSheet);

	SpriteSetSpriteSource(BossGetSprite(Lobo), deathSource);

	SpriteSetFlip(BossGetSprite(Lobo), 0);
	AnimationPlaySequence(BossGetAnimation(Lobo), AfterDeathAnimation);
	BossSetAnimationType(Lobo, AfterDeath);
}

 
TornadoPtr LoboStateGetCurrentTornado(void)
{
	return currentTornado;
}

bombPtr LoboStateGetCurrentBomb(void)
{
	return currentBomb;
}