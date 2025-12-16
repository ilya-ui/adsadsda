# Requirements Document

## Introduction

Функция рисования на экране для клиента DDNet позволяет игрокам рисовать произвольные линии на экране с помощью курсора мыши. Рисунки сохраняются на экране даже после отключения режима рисования и могут быть очищены командой clear. Это полезно для планирования маршрутов, обозначения позиций и общения с другими игроками.

## Glossary

- **Drawing_System**: Компонент клиента DDNet, отвечающий за рисование линий на экране
- **Drawing_Mode**: Режим, в котором движения курсора создают линии на экране
- **Drawing_Point**: Точка с координатами (x, y) и цветом, являющаяся частью линии
- **Drawing_Line**: Последовательность точек, соединённых линиями
- **Screen_Space**: Координатное пространство экрана игры

## Requirements

### Requirement 1

**User Story:** As a player, I want to toggle drawing mode on and off, so that I can draw on the screen when needed and play normally otherwise.

#### Acceptance Criteria

1. WHEN a player presses the drawing toggle key THEN the Drawing_System SHALL enable drawing mode and display a visual indicator
2. WHEN a player presses the drawing toggle key while drawing mode is active THEN the Drawing_System SHALL disable drawing mode
3. WHILE drawing mode is disabled THEN the Drawing_System SHALL preserve all existing drawings on screen
4. WHEN drawing mode state changes THEN the Drawing_System SHALL update the visual indicator within one frame

### Requirement 2

**User Story:** As a player, I want to draw lines on the screen by moving my cursor, so that I can create visible marks and shapes.

#### Acceptance Criteria

1. WHILE drawing mode is active AND the player holds the draw button THEN the Drawing_System SHALL record cursor positions as Drawing_Points
2. WHEN the player releases the draw button THEN the Drawing_System SHALL complete the current Drawing_Line
3. WHILE drawing mode is active THEN the Drawing_System SHALL render all Drawing_Lines as connected line segments
4. WHEN a new Drawing_Point is added THEN the Drawing_System SHALL connect it to the previous point with a line segment

### Requirement 3

**User Story:** As a player, I want to clear all my drawings, so that I can start fresh or remove unwanted marks.

#### Acceptance Criteria

1. WHEN a player executes the clear command THEN the Drawing_System SHALL remove all Drawing_Lines from memory
2. WHEN the clear command completes THEN the Drawing_System SHALL render an empty drawing canvas
3. WHEN the clear command is executed THEN the Drawing_System SHALL complete the operation within one frame

### Requirement 4

**User Story:** As a player, I want my drawings to persist during gameplay, so that I can reference them while playing.

#### Acceptance Criteria

1. WHILE the game is running THEN the Drawing_System SHALL maintain all Drawing_Lines in memory
2. WHEN the game renders a frame THEN the Drawing_System SHALL draw all stored lines on top of the game view
3. WHEN the player moves the camera THEN the Drawing_System SHALL render drawings in screen-space coordinates (fixed to screen, not world)

### Requirement 5

**User Story:** As a player, I want to customize my drawing color, so that I can create more visible or personalized drawings.

#### Acceptance Criteria

1. WHEN the Drawing_System initializes THEN the Drawing_System SHALL use a default drawing color (white)
2. WHEN a player sets a custom color via console command THEN the Drawing_System SHALL apply the new color to subsequent Drawing_Lines
3. WHEN rendering Drawing_Lines THEN the Drawing_System SHALL use the color stored with each line

### Requirement 6

**User Story:** As a player, I want to adjust the line thickness, so that I can create drawings that are easy to see.

#### Acceptance Criteria

1. WHEN the Drawing_System initializes THEN the Drawing_System SHALL use a default line thickness of 2 pixels
2. WHEN a player sets a custom thickness via console command THEN the Drawing_System SHALL apply the new thickness to subsequent Drawing_Lines
3. WHEN rendering Drawing_Lines THEN the Drawing_System SHALL use the thickness stored with each line

### Requirement 7

**User Story:** As a developer, I want the drawing data to be serializable, so that drawings can be saved and loaded.

#### Acceptance Criteria

1. WHEN serializing a Drawing_Line THEN the Drawing_System SHALL output a valid representation containing all points, color, and thickness
2. WHEN deserializing drawing data THEN the Drawing_System SHALL reconstruct the original Drawing_Lines
3. WHEN serializing then deserializing a Drawing_Line THEN the Drawing_System SHALL produce an equivalent Drawing_Line (round-trip property)
