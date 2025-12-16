/* DDNet Cannon Entity - Sucks in players and shoots them */
#ifndef GAME_SERVER_ENTITIES_CANNON_H
#define GAME_SERVER_ENTITIES_CANNON_H

#include <game/server/entity.h>

class CCannon : public CEntity
{
public:
	CCannon(CGameWorld *pGameWorld, vec2 Pos, int Owner);

	void Reset() override;
	void Tick() override;
	void Snap(int SnappingClient) override;

	void SetOwner(int ClientId);
	void RemoveOwner();
	
	int GetOwner() const { return m_Owner; }
	int GetLoadedCount() const { return m_NumLoaded; }

private:
	void SuckPlayer();
	void FirePlayer();

	int m_Owner;
	float m_Angle;
	
	// Loaded players (up to 5)
	static const int MAX_LOADED = 5;
	int m_aLoadedPlayers[MAX_LOADED];
	int m_NumLoaded;
	
	int m_FireCooldown;
	int m_SuckCooldown;
	
	int m_aIds[20]; // For laser rendering
};

#endif
