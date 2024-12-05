//
// Created by ayanami on 12/2/24.
//
#pragma once
#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <filesystem>
#include <string>
#include <vector>
#include <regex>
#include "util.h"
#include <fmt/core.h>
#include <set>
namespace Token {
enum class TokenType {

    OP_EQ,
    OP_GE,
    OP_LE, // GE, LE > GT, LT
    OP_GT,
    OP_LT,
    OP_NE,

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
    TK_EOF,
    UNKNOWN,
};
static inline bool typeIn(TokenType tk, std::set<TokenType> tks) {
    return tks.find(tk) != tks.end();
}
inline std::string tk2Str(TokenType tk) {
    switch (tk) {
    case TokenType::OP_GT: return ">";
    case TokenType::OP_ADD: return "+";
    case TokenType::IF: return "IF";
    case TokenType::END: return "END";
    case TokenType::REM: return "REM(//)";
    case TokenType::OP_LT: return "<";
    case TokenType::OP_EQ: return "==";
    case TokenType::OP_GE: return ">=";
    case TokenType::OP_LE: return "<=";
    case TokenType::OP_NE: return "!=";
    case TokenType::OP_POW: return "**";
    case TokenType::OP_SUB: return "-";
    case TokenType::OP_MUL: return "*";
    case TokenType::OP_DIV: return "/";
    case TokenType::OP_MOD: return "MOD";
    case TokenType::ASSIGN: return "=";
    case TokenType::LET: return "LET";
    case TokenType::ELSE: return "ELSE";
    case TokenType::THEN: return "THEN";
    case TokenType::LPAREN: return "(";
    case TokenType::RPAREN: return ")";
    case TokenType::GOTO: return "GOTO";
    case TokenType::PRINT: return "PRINT";
    case TokenType::INPUT: return "INPUT";
    case TokenType::VAR: return "VAR";
    case TokenType::NUM: return "NUM";
    case TokenType::LITERAL: return "LITERAL";
    case TokenType::SPACE: return "SPACE";
    default:
        return "UNKNOWN";
    }
    throw std::runtime_error("tk2Str: Should not reach here");
}
inline bool isBinOp(TokenType tk) {
    switch (tk) {
        case TokenType::OP_GT:
        case TokenType::OP_LT:
        case TokenType::OP_EQ:
        case TokenType::OP_GE:
        case TokenType::OP_LE:
        case TokenType::OP_NE:
        case TokenType::OP_POW:
        case TokenType::OP_ADD:
        case TokenType::OP_SUB:
        case TokenType::OP_MUL:
        case TokenType::OP_DIV:
        case TokenType::OP_MOD:
            return true;
        default:
            return false;
    }
}
inline bool isRightAssociative(TokenType tk) {
    switch (tk) {
            case TokenType::OP_POW:
            return true;
            default:
            return false;
    }
}

inline bool isUnaryOp(TokenType tk) {
    switch (tk) {
        case TokenType::OP_ADD:
        case TokenType::OP_SUB:
            return true;
        default:
            return false;
    }
}
inline bool isKeyword(TokenType tk) {
    switch (tk) {
        case TokenType::IF:
        case TokenType::ELSE:
        case TokenType::LET:
        case TokenType::REM:
        case TokenType::INPUT:
        case TokenType::GOTO:
            return true;
        default:
            return false;
    }
}

inline bool isStmt(TokenType tk) {
    switch (tk) {
    case TokenType::IF:
    case TokenType::LET:
    case TokenType::INPUT:
    case TokenType::GOTO:
    case TokenType::END:
    case TokenType::PRINT:
        return true;
    default:
        return false;
    }
}

inline int getPriority(TokenType tk) {
    // 优先级越大, 越后计算
    switch (tk) {
    case TokenType::OP_POW:
        return 1;
    case TokenType::OP_MUL:
    case TokenType::OP_DIV:
    case TokenType::OP_MOD:
        return 2;
    case TokenType::OP_ADD:
    case TokenType::OP_SUB:
        return 3;
    case TokenType::OP_GT:
    case TokenType::OP_LT:
    case TokenType::OP_EQ:
    case TokenType::OP_GE:
    case TokenType::OP_LE:
    case TokenType::OP_NE:
        return 4;
    default:
        throw std::runtime_error(fmt::format("getPriority: {} has no priority", tk2Str(tk)));
    }
    throw std::runtime_error("getPriority: Should not reach here");
}

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
private:
    std::vector<TokenLine> token_lines;
    const std::map<TokenType, std::regex> regex_table;
    BasicProgram src_program;
    std::vector<Token> read_line(const std::string& line);
    int line_offset; // multi line 的 offset
    int inline_offset; // single line 的 offset
public:
    Tokenizer();
    ~Tokenizer() = default;
    void tokenize(BasicProgram&& program);
    void tokenize(const std::filesystem::path& file_path);
    void resetOff() {
        line_offset = 0;
        inline_offset = 0;
    }
    void resetAll() {
        resetOff();
        token_lines.clear();
    }
    void reload(const std::filesystem::path& p) {
        resetAll();
        tokenize(p);
    }
    void reload(BasicProgram&& p) {
        resetAll();
        tokenize(std::move(p));
    }
    // for test
    [[nodiscard]] std::vector<TokenLine> read_lines(const std::vector<std::string>& lines);
    [[nodiscard]] auto get_token_lines() const { return token_lines; }
    [[nodiscard]] auto get_single_line() const { return token_lines[line_offset]; }
    [[nodiscard]] Token peek() const;
    [[nodiscard]] Token prev() const;
    Token eat(TokenType tk);
    [[nodiscard]] int get_line_offset() const {
        return line_offset;
    }

    void set_line_offset(int line_offset) {
        this->line_offset = line_offset;
    }

    [[nodiscard]] int get_inline_offset() const {
        return inline_offset;
    }

    void set_inline_offset(int inline_offset) {
        this->inline_offset = inline_offset;
    }

};
class TokenEOFErr: public std::runtime_error {
    std::string msg;
public:
    TokenEOFErr(const std::string& msg) : std::runtime_error(msg) {}
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
