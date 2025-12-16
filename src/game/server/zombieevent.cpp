/* DDNet Zombie Event - survival game mode */
#include "zombieevent.h"

#include "gamecontext.h"
#include "entities/character.h"
#include "entities/autoturret.h"
#include "entities/autotank.h"
#include "entities/autocannon.h"
#include "player.h"

#include <engine/server.h>
#include <algorithm>
#include <random>

// Timing constants (in ticks at 50 ticks/sec)
static const int BUILD_PHASE_DURATION = 150 * 50;  // 2.5 minutes
static const int ATTACK_PHASE_DURATION = 150 * 50; // 2.5 minutes
static const int BROADCAST_INTERVAL = 30 * 50;     // 30 seconds

// Structure costs
static const int TURRET_COST = 10;
static const int TANK_COST = 30;
static const int CANNON_COST = 25;

// Initial build points
static const int INITIAL_BUILD_POINTS = 100;

CZombieEvent::CZombieEvent(CGameContext *pGameServer) :
	m_pGameServer(pGameServer),
	m_Active(false),
	m_Phase(PHASE_NONE),
	m_StartTick(0),
	m_PhaseStartTick(0),
	m_LastBroadcastTick(0)
{
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		m_aIsBuilder[i] = false;
		m_aBuildPoints[i] = 0;
		m_aSelectedStructure[i] = STRUCT_TURRET;
	}
}

void CZombieEvent::Start()
{
	if(m_Active)
	{
		m_pGameServer->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "zombie", "Event already running!");
		return;
	}

	// Count active players
	int NumPlayers = 0;
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_pGameServer->m_apPlayers[i])
			NumPlayers++;
	}

	if(NumPlayers < 2)
	{
		m_pGameServer->Console()->Print(IConsole::OUTPUT_LEVEL_STANDARD, "zombie", "Need at least 2 players!");
		return;
	}

	m_Active = true;
	m_StartTick = m_pGameServer->Server()->Tick();
	
	AssignTeams();
	StartBuildPhase();

	m_pGameServer->SendChat(-1, TEAM_ALL, "*** ZOMBIE EVENT STARTED! ***");
}


void CZombieEvent::End()
{
	if(!m_Active)
		return;

	Cleanup();
	m_Active = false;
	m_Phase = PHASE_NONE;
}

void CZombieEvent::Tick()
{
	if(!m_Active)
		return;

	int CurrentTick = m_pGameServer->Server()->Tick();
	int PhaseElapsed = CurrentTick - m_PhaseStartTick;

	// Phase transitions
	if(m_Phase == PHASE_BUILD && PhaseElapsed >= BUILD_PHASE_DURATION)
	{
		StartAttackPhase();
	}
	else if(m_Phase == PHASE_ATTACK && PhaseElapsed >= ATTACK_PHASE_DURATION)
	{
		// Time expired - builders win if any survive
		if(GetBuilderCount() > 0)
		{
			m_pGameServer->SendChat(-1, TEAM_ALL, "*** BUILDERS WIN! Time expired! ***");
		}
		else
		{
			m_pGameServer->SendChat(-1, TEAM_ALL, "*** ZOMBIES WIN! All builders infected! ***");
		}
		End();
		return;
	}

	// Periodic time broadcast
	if(CurrentTick - m_LastBroadcastTick >= BROADCAST_INTERVAL)
	{
		BroadcastTime();
		m_LastBroadcastTick = CurrentTick;
	}

	// Check win condition during attack phase
	if(m_Phase == PHASE_ATTACK)
	{
		CheckWinCondition();
	}
}

void CZombieEvent::AssignTeams()
{
	// Collect active player IDs
	std::vector<int> PlayerIds;
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_pGameServer->m_apPlayers[i])
			PlayerIds.push_back(i);
	}

	// Shuffle players randomly
	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(PlayerIds.begin(), PlayerIds.end(), g);

	// Split 50/50 - first half are builders
	int NumBuilders = (int)PlayerIds.size() / 2;
	if(PlayerIds.size() % 2 == 1)
	{
		// Odd number - randomly decide if extra player is builder or zombie
		if(g() % 2 == 0)
			NumBuilders++;
	}

	// Reset all players
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		m_aIsBuilder[i] = false;
		m_aBuildPoints[i] = 0;
		m_aSelectedStructure[i] = STRUCT_TURRET;
	}

	// Assign teams
	for(int i = 0; i < (int)PlayerIds.size(); i++)
	{
		int ClientId = PlayerIds[i];
		if(i < NumBuilders)
		{
			m_aIsBuilder[ClientId] = true;
			m_aBuildPoints[ClientId] = INITIAL_BUILD_POINTS;
			m_pGameServer->SendChatTarget(ClientId, "You are a BUILDER! Place defenses with hammer.");
		}
		else
		{
			m_aIsBuilder[ClientId] = false;
			m_pGameServer->SendChatTarget(ClientId, "You are a ZOMBIE! Wait for attack phase...");
		}
	}
}

void CZombieEvent::AssignTeamsForTesting(const bool *pIsBuilder, int NumPlayers)
{
	// Reset all
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		m_aIsBuilder[i] = false;
		m_aBuildPoints[i] = 0;
		m_aSelectedStructure[i] = STRUCT_TURRET;
	}

	// Apply test assignments
	for(int i = 0; i < NumPlayers && i < MAX_CLIENTS; i++)
	{
		m_aIsBuilder[i] = pIsBuilder[i];
		if(pIsBuilder[i])
			m_aBuildPoints[i] = INITIAL_BUILD_POINTS;
	}
}


void CZombieEvent::StartBuildPhase()
{
	m_Phase = PHASE_BUILD;
	m_PhaseStartTick = m_pGameServer->Server()->Tick();
	m_LastBroadcastTick = m_PhaseStartTick;

	m_pGameServer->SendChat(-1, TEAM_ALL, "*** BUILD PHASE - 2:30 ***");
	FreezeZombies();
}

void CZombieEvent::StartAttackPhase()
{
	m_Phase = PHASE_ATTACK;
	m_PhaseStartTick = m_pGameServer->Server()->Tick();
	m_LastBroadcastTick = m_PhaseStartTick;

	m_pGameServer->SendChat(-1, TEAM_ALL, "*** ATTACK PHASE - Zombies released! ***");
	UnfreezeZombies();
}

void CZombieEvent::CheckWinCondition()
{
	if(GetBuilderCount() == 0)
	{
		m_pGameServer->SendChat(-1, TEAM_ALL, "*** ZOMBIES WIN! All builders infected! ***");
		End();
	}
}

void CZombieEvent::Cleanup()
{
	// Remove all placed structures
	for(CEntity *pEnt : m_vpStructures)
	{
		if(pEnt)
			pEnt->Destroy();
	}
	m_vpStructures.clear();

	// Reset player states
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		m_aIsBuilder[i] = false;
		m_aBuildPoints[i] = 0;
		m_aSelectedStructure[i] = STRUCT_TURRET;

		// Unfreeze any frozen zombies
		CCharacter *pChr = m_pGameServer->GetPlayerChar(i);
		if(pChr)
		{
			pChr->UnFreeze();
		}
	}
}

void CZombieEvent::BroadcastTime()
{
	int PhaseElapsed = m_pGameServer->Server()->Tick() - m_PhaseStartTick;
	int PhaseDuration = (m_Phase == PHASE_BUILD) ? BUILD_PHASE_DURATION : ATTACK_PHASE_DURATION;
	int RemainingTicks = PhaseDuration - PhaseElapsed;
	int RemainingSeconds = RemainingTicks / 50;

	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "%s Phase: %d:%02d remaining",
		m_Phase == PHASE_BUILD ? "Build" : "Attack",
		RemainingSeconds / 60, RemainingSeconds % 60);

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_pGameServer->m_apPlayers[i])
			m_pGameServer->SendBroadcast(aBuf, i);
	}
}

void CZombieEvent::FreezeZombies()
{
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(!m_aIsBuilder[i] && m_pGameServer->m_apPlayers[i])
		{
			CCharacter *pChr = m_pGameServer->GetPlayerChar(i);
			if(pChr)
			{
				pChr->Freeze();
			}
		}
	}
}

void CZombieEvent::UnfreezeZombies()
{
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(!m_aIsBuilder[i] && m_pGameServer->m_apPlayers[i])
		{
			CCharacter *pChr = m_pGameServer->GetPlayerChar(i);
			if(pChr)
			{
				pChr->UnFreeze();
				// TODO: Apply 1.3x speed boost in later task
			}
		}
	}
}

int CZombieEvent::GetRemainingTime() const
{
	if(!m_Active)
		return 0;

	int TotalElapsed = m_pGameServer->Server()->Tick() - m_StartTick;
	int TotalDuration = BUILD_PHASE_DURATION + ATTACK_PHASE_DURATION;
	int RemainingTicks = TotalDuration - TotalElapsed;
	return RemainingTicks / 50; // Return seconds
}

bool CZombieEvent::IsBuilder(int ClientId) const
{
	if(ClientId < 0 || ClientId >= MAX_CLIENTS)
		return false;
	return m_aIsBuilder[ClientId];
}

bool CZombieEvent::IsZombie(int ClientId) const
{
	if(ClientId < 0 || ClientId >= MAX_CLIENTS)
		return false;
	if(!m_pGameServer->m_apPlayers[ClientId])
		return false;
	return !m_aIsBuilder[ClientId];
}

int CZombieEvent::GetBuilderCount() const
{
	int Count = 0;
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(m_aIsBuilder[i] && m_pGameServer->m_apPlayers[i])
			Count++;
	}
	return Count;
}

int CZombieEvent::GetZombieCount() const
{
	int Count = 0;
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(!m_aIsBuilder[i] && m_pGameServer->m_apPlayers[i])
			Count++;
	}
	return Count;
}

int CZombieEvent::GetBuildPoints(int ClientId) const
{
	if(ClientId < 0 || ClientId >= MAX_CLIENTS)
		return 0;
	return m_aBuildPoints[ClientId];
}

CZombieEvent::EStructure CZombieEvent::GetSelectedStructure(int ClientId) const
{
	if(ClientId < 0 || ClientId >= MAX_CLIENTS)
		return STRUCT_TURRET;
	return m_aSelectedStructure[ClientId];
}

void CZombieEvent::CycleStructure(int ClientId)
{
	if(ClientId < 0 || ClientId >= MAX_CLIENTS)
		return;
	if(!m_aIsBuilder[ClientId])
		return;

	m_aSelectedStructure[ClientId] = (EStructure)((m_aSelectedStructure[ClientId] + 1) % NUM_STRUCTURES);

	const char *aNames[] = {"Turret (10pts)", "Tank (30pts)", "Cannon (25pts)"};
	char aBuf[64];
	str_format(aBuf, sizeof(aBuf), "Selected: %s", aNames[m_aSelectedStructure[ClientId]]);
	m_pGameServer->SendChatTarget(ClientId, aBuf);
}

int CZombieEvent::GetStructureCost(EStructure Type) const
{
	switch(Type)
	{
	case STRUCT_TURRET: return TURRET_COST;
	case STRUCT_TANK: return TANK_COST;
	case STRUCT_CANNON: return CANNON_COST;
	default: return 0;
	}
}

bool CZombieEvent::TryPlaceStructure(int ClientId, vec2 Pos)
{
	if(!m_Active || m_Phase != PHASE_BUILD)
		return false;
	if(!IsBuilder(ClientId))
		return false;

	EStructure Type = m_aSelectedStructure[ClientId];
	int Cost = GetStructureCost(Type);

	if(m_aBuildPoints[ClientId] < Cost)
	{
		char aBuf[64];
		str_format(aBuf, sizeof(aBuf), "Not enough points! Need %d, have %d", Cost, m_aBuildPoints[ClientId]);
		m_pGameServer->SendChatTarget(ClientId, aBuf);
		return false;
	}

	// Deduct points
	m_aBuildPoints[ClientId] -= Cost;

	// Spawn the structure entity
	CEntity *pStructure = nullptr;
	switch(Type)
	{
	case STRUCT_TURRET:
		pStructure = new CAutoTurret(&m_pGameServer->m_World, Pos, ClientId);
		break;
	case STRUCT_TANK:
		pStructure = new CAutoTank(&m_pGameServer->m_World, Pos, ClientId);
		break;
	case STRUCT_CANNON:
		pStructure = new CAutoCannon(&m_pGameServer->m_World, Pos, ClientId);
		break;
	default:
		break;
	}

	if(pStructure)
		m_vpStructures.push_back(pStructure);

	char aBuf[64];
	str_format(aBuf, sizeof(aBuf), "Placed structure! %d points remaining", m_aBuildPoints[ClientId]);
	m_pGameServer->SendChatTarget(ClientId, aBuf);

	return true;
}

void CZombieEvent::InfectBuilder(int ClientId)
{
	if(!m_Active || m_Phase != PHASE_ATTACK)
		return;
	if(!IsBuilder(ClientId))
		return;

	m_aIsBuilder[ClientId] = false;
	m_aBuildPoints[ClientId] = 0;

	m_pGameServer->SendChatTarget(ClientId, "You have been INFECTED! You are now a zombie!");
	m_pGameServer->SendChat(-1, TEAM_ALL, "A builder has been infected!");

	CheckWinCondition();
}
