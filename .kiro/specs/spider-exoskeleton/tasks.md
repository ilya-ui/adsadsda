# Implementation Plan

- [x] 1. Create Spider entity class structure




  - [ ] 1.1 Create spider.h header file with CSpider class declaration
    - Define class inheriting from CEntity
    - Declare member variables: m_Owner, m_Pilot, m_Velocity, m_FireCooldown, m_WalkTick, m_CannonAngle
    - Declare m_aIds[18] for snap objects


    - Declare public methods: SetPilot, RemovePilot, HasPilot, GetOwner, GetPilot
    - _Requirements: 1.1, 1.2_
  - [ ] 1.2 Create spider.cpp with constructor and Reset()
    - Initialize all member variables
    - Allocate snap IDs in constructor
    - Free snap IDs in Reset()



    - _Requirements: 1.1_
  - [ ]* 1.3 Write property test for spider creation
    - **Property 1: Spider creation assigns pilot**

    - **Validates: Requirements 1.1, 1.2**

- [ ] 2. Implement pilot management
  - [ ] 2.1 Implement SetPilot() method
    - Set m_Pilot to ClientId
    - Update player's m_InSpider and m_pSpider flags


    - _Requirements: 1.2_
  - [ ] 2.2 Implement RemovePilot() method
    - Clear player's spider flags
    - Set player position outside spider

    - Set m_Pilot to -1

    - _Requirements: 4.1, 4.2_
  - [ ] 2.3 Add m_InSpider and m_pSpider to CPlayer class
    - Add bool m_InSpider member
    - Add CSpider* m_pSpider pointer
    - Initialize in constructor

    - _Requirements: 1.2, 2.4_
  - [ ]* 2.4 Write property test for exit destroys spider
    - **Property 8: Exit destroys spider**
    - **Validates: Requirements 4.1, 4.2**

- [x] 3. Implement movement and Tick()

  - [ ] 3.1 Implement basic Tick() with input handling
    - Get player input direction

    - Update m_Velocity based on input
    - Apply gravity
    - Handle collision with walls/floor
    - _Requirements: 2.1_

  - [ ] 3.2 Implement pilot position synchronization
    - Keep pilot character at spider center
    - Reset pilot velocity
    - _Requirements: 2.4_
  - [x]* 3.3 Write property test for movement updates position

    - **Property 4: Movement updates position**
    - **Validates: Requirements 2.1, 2.4**


- [ ] 4. Implement leg animation system
  - [ ] 4.1 Implement CalculateLegPositions() method
    - Calculate 8 leg base positions with offsets

    - Apply sinusoidal animation based on m_WalkTick
    - Alternate phases for odd/even legs
    - _Requirements: 2.2, 2.3, 5.3_
  - [x] 4.2 Increment m_WalkTick during movement in Tick()

    - Only increment when velocity is non-zero
    - Reset animation when stationary
    - _Requirements: 2.2, 2.3_
  - [ ]* 4.3 Write property test for leg animation
    - **Property 5: Leg animation during movement**
    - **Validates: Requirements 2.2, 5.3**


- [ ] 5. Implement Snap() rendering
  - [x] 5.1 Implement leg rendering in Snap()

    - Draw 8 laser lines from body to calculated leg positions
    - Use m_aIds[0-7] for legs
    - _Requirements: 1.3, 5.1_
  - [x] 5.2 Implement cannon rendering in Snap()

    - Draw 2 laser lines for cannons at symmetric positions
    - Rotate cannons based on m_CannonAngle
    - Use m_aIds[8-9] for cannons
    - _Requirements: 1.4, 5.2_
  - [ ] 5.3 Implement body rendering in Snap()
    - Draw body outline using remaining laser IDs

    - _Requirements: 5.1, 5.2_
  - [ ]* 5.4 Write property test for leg count invariant
    - **Property 2: Leg count invariant**
    - **Validates: Requirements 1.3, 5.1**
  - [ ]* 5.5 Write property test for cannon symmetry
    - **Property 3: Cannon symmetry**
    - **Validates: Requirements 1.4, 5.2**




- [x] 6. Checkpoint - Ensure all tests pass

  - Ensure all tests pass, ask the user if questions arise.

- [ ] 7. Implement firing system
  - [x] 7.1 Implement Fire() method

    - Check cooldown before firing
    - Create two CProjectile grenades from cannon positions




    - Set projectile direction from m_CannonAngle
    - Set m_FireCooldown
    - Play fire sound
    - _Requirements: 3.1, 3.2, 3.3_
  - [ ] 7.2 Add fire input handling in Tick()
    - Update m_CannonAngle from cursor position
    - Call Fire() when fire button pressed
    - Decrement m_FireCooldown each tick
    - _Requirements: 3.1, 3.2_






  - [ ]* 7.3 Write property test for dual cannon fire
    - **Property 6: Dual cannon fire**
    - **Validates: Requirements 3.1**
  - [ ]* 7.4 Write property test for fire cooldown
    - **Property 7: Fire cooldown enforcement**
    - **Validates: Requirements 3.3**

- [ ] 8. Implement "spider" command
  - [ ] 8.1 Add ENTTYPE_SPIDER to CGameWorld enum
    - Add new entity type constant
    - _Requirements: 1.1_
  - [ ] 8.2 Add ConSpider command to ddracecommands.cpp
    - Check if player already in spider → exit and destroy
    - Otherwise create new CSpider at player position
    - Set player as pilot
    - _Requirements: 1.1, 4.1, 4.2_
  - [ ] 8.3 Register command in gamecontext.cpp
    - Add Console()->Register for "spider" command
    - _Requirements: 1.1_

- [ ] 9. Handle disconnect cleanup
  - [ ] 9.1 Add spider cleanup in CPlayer destructor/disconnect
    - Check if player has m_pSpider
    - Call spider Reset() to destroy
    - _Requirements: 4.3_
  - [ ]* 9.2 Write property test for disconnect cleanup
    - **Property 9: Disconnect cleanup**
    - **Validates: Requirements 4.3**

- [ ] 10. Update CMakeLists.txt
  - [ ] 10.1 Add spider.cpp and spider.h to source files
    - Add to game-server source list
    - _Requirements: 1.1_

- [ ] 11. Final Checkpoint - Ensure all tests pass
  - Ensure all tests pass, ask the user if questions arise.
