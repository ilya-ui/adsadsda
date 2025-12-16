/* DDNet Tank Entity */
#ifndef GAME_SERVER_ENTITIES_TANK_H
#define GAME_SERVER_ENTITIES_TANK_H

#include <game/server/entity.h>

class CTank : public CEntity
{
public:
	CTank(CGameWorld *pGameWorld, vec2 Pos, int Owner);

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
	
	int m_Owner;
	int m_Pilot;
	vec2 m_Velocity;
	float m_TurretAngle;
	int m_FireCooldown;
	
	int m_aIds[12];
};

#endif
