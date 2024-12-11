//
// Created by ayanami on 12/2/24.
//
#include "tokenizer.h"
#include "util.h"
#include <cassert>
#include <optional>
#include "message.h"
#include <map>
#include <fstream>
#include <ranges>

namespace Token {

// 所有都只有一个捕获组
// ATTENTION: 按照TokenType的优先级排序!!!
const std::map<TokenType, std::regex> basic_language_regex_table = {
        {TokenType::OP_GE, std::regex(R"(>=)")},
        {TokenType::OP_LE, std::regex(R"(<=)")},
        {TokenType::OP_GT, std::regex(R"(>)")},
        {TokenType::OP_LT, std::regex(R"(<)")},
        {TokenType::OP_EQ, std::regex(R"(=)")}, // changed this from == to =
        {TokenType::OP_NE, std::regex(R"(\!=)")},
        {TokenType::OP_POW, std::regex(R"(\*\*)")},
        {TokenType::OP_ADD, std::regex(R"(\+)")},
        {TokenType::OP_SUB, std::regex(R"(-)")},
        {TokenType::OP_MUL, std::regex(R"(\*)")},
        {TokenType::OP_DIV, std::regex(R"(/)")},
        {TokenType::OP_MOD, std::regex(R"(MOD)")},
        {TokenType::ASSIGN, std::regex(R"(=)")},
        {TokenType::LET, std::regex(R"(LET)")},
        {TokenType::IF, std::regex(R"(IF)")},
        {TokenType::ELSE, std::regex(R"(ELSE)")},
        {TokenType::THEN, std::regex(R"(THEN)")},
        {TokenType::REM, std::regex(R"(REM)")},
        {TokenType::LPAREN, std::regex(R"(\()")},
        {TokenType::RPAREN, std::regex(R"(\))")},
        {TokenType::GOTO, std::regex(R"(GOTO)")},
        {TokenType::END, std::regex(R"(END)")},
        {TokenType::PRINT, std::regex(R"(PRINT)")},
        {TokenType::INPUT, std::regex(R"(INPUT)")},
        {TokenType::VAR, std::regex(R"([a-zA-Z_][a-zA-Z0-9_]*)")},
        {TokenType::NUM, std::regex(R"(\d+)")},
        {TokenType::LITERAL, std::regex(R"(".*?")")}, // "some string"
        {TokenType::SPACE, std::regex(R"(\s+)")},
};
Tokenizer::Tokenizer() :regex_table(basic_language_regex_table){
    line_offset = 0;
    inline_offset = 0;
}

std::vector<Token> Tokenizer::read_line(const std::string &line) const {
    std::vector<std::string> words = util::split_by_space(line);
    // HINT: regex_match只考虑完全匹配
    size_t offset = 0;
    const std::string match_str = line;
    size_t limit = line.size();
    std::vector<Token> tokens;
    while(offset < limit) {
        bool found = false;
        for(const auto& [tk_type, tk_re]: regex_table) {
            std::smatch m;
            std::regex_search(match_str.begin() + offset, match_str.end(), m, tk_re);

            // 只匹配相同开头(position == 0)的,
            if(!m.empty() && m.position(0) == 0) {
                if (tk_type == TokenType::REM) {
                    // return tokens;
                    // lab doc says rem should be a stmt, make it happy
                    tokens.push_back({TokenType::REM, ""});
                    offset += m.length(0); // += rem length
                    tokens.push_back({TokenType::LITERAL, line.substr(offset)});
                    return tokens;
                }
                if(tk_type == TokenType::OP_EQ || tk_type == TokenType::ASSIGN) {
                    auto first = tokens.empty() ? TokenType::UNKNOWN: tokens.front().type;
                    // ATTENTION: change == to = make OP_EQ can't be used in expr
                    // Just make lab doc happy
                    if(first == TokenType::IF) {
                        fmt::print("found token: {}, {}\n", m[0].str(), tk2Str(tk_type));
                        tokens.push_back({TokenType::OP_EQ, "="});
                    } else {
                        fmt::print("found token: {}, {}\n", m[0].str(), tk2Str(tk_type));
                        tokens.push_back({TokenType::ASSIGN, "="});
                    }
                } else if(tk_type != TokenType::SPACE) {
                    fmt::print("found token: {}\n", m[0].str());
                    tokens.push_back({tk_type, m[0].str()});
                }
                offset += m.length(0);
                found = true;
                break;
            }
        }
        if (!found) {
            fmt::print("Not found token in: {}\n", line.substr(offset));
             // throw TokenizerErr(fmt::format("invalid seq in line: {}", line));
            break;
        }
    }
    return tokens;
}

void Tokenizer::tokenize(BasicProgram&& program) {
    std::vector<TokenLine> res;
    int max_line_no = 0;
    for(const auto& [line_no, line]: program.lines) {
        auto tokens = read_line(line);
        if(tokens.empty()) {
            continue;
        }
        max_line_no = std::max(max_line_no, line_no);
        res.push_back({line_no, tokens});
    }
    std::ranges::sort(res, [](const TokenLine& a, const TokenLine& b) {
        return a.line_no < b.line_no;
    });
    this->token_lines = res;
    this->src_program = program;
}

void Tokenizer::tokenize(const std::filesystem::path& file_path) {
    std::ifstream ifs(file_path);
    if(!ifs.is_open()) {
        // HINT: fmt::format 只能接受字面量, constexpr需要改造
        throw TokenizerErr(fmt::format("Failed to open file: {}", file_path.string()));
    }
    std::vector<std::string> lines;
    std::string line;
    while(std::getline(ifs, line)) {
        lines.push_back(line);
    }
    auto program = programFromlines(lines);
    this->tokenize(std::move(program));
    ifs.close();
}
// hint: line can be with or without line_no
// should only be called by alone one line tokenizer
void Tokenizer::read_anonymous_line(const std::string& line) {
    std::string line_with_no = line;

    std::istringstream iss(line);
    int line_no;

    iss >> line_no;
    if (iss.fail()) {
        line_with_no = "1 " + line;
    }
    return this->tokenize(programFromlines({line_with_no}));
}
std::vector<TokenLine> Tokenizer::read_lines(const std::vector<std::string>& lines) {
    BasicProgram p;
    for(const auto& l: lines) {
        auto line = linefromStr(l);
        p.lines[line.line_no] = line.line;
    }
    this->tokenize(std::move(p));
    return this->token_lines;
}

Token Tokenizer::peek() const {
    // 进位在eat中处理
    if(line_offset >= token_lines.size()) {
        return {TokenType::TK_EOF, ""};
    }
    return token_lines[line_offset].tokens[inline_offset];
}
[[nodiscard]] Token Tokenizer::prev() const {
    try {
        int line_off = inline_offset == 0 ? line_offset - 1: line_offset;
        int inline_off = inline_offset == 0 ? int(token_lines[line_off].tokens.size()) - 1: inline_offset - 1;
        Token ret = token_lines[line_off].tokens[inline_off];
        return ret;
    } catch (std::exception& e) {
        throw TokenizerErr(fmt::format("prev token error: {}", e.what()));
    }
}
Token Tokenizer::eat(TokenType tk) {
    auto next = this->peek();
    if(next.type != tk) {
        throw TokenizerErr(fmt::format("expect token: {}, but got: {}", tk2Str(tk), tk2Str(next.type)));
    }
    inline_offset++;
    if(inline_offset >= token_lines[line_offset].tokens.size()) {
        line_offset++;
        inline_offset = 0;
    }
    return next;
}


} // namespace Token
