/* DDNet Zombie Event - survival game mode */
#ifndef GAME_SERVER_ZOMBIEEVENT_H
#define GAME_SERVER_ZOMBIEEVENT_H

#include <base/vmath.h>
#include <engine/shared/protocol.h>
#include <vector>

class CEntity;
class CGameContext;

class CZombieEvent
{
public:
	enum EPhase
	{
		PHASE_NONE = 0,
		PHASE_BUILD,
		PHASE_ATTACK
	};

	enum EStructure
	{
		STRUCT_TURRET = 0,
		STRUCT_TANK,
		STRUCT_CANNON,
		NUM_STRUCTURES
	};

	CZombieEvent(CGameContext *pGameServer);

	// Core methods
	void Start();
	void Tick();
	void End();

	// State queries
	bool IsActive() const { return m_Active; }
	EPhase GetPhase() const { return m_Phase; }
	int GetRemainingTime() const;

	// Team queries
	bool IsBuilder(int ClientId) const;
	bool IsZombie(int ClientId) const;
	int GetBuilderCount() const;
	int GetZombieCount() const;

	// Builder mechanics
	int GetBuildPoints(int ClientId) const;
	EStructure GetSelectedStructure(int ClientId) const;
	void CycleStructure(int ClientId);
	bool TryPlaceStructure(int ClientId, vec2 Pos);
	int GetStructureCost(EStructure Type) const;

	// Infection
	void InfectBuilder(int ClientId);

	// For testing - allows direct team assignment
	void AssignTeamsForTesting(const bool *pIsBuilder, int NumPlayers);

private:
	CGameContext *m_pGameServer;

	// Event state
	bool m_Active;
	EPhase m_Phase;
	int m_StartTick;
	int m_PhaseStartTick;
	int m_LastBroadcastTick;

	// Player data
	bool m_aIsBuilder[MAX_CLIENTS];
	int m_aBuildPoints[MAX_CLIENTS];
	EStructure m_aSelectedStructure[MAX_CLIENTS];

	// Placed structures
	std::vector<CEntity *> m_vpStructures;

	// Internal methods
	void AssignTeams();
	void StartBuildPhase();
	void StartAttackPhase();
	void CheckWinCondition();
	void Cleanup();
	void BroadcastTime();
	void FreezeZombies();
	void UnfreezeZombies();
};

#endif
