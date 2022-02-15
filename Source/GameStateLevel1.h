/********************************************************************************/
/*
	File name: GameStateLevel1.h
	Project name: Neon 'n' Slash
	Author: Victor Soto, Jack O'Brien, Austin Sherrod, Aditya Ravichandran, Kalby Jang
	
	All content © 2018 DigiPen (USA) Corporation, all rights reserved.
*/
/********************************************************************************/
#pragma once

/* Load the resources associated with the Stub game state. */
void GameStateLevel1Load(void);

/* Initialize the memory associated with the Stub game state. */
void GameStateLevel1Init(void);

/*
 * Update the Stub game state.
 * Params:
 *	 dt = Change in time (in seconds) since the last game loop.
 */
void GameStateLevel1Update(f64 dt);

/* Shutdown any memory associated with the Stub game state. */
void GameStateLevel1Shutdown(void);

/* Unload the resources associated with the Stub game state. */
void GameStateLevel1Unload(void);

