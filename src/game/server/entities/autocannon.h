/* DDNet Auto Cannon Entity - automatic targeting cannon with knockback for zombie event */
#ifndef GAME_SERVER_ENTITIES_AUTOCANNON_H
#define GAME_SERVER_ENTITIES_AUTOCANNON_H

#include <game/server/entity.h>

class CAutoCannon : public CEntity
{
public:
	CAutoCannon(CGameWorld *pGameWorld, vec2 Pos, int Owner);

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

	static constexpr float RANGE = 500.0f;
	static constexpr int FIRE_RATE = 40; // Slow fire rate
	static constexpr int DAMAGE = 3;     // Medium damage + knockback

	int m_aIds[10];
};

#endif
