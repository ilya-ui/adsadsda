/* DDNet Cannon Entity - Sucks in players and shoots them */
#include "cannon.h"
#include "character.h"

#include <game/server/gamecontext.h>
#include <game/server/player.h>
#include <generated/protocol.h>

CCannon::CCannon(CGameWorld *pGameWorld, vec2 Pos, int Owner) :
	CEntity(pGameWorld, CGameWorld::ENTTYPE_PICKUP, Pos, 60)
{
	m_Owner = Owner;
	m_Angle = 0;
	m_NumLoaded = 0;
	m_FireCooldown = 0;
	m_SuckCooldown = 0;
	
	for(int i = 0; i < MAX_LOADED; i++)
		m_aLoadedPlayers[i] = -1;

	for(int i = 0; i < 20; i++)
		m_aIds[i] = Server()->SnapNewId();

	GameWorld()->InsertEntity(this);
}

void CCannon::Reset()
{
	// Release all loaded players
	for(int i = 0; i < m_NumLoaded; i++)
	{
		if(m_aLoadedPlayers[i] >= 0 && m_aLoadedPlayers[i] < MAX_CLIENTS)
		{
			CPlayer *pPlayer = GameServer()->m_apPlayers[m_aLoadedPlayers[i]];
			if(pPlayer)
			{
				CCharacter *pChr = pPlayer->GetCharacter();
				if(pChr)
				{
					pChr->UnFreeze();
				}
			}
		}
	}
	
	RemoveOwner();

	for(int i = 0; i < 20; i++)
		Server()->SnapFreeId(m_aIds[i]);

	Destroy();
}

void CCannon::SetOwner(int ClientId)
{
	m_Owner = ClientId;
	if(ClientId >= 0 && ClientId < MAX_CLIENTS)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[ClientId];
		if(pPlayer)
		{
			pPlayer->m_HasCannon = true;
			pPlayer->m_pCannon = this;
		}
	}
}

void CCannon::RemoveOwner()
{
	// Release all loaded players first
	for(int i = 0; i < m_NumLoaded; i++)
	{
		if(m_aLoadedPlayers[i] >= 0 && m_aLoadedPlayers[i] < MAX_CLIENTS)
		{
			CPlayer *pPlayer = GameServer()->m_apPlayers[m_aLoadedPlayers[i]];
			if(pPlayer)
			{
				CCharacter *pChr = pPlayer->GetCharacter();
				if(pChr)
				{
					pChr->UnFreeze();
					// Spawn them near cannon
					vec2 SpawnPos = m_Pos + vec2((i - m_NumLoaded/2) * 40, -50);
					pChr->SetPosition(SpawnPos);
					pChr->m_Pos = SpawnPos;
					pChr->m_PrevPos = SpawnPos;
				}
			}
		}
		m_aLoadedPlayers[i] = -1;
	}
	m_NumLoaded = 0;
	
	if(m_Owner >= 0 && m_Owner < MAX_CLIENTS)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[m_Owner];
		if(pPlayer)
		{
			pPlayer->m_HasCannon = false;
			pPlayer->m_pCannon = nullptr;
		}
	}
	m_Owner = -1;
}

void CCannon::SuckPlayer()
{
	if(m_NumLoaded >= MAX_LOADED)
		return;
	
	// Find closest player in front of cannon
	vec2 Dir = vec2(cos(m_Angle), sin(m_Angle));
	float BestDist = 150.0f; // Suck range
	int BestPlayer = -1;
	
	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		if(i == m_Owner)
			continue;
			
		// Check if already loaded
		bool AlreadyLoaded = false;
		for(int j = 0; j < m_NumLoaded; j++)
		{
			if(m_aLoadedPlayers[j] == i)
			{
				AlreadyLoaded = true;
				break;
			}
		}
		if(AlreadyLoaded)
			continue;
		
		CCharacter *pChr = GameServer()->GetPlayerChar(i);
		if(!pChr)
			continue;
		
		vec2 ToPlayer = pChr->GetPos() - m_Pos;
		float Dist = length(ToPlayer);
		
		if(Dist < BestDist)
		{
			BestDist = Dist;
			BestPlayer = i;
		}
	}
	
	if(BestPlayer >= 0)
	{
		// Suck player into cannon
		m_aLoadedPlayers[m_NumLoaded] = BestPlayer;
		m_NumLoaded++;
		
		CCharacter *pChr = GameServer()->GetPlayerChar(BestPlayer);
		if(pChr)
		{
			pChr->Freeze();
			// Hide them inside cannon
			pChr->SetPosition(m_Pos);
			pChr->m_Pos = m_Pos;
			pChr->m_PrevPos = m_Pos;
		}
		
		char aBuf[64];
		str_format(aBuf, sizeof(aBuf), "Loaded %s into cannon! (%d/%d)", 
			GameServer()->Server()->ClientName(BestPlayer), m_NumLoaded, MAX_LOADED);
		GameServer()->SendChatTarget(m_Owner, aBuf);
		
		GameServer()->CreateSound(m_Pos, SOUND_PICKUP_ARMOR);
	}
}

void CCannon::FirePlayer()
{
	if(m_NumLoaded <= 0)
		return;
	
	// Fire the first loaded player
	int PlayerToFire = m_aLoadedPlayers[0];
	
	// Shift remaining players
	for(int i = 0; i < m_NumLoaded - 1; i++)
		m_aLoadedPlayers[i] = m_aLoadedPlayers[i + 1];
	m_aLoadedPlayers[m_NumLoaded - 1] = -1;
	m_NumLoaded--;
	
	if(PlayerToFire < 0 || PlayerToFire >= MAX_CLIENTS)
		return;
	
	CCharacter *pChr = GameServer()->GetPlayerChar(PlayerToFire);
	if(!pChr)
		return;
	
	// Unfreeze and launch!
	pChr->UnFreeze();
	
	vec2 Dir = vec2(cos(m_Angle), sin(m_Angle));
	vec2 LaunchPos = m_Pos + Dir * 130.0f;
	
	pChr->SetPosition(LaunchPos);
	pChr->m_Pos = LaunchPos;
	pChr->m_PrevPos = LaunchPos;
	
	// Launch with high velocity
	float LaunchSpeed = 35.0f;
	pChr->SetVelocity(Dir * LaunchSpeed);
	
	GameServer()->CreateSound(m_Pos, SOUND_GRENADE_FIRE);
	GameServer()->CreateSound(m_Pos, SOUND_HOOK_ATTACH_PLAYER);
	
	char aBuf[64];
	str_format(aBuf, sizeof(aBuf), "FIRED %s! (%d left)", 
		GameServer()->Server()->ClientName(PlayerToFire), m_NumLoaded);
	GameServer()->SendChatTarget(m_Owner, aBuf);
}

void CCannon::Tick()
{
	if(m_FireCooldown > 0)
		m_FireCooldown--;
	if(m_SuckCooldown > 0)
		m_SuckCooldown--;

	if(m_Owner < 0)
		return;

	CPlayer *pOwner = GameServer()->m_apPlayers[m_Owner];
	if(!pOwner)
	{
		RemoveOwner();
		return;
	}

	CCharacter *pOwnerChr = pOwner->GetCharacter();
	if(!pOwnerChr)
	{
		RemoveOwner();
		return;
	}

	// Follow owner - position cannon slightly above player
	m_Pos = pOwnerChr->GetPos() + vec2(0, -35);
	
	// Get input
	CNetObj_PlayerInput Input = GameServer()->GetLastPlayerInput(m_Owner);
	
	// Update angle to point at cursor
	vec2 CursorPos = m_Pos + vec2(Input.m_TargetX, Input.m_TargetY);
	vec2 Dir = CursorPos - m_Pos;
	m_Angle = atan2(Dir.y, Dir.x);
	
	// RMB to suck players
	if((Input.m_Hook) && m_SuckCooldown <= 0)
	{
		SuckPlayer();
		m_SuckCooldown = 15; // Cooldown between sucks
	}
	
	// LMB to fire
	if((Input.m_Fire & 1) && m_FireCooldown <= 0 && m_NumLoaded > 0)
	{
		FirePlayer();
		m_FireCooldown = 20; // Cooldown between shots
	}
	
	// Keep loaded players hidden - position them 1 block above cannon
	vec2 HidePos = m_Pos + vec2(0, -32);
	for(int i = 0; i < m_NumLoaded; i++)
	{
		if(m_aLoadedPlayers[i] >= 0 && m_aLoadedPlayers[i] < MAX_CLIENTS)
		{
			CCharacter *pChr = GameServer()->GetPlayerChar(m_aLoadedPlayers[i]);
			if(pChr)
			{
				pChr->SetPosition(HidePos);
				pChr->m_Pos = HidePos;
				pChr->m_PrevPos = HidePos;
				pChr->ResetVelocity();
			}
		}
	}
}

void CCannon::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	int StartTick = Server()->Tick() - 2;
	CNetObj_Laser *pObj;

	vec2 Dir = vec2(cos(m_Angle), sin(m_Angle));
	vec2 Perp = vec2(-Dir.y, Dir.x);
	
	float BarrelLen = 120.0f;
	float BarrelWidth = 30.0f;
	float BodySize = 45.0f;
	
	// Cannon body (circle-ish with lines)
	vec2 BodyCenter = m_Pos;
	
	// Body outline (hexagon)
	int idx = 0;
	for(int i = 0; i < 6 && idx < 20; i++)
	{
		float a1 = (float)i / 6.0f * 2.0f * pi;
		float a2 = (float)(i + 1) / 6.0f * 2.0f * pi;
		vec2 p1 = BodyCenter + vec2(cos(a1), sin(a1)) * BodySize;
		vec2 p2 = BodyCenter + vec2(cos(a2), sin(a2)) * BodySize;
		
		pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[idx++]);
		if(pObj)
		{
			pObj->m_X = (int)p2.x;
			pObj->m_Y = (int)p2.y;
			pObj->m_FromX = (int)p1.x;
			pObj->m_FromY = (int)p1.y;
			pObj->m_StartTick = StartTick;
		}
	}
	
	// Barrel - top line
	vec2 BarrelStart = m_Pos + Perp * BarrelWidth;
	vec2 BarrelEnd = m_Pos + Dir * BarrelLen + Perp * BarrelWidth;
	
	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[idx++]);
	if(pObj)
	{
		pObj->m_X = (int)BarrelEnd.x;
		pObj->m_Y = (int)BarrelEnd.y;
		pObj->m_FromX = (int)BarrelStart.x;
		pObj->m_FromY = (int)BarrelStart.y;
		pObj->m_StartTick = StartTick;
	}
	
	// Barrel - bottom line
	BarrelStart = m_Pos - Perp * BarrelWidth;
	BarrelEnd = m_Pos + Dir * BarrelLen - Perp * BarrelWidth;
	
	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[idx++]);
	if(pObj)
	{
		pObj->m_X = (int)BarrelEnd.x;
		pObj->m_Y = (int)BarrelEnd.y;
		pObj->m_FromX = (int)BarrelStart.x;
		pObj->m_FromY = (int)BarrelStart.y;
		pObj->m_StartTick = StartTick;
	}
	
	// Barrel - end cap
	vec2 EndTop = m_Pos + Dir * BarrelLen + Perp * BarrelWidth;
	vec2 EndBot = m_Pos + Dir * BarrelLen - Perp * BarrelWidth;
	
	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[idx++]);
	if(pObj)
	{
		pObj->m_X = (int)EndBot.x;
		pObj->m_Y = (int)EndBot.y;
		pObj->m_FromX = (int)EndTop.x;
		pObj->m_FromY = (int)EndTop.y;
		pObj->m_StartTick = StartTick;
	}
	
	// Show loaded count with small lines inside barrel
	for(int i = 0; i < m_NumLoaded && idx < 20; i++)
	{
		float offset = 20.0f + i * 12.0f;
		vec2 LineTop = m_Pos + Dir * offset + Perp * (BarrelWidth - 5);
		vec2 LineBot = m_Pos + Dir * offset - Perp * (BarrelWidth - 5);
		
		pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[idx++]);
		if(pObj)
		{
			pObj->m_X = (int)LineBot.x;
			pObj->m_Y = (int)LineBot.y;
			pObj->m_FromX = (int)LineTop.x;
			pObj->m_FromY = (int)LineTop.y;
			pObj->m_StartTick = StartTick;
		}
	}
}
