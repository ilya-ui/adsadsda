# Implementation Plan

- [x] 1. Add practice state variables to CGameClient





  - [x] 1.1 Add practice member variables to gameclient.h





    - Add m_PracticeActive, m_PracticeCore, m_PracticePrevCore
    - Add m_PracticeWorld, m_PracticeSafePos, m_PracticeStartPos


    - Add m_PracticeLastTick



    - _Requirements: 1.1, 2.1_
  - [x] 1.2 Initialize practice state in OnReset()


    - Reset all practice variables to default values
    - _Requirements: 1.1_


- [x] 2. Implement practice toggle functionality
  - [x] 2.1 Add practice console commands in OnConsoleInit()
    - Register practice_toggle, practice_reset, practice_checkpoint
    - _Requirements: 1.1, 4.1, 5.1_
  - [x] 2.2 Implement TogglePractice() method
    - Enable: save position, init PracticeCore from PredictedChar
    - Disable: sync back to server state
    - _Requirements: 1.1, 1.2_
  - [x] 2.3 Implement ResetPractice() method
    - Teleport to safe position, reset velocity
    - _Requirements: 4.1, 4.2_


  - [x] 2.4 Implement SetPracticeCheckpoint() method
    - Save current position as safe position
    - _Requirements: 5.1_

- [x] 3. Implement practice physics simulation
  - [x] 3.1 Add practice simulation in OnPredict()
    - Similar to FakeSuper simulation
    - Apply input, tick physics, move, quantize
    - _Requirements: 2.1_
  - [x] 3.2 Implement freeze detection
    - Check if character is in freeze tile
    - Teleport to safe position when frozen
    - _Requirements: 3.1_
  - [x] 3.3 Implement safe position tracking

    - Update safe position when not frozen
    - _Requirements: 3.2_

- [x] 4. Implement practice rendering

  - [x] 4.1 Modify UpdateRenderedCharacters() for practice mode

    - Render practice core instead of predicted when active
    - _Requirements: 2.2_
  - [x] 4.2 Add practice mode indicator in HUD

    - Display "PRACTICE" text when active
    - _Requirements: 1.3_

- [x] 5. Add 5years settings tab

  - [x] 5.1 Add SETTINGS_5YEARS enum in menus.h

    - Add new settings page constant
    - _Requirements: 6.1_
  - [x] 5.2 Create RenderSettings5Years() method

    - Add practice toggle checkbox
    - Add keybind options
    - _Requirements: 6.2, 6.3_
  - [x] 5.3 Register 5years tab in settings menu

    - Add tab button and rendering
    - _Requirements: 6.1_

- [ ] 6. Checkpoint - Make sure all tests pass
  - Ensure all tests pass, ask the user if questions arise.
