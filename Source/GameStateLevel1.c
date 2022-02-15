/********************************************************************************/
/*
	File name: GameStateLevel1.c
	Project name: Neon 'n' Slash
	Author: Victor Soto, Jack O'Brien, Austin Sherrod, Aditya Ravichandran, Kalby Jang
	
	All content © 2018 DigiPen (USA) Corporation, all rights reserved.
*/
/********************************************************************************/
#include "GameStateManager.h"
#include "GameStateTable.h"
#include "Trace.h"
#include "stdafx.h"
#include "Mesh.h"
#include "LoboState.h"
#include "Boss.h"
#include "Emitter.h"
#include "Player.h"
#include "GameTime.h"
#include "PlayerMovement.h"
#include "AEEngine.h"
#include "RenderManager.h"
#include "PlayerState.h"
#include "shapes.h"
#include "PlayerAttack.h"
#include "Bullet.h"
#include "Star.h"
#include "Stats.h"
#include "UIManager.h"
#include "collisionHandler.h"
#include "Camera.h"
#include "PauseMenu.h"
#include "Timer.h"
#include "AOE.h"
#include "audio.h"

/* The boss of Level1 */
static BossPtr Lobo;

static Player_Object *Player;

static AEGfxVertexList *Tile;
static AEGfxTexture *Tile_Texture;

static int PlayerDeathAniFlag = 0;

float bosstimer = -1.0f;

static void Draw_Tile()
{
	
	AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
	/* Set position for Lobo */
	AEGfxSetFullTransform(0, 0, 0, LEVEL_SCALE, LEVEL_SCALE);
	/* No texture for Lobo */
	AEGfxTextureSet(Tile_Texture, 0, 0);
	/* Drawing the mesh (list of triangles) */
	AEGfxSetTransparency(1.0);
	AEGfxSetBlendColor(0.0f, 0.0f, 0.0, 0.0f);
	AEGfxMeshDraw(Tile, AE_GFX_MDM_TRIANGLES);	
	/* Drawing Lobo */

}

/* Load the resources associated with the Level1 game state. */
void GameStateLevel1Load(void)
{
	TraceMessage("Level1: Load");

	/* Load Lobo's assets */
	LoboStateLoad(&Lobo);

	StarParticleLoad();

	AOE_Init(); // Init the AOE attack
}

/* Initialize the memory associated with the Level1 game state. */
void GameStateLevel1Init(void)
{
	AEVec2 PlayerPos = StatsGetPlayerInitialPos();

	TraceMessage("Level1: Init");

	/* Initializes the data in Lobo */
	LoboStateInit(Lobo);

	ResetParticleSystem();

	PauseMenuLoad();

	PlayerDeathAniFlag = 0;

	/* Initialize the player */
	PlayerStateLoad(&Player, PlayerPos);

  Tile = CreateQuadMesh(.5f, .5f, 1.0f, 1.0f);
	//createQuadMesh(&Tile, 1.0f, 1.0f);
	Tile_Texture = AEGfxTextureLoad("Assets\\Level1.png");
	
	StopMusic();

	PlayMusic("Assets\\Sounds\\Music\\BossMusicJoey.wav", 1);
	//FadeMusic();


	UILoad(Lobo);

	TimerStart(0.15f, "Level1Delay");

	StarParticleInit();

	/* Sets Background color and blend mode. Must go before  */
	frameInit();
}

/* 
 * Update the Level1 game state.
 * Params:
 *	 dt = Change in time (in seconds) since the last game loop.
 */
void GameStateLevel1Update(f64 dt)
{
	TraceMessage("Level1e: Update");

	AESysFrameStart();/* The start of a new frame */
	
	AEInputUpdate();/* Handling Input */

	if (TimerIsDone("Level1Delay") == TimerNotDone)
	{
		TimerUpdate((float)dt);

		AESysFrameEnd();

		return;
	}

	if (PlayerDeathAniFlag)
	{
		if (!PlayerAnimationIsDone(Player))
		{
			Draw_Tile();

			LoboStateUpdate(Player, Lobo, dt); /* Updates Lobo's action this frame */

			AnimationUpdate(Get_Player_Animation(Player), (float)dt);

			Draw_Player(Player);
		}
		else
		{
			GameStateManagerSetNextState(GsIntroLevel);
			StatsDecreaseNumLives(1);
		}

		AESysFrameEnd();

		/* Whenever you press escape or close the window, the game stops */
		if (0 == AESysDoesWindowExist())
		{
			GameStateManagerSetNextState(GsQuit);
		}

		return;

	}

	if (!GetpauseCheck())
	{
		TimerUpdate((float)dt);

		StatsUpdate((float)dt);

		UpdateParticleSystem(dt);

		StarParticleUpdate(dt);

		GameTimeUpdate(dt);

	}


	//if(!GetCameraIsShaking())
	CameraMove(Player, Lobo, CtAuto);

	StarParticleDraw();

	Draw_Tile();

	PlayerStateUpdate(Player, Lobo, dt);

	RenderParticleSystem();

	LoboStateUpdate(Player, Lobo, dt); /* Updates Lobo's action this frame */


	//renderObject *temp = BossGetRenderObject(Lobo);
	//temp->mesh = BossGetMesh(Lobo);
	//
	//updateRenderObject(temp, BossGetPos(Lobo));

	UIUpdate(Player, Lobo);

	PauseMenuUpdate();

#ifdef _DEBUG

	if (AEInputCheckCurr('K'))
	{
		Change_Player_Health(Player, Get_Player_Health(Player));
	}
	if (AEInputCheckCurr('L'))
	{
		BossSetHealth(Lobo, 0);
	}

#endif

	AEVec2 Pos = { 22 , 900 };

	GameTimeDraw(Pos, 2, 1);

	AESysFrameEnd();

  /* Whenever you press escape or close the window, the game stops */
	if (0 == AESysDoesWindowExist())
	{
		GameStateManagerSetNextState(GsQuit);
	}

	if (Get_Player_Health(Player) == 0)
	{
		if (StatsGetNumLives() > 1)
		{
			PlayerDeathAniFlag = 1;
			SpriteSetBlendColor(Get_Player_Sprite(Player), 1.0f, 1.0f, 1.0f, 0.0f);
			PlayerDeath(Player);
			
		}
		else
		{
			GameStateManagerSetNextState(GsGameOver);

		}

	}
	if (BossGetHealth(Lobo) <= 0)
	{
		AEVec2 PlayerPos;
		PlayerPos.x = Get_Player_PositionX(Player);
		PlayerPos.y = Get_Player_PositionY(Player);
		StatsSetBossInitialPos(BossGetPos(Lobo));
		StatsSetPlayerInitialPos(PlayerPos);
		GameStateManagerSetNextState(GsOutroLevel);
	}

}

/* Shutdown any memory associated with the Level1 game state. */
void GameStateLevel1Shutdown(void)
{
	TraceMessage("Level1: Shutdown");

	StarParticleShutdown();

	BulletFreeAllBullets();
	LoboStateShutdown(Lobo);
	UIShutdown();
	EndTimer("TIMER_ALL");
	StopMusic();
	SetCameraIsShaking(0);

	PauseMenuUnload();

	PlayerStateUnload(&Player);


	AOE_Reset();
	AOE_Free();


	AEGfxSetCamPosition(0.0f, 0.0f);
}

/* Unload the resources associated with the Level1 game state. */
void GameStateLevel1Unload(void)
{
	TraceMessage("Level1: Unload");

	StarParticleUnload();
	
	
	LoboStateUnload(&Lobo);
	
}