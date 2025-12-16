/* DDNet Spider Exoskeleton Entity */
#include "spider.h"
#include "character.h"
#include "projectile.h"

#include <game/server/gamecontext.h>
#include <game/server/player.h>
#include <generated/protocol.h>

CSpider::CSpider(CGameWorld *pGameWorld, vec2 Pos, int Owner) :
	CEntity(pGameWorld, CGameWorld::ENTTYPE_SPIDER, Pos, 80)
{
	m_Owner = Owner;
	m_Pilot = -1;
	m_Velocity = vec2(0, 0);
	m_FireCooldown = 0;
	m_WalkTick = 0;
	m_CannonAngle = 0;
	m_LastJump = false;

	for(int i = 0; i < 26; i++)
		m_aIds[i] = Server()->SnapNewId();

	GameWorld()->InsertEntity(this);
}

void CSpider::Reset()
{
	RemovePilot();

	for(int i = 0; i < 26; i++)
		Server()->SnapFreeId(m_aIds[i]);

	Destroy();
}

void CSpider::SetPilot(int ClientId)
{
	m_Pilot = ClientId;
	if(ClientId >= 0 && ClientId < MAX_CLIENTS)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[ClientId];
		if(pPlayer)
		{
			pPlayer->m_InSpider = true;
			pPlayer->m_pSpider = this;
		}
	}
}

void CSpider::RemovePilot()
{
	if(m_Pilot >= 0 && m_Pilot < MAX_CLIENTS)
	{
		CPlayer *pPlayer = GameServer()->m_apPlayers[m_Pilot];
		if(pPlayer)
		{
			pPlayer->m_InSpider = false;
			pPlayer->m_pSpider = nullptr;

			CCharacter *pChr = pPlayer->GetCharacter();
			if(pChr)
			{
				// Exit to the right side
				pChr->SetPosition(m_Pos + vec2(60, -20));
				pChr->m_Pos = m_Pos + vec2(60, -20);
				pChr->m_PrevPos = m_Pos + vec2(60, -20);
			}
		}
	}
	m_Pilot = -1;
}


void CSpider::CalculateLegPositions(vec2 *pLegs)
{
	// Spider legs with proper bent shape: hip -> knee -> foot
	// Each leg has 3 points: hip (body), knee (bent outward/up), foot (ground)
	// pLegs layout: [hip0, knee0, foot0, hip1, knee1, foot1, ...]
	
	// Hip positions on body (where legs attach)
	const vec2 HipOffsets[8] = {
		vec2(-25, -5),   // 0: front-left
		vec2(25, -5),    // 1: front-right
		vec2(-30, 5),    // 2: mid-front-left
		vec2(30, 5),     // 3: mid-front-right
		vec2(-30, 15),   // 4: mid-back-left
		vec2(30, 15),    // 5: mid-back-right
		vec2(-25, 25),   // 6: back-left
		vec2(25, 25),    // 7: back-right
	};

	float AnimSpeed = 0.2f;
	float AnimAmplitude = 8.0f;

	for(int i = 0; i < 8; i++)
	{
		bool IsLeft = (i % 2 == 0);
		float Side = IsLeft ? -1.0f : 1.0f;
		
		// Animation: alternating phases for odd/even legs
		float Phase = IsLeft ? 0.0f : 3.14159f;
		float AnimOffset = sin(m_WalkTick * AnimSpeed + Phase) * AnimAmplitude;
		
		// Hip position (on body)
		vec2 HipPos = m_Pos + HipOffsets[i];
		
		// Knee position - goes OUT and UP from hip (bent leg shape)
		// Legs spread outward more at front/back, less in middle
		float SpreadFactor = (i < 2 || i > 5) ? 45.0f : 55.0f;
		float KneeHeight = -15.0f; // Knee is ABOVE hip level
		vec2 KneePos = HipPos + vec2(Side * SpreadFactor, KneeHeight + AnimOffset * 0.5f);
		
		// Foot position - goes DOWN from knee to ground
		float FootDrop = 55.0f;
		vec2 FootPos = KneePos + vec2(AnimOffset, FootDrop);
		
		// Store all 3 points
		pLegs[i * 3] = HipPos;
		pLegs[i * 3 + 1] = KneePos;
		pLegs[i * 3 + 2] = FootPos;
	}
}

void CSpider::Fire()
{
	if(m_FireCooldown > 0)
		return;

	m_FireCooldown = 25; // ~0.5 second cooldown

	vec2 CannonDir = vec2(cos(m_CannonAngle), sin(m_CannonAngle));

	// Left cannon position
	vec2 LeftCannonPos = m_Pos + vec2(-30, -10);
	// Right cannon position
	vec2 RightCannonPos = m_Pos + vec2(30, -10);

	// Get mouse target for explosion direction
	CNetObj_PlayerInput Input = GameServer()->GetLastPlayerInput(m_Pilot);
	vec2 MouseTarget = vec2(Input.m_TargetX, Input.m_TargetY);

	// Fire from left cannon
	new CProjectile(
		GameWorld(),
		WEAPON_GRENADE,
		m_Pilot,
		LeftCannonPos,
		CannonDir,
		(int)(Server()->TickSpeed() * 2.0f),
		false, true, SOUND_GRENADE_EXPLODE, MouseTarget);

	// Fire from right cannon
	new CProjectile(
		GameWorld(),
		WEAPON_GRENADE,
		m_Pilot,
		RightCannonPos,
		CannonDir,
		(int)(Server()->TickSpeed() * 2.0f),
		false, true, SOUND_GRENADE_EXPLODE, MouseTarget);

	GameServer()->CreateSound(m_Pos, SOUND_GRENADE_FIRE);
}

void CSpider::Tick()
{
	if(m_FireCooldown > 0)
		m_FireCooldown--;

	if(m_Pilot < 0)
	{
		// No pilot - apply gravity and friction
		m_Velocity.y += 0.5f;
		m_Velocity.x *= 0.95f;
	}
	else
	{
		CPlayer *pPilot = GameServer()->m_apPlayers[m_Pilot];
		if(!pPilot)
		{
			RemovePilot();
			return;
		}

		CCharacter *pChr = pPilot->GetCharacter();
		if(!pChr)
		{
			RemovePilot();
			return;
		}

		// Get input
		CNetObj_PlayerInput Input = GameServer()->GetLastPlayerInput(m_Pilot);

		// Update cannon angle to point at cursor
		vec2 CursorPos = m_Pos + vec2(Input.m_TargetX, Input.m_TargetY);
		vec2 Dir = CursorPos - m_Pos;
		m_CannonAngle = atan2(Dir.y, Dir.x);

		// Movement - left/right
		if(Input.m_Direction < 0)
			m_Velocity.x -= 0.6f;
		else if(Input.m_Direction > 0)
			m_Velocity.x += 0.6f;
		else
			m_Velocity.x *= 0.9f;

		// Limit horizontal speed
		if(m_Velocity.x > 6.0f) m_Velocity.x = 6.0f;
		if(m_Velocity.x < -6.0f) m_Velocity.x = -6.0f;

		// Gravity
		m_Velocity.y += 0.5f;

		// Jump on space - detect key press (not hold)
		bool JumpPressed = Input.m_Jump != 0;
		if(JumpPressed && !m_LastJump)
		{
			// Check if on ground (check multiple points under spider)
			bool OnGround = Collision()->CheckPoint(m_Pos + vec2(0, 60)) ||
			                Collision()->CheckPoint(m_Pos + vec2(-30, 60)) ||
			                Collision()->CheckPoint(m_Pos + vec2(30, 60));
			if(OnGround)
				m_Velocity.y = -12.0f;
		}
		m_LastJump = JumpPressed;

		// Fire on LMB
		if(Input.m_Fire & 1)
			Fire();

		// Animate legs when moving
		if(fabs(m_Velocity.x) > 0.5f)
			m_WalkTick++;
		else
			m_WalkTick = 0;

		// Keep pilot inside spider
		pChr->SetPosition(m_Pos + vec2(0, -10));
		pChr->m_Pos = m_Pos + vec2(0, -10);
		pChr->m_PrevPos = m_Pos + vec2(0, -10);
		pChr->ResetVelocity();
	}

	// Apply velocity with collision
	float BodyW = 90.0f;
	float BodyH = 60.0f;

	// Horizontal collision
	vec2 NewPosX = m_Pos + vec2(m_Velocity.x, 0);
	bool HitWallLeft = Collision()->CheckPoint(NewPosX + vec2(-BodyW/2, 0)) ||
	                   Collision()->CheckPoint(NewPosX + vec2(-BodyW/2, 30));
	bool HitWallRight = Collision()->CheckPoint(NewPosX + vec2(BodyW/2, 0)) ||
	                    Collision()->CheckPoint(NewPosX + vec2(BodyW/2, 30));

	if(HitWallLeft && m_Velocity.x < 0)
		m_Velocity.x = 0;
	if(HitWallRight && m_Velocity.x > 0)
		m_Velocity.x = 0;

	// Vertical collision
	vec2 NewPosY = m_Pos + vec2(0, m_Velocity.y);
	bool HitCeiling = Collision()->CheckPoint(NewPosY + vec2(0, -BodyH/2));
	bool HitFloor = Collision()->CheckPoint(NewPosY + vec2(0, BodyH)) ||
	                Collision()->CheckPoint(NewPosY + vec2(-BodyW/3, BodyH)) ||
	                Collision()->CheckPoint(NewPosY + vec2(BodyW/3, BodyH));

	if(HitCeiling && m_Velocity.y < 0)
		m_Velocity.y = 0;
	if(HitFloor && m_Velocity.y > 0)
		m_Velocity.y = 0;

	m_Pos = m_Pos + m_Velocity;
}


void CSpider::Snap(int SnappingClient)
{
	if(NetworkClipped(SnappingClient))
		return;

	int StartTick = Server()->Tick() - 2;
	CNetObj_Laser *pObj;

	// Calculate leg positions (3 points per leg: hip, knee, foot)
	vec2 Legs[24]; // 8 legs * 3 points
	CalculateLegPositions(Legs);

	// === DRAW 8 LEGS (2 segments each: hip->knee, knee->foot) ===
	for(int i = 0; i < 8; i++)
	{
		vec2 Hip = Legs[i * 3];
		vec2 Knee = Legs[i * 3 + 1];
		vec2 Foot = Legs[i * 3 + 2];

		// Upper leg segment: hip to knee
		pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[i * 2]);
		if(pObj)
		{
			pObj->m_X = (int)Knee.x;
			pObj->m_Y = (int)Knee.y;
			pObj->m_FromX = (int)Hip.x;
			pObj->m_FromY = (int)Hip.y;
			pObj->m_StartTick = StartTick;
		}

		// Lower leg segment: knee to foot
		pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[i * 2 + 1]);
		if(pObj)
		{
			pObj->m_X = (int)Foot.x;
			pObj->m_Y = (int)Foot.y;
			pObj->m_FromX = (int)Knee.x;
			pObj->m_FromY = (int)Knee.y;
			pObj->m_StartTick = StartTick;
		}
	}

	// === DRAW 2 CANNONS === (indices 16-17)
	float CannonLen = 45.0f;
	vec2 CannonDir = vec2(cos(m_CannonAngle), sin(m_CannonAngle));

	// Left cannon
	vec2 LeftCannonBase = m_Pos + vec2(-30, -10);
	vec2 LeftCannonEnd = LeftCannonBase + CannonDir * CannonLen;

	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[16]);
	if(pObj)
	{
		pObj->m_X = (int)LeftCannonEnd.x;
		pObj->m_Y = (int)LeftCannonEnd.y;
		pObj->m_FromX = (int)LeftCannonBase.x;
		pObj->m_FromY = (int)LeftCannonBase.y;
		pObj->m_StartTick = StartTick;
	}

	// Right cannon
	vec2 RightCannonBase = m_Pos + vec2(30, -10);
	vec2 RightCannonEnd = RightCannonBase + CannonDir * CannonLen;

	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[17]);
	if(pObj)
	{
		pObj->m_X = (int)RightCannonEnd.x;
		pObj->m_Y = (int)RightCannonEnd.y;
		pObj->m_FromX = (int)RightCannonBase.x;
		pObj->m_FromY = (int)RightCannonBase.y;
		pObj->m_StartTick = StartTick;
	}

	// === DRAW BODY (oval shape using lines) === (indices 18-25)
	float BodyW = 25.0f;
	float BodyH = 20.0f;

	// Body outline
	vec2 BodyTL = m_Pos + vec2(-BodyW, -BodyH);
	vec2 BodyTR = m_Pos + vec2(BodyW, -BodyH);
	vec2 BodyML = m_Pos + vec2(-BodyW - 5, 5);
	vec2 BodyMR = m_Pos + vec2(BodyW + 5, 5);
	vec2 BodyBL = m_Pos + vec2(-BodyW, BodyH + 10);
	vec2 BodyBR = m_Pos + vec2(BodyW, BodyH + 10);

	// Top line
	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[18]);
	if(pObj)
	{
		pObj->m_X = (int)BodyTR.x;
		pObj->m_Y = (int)BodyTR.y;
		pObj->m_FromX = (int)BodyTL.x;
		pObj->m_FromY = (int)BodyTL.y;
		pObj->m_StartTick = StartTick;
	}

	// Bottom line
	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[19]);
	if(pObj)
	{
		pObj->m_X = (int)BodyBR.x;
		pObj->m_Y = (int)BodyBR.y;
		pObj->m_FromX = (int)BodyBL.x;
		pObj->m_FromY = (int)BodyBL.y;
		pObj->m_StartTick = StartTick;
	}

	// Left top to middle
	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[20]);
	if(pObj)
	{
		pObj->m_X = (int)BodyML.x;
		pObj->m_Y = (int)BodyML.y;
		pObj->m_FromX = (int)BodyTL.x;
		pObj->m_FromY = (int)BodyTL.y;
		pObj->m_StartTick = StartTick;
	}

	// Left middle to bottom
	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[21]);
	if(pObj)
	{
		pObj->m_X = (int)BodyBL.x;
		pObj->m_Y = (int)BodyBL.y;
		pObj->m_FromX = (int)BodyML.x;
		pObj->m_FromY = (int)BodyML.y;
		pObj->m_StartTick = StartTick;
	}

	// Right top to middle
	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[22]);
	if(pObj)
	{
		pObj->m_X = (int)BodyMR.x;
		pObj->m_Y = (int)BodyMR.y;
		pObj->m_FromX = (int)BodyTR.x;
		pObj->m_FromY = (int)BodyTR.y;
		pObj->m_StartTick = StartTick;
	}

	// Right middle to bottom
	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[23]);
	if(pObj)
	{
		pObj->m_X = (int)BodyBR.x;
		pObj->m_Y = (int)BodyBR.y;
		pObj->m_FromX = (int)BodyMR.x;
		pObj->m_FromY = (int)BodyMR.y;
		pObj->m_StartTick = StartTick;
	}

	// Head (front of spider) - two lines forming V shape
	vec2 HeadTop = m_Pos + vec2(0, -BodyH - 10);
	vec2 HeadL = m_Pos + vec2(-12, -BodyH + 5);
	vec2 HeadR = m_Pos + vec2(12, -BodyH + 5);

	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[24]);
	if(pObj)
	{
		pObj->m_X = (int)HeadTop.x;
		pObj->m_Y = (int)HeadTop.y;
		pObj->m_FromX = (int)HeadL.x;
		pObj->m_FromY = (int)HeadL.y;
		pObj->m_StartTick = StartTick;
	}

	pObj = Server()->SnapNewItem<CNetObj_Laser>(m_aIds[25]);
	if(pObj)
	{
		pObj->m_X = (int)HeadTop.x;
		pObj->m_Y = (int)HeadTop.y;
		pObj->m_FromX = (int)HeadR.x;
		pObj->m_FromY = (int)HeadR.y;
		pObj->m_StartTick = StartTick;
	}
}
