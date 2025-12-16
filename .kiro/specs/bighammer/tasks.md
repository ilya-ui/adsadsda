# Implementation Plan

- [x] 1. Add BigHammer flag to CPlayer




  - [x] 1.1 Add `m_HasBigHammer` boolean member to CPlayer class in player.h

    - Initialize to false in constructor



    - _Requirements: 1.1_

  - [x] 1.2 Initialize m_HasBigHammer in CPlayer constructor

    - _Requirements: 1.1_

- [x] 2. Implement console commands


  - [ ] 2.1 Add ConBigHammer function declaration in gamecontext.h
    - _Requirements: 1.1, 1.2_

  - [x] 2.2 Add ConUnBigHammer function declaration in gamecontext.h




    - _Requirements: 1.3_
  - [x] 2.3 Implement ConBigHammer in ddracecommands.cpp

    - Set m_HasBigHammer = true
    - Print confirmation message
    - _Requirements: 1.1, 1.2_
  - [ ] 2.4 Implement ConUnBigHammer in ddracecommands.cpp
    - Set m_HasBigHammer = false
    - Print confirmation message
    - _Requirements: 1.3_
  - [x] 2.5 Register commands in RegisterDDRaceCommands


    - _Requirements: 1.1, 1.3_

- [ ] 3. Implement BigHammer visual effect and gameplay
  - [ ] 3.1 Modify FireWeapon() in character.cpp for WEAPON_HAMMER case
    - Check if m_pPlayer->m_HasBigHammer is true
    - Create laser entities in hammer shape pattern
    - _Requirements: 2.1, 2.2, 2.3_
  - [ ] 3.2 Implement extended knockback radius
    - Find all players within BIGHAMMER_RADIUS (200 units)
    - Apply knockback to each player
    - Unfreeze frozen players
    - _Requirements: 3.1, 3.2, 3.3_
  - [ ]* 3.3 Write property test for BigHammer toggle
    - **Property 1: BigHammer toggle on**
    - **Property 2: BigHammer toggle off**
    - **Validates: Requirements 1.1, 1.3**

- [ ] 4. Checkpoint - Make sure all tests pass
  - Ensure all tests pass, ask the user if questions arise.
