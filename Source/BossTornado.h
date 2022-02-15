/********************************************************************************/
/*
File name: PlayerAttack.c
Project name: Neon 'n' Slash
Author: Kalby Jang

All content © 2018 DigiPen (USA) Corporation, all rights reserved.
*/
/********************************************************************************/
#pragma once
#include "Boss.h"
#include "Player.h"


typedef struct Tornado *TornadoPtr;

//-----------------------------------------------------------------------------
// Private const
//-----------------------------------------------------------------------------
typedef enum TornadoStates
{
  sProjectile,
  sFollow,
  /*sCloseIn,*/
  sTrap,
  sDissipate,

  sStateState = sFollow,

}TornadoStates;



//-----------------------------------------------------------------------------
// Public Functions
//-----------------------------------------------------------------------------
TornadoPtr TornadoCreate(Player_Object* player);

void TornadoFree(TornadoPtr* tornado);

void TornadoUpdate(TornadoPtr tornado, f64 dt);

void TornadoLoad();

void TornadoUnload();


// Tornado Methods
void TornadoLaunch(TornadoPtr tonrnado, AEVec2 bossPosition);

void TornadoUpdate(TornadoPtr tornado, f64 dt);

void TornadoFreePlayer(TornadoPtr tornado, Player_Object* player);

void TornadoDraw(TornadoPtr tornado);


// Getters
bool TornadoIsActive(TornadoPtr tornado);

AEVec2* TornadoGetPosition(TornadoPtr tornado);

AEVec2* TornadoGetVelocity(TornadoPtr tornado);

Player_Object* TornadoGetTarget(TornadoPtr tornado);

SpritePtr TornadoGetSprite(TornadoPtr tornado);

// Setters
void TornadoSetActive(TornadoPtr tornado, bool active);

void TornadoSetPosition(TornadoPtr tornado, AEVec2* position);

void TornadoSetVelocity(TornadoPtr tornado, AEVec2* velocity);

void TornadoSetTarget(TornadoPtr tornado, Player_Object* target);

void TornadoSetSprite(TornadoPtr tornado, SpritePtr sprite);