# Implementation Plan

- [x] 1. Create CZombieEvent class and basic infrastructure





  - [x] 1.1 Create zombieevent.h with CZombieEvent class declaration


    - Define EPhase enum (PHASE_NONE, PHASE_BUILD, PHASE_ATTACK)
    - Define EStructure enum (STRUCT_TURRET, STRUCT_TANK, STRUCT_CANNON)
    - Declare member variables and methods
    - _Requirements: 1.1, 1.2_
  - [x] 1.2 Create zombieevent.cpp with basic implementation


    - Implement constructor, Start(), End(), IsActive()
    - Implement team assignment logic (50/50 split)
    - _Requirements: 1.1, 1.2, 1.3_

  - [x] 1.3 Write property test for team balance

    - **Property 1: Team assignment balance**
    - **Validates: Requirements 1.2**

- [x] 2. Implement phase management and timing



  - [x] 2.1 Implement Tick() method with phase transitions

    - Track time elapsed since event start
    - Transition from BUILD to ATTACK at 2.5 minutes
    - End event at 5 minutes
    - _Requirements: 2.2, 2.3_
  - [x] 2.2 Implement time display broadcasts

    - Show remaining time every 30 seconds
    - Announce phase changes
    - _Requirements: 2.2, 2.3_

- [-] 3. Implement builder mechanics

  - [x] 3.1 Add build points system

    - Initialize builders with 100 points
    - Track selected structure per player
    - _Requirements: 3.4_
  - [x] 3.2 Write property test for initial build points


    - **Property 4: Initial build points**
    - **Validates: Requirements 3.4**

  - [ ] 3.3 Implement structure placement via hammer
    - Detect hammer use during build phase
    - Place structure at cursor position
    - Deduct points based on structure type
    - _Requirements: 3.1, 3.2, 3.3_

  - [ ] 3.4 Write property test for build points deduction
    - **Property 2: Build points deduction**
    - **Validates: Requirements 3.2**

  - [ ] 3.5 Write property test for insufficient points rejection
    - **Property 3: Insufficient points rejection**
    - **Validates: Requirements 3.3**
  - [x] 3.6 Implement weapon switch to cycle structures

    - Cycle through turret→tank→cannon on weapon switch
    - Show current selection to player
    - _Requirements: 3.5_

- [-] 4. Implement zombie mechanics

  - [x] 4.1 Implement zombie freeze during build phase

    - Freeze all zombies when build phase starts
    - Display waiting message
    - _Requirements: 4.1, 4.3_
  - [x] 4.2 Write property test for zombie freeze

    - **Property 5: Zombie freeze during build**
    - **Validates: Requirements 4.1**

  - [ ] 4.3 Implement zombie unfreeze and speed boost
    - Unfreeze zombies when attack phase starts
    - Apply 1.3x speed modifier
    - _Requirements: 4.2, 4.4_

  - [ ] 4.4 Write property test for zombie unfreeze
    - **Property 6: Zombie unfreeze during attack**
    - **Validates: Requirements 4.2**

- [-] 5. Create auto-targeting defense structures

  - [x] 5.1 Create CAutoTurret entity

    - Inherit from CEntity
    - Implement auto-targeting logic
    - Fast fire rate, low damage, 400 range
    - _Requirements: 5.1, 5.2_

  - [ ] 5.2 Create CAutoTank entity
    - Inherit from CEntity
    - Implement auto-targeting logic
    - Medium fire rate, high damage, 600 range
    - _Requirements: 5.1, 5.2_

  - [ ] 5.3 Create CAutoCannon entity
    - Inherit from CEntity
    - Implement auto-targeting with knockback
    - Slow fire rate, medium damage, 500 range
    - _Requirements: 5.1, 5.2_
  - [x] 5.4 Write property test for auto-targeting

    - **Property 7: Auto-targeting range**
    - **Validates: Requirements 5.2**

- [-] 6. Implement infection and win conditions

  - [x] 6.1 Implement infection on zombie-builder contact

    - Detect collision between zombie and builder
    - Convert builder to zombie team
    - _Requirements: 5.4_
  - [x] 6.2 Write property test for infection

    - **Property 8: Infection on contact**
    - **Validates: Requirements 5.4**

  - [ ] 6.3 Implement win condition checks
    - Check if all builders infected (zombie win)
    - Check if time expired with builders alive (builder win)
    - _Requirements: 6.1, 6.2_

  - [ ] 6.4 Write property tests for win conditions
    - **Property 9: Zombie win condition**
    - **Property 10: Builder win condition**
    - **Validates: Requirements 6.1, 6.2**


- [ ] 7. Integrate with game systems
  - [x] 7.1 Add CZombieEvent instance to CGameContext

    - Add member variable and initialization
    - Call Tick() from game tick
    - _Requirements: 1.1_

  - [ ] 7.2 Register console command "zombieevent"
    - Add ConZombieEvent function
    - Register in RegisterDDRaceCommands
    - _Requirements: 1.1, 1.4_

  - [x] 7.3 Hook builder hammer input

    - Intercept hammer in character.cpp during event
    - Call TryPlaceStructure
    - _Requirements: 3.1_

  - [ ] 7.4 Hook weapon switch for structure selection
    - Intercept weapon switch during event
    - Cycle selected structure
    - _Requirements: 3.5_


- [ ] 8. Implement cleanup and reset
  - [x] 8.1 Implement event cleanup

    - Remove all placed structures
    - Reset player states (unfreeze, normal speed)
    - Clear team assignments
    - _Requirements: 6.3, 6.4_

  - [ ] 8.2 Add CMakeLists.txt entries for new files
    - Add zombieevent.cpp/h
    - Add autoturret.cpp/h, autotank.cpp/h, autocannon.cpp/h
    - _Requirements: N/A_


- [x] 9. Checkpoint - Make sure all tests pass

  - Ensure all tests pass, ask the user if questions arise.
