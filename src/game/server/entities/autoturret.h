/* DDNet Auto Turret Entity - automatic targeting turret for zombie event */
#ifndef GAME_SERVER_ENTITIES_AUTOTURRET_H
#define GAME_SERVER_ENTITIES_AUTOTURRET_H

#include <game/server/entity.h>

class CAutoTurret : public CEntity
{
public:
	CAutoTurret(CGameWorld *pGameWorld, vec2 Pos, int Owner);

	void Reset() override;
	void Tick() override;
	void Snap(int SnappingClient) override;

	int GetOwner() const { return m_Owner; }
	void SetEventMode(bool Active) { m_EventMode = Active; }

private:
	void Fire();
	CCharacter *FindNearestZombie();

	int m_Owner;
	float m_Angle;
	int m_FireCooldown;
	bool m_EventMode;

	static constexpr float RANGE = 400.0f;
	static constexpr int FIRE_RATE = 8; // Fast fire rate
	static constexpr int DAMAGE = 1;    // Low damage

	int m_aIds[8];
};

#endif
