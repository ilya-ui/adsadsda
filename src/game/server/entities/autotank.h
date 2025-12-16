/* DDNet Auto Tank Entity - automatic targeting tank for zombie event */
#ifndef GAME_SERVER_ENTITIES_AUTOTANK_H
#define GAME_SERVER_ENTITIES_AUTOTANK_H

#include <game/server/entity.h>

class CAutoTank : public CEntity
{
public:
	CAutoTank(CGameWorld *pGameWorld, vec2 Pos, int Owner);

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

	static constexpr float RANGE = 600.0f;
	static constexpr int FIRE_RATE = 25; // Medium fire rate
	static constexpr int DAMAGE = 5;     // High damage

	int m_aIds[12];
};

#endif
