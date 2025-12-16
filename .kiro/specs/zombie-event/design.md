# Design Document: Zombie Event

## Overview

Zombie Event — это 5-минутный игровой режим для DDNet сервера, где игроки делятся на строителей и зомби. Система управляется через консольную команду и включает две фазы: строительство (2.5 мин) и атака (2.5 мин).

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    CGameContext                              │
│  ┌─────────────────────────────────────────────────────┐    │
│  │              CZombieEvent                            │    │
│  │  - m_Active, m_Phase, m_StartTick                   │    │
│  │  - m_aBuilders[], m_aZombies[]                      │    │
│  │  - m_aBuildPoints[]                                 │    │
│  │  - Tick(), StartEvent(), EndEvent()                 │    │
│  └─────────────────────────────────────────────────────┘    │
│                           │                                  │
│           ┌───────────────┼───────────────┐                 │
│           ▼               ▼               ▼                 │
│    ┌──────────┐    ┌──────────┐    ┌──────────┐            │
│    │CAutoTurret│   │CAutoTank │    │CAutoCannon│            │
│    │(10 pts)   │   │(30 pts)  │    │(25 pts)   │            │
│    └──────────┘    └──────────┘    └──────────┘            │
└─────────────────────────────────────────────────────────────┘
```

## Components and Interfaces

### CZombieEvent (новый класс в gamecontext.h)

```cpp
class CZombieEvent
{
public:
    enum EPhase { PHASE_NONE, PHASE_BUILD, PHASE_ATTACK };
    enum EStructure { STRUCT_TURRET, STRUCT_TANK, STRUCT_CANNON, NUM_STRUCTURES };
    
    void Start();
    void Tick();
    void End();
    
    bool IsActive() const;
    EPhase GetPhase() const;
    int GetRemainingTime() const;
    
    bool IsBuilder(int ClientId) const;
    bool IsZombie(int ClientId) const;
    
    int GetBuildPoints(int ClientId) const;
    bool TryPlaceStructure(int ClientId, vec2 Pos, EStructure Type);
    void InfectBuilder(int ClientId);
    
private:
    bool m_Active;
    EPhase m_Phase;
    int m_StartTick;
    int m_PhaseStartTick;
    
    bool m_aIsBuilder[MAX_CLIENTS];
    int m_aBuildPoints[MAX_CLIENTS];
    EStructure m_aSelectedStructure[MAX_CLIENTS];
    
    std::vector<CEntity*> m_vpStructures;
    
    void AssignTeams();
    void StartBuildPhase();
    void StartAttackPhase();
    void CheckWinCondition();
    void Cleanup();
};
```

### Auto-targeting Structures

Модифицированные версии существующих сущностей с автоматическим наведением:

```cpp
// Базовый класс для авто-сооружений
class CAutoDefense : public CEntity
{
public:
    void SetEventMode(bool Active);
    void SetTargetTeam(bool TargetZombies);
    
protected:
    CCharacter* FindNearestTarget();
    void AutoAim();
    void AutoFire();
    
    bool m_EventMode;
    float m_Range;
    int m_FireCooldown;
};
```

## Data Models

### Event State
```cpp
struct ZombieEventState {
    bool active;
    int phase;           // 0=none, 1=build, 2=attack
    int startTick;
    int builderCount;
    int zombieCount;
};
```

### Player Event Data
```cpp
struct PlayerEventData {
    bool isBuilder;      // true=builder, false=zombie
    int buildPoints;     // 0-100
    int selectedStructure; // 0=turret, 1=tank, 2=cannon
};
```

### Structure Costs
| Structure | Cost | Range | Fire Rate | Damage |
|-----------|------|-------|-----------|--------|
| Turret    | 10   | 400   | Fast      | Low    |
| Tank      | 30   | 600   | Medium    | High   |
| Cannon    | 25   | 500   | Slow      | Medium + Knockback |

## Correctness Properties

*A property is a characteristic or behavior that should hold true across all valid executions of a system-essentially, a formal statement about what the system should do. Properties serve as the bridge between human-readable specifications and machine-verifiable correctness guarantees.*

### Property 1: Team assignment balance
*For any* number of players N >= 2, the team assignment should result in |builders - zombies| <= 1
**Validates: Requirements 1.2**

### Property 2: Build points deduction
*For any* structure placement by a builder, the builder's points should decrease by exactly the structure's cost
**Validates: Requirements 3.2**

### Property 3: Insufficient points rejection
*For any* builder with points P and structure cost C where P < C, the placement attempt should fail and points should remain unchanged
**Validates: Requirements 3.3**

### Property 4: Initial build points
*For any* builder at the start of build phase, their build points should equal exactly 100
**Validates: Requirements 3.4**

### Property 5: Zombie freeze during build
*For any* zombie player during build phase, their movement should be frozen
**Validates: Requirements 4.1**

### Property 6: Zombie unfreeze during attack
*For any* zombie player during attack phase, their movement should not be frozen
**Validates: Requirements 4.2**

### Property 7: Auto-targeting range
*For any* defense structure and zombie within its range, the structure should fire at the zombie
**Validates: Requirements 5.2**

### Property 8: Infection on contact
*For any* builder touched by a zombie during attack phase, the builder should become a zombie
**Validates: Requirements 5.4**

### Property 9: Zombie win condition
*For any* game state where builder count equals 0, zombies should be declared winners
**Validates: Requirements 6.1**

### Property 10: Builder win condition
*For any* game state where time expires and builder count > 0, builders should be declared winners
**Validates: Requirements 6.2**

## Error Handling

| Error | Handling |
|-------|----------|
| Event already running | Reject command, show error message |
| Not enough players (<2) | Reject command, show error message |
| Insufficient build points | Reject placement, show remaining points |
| Invalid placement position | Reject placement, show error |
| Player disconnects | Remove from team, check win condition |

## Testing Strategy

### Unit Tests
- Test team assignment with various player counts
- Test build point deduction calculations
- Test phase transition timing
- Test win condition checks

### Property-Based Tests
Using a property-based testing library (e.g., custom test harness):

1. **Team Balance Property**: Generate random player counts, verify |builders - zombies| <= 1
2. **Build Points Property**: Generate random structure placements, verify correct deduction
3. **Infection Property**: Generate random collision scenarios, verify infection occurs
4. **Win Condition Properties**: Generate various end-game states, verify correct winner

### Integration Tests
- Full event flow from start to end
- Multiple structure placements
- Zombie infection chain
- Event cleanup verification
