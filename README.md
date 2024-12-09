# QBasic Project
- en [English](README.md)
- zh_CN [简体中文](readme/README.zh_CN.md)

### QBasic Project Help Document

Welcome to the QBasic project, a minimal BASIC language interpreter. This project allows you to learn about expression trees and class inheritance, and gain a deeper understanding of how programming languages work. You can adapt existing programs to solve different but related tasks, experiencing the process of modifying existing systems.

**Features Overview:**
- Supports basic BASIC statements such as `REM`, `LET`, `PRINT`, `INPUT`, `GOTO`, `IF`, `END`.
- Supports expression parsing and syntax tree display.
- Provides a debug mode that allows setting breakpoints and viewing variable values.
- Supports loading, editing, and running programs.

**Usage:**
- Load and edit BASIC programs through the input box or LOAD button.
- Execute the program using the RUN button.
- In debug mode, manage breakpoints using ADD and DELETE commands.


**Notes:**
1. Supported commands in the command window:
- Built-in commands: QUIT to exit, HELP to display the help document
- ADD to add breakpoints, DELETE to remove breakpoints
- Commands corresponding to buttons, such as LOAD corresponding to the LOAD command, but it is not recommended to use
- Edit commands: Commands starting with a line number, such as `10 PRINT "HELLO"`, will insert the command at line number 10. If there is already a command on that line, it will be replaced

2. Function of the input window:
- During program execution, there may be situations where `INPUT` waits for input. The input window will display a "waiting for input" icon. At this time, you can enter content in the input window and press Enter to continue the program

3. Symbols and syntax tree:
- Displayed in postfix notation, for example, `1 + 2 * 3` will be parsed as `1 2 3 * +` and form a tree structure with different lengths of whitespace prefixes
- (This is actually ugly, but the lab requires it. You can swap the two in `interpreter.cpp` to display in BNF format
```cpp
  // astOutput(stmts[status.next_line]->toString());
  auto strs = stmts[status.next_line]->toTabbedString();
  for(const auto& s: strs) {
  astOutput(s);
  }
```
4. Save and load:
- Code edited through commands will not be automatically saved
- Use ctrl+s to save the current code, which will bring up the save file dialog
- The program cannot be run directly before loading, please save the code first
5. Other windows: No special instructions
6. Example programs: Please refer to the example programs in the `programs` folder
