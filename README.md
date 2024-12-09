# QBasic Project

[English](#english) | [中文](#中文)

## English

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

## 中文

### QBasic 项目帮助文档

欢迎使用 QBasic 项目，这是一个简易的 BASIC 语言解释器。通过本项目，您可以学习表达式树和类继承，深入了解编程语言的工作原理。您可以将现有程序改编为解决不同但相关任务的程序，体验修改现有系统的过程。

**功能概述：**
- 支持基本的 BASIC 语句，如 `REM`, `LET`, `PRINT`, `INPUT`, `GOTO`, `IF`, `END`。
- 支持表达式解析和语法树显示。
- 提供调试模式，允许设置断点和查看变量值。
- 支持程序的加载、编辑和运行。

**使用方法：**
- 通过输入框或 LOAD 按钮加载和编辑 BASIC 程序。
- 使用 RUN 按钮执行程序。
- 在调试模式下，使用 ADD 和 DELETE 命令管理断点。

**注意事项：**
1. 命令窗口支持的命令:
- 内置命令: QUIT 退出, HELP 呼出帮助文档
- ADD 添加断点, DELETE 删除断点
- 按钮对应的命令, 如 LOAD 对应的命令是 LOAD, 但不推荐使用
- 编辑命令: 以行号开头的命令, 如 10 PRINT "HELLO", 会将该命令插入到行号为 10 的位置, 如果该行已有命令, 则会替换掉原有命令

2. 输入窗口的作用:
- 程序运行的中间, 可能会有`INPUT`带来的等待输入的情况, 输入窗口会出现“等待输入”的图标, 此时可以在输入窗口输入内容, 然后按回车键, 程序会继续运行

3. 符号与语法树: 
- 以后置表达式的形式展示，例如 `1 + 2 * 3` 会被解析为 `1 2 3 * +`，并以不同长度的空白前缀形成一颗树状结构
- (其实这样很丑，但lab要求这样做，可以进入`interpreter.cpp`之中将两者调换, 就可以以BNF的形式展示了
```cpp
  // astOutput(stmts[status.next_line]->toString());
  auto strs = stmts[status.next_line]->toTabbedString();
  for(const auto& s: strs) {
  astOutput(s);
  }
```
4. 保存与加载:
- 通过命令编辑的代码不会自动保存
- 使用ctrl+s保存当前代码, 会呼出保存文件对话框
- 在加载之前不可以直接运行, 请先保存代码

5. 其他窗口: 无需特别说明