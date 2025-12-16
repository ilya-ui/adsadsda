/* DDNet Helicopter Entity */
#ifndef GAME_SERVER_ENTITIES_HELICOPTER_H
#define GAME_SERVER_ENTITIES_HELICOPTER_H

#include <game/server/entity.h>

class CHelicopter : public CEntity
{
public:
	CHelicopter(CGameWorld *pGameWorld, vec2 Pos, int Owner);

	void Reset() override;
	void Tick() override;
	void Snap(int SnappingClient) override;

	int GetOwner() const { return m_Owner; }
	int GetPilot() const { return m_Pilot; }
	
	void SetPilot(int ClientId);
	void RemovePilot();
	bool HasPilot() const { return m_Pilot >= 0; }

private:
	int m_Owner;      // Who spawned the helicopter
	int m_Pilot;      // Who is piloting (-1 if empty)
	vec2 m_Velocity;
	int m_RotorTick;  // For rotor animation
	
	// IDs for snap objects
	int m_aIds[8];    // For laser lines forming helicopter shape
};

#endif
