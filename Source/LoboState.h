/********************************************************************************/
/*
	File name: LoboState.h
	Project name: Neon 'n' Slash
	Author: Victor Soto, Jack O'Brien, Austin Sherrod, Aditya Ravichandran, Kalby Jang

	All content © 2018 DigiPen (USA) Corporation, all rights reserved.
*/
/********************************************************************************/
#pragma once
#include "Boss.h"
#include "Player.h"
#include "AETypes.h"
#include "Bomb.h"
#include "BossTornado.h"

typedef enum LoboStates
{
	IDLE,
	PASSIVE,
	ACTIVE
}LoboStates;

void LoboStateLoad(BossPtr *Lobo);

void LoboStateInit(BossPtr Lobo);

void LoboStateUpdate(Player_Object * Player, BossPtr Lobo, f64 dt);

void LoboStateShutdown(BossPtr Lobo);

void LoboStateUnload(BossPtr *Lobo);

// Plays the awaken animation
void LoboPlayAwaken(BossPtr Lobo);

// Plays the idel animation
void LoboPlayIdle(BossPtr Lobo);

// Plays the Melee animation to the left
void LoboPlayMeleeLeft(BossPtr Lobo);

// Plays the Melee animation to the right
void LoboPlayMeleeRight(BossPtr Lobo);

// Plays the shoot animation
void LoboPlayShoot(BossPtr Lobo);

// Plays the AOE animation
void LoboPlayAOE(BossPtr Lobo);

void LoboPlayDeath(BossPtr Lobo);

void LoboPlayAfterDeath(BossPtr Lobo);

TornadoPtr LoboStateGetCurrentTornado(void);

bombPtr LoboStateGetCurrentBomb(void);