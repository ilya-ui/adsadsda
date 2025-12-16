# Implementation Plan

- [x] 1. Create CScreenDrawing component header and basic structure









  - [ ] 1.1 Create `src/game/client/components/screen_drawing.h` with class definition
    - Define CDrawingPoint and CDrawingLine structs
    - Define CScreenDrawing class inheriting from CComponent
    - Declare all public methods and private members
    - _Requirements: 1.1, 2.1, 3.1, 4.1_

  - [x] 1.2 Create `src/game/client/components/screen_drawing.cpp` with basic implementation

    - Implement constructor with default values
    - Implement Sizeof(), OnInit(), OnReset()

    - _Requirements: 5.1, 6.1_
  - [ ]* 1.3 Write property test for toggle involutory
    - **Property 1: Toggle is involutory**
    - **Validates: Requirements 1.1, 1.2**


- [ ] 2. Implement drawing mode toggle and state management
  - [x] 2.1 Implement Toggle() method

    - Toggle m_Active boolean state
    - _Requirements: 1.1, 1.2_
  - [x] 2.2 Implement IsActive() getter

    - Return current active state
    - _Requirements: 1.1_

  - [ ]* 2.3 Write property test for lines persist after mode change
    - **Property 2: Lines persist after mode change**

    - **Validates: Requirements 1.3, 4.1**

- [ ] 3. Implement drawing data management
  - [ ] 3.1 Implement AddPoint() method
    - Add point to current line when drawing
    - Create new line if needed
    - _Requirements: 2.1, 2.4_

  - [ ] 3.2 Implement FinishLine() method
    - Complete current line when mouse released
    - _Requirements: 2.2_

  - [ ] 3.3 Implement Clear() method
    - Remove all lines from m_vLines vector

    - _Requirements: 3.1, 3.2_
  - [ ] 3.4 Implement GetLineCount() and GetTotalPointCount() getters
    - Return counts for testing
    - _Requirements: 4.1_
  - [ ]* 3.5 Write property test for clear removes all lines
    - **Property 3: Clear removes all lines**

    - **Validates: Requirements 3.1**
  - [ ]* 3.6 Write property test for adding points increases count
    - **Property 4: Adding points increases count**
    - **Validates: Requirements 2.1**


- [x] 4. Implement color and thickness settings


  - [ ] 4.1 Implement SetColor() and GetCurrentColor() methods
    - Store and retrieve current drawing color
    - _Requirements: 5.2, 5.3_
  - [ ] 4.2 Implement SetThickness() and GetCurrentThickness() methods
    - Store and retrieve current line thickness with clamping

    - _Requirements: 6.2, 6.3_
  - [ ]* 4.3 Write property test for color setting
    - **Property 5: Color setting is applied**

    - **Validates: Requirements 5.2**

  - [ ]* 4.4 Write property test for thickness setting
    - **Property 6: Thickness setting is applied**
    - **Validates: Requirements 6.2**


- [x] 5. Implement input handling


  - [ ] 5.1 Implement OnInput() for key events
    - Handle draw toggle key press

    - _Requirements: 1.1, 1.2_
  - [ ] 5.2 Implement OnCursorMove() for mouse tracking
    - Track cursor position when drawing mode active
    - Add points when draw button held

    - _Requirements: 2.1, 2.4_

- [x] 6. Implement rendering

  - [ ] 6.1 Implement OnRender() method
    - Use Graphics()->LinesBegin()/LinesEnd() to draw all lines
    - Apply stored color and thickness for each line
    - Render in screen-space coordinates
    - _Requirements: 2.3, 4.2, 4.3, 5.3, 6.3_
  - [ ] 6.2 Implement visual indicator for drawing mode
    - Show small icon or text when drawing mode is active
    - _Requirements: 1.1, 1.4_

- [ ] 7. Implement console commands
  - [ ] 7.1 Implement OnConsoleInit() with command registration
    - Register +draw, draw_toggle, draw_clear commands
    - Register draw_color and draw_thickness commands
    - _Requirements: 1.1, 3.1, 5.2, 6.2_
  - [ ] 7.2 Implement console command callbacks
    - ConToggle, ConClear, ConColor, ConThickness static methods
    - _Requirements: 1.1, 3.1, 5.2, 6.2_

- [ ] 8. Implement serialization
  - [ ] 8.1 Implement Serialize() method
    - Output drawing data in text format
    - Include version header, line count, colors, thicknesses, points
    - _Requirements: 7.1_




  - [x] 8.2 Implement Deserialize() method

    - Parse text format and reconstruct drawing data
    - Handle errors gracefully
    - _Requirements: 7.2_
  - [ ]* 8.3 Write property test for serialization round-trip
    - **Property 7: Serialization round-trip**
    - **Validates: Requirements 7.1, 7.2, 7.3**

- [ ] 9. Integrate component into CGameClient
  - [ ] 9.1 Add CScreenDrawing member to CGameClient
    - Add include and member variable in gameclient.h
    - _Requirements: 1.1_
  - [ ] 9.2 Register component in CGameClient initialization
    - Add to m_vpAll vector in gameclient.cpp
    - _Requirements: 1.1_

- [ ] 10. Checkpoint - Make sure all tests pass
  - Ensure all tests pass, ask the user if questions arise.

- [ ] 11. Add config variables
  - [ ] 11.1 Add cl_draw_color and cl_draw_thickness to config
    - Add MACRO_CONFIG entries in config_variables.h
    - _Requirements: 5.1, 5.2, 6.1, 6.2_
  - [ ] 11.2 Load config values on component init
    - Apply saved color and thickness on startup
    - _Requirements: 5.1, 6.1_

- [ ] 12. Final Checkpoint - Make sure all tests pass
  - Ensure all tests pass, ask the user if questions arise.
