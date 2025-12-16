/* DDNet UFO Entity */
#ifndef GAME_SERVER_ENTITIES_UFO_H
#define GAME_SERVER_ENTITIES_UFO_H

#include <game/server/entity.h>

class CUfo : public CEntity
{
public:
	CUfo(CGameWorld *pGameWorld, vec2 Pos, int Owner);

	void Reset() override;
	void Tick() override;
	void Snap(int SnappingClient) override;

	int GetOwner() const { return m_Owner; }
	int GetPilot() const { return m_Pilot; }
	int GetVictim() const { return m_Victim; }
	
	void SetPilot(int ClientId);
	void RemovePilot();
	bool HasPilot() const { return m_Pilot >= 0; }
	
	void SetVictim(int ClientId);
	void ReleaseVictim();
	bool HasVictim() const { return m_Victim >= 0; }

private:
	int m_Owner;      // Who spawned the UFO
	int m_Pilot;      // Who is piloting (-1 if empty)
	int m_Victim;     // Who is being abducted (-1 if none)
	vec2 m_Velocity;
	int m_AnimTick;
	float m_BeamPhase;
	
	int m_aIds[12];
};

#endif
