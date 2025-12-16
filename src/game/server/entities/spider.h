/* DDNet Spider Exoskeleton Entity */
#ifndef GAME_SERVER_ENTITIES_SPIDER_H
#define GAME_SERVER_ENTITIES_SPIDER_H

#include <game/server/entity.h>

class CSpider : public CEntity
{
public:
	CSpider(CGameWorld *pGameWorld, vec2 Pos, int Owner);

	void Reset() override;
	void Tick() override;
	void Snap(int SnappingClient) override;

	int GetOwner() const { return m_Owner; }
	int GetPilot() const { return m_Pilot; }

	void SetPilot(int ClientId);
	void RemovePilot();
	bool HasPilot() const { return m_Pilot >= 0; }

private:
	void Fire();
	void CalculateLegPositions(vec2 *pLegs);

	int m_Owner;           // Who spawned the spider
	int m_Pilot;           // Who is piloting (-1 if empty)
	vec2 m_Velocity;       // Movement velocity
	int m_FireCooldown;    // Cooldown between shots
	int m_WalkTick;        // Animation counter for legs
	float m_CannonAngle;   // Cannon aim angle
	bool m_LastJump;       // Previous jump state for edge detection

	// IDs for snap objects: 16 leg segments + 2 cannons + 8 body lines = 26
	int m_aIds[26];
};

#endif
