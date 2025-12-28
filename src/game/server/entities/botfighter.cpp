#include "botfighter.h"
#include "character.h"
#include <game/server/gamecontext.h>
#include <game/server/player.h>
#include <generated/protocol.h>

CBotFighter::CBotFighter(CGameWorld *pGameWorld, vec2 Pos) :
	CEntity(pGameWorld, CGameWorld::ENTTYPE_LASER, Pos, 28) // Use laser type for now or pick appropriate
{
	m_Velocity = vec2(0, 0);
    m_JumpTick = 0;
    m_AttackTick = 0;
    m_MoveTick = 0;
    m_Dir = 0;

	for(int i = 0; i < 16; i++)
		m_Ids[i] = Server()->SnapNewId();

	GameWorld()->InsertEntity(this);
}

void CBotFighter::Reset()
{
    if (GameServer()->m_pBotFighter == this)
        GameServer()->m_pBotFighter = nullptr;

	Destroy();
    delete this;
}

void CBotFighter::Destroy()
{
	for(int i = 0; i < 16; i++)
		Server()->SnapFreeId(m_Ids[i]);
    
    CEntity::Destroy();
}

CCharacter *CBotFighter::FindNearestTarget()
{
	CCharacter *pClosest = nullptr;
	float ClosestDist = 1000000.0f; // Large number

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[i];
		if(!pPlayer)
			continue;

        // Don't attack spectators
        if(pPlayer->GetTeam() == TEAM_SPECTATORS)
            continue;

		CCharacter *pChr = pPlayer->GetCharacter();
		if(!pChr || !pChr->IsAlive())
			continue;

		float Dist = distance(m_Pos, pChr->GetPos());
		if(Dist < ClosestDist)
		{
			ClosestDist = Dist;
			pClosest = pChr;
		}
	}

	return pClosest;
}

void CBotFighter::FireHammer()
{
    // Visual effect
    GameServer()->CreateSound(m_Pos, SOUND_HAMMER_FIRE);
    GameServer()->CreateSound(m_Pos, SOUND_HAMMER_HIT);
    
    // Logic: hit everyone in range
    for(int i = 0; i < MAX_CLIENTS; i++)
    {
        CPlayer *pPlayer = GameServer()->m_apPlayers[i];
        if(!pPlayer) continue;
        CCharacter *pChr = pPlayer->GetCharacter();
        if(!pChr || !pChr->IsAlive()) continue;
        
        vec2 Diff = pChr->GetPos() - m_Pos;
        if(length(Diff) < 50.0f) // Hammer range
        {
            // Calculate hammer force
            vec2 Dir;
            if (length(Diff) > 0)
                Dir = normalize(Diff);
            else
                Dir = vec2(0, -1);
                
            pChr->TakeDamage(Dir * vec2(10.0f, 10.0f), 3, -1, WEAPON_HAMMER);
        }
    }
}

void CBotFighter::Tick()
{
    CCharacter *pTarget = FindNearestTarget();
    
    vec2 TargetPos = m_Pos;
    bool HasTarget = false;
    
    if(pTarget)
    {
        TargetPos = pTarget->GetPos();
        HasTarget = true;
    }
    
    // Movement Logic
    if(HasTarget)
    {
        if(TargetPos.x < m_Pos.x - 40.0f)
            m_Dir = -1;
        else if(TargetPos.x > m_Pos.x + 40.0f)
            m_Dir = 1;
        else
            m_Dir = 0; // Close enough x-wise
            
        // Jump if target is above or wall ahead
        bool WallAhead = false;
        if(m_Dir != 0)
        {
             if(Collision()->CheckPoint(m_Pos + vec2(m_Dir * 40.0f, 0)))
                WallAhead = true;
        }
        
        if((TargetPos.y < m_Pos.y - 60.0f || WallAhead) && Collision()->IsSolid(m_Pos.x, m_Pos.y + 16.0f + 2.0f)) // Grounded check rough
        {
             if(m_JumpTick < Server()->Tick())
             {
                 m_Velocity.y = -12.0f;
                 m_JumpTick = Server()->Tick() + 25; // Cooldown
             }
        }
        
        // Attack
        float Dist = distance(m_Pos, TargetPos);
        if(Dist < 60.0f && m_AttackTick < Server()->Tick())
        {
            FireHammer();
            m_AttackTick = Server()->Tick() + 25; // ~0.5s cooldown
        }
    }
    else
    {
        m_Dir = 0; // Stand still if no target
    }

    // Apply movement
    float MaxSpeed = 10.0f;
    if(m_Dir != 0)
        m_Velocity.x = std::clamp(m_Velocity.x + m_Dir * 1.5f, -MaxSpeed, MaxSpeed);
    else
        m_Velocity.x *= 0.8f; // Friction
        
    m_Velocity.y += 0.5f; // Gravity
    
    // Physics
    GameServer()->Collision()->MoveBox(&m_Pos, &m_Velocity, vec2(28.0f, 28.0f), vec2(0.5f, 0.0f));
}

void CBotFighter::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

    int IDIdx = 0;
	int StartTick = Server()->Tick() - 2;
    
    // Draw "Robot" body
    vec2 Center = m_Pos;
    float W = 20.0f;
    float H = 30.0f;
    
    vec2 TL = Center + vec2(-W, -H);
    vec2 TR = Center + vec2(W, -H);
    vec2 BL = Center + vec2(-W, H);
    vec2 BR = Center + vec2(W, H);
    
    // Body Box (4 lasers)
    CNetObj_Laser *pObj;
    
    auto DrawLaser = [&](vec2 From, vec2 To) {
        if(IDIdx >= 16) return;
        pObj = Server()->SnapNewItem<CNetObj_Laser>(m_Ids[IDIdx++]);
        if(pObj)
        {
            pObj->m_X = (int)To.x;
            pObj->m_Y = (int)To.y;
            pObj->m_FromX = (int)From.x;
            pObj->m_FromY = (int)From.y;
            pObj->m_StartTick = StartTick;
        }
    };
    
    DrawLaser(TL, TR);
    DrawLaser(TR, BR);
    DrawLaser(BR, BL);
    DrawLaser(BL, TL);
    
    // Head (Triangle)
    vec2 HeadTop = Center + vec2(0, -H - 15.0f);
    DrawLaser(TL, HeadTop);
    DrawLaser(TR, HeadTop);
    
    // Arms
    float ArmAngle = (Server()->Tick() * 0.2f);
    vec2 ArmLStart = Center + vec2(-W, -H/2);
    vec2 ArmRStart = Center + vec2(W, -H/2);
    vec2 ArmLEnd = ArmLStart + vec2(-20, 10 + sin(ArmAngle)*10);
    vec2 ArmREnd = ArmRStart + vec2(20, 10 + cos(ArmAngle)*10);
    
    DrawLaser(ArmLStart, ArmLEnd);
    DrawLaser(ArmRStart, ArmREnd);
    
    // "Hammer" on Right Arm
    if(Server()->Tick() < m_AttackTick) // Attack animation
    {
        DrawLaser(ArmREnd, ArmREnd + vec2(15, 0)); // Visual Swing
    }
}
