//
// Created by ayanami on 12/2/24.
//

#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <filesystem>
#include <string>
#include <vector>
#include <regex>
#include "util.h"
#include <fmt/core.h>

namespace Token {
enum class TokenType {
    OP_GT,
    OP_LT,
    OP_EQ,
    OP_GE,
    OP_LE,

    OP_POW,

    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,

    OP_MOD,
    ASSIGN,
    LET,
    IF,
    ELSE,
    THEN,
    REM,

    LPAREN,
    RPAREN,

    GOTO,
    END,
    PRINT,
    INPUT,

    VAR,
    NUM,
    LITERAL,
    SPACE,
    UNKNOWN,
};
using Token = struct Token {
    TokenType type = TokenType::UNKNOWN;
    std::optional<std::string> value; // only data token has value
};
using TokenLine = struct TokenLine {
    int line_no;
    std::vector<Token> tokens;
};
using BasicProgramLine = struct BasicProgramLine {
    int line_no;
    std::string line;
    bool operator<(const BasicProgramLine& other) const {
        return line_no < other.line_no;
    }
};
class TokenizerErr:public std::runtime_error {
public:
    explicit TokenizerErr(const std::string& msg) : std::runtime_error(msg) {}
};

// throws: TokenizerErr
static inline BasicProgramLine linefromStr(const std::string& str) {
    int line_no;
    std::istringstream iss(str);
    if(iss >> line_no) {
        std::string line;
        std::getline(iss, line);
        return {.line_no = line_no, .line=line};
    }
    std::vector<std::string> errStack;
    errStack.push_back(fmt::format("read invalid line: {}", str));
    errStack.push_back(fmt::format("lineno is not a number: {}", line_no));

    auto what = fmt::join(errStack.begin(), errStack.end(), "\n\t");
    throw TokenizerErr(fmt::to_string(what));
}
using BasicProgram = struct BasicProgram {
    std::map<int, std::string> lines;
};

// 保证program的数组按照line_no排序
static inline BasicProgram programFromlines(const std::vector<std::string>& lines) {
    BasicProgram p;
    for(const auto &l: lines) {
        auto line = linefromStr(l);
        p.lines[line.line_no] = line.line;
    }
    return p;
}

class Tokenizer {
    std::vector<TokenLine> token_lines;
    const std::map<TokenType, std::regex> regex_table;
    BasicProgram src_program;
    std::vector<Token> read_line(const std::string& line);
public:
    Tokenizer();
    void tokenize(BasicProgram&& program);
    void tokenize(const std::filesystem::path& file_path);
    // for test
    [[nodiscard]] std::vector<TokenLine> read_lines(const std::vector<std::string>& lines);
    ~Tokenizer() = default;
};
class Statement {
  public:
    virtual Result<int> read(const std::string &line);
};
class EndStatement : public Statement {
  public:
    Result<int> read(const std::string &line) override;
};
class InputStatement : public Statement {
  public:
    Result<int> read(const std::string &line) override;
};
class GotoStatement : public Statement {
    int jmp_line_no = -1;

  public:
    // getter & setter
    [[nodiscard]] int getJmp() const { return jmp_line_no; }
    void setJmp(int jmp) { jmp_line_no = jmp; }
    Result<int> read(const std::string &line) override;
};

class IFStatement : public Statement {
    int jmp_line_no = -1;

  public:
    // getter & setter
    [[nodiscard]] int getJmp() const { return jmp_line_no; }
    void setJmp(int jmp) { jmp_line_no = jmp; }
    Result<int> read(const std::string &line) override;
};

class PrintStatement : public Statement {
  public:
    Result<int> read(const std::string &line) override;
};

class AssignStatement : public Statement {

  public:
    std::string var_name;
    std::string exp;
    Result<int> read(const std::string &line) override;
};

} // namespace Token

#endif // TOKENIZER_H
