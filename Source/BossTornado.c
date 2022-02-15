/********************************************************************************/
/*
	File name: PlayerAttack.c
	Project name: Neon 'n' Slash
	Author: Kalby Jang
	
	All content © 2018 DigiPen (USA) Corporation, all rights reserved.
*/
/********************************************************************************/
#include "stdafx.h"
#include "BossTornado.h"
#include "Animation.h"
#include "Player.h"
#include "AEVec2.h"
#include "Emitter.h"
#include "Sprite.h"

#define MAX_TORNADOS 5
#define TORNADO_SPRITE_SIZE 70
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

//-----------------------------------------------------------------------------
// Private structures
//-----------------------------------------------------------------------------
typedef struct Tornado
{
  Player_Object* target;
 
  bool isActive;

  TornadoStates tornadoState;

  // Steering physics
  AEVec2 position;
  AEVec2 velocity;
  float  maxSpeed;
  float  mass;
  AEVec2 steering;

  // Tornado Effect
  float captureRadius;
  float absorbtionRadius;

  // Behavior timers
  float chaseTime;
  float trapTime;
  float curTime;

  SpritePtr sprite;

  AnimationPtr animation;

}Tornado, *TornadoPtr;


//-----------------------------------------------------------------------------
// Private Variables
//-----------------------------------------------------------------------------
static float vortexStrength = 300.0f;
//static TornadoPtr tornadoList[MAX_TORNADOS];    

static AEGfxVertexList* tornadoMesh;                  
static AEGfxTexture*    tornadoTexture;         
static SpriteSourcePtr  tornadoSpriteSource;

//-----------------------------------------------------------------------------
// Private declarations
//-----------------------------------------------------------------------------
static void TornadoBehaviorSuckPlayer(TornadoPtr tornado, f64 dt);
static void TornadoBehaviorFollow(TornadoPtr tornado, f64 dt);
static void TornadoBehaviorTrapPlayer(TornadoPtr tornado);
static void TornadoBehaviorFreePlayer(TornadoPtr tornado);
static SpritePtr CreateNewSprite();


//-----------------------------------------------------------------------------
// Public Functions
//-----------------------------------------------------------------------------
TornadoPtr TornadoCreate(Player_Object* player)
{
  TornadoPtr newTornado = (TornadoPtr) calloc (1, sizeof(struct Tornado));

  if (!newTornado) return NULL;

  newTornado->target                    = player;
  newTornado->tornadoState              = sFollow;
  AEVec2Set(&newTornado->position, 0.0f, 0.0f);
  AEVec2Set(&newTornado->velocity, 0.0f, 0.0f);
  newTornado->maxSpeed                  = 300.0f;
  newTornado->mass                      = 3000.0f;
  AEVec2Set(&newTornado->steering, 0.0f, 0.0f);
  newTornado->captureRadius             = 70.0f;
  newTornado->absorbtionRadius          = 40000.0f;
  newTornado->chaseTime                 = 8.0f;
  newTornado->trapTime                  = 3.0f;

  newTornado->sprite = CreateNewSprite();
  newTornado->animation = AnimationCreate(newTornado->sprite);

  AnimationPlay(newTornado->animation, SpriteSourceGetFrameCount(tornadoSpriteSource), 0.10f, 1);

  return newTornado;
}

void TornadoFree(TornadoPtr* tornado)
{
  if((*tornado)->sprite)
    SpriteFree(&(*tornado)->sprite);

  if ((*tornado)->animation)
	  AnimationFree(&(*tornado)->animation);


  free(*tornado);
  *tornado = NULL;
}

// Loads the textures into the game
void TornadoLoad()
{
  tornadoMesh         = CreateQuadMesh(TORNADO_SPRITE_SIZE, TORNADO_SPRITE_SIZE, 1.0f / 3.0f, 1.0f / 3.0f);
  tornadoTexture      = AEGfxTextureLoad("Assets\\Tornado.png");
  tornadoSpriteSource = SpriteSourceCreate(3, 3, tornadoTexture);
}

// Frees the mesh 
 void TornadoUnload()
{
  AEGfxMeshFree(tornadoMesh);
  SpriteSourceFree(&tornadoSpriteSource);
  AEGfxTextureUnload(tornadoTexture);
}




//-----------------------------------------------------------------------------
// Gameplay Functions
//-----------------------------------------------------------------------------
void TornadoLaunch(TornadoPtr tornado, AEVec2 bossPosition)
{
	AEVec2Set(&tornado->position, bossPosition.x, bossPosition.y);
	AEVec2Set(&tornado->velocity, 0, 0);
	tornado->isActive = true;
	tornado->tornadoState = sProjectile;
}


void TornadoUpdate(TornadoPtr tornado, f64 dt)
{
  if (!tornado || !tornado->isActive) return;
  
  float distance;

   distance = AEVec2Distance(PlayerGetPosition(tornado->target), TornadoGetPosition(tornado));



  /*int t;

  for (t = 0; t < MAX_TORNADOS; t++)
  {*/


    switch (tornado->tornadoState)
    {
    case sProjectile:

      tornado->curTime = tornado->chaseTime;
      tornado->tornadoState = sFollow;

      break;
    case sFollow:

      TornadoBehaviorFollow(tornado, dt);
      TornadoBehaviorSuckPlayer(tornado, dt);

      // Update timer
      tornado->curTime -= (float)dt;

      // When tornado captures the player
      if (distance < tornado->captureRadius)
      {
        tornado->tornadoState = /*sTrap*/ sFollow;
        tornado->curTime = tornado->trapTime;
      }
      // When tornado times out
      else if (tornado->curTime <= 0.0f)
      {
        tornado->tornadoState = sDissipate;
        tornado->curTime = tornado->trapTime;
      }

      break;
    /*case sCloseIn:

      break;*/
    case sTrap:

      TornadoBehaviorTrapPlayer(tornado);


      // When tornado times out
      tornado->curTime -= (float)dt;
      if (tornado->curTime <= 0.0f)
      {
        tornado->tornadoState = sDissipate;
        tornado->curTime = tornado->trapTime;
      }

      break;
    case sDissipate:

      TornadoBehaviorFreePlayer(tornado);

      tornado->isActive = false;

    /*  tornado->curTime -= (float)dt;
      if (tornado->curTime <= 0.0f)
      {
        tornado->tornadoState = sProjectile;
      }      */  
      
      break;
    }

	AnimationUpdate(tornado->animation, (float)dt);
	CreateChargeParticles(tornado->target, *TornadoGetPosition(tornado));


}

void TornadoDraw(TornadoPtr tornado)
{
	if (tornado && tornado->isActive)
	{
		SpriteDraw(tornado->sprite, tornado->position);
	}
}


//-----------------------------------------------------------------------------
// Behavior Functions
//-----------------------------------------------------------------------------
static void TornadoBehaviorSuckPlayer(TornadoPtr tornado, f64 dt)
{

  AEVec2 tornadoPos = *TornadoGetPosition(tornado);
  AEVec2 playerPos = { 0 };
  AEVec2 dir = { 0 };
  AEVec2 vel = { 0 };
  float  dis;


  // Getting the direction between player and tornato
  AEVec2Set(&playerPos, Get_Player_PositionX(tornado->target), Get_Player_PositionY(tornado->target));

  AEVec2Sub(&dir, &tornadoPos, &playerPos);
  AEVec2Normalize(&dir, &dir);
  AEVec2Scale(&vel, &dir, vortexStrength);

  dis = AEVec2SquareDistance(&playerPos, &tornadoPos);

  if (dis <= tornado->absorbtionRadius * tornado->absorbtionRadius)
    Change_Player_Position(tornado->target, vel.x, vel.y, dt);

}


static void TornadoBehaviorFollow(TornadoPtr tornado, f64 dt)
{
  if (!tornado) return;

  float curSpeed;
  AEVec2 curDir;
  AEVec2 normalDir;
  AEVec2 targetPos;

  // Direction towards target
  AEVec2Set(&targetPos, Get_Player_PositionX(tornado->target), Get_Player_PositionY(tornado->target));
  AEVec2Sub(&curDir, &targetPos, &tornado->position);

  AEVec2Set(&tornado->steering, curDir.x, curDir.y);

  // Turncating to max speed
  curSpeed = AEVec2Length(&tornado->steering);
  AEVec2Normalize(&normalDir, &tornado->steering);

  curSpeed = MAX(curSpeed, tornado->maxSpeed);

  AEVec2Scale(&tornado->steering, &normalDir, curSpeed);

  // Steernig / Mass
  AEVec2Set(&tornado->steering, tornado->steering.x / tornado->mass, tornado->steering.y / tornado->mass);

  // Max of velocity + steering & max_speed
  AEVec2Add(&tornado->velocity, &tornado->velocity, &tornado->steering);

  curSpeed = AEVec2Length(&tornado->velocity);
  AEVec2Normalize(&normalDir, &tornado->velocity);

  curSpeed = MAX(curSpeed, tornado->maxSpeed);

  AEVec2Scale(&tornado->velocity, &normalDir, curSpeed * (f32)dt);

  // position += velocity
  AEVec2Add(&tornado->position, &tornado->position, &tornado->velocity);
}


static void TornadoBehaviorTrapPlayer(TornadoPtr tornado)
{
  // Applying the effects
  TornadoSetPosition(tornado, PlayerGetPosition(tornado->target));
  PlayerSetModSpeed(tornado->target, 0.4f);

}


static void TornadoBehaviorFreePlayer(TornadoPtr tornado)
{
  // Applying the effects
  TornadoSetPosition(tornado, TornadoGetPosition(tornado));
  PlayerSetModSpeed(tornado->target, 1.0f);

}


//-----------------------------------------------------------------------------
// Public Functions
//-----------------------------------------------------------------------------

// Getters
bool TornadoIsActive(TornadoPtr tornado)
{
  return (tornado ? tornado->isActive : false);
}

AEVec2* TornadoGetPosition(TornadoPtr tornado)
{
  return (tornado ? &tornado->position : NULL);
}

AEVec2* TornadoGetVelocity(TornadoPtr tornado)
{
  return (tornado ? &tornado->velocity : NULL);
}

Player_Object* TornadoGetTarget(TornadoPtr tornado)
{
  return (tornado ? tornado->target : NULL);
}

SpritePtr TornadoGetSprite(TornadoPtr tornado)
{
  return (tornado ? tornado->sprite : NULL);
}


// Setters
void TornadoSetActive(TornadoPtr tornado, bool active)
{
  if (!tornado) return;

  tornado->isActive = active;
}

void TornadoSetPosition(TornadoPtr tornado, AEVec2* position)
{
  if (!tornado) return;

    tornado->position = *position;
}

void TornadoSetVelocity(TornadoPtr tornado, AEVec2* velocity)
{
  if (!tornado) return;

  tornado->velocity = *velocity;
}

void TornadoSetTarget(TornadoPtr tornado, Player_Object* target)
{
  if (!tornado || !target) return;

  tornado->target = target;
}

void TornadoSetSprite(TornadoPtr tornado, SpritePtr sprite)
{
  if (!tornado || !sprite) return;

  tornado->sprite = sprite;
}

//-----------------------------------------------------------------------------
// Private Functions
//-----------------------------------------------------------------------------

// Handles making a new mesh
static SpritePtr CreateNewSprite()
{
  SpritePtr tornadoSprite = SpriteCreate("Tornado");

  // Setting components
  SpriteSetMesh(tornadoSprite, tornadoMesh);
  SpriteSetSpriteSource(tornadoSprite, tornadoSpriteSource);

  return tornadoSprite;
}





