# Requirements Document

## Introduction

Zombie Event — это 5-минутный игровой режим для DDNet сервера. Игроки делятся на две команды: Строители и Зомби. В первой фазе (2.5 минуты) строители размещают защитные сооружения (турели, танки, пушки), пока зомби заморожены. Во второй фазе зомби атакуют, а построенные сооружения автоматически защищают строителей.

## Glossary

- **Zombie Event System**: Система управления ивентом зомби-выживания
- **Builder**: Игрок-строитель, размещающий защитные сооружения
- **Zombie**: Игрок-зомби, атакующий строителей после фазы строительства
- **Build Phase**: Первая фаза ивента (2.5 минуты), когда строители размещают сооружения
- **Attack Phase**: Вторая фаза ивента (2.5 минуты), когда зомби атакуют
- **Defense Structure**: Защитное сооружение (турель, танк или пушка)
- **Build Points**: Очки строительства, расходуемые на размещение сооружений

## Requirements

### Requirement 1

**User Story:** As a server admin, I want to start a zombie event with a console command, so that I can run special game modes for players.

#### Acceptance Criteria

1. WHEN an admin executes the "zombieevent" command THEN the Zombie Event System SHALL start a new 5-minute event
2. WHEN the event starts THEN the Zombie Event System SHALL randomly assign players to Builder or Zombie teams (50/50 split)
3. WHEN a player is assigned to a team THEN the Zombie Event System SHALL notify that player via chat message
4. WHEN an event is already running THEN the Zombie Event System SHALL reject new event start commands with an error message

### Requirement 2

**User Story:** As a player, I want to know which team I'm on and how much time remains, so that I can plan my actions.

#### Acceptance Criteria

1. WHEN the event starts THEN the Zombie Event System SHALL display team assignment to each player
2. WHILE the event is active THEN the Zombie Event System SHALL display remaining time via broadcast every 30 seconds
3. WHEN a phase changes THEN the Zombie Event System SHALL announce the new phase to all players
4. WHEN the event ends THEN the Zombie Event System SHALL announce the winning team

### Requirement 3

**User Story:** As a builder, I want to place defense structures during the build phase, so that I can protect my team from zombies.

#### Acceptance Criteria

1. WHEN a builder uses hammer during build phase THEN the Zombie Event System SHALL place a defense structure at the cursor position
2. WHEN a builder places a structure THEN the Zombie Event System SHALL deduct build points from the builder's pool
3. WHEN a builder has insufficient build points THEN the Zombie Event System SHALL reject the placement with a message
4. WHEN build phase starts THEN the Zombie Event System SHALL give each builder 100 build points
5. WHEN a builder switches weapons THEN the Zombie Event System SHALL cycle through structure types (turret=10pts, tank=30pts, cannon=25pts)

### Requirement 4

**User Story:** As a zombie, I want to be frozen during build phase and released during attack phase, so that the game has fair phases.

#### Acceptance Criteria

1. WHEN build phase starts THEN the Zombie Event System SHALL freeze all zombie players in place
2. WHEN attack phase starts THEN the Zombie Event System SHALL unfreeze all zombie players
3. WHILE a zombie is frozen THEN the Zombie Event System SHALL display "Waiting for build phase to end..." message
4. WHEN attack phase starts THEN the Zombie Event System SHALL give zombies increased speed (1.3x)

### Requirement 5

**User Story:** As a builder, I want my placed structures to automatically attack zombies, so that I have defense during attack phase.

#### Acceptance Criteria

1. WHEN attack phase starts THEN the Zombie Event System SHALL activate all placed defense structures
2. WHEN a defense structure detects a zombie within range THEN the structure SHALL automatically fire at the zombie
3. WHEN a zombie is hit by a defense structure THEN the zombie SHALL take damage and knockback
4. WHEN a zombie touches a builder THEN the builder SHALL become infected (converted to zombie team)

### Requirement 6

**User Story:** As a player, I want the event to have clear win conditions, so that I know what to achieve.

#### Acceptance Criteria

1. WHEN all builders are infected before time ends THEN the Zombie Event System SHALL declare zombies as winners
2. WHEN at least one builder survives until time ends THEN the Zombie Event System SHALL declare builders as winners
3. WHEN the event ends THEN the Zombie Event System SHALL reset all players to normal state
4. WHEN the event ends THEN the Zombie Event System SHALL remove all placed defense structures
