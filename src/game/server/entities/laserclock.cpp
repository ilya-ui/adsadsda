/* Wall Clock */
#include "laserclock.h"
#include <cmath>
#include <ctime>
#include <engine/server.h>
#include <game/server/gamecontext.h>
#include <generated/protocol.h>

CLaserClock::CLaserClock(CGameWorld *pGameWorld, vec2 Pos, int Owner) : CEntity(pGameWorld, CGameWorld::ENTTYPE_LASER) { m_Pos = Pos; m_Owner = Owner; m_LastSecond = -1; UpdateClock(); GameWorld()->InsertEntity(this); }
CLaserClock::~CLaserClock() {}
void CLaserClock::Reset() { m_MarkedForDestroy = true; }
void CLaserClock::Tick() { time_t now = time(nullptr); struct tm *ltm = localtime(&now); if(ltm->tm_sec != m_LastSecond) { m_LastSecond = ltm->tm_sec; UpdateClock(); } }
void CLaserClock::UpdateClock() { m_Lines.clear(); time_t now = time(nullptr); struct tm *ltm = localtime(&now); int hours = ltm->tm_hour % 12; int minutes = ltm->tm_min; int seconds = ltm->tm_sec; DrawCircle(m_Pos, CLOCK_RADIUS, 24); for(int i = 0; i < 12; i++) { float a = (float)i * 6.28318f / 12.0f - 1.5708f; vec2 o = m_Pos + vec2(cosf(a), sinf(a)) * CLOCK_RADIUS; vec2 n = m_Pos + vec2(cosf(a), sinf(a)) * (CLOCK_RADIUS - MARK_LENGTH); m_Lines.push_back({n, o}); } m_Lines.push_back({m_Pos - vec2(5, 0), m_Pos + vec2(5, 0)}); m_Lines.push_back({m_Pos - vec2(0, 5), m_Pos + vec2(0, 5)}); float ha = ((float)hours + (float)minutes / 60.0f) * 6.28318f / 12.0f - 1.5708f; vec2 he = m_Pos + vec2(cosf(ha), sinf(ha)) * HOUR_HAND_LENGTH; m_Lines.push_back({m_Pos, he}); m_Lines.push_back({m_Pos + vec2(3, 0), he + vec2(3, 0)}); m_Lines.push_back({m_Pos - vec2(3, 0), he - vec2(3, 0)}); float ma = ((float)minutes + (float)seconds / 60.0f) * 6.28318f / 60.0f - 1.5708f; vec2 me = m_Pos + vec2(cosf(ma), sinf(ma)) * MINUTE_HAND_LENGTH; m_Lines.push_back({m_Pos, me}); float sa = (float)seconds * 6.28318f / 60.0f - 1.5708f; vec2 se = m_Pos + vec2(cosf(sa), sinf(sa)) * SECOND_HAND_LENGTH; m_Lines.push_back({m_Pos, se}); }
void CLaserClock::DrawCircle(vec2 C, float R, int S) { for(int i = 0; i < S; i++) { float a1 = (float)i * 6.28318f / (float)S; float a2 = (float)(i + 1) * 6.28318f / (float)S; m_Lines.push_back({C + vec2(cosf(a1), sinf(a1)) * R, C + vec2(cosf(a2), sinf(a2)) * R}); } }
void CLaserClock::Snap(int SnappingClient) { if(NetworkClipped(SnappingClient)) return; int ST = Server()->Tick() - 3; for(size_t i = 0; i < m_Lines.size() && i < MAX_LINES; i++) { CNetObj_Laser *p = static_cast<CNetObj_Laser *>(Server()->SnapNewItem(NETOBJTYPE_LASER, GetId() * MAX_LINES + i, sizeof(CNetObj_Laser))); if(!p) return; p->m_X = (int)m_Lines[i].second.x; p->m_Y = (int)m_Lines[i].second.y; p->m_FromX = (int)m_Lines[i].first.x; p->m_FromY = (int)m_Lines[i].first.y; p->m_StartTick = ST; } }
