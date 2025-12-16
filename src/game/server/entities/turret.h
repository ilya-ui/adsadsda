/* DDNet Turret Entity - stationary laser turret */
#ifndef GAME_SERVER_ENTITIES_TURRET_H
#define GAME_SERVER_ENTITIES_TURRET_H

#include <game/server/entity.h>

class CTurret : public CEntity
{
public:
	CTurret(CGameWorld *pGameWorld, vec2 Pos, int Owner);

	void Reset() override;
	void Tick() override;
	void Snap(int SnappingClient) override;

	int GetOwner() const { return m_Owner; }
	int GetPilot() const { return m_Pilot; }
	
	void SetPilot(int ClientId);
	void RemovePilot();
	bool HasPilot() const { return m_Pilot >= 0; }
	
	// Called when player hammers the turret
	void OnHammerHit(int ClientId);

private:
	void Fire();
	
	int m_Owner;
	int m_Pilot;
	float m_Angle;
	int m_FireCooldown;
	
	// Snap IDs for laser rendering
	int m_aIds[8];
};

#endif
