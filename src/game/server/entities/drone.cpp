#include "drone.h"
#include <game/server/gamecontext.h>
#include <game/server/entities/character.h>
#include <game/server/player.h>
#include "projectile.h"

CDrone::CDrone(CGameWorld *pGameWorld, vec2 Pos, int Owner)
: CEntity(pGameWorld, CGameWorld::ENTTYPE_DRONE, Pos)
{
	m_Owner = Owner;
	m_Velocity = vec2(0, 0);
	m_LastBombTick = 0;
	for(int i = 0; i < 16; i++)
		m_IDs[i] = Server()->SnapNewId();
	GameWorld()->InsertEntity(this);
}

void CDrone::Reset()
{
	GameServer()->m_apPlayers[m_Owner]->m_pDrone = nullptr;
	for(int i = 0; i < 16; i++)
		Server()->SnapFreeId(m_IDs[i]);
	GameWorld()->RemoveEntity(this);
	delete this;
}

void CDrone::Tick()
{
	CPlayer *pOwner = GameServer()->m_apPlayers[m_Owner];
	CNetObj_PlayerInput Input = GameServer()->GetLastPlayerInput(m_Owner);
	CNetObj_PlayerInput *pInput = &Input;
	if(!pInput)
		return;

	// Movement
	vec2 Dir = vec2(pInput->m_Direction, 0);
	if(pInput->m_Jump)
		Dir.y -= 1.0f;
	if(pInput->m_Hook) // We can use hook as "down" for drone
		Dir.y += 1.0f;

	if(length(Dir) > 0.001f)
		m_Velocity += normalize(Dir) * 1.5f;

	m_Velocity *= 0.95f; // Friction
	m_Pos += m_Velocity;

	// Collisions with map
	GameServer()->Collision()->MoveBox(&m_Pos, &m_Velocity, vec2(20.0f, 20.0f), vec2(0.5f, 0.5f));

	// Bombing on LMB
	if((pInput->m_Fire & 1) && Server()->Tick() > m_LastBombTick + Server()->TickSpeed() / 2)
	{
		m_LastBombTick = Server()->Tick();
		new CProjectile(GameWorld(), WEAPON_GRENADE, m_Owner, m_Pos + vec2(0, 15), vec2(0, 5), (int)(Server()->TickSpeed() * 2.0f), false, true, SOUND_GRENADE_EXPLODE, vec2(0, 1));
		GameServer()->CreateSound(m_Pos, SOUND_GRENADE_FIRE);
	}
}

void CDrone::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	int IDIdx = 0;
    
    // 1. Central Square (4 lines)
    vec2 CoreOffsets[4] = { vec2(-8, -8), vec2(8, -8), vec2(8, 8), vec2(-8, 8) };
    for(int i = 0; i < 4; i++)
    {
        CNetObj_Laser *pObj = Server()->SnapNewItem<CNetObj_Laser>(m_IDs[IDIdx++]);
        if(!pObj) continue;
        pObj->m_X = (int)(m_Pos.x + CoreOffsets[i].x);
        pObj->m_Y = (int)(m_Pos.y + CoreOffsets[i].y);
        pObj->m_FromX = (int)(m_Pos.x + CoreOffsets[(i+1)%4].x);
        pObj->m_FromY = (int)(m_Pos.y + CoreOffsets[(i+1)%4].y);
        pObj->m_StartTick = Server()->Tick();
    }

    // 2. Arms (4 lines)
    vec2 ArmDirs[4] = { vec2(-1, -1), vec2(1, -1), vec2(1, 1), vec2(-1, 1) };
    for(int i = 0; i < 4; i++)
    {
        CNetObj_Laser *pObj = Server()->SnapNewItem<CNetObj_Laser>(m_IDs[IDIdx++]);
        if(!pObj) continue;
        vec2 Start = m_Pos + ArmDirs[i] * 8.0f;
        vec2 End = m_Pos + ArmDirs[i] * 25.0f;
        pObj->m_X = (int)End.x;
        pObj->m_Y = (int)End.y;
        pObj->m_FromX = (int)Start.x;
        pObj->m_FromY = (int)Start.y;
        pObj->m_StartTick = Server()->Tick();
    }

    // 3. Rotating Rotors at the end of arms (2 lines per arm = 8 lines)
    float Angle = Server()->Tick() * 0.2f;
    for(int i = 0; i < 4; i++)
    {
        vec2 Tip = m_Pos + ArmDirs[i] * 25.0f;
        vec2 Rot1 = rotate(vec2(10, 0), Angle);
        vec2 Rot2 = rotate(vec2(10, 0), Angle + 3.14159f/2.0f);

        for(int j = 0; j < 2; j++)
        {
            CNetObj_Laser *pObj = Server()->SnapNewItem<CNetObj_Laser>(m_IDs[IDIdx++]);
            if(!pObj) continue;
            vec2 RDir = (j == 0) ? Rot1 : Rot2;
            pObj->m_X = (int)(Tip.x + RDir.x);
            pObj->m_Y = (int)(Tip.y + RDir.y);
            pObj->m_FromX = (int)(Tip.x - RDir.x);
            pObj->m_FromY = (int)(Tip.y - RDir.y);
            pObj->m_StartTick = Server()->Tick();
        }
    }
}
