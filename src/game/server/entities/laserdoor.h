#ifndef GAME_SERVER_ENTITIES_LASERDOOR_H
#define GAME_SERVER_ENTITIES_LASERDOOR_H
#include <game/server/entity.h>

class CLaserDoor : public CEntity
{
public:
	CLaserDoor(CGameWorld *pGameWorld, vec2 From, vec2 To, int Owner);
	void Reset() override;
	void Tick() override;
	void Snap(int SnappingClient) override;
	void Toggle(int ClientId);
	bool IsOpen() const { return m_Open; }
	int GetOwner() const { return m_Owner; }
private:
	vec2 m_From;
	vec2 m_To;
	int m_Owner;
	bool m_Open;
};
#endif
