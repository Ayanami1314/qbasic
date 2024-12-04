//
// Created by ayanami on 12/3/24.
//
#pragma once

#ifndef PARSER_H
#define PARSER_H
#include <string>
#include <utility>
#include <vector>
#include <fmt/core.h>
#include <map>
#include <unordered_map>
#include <any>
#include <optional>
#include <variant>
#include <iostream>
#include "tokenizer.h"
using std::string;
using std::vector;
using fmt::print;
using std::map;
using std::unordered_map;

// QBasic 不支持函数所以暂时没有stack frame
class SymbolTable {
private:
    unordered_map<string, std::any> symbols;
public:
    template<typename T>
    void set(const string& key, const T& value) {
        symbols[key] = value;
    }
    template<typename T>
    bool setIfExist(const string& key, const T& value) {
        if(symbols.contains(key)) {
            symbols[key] = value;
            return true;
        }
        return false;
    }
    template<typename T>
    std::optional<T> get(const string& key) const {
        auto it = symbols.find(key);
        if (it == symbols.end()) {
            return std::nullopt;
        }
        if constexpr (std::is_same_v<T, std::any>) {
            return it->second;
        }
        return std::any_cast<T>(it->second);
    }

    bool contains(const string& key) const {
        return symbols.contains(key);
    }
    void clear() {
        symbols.clear();
    }
    void printSymbols() {
        for(const auto& [key, value]: symbols) {
            print("key: {}, value: {}\n", key, std::any_cast<int>(value));
        }
    }

};


enum class ASTNodeType {
    BinOp,
    UnaryOp,
    AssignStmt,
    Var,
    Num,
    String,
    Data,
    GOTOStmt,
    EndStmt,
    PrintStmt,
    InputStmt,
    IFStmt,
    NoOp,
};
inline string ast2Str(ASTNodeType type) {
    switch (type) {
    case ASTNodeType::BinOp:
        return "BinOp";
    case ASTNodeType::UnaryOp:
        return "UnaryOp";
        case ASTNodeType::AssignStmt:
            return "=";
        case ASTNodeType::Var:
            return "var";
        case ASTNodeType::Num:
            return "num";
        case ASTNodeType::String:
            return "string";
        case ASTNodeType::Data:
            return "data";
        case ASTNodeType::GOTOStmt:
            return "GOTO";
        case ASTNodeType::EndStmt:
            return "END";
        case ASTNodeType::PrintStmt:
            return "PRINT";
        case ASTNodeType::InputStmt:
            return "INPUT";
        case ASTNodeType::IFStmt:
            return "IF";
        case ASTNodeType::NoOp:
            return "NoOp";
        default:
            throw std::runtime_error("ast2Str: Invalid ASTNodeType");
    }
}
class ASTNode {
    std::any value {};
public:
    virtual ~ASTNode() = default;
    ASTNode() = default;
    virtual void setValue(std::any v) {
        value = std::move(v);
    }
    [[nodiscard]] virtual std::any getVal() const {
        return value;
    }
    virtual ASTNodeType type() = 0;
    virtual string toString() = 0;

};
using NumType = std::variant<int, double>;
// throws std::runtime_error
inline NumType str2Number(const std::string& str) {
    try {
        size_t pos;
        int intValue = std::stoi(str, &pos);
        if (pos == str.size()) {
            return intValue;
        }
    } catch (const std::invalid_argument&) {
        // Ignore and try to parse as double
    }

    try {
        size_t pos;
        double doubleValue = std::stod(str, &pos);
        if (pos == str.size()) {
            return doubleValue;
        }
    } catch (const std::invalid_argument&) {
        // Ignore and throw error
    }
    throw std::runtime_error("Invalid number format: " + str);
}
inline int str2Int(const std::string& str) {
    return std::stoi(str);
}
inline double str2Double(const std::string& str) {
    return std::stod(str);
}

// 为了简化代码, 不存储variant，存原始类型
class NumNode: public ASTNode {
public:
    explicit NumNode(NumType num) {
        if(std::holds_alternative<int>(num)) {
            ASTNode::setValue(std::get<int>(num));
        } else {
            ASTNode::setValue(std::get<double>(num));
        }
    }
    ASTNodeType type() override {
        return ASTNodeType::Num;
    }
    string toString() override {
        auto num = getVal();
        if(util::ConvAny<int>(num)) {
            return std::to_string(std::any_cast<int>(num));
        }
        return std::to_string(std::any_cast<double>(num));
    }
    [[nodiscard]] int getInt() const {
        if(util::ConvAny<int>(getVal())) {
            return std::any_cast<int>(getVal());
        }
        return static_cast<int>(std::any_cast<double>(getVal()));
    }
    [[nodiscard]] double getDouble() const {
        if(util::ConvAny<double>(getVal())) {
            return std::any_cast<double>(getVal());
        }
        return static_cast<double>(std::any_cast<int>(getVal()));
    }
};


class StringNode: public ASTNode {
public:
    explicit StringNode(const string& s) {
        ASTNode::setValue(s);
    }
    ASTNodeType type() override {
            return ASTNodeType::String;
    }
    string toString() override {
            return std::any_cast<string>(getVal());
    }
    [[nodiscard]] string getString() const {
            return std::any_cast<string>(getVal());
    }
};
class DataNode: public ASTNode {
    std::any data;
public:
    explicit DataNode(std::any v) {
        ASTNode::setValue(std::move(v));
    }
    ASTNodeType type() override {
        return ASTNodeType::Data;
    }
    string toString() override {
            return "Data";
    }
    [[nodiscard]] std::any getData() const {
            return std::any_cast<std::any>(getVal());
    }
};

class BinOpNode: public ASTNode {
    ASTNode* left;
    ASTNode* right;
    Token::TokenType op;
public:
    BinOpNode(ASTNode* left, ASTNode* right, Token::TokenType op): left(left), right(right), op(op) {
        if(!isBinOp(op)) {
            throw std::runtime_error("BinOpNode: Invalid binary operator");
        }
    }
    // getter setter
    ASTNode* getLeft() {
        return left;
    }
    ASTNode* getRight() {
        return right;
    }
    Token::TokenType getOp() {
            return op;
    }
    void setLeft(ASTNode *l) {
        this->left = l;
    }
    void setRight(ASTNode *r) {
        this->right = r;
    }
    void setOp(Token::TokenType o) {
            this->op = o;
    }
    ASTNodeType type() override {
        return ASTNodeType::BinOp;
    }
    string toString() override {
        return fmt::format("BinOpNode: {} {} {}", left->toString(),  tk2Str(op), right->toString());
    }
};
class UnaryOpNode: public ASTNode {
    ASTNode* expr;
    Token::TokenType op;
public:
    UnaryOpNode(ASTNode* expr, Token::TokenType op): expr(expr), op(op) {
        if(!Token::isUnaryOp(op)) {
            auto s = fmt::format("UnaryOpNode: Invalid unary operator {}", tk2Str(op));
            throw std::runtime_error(s);
        }
    }
    string toString() override {
            return fmt::format("UnaryOpNode: {}{}", tk2Str(op), expr->toString());
    }
    ASTNodeType type() override {
            return ASTNodeType::UnaryOp;
    }
    // getter setter
    ASTNode* getExpr() {
            return expr;
    }
    Token::TokenType getOp() {
            return op;
    }
    void setExpr(ASTNode* e) {
        this->expr = e;
    }
    void setOp(Token::TokenType o) {
        this->op = o;
    }

};


class VarNode: public ASTNode {
    string name;
public:
    explicit VarNode(string varName): name(std::move(varName)) {}
    ASTNodeType type() override {
            return ASTNodeType::Var;
    }
    [[nodiscard]] string getName() const {
            return name;
    }
    string toString() override {
            return name;
    }
    void setName(string n) {
        name = std::move(n);
    }
    [[nodiscard]] std::any getVal() const override {
        throw std::runtime_error("You should never getVal from var: lookup var in env");
    }
    void setValue(std::any v) override {
        throw std::runtime_error("You should never setVal from var: lookup var in env");
    }

};

class AssignStmtNode: public ASTNode {
private:
    VarNode* left;
    ASTNode* right;
public:
    AssignStmtNode(VarNode* left, ASTNode* right): left(left), right(right){}
    ~AssignStmtNode() override = default;
    ASTNodeType type() override {
        return ASTNodeType::AssignStmt;
    }
    string toString() override {
            return fmt::format("AssignStmtNode: {} = {}", left->toString(), right->toString());
    }
    [[nodiscard]] VarNode * getLeft() const {
        return left;
    }

    void setLeft(VarNode *left) {
        this->left = left;
    }

    [[nodiscard]] ASTNode * getRight() const {
        return right;
    }

    void setRight(ASTNode *right) {
        this->right = right;
    }

};

// QBasic 只支持goto 常数
class GOTOStmtNode: public ASTNode {
    // 是 next_stmt更好还是line_no？
    int line_no;
public:
    explicit GOTOStmtNode(int lineNo): line_no(lineNo) {}
    ASTNodeType type() override {
        return ASTNodeType::GOTOStmt;
    }
    string toString() override {
            return fmt::format("GOTOStmtNode: GOTO {}", line_no);
    }
    [[nodiscard]] int getLineNo() const{
        return line_no;
    }
    void setLineNo(int lineNo) {
        line_no = lineNo;
    }
    ~GOTOStmtNode() override = default;
};


class EndStmtNode: public ASTNode {
public:
    ASTNodeType type() override {
        return ASTNodeType::EndStmt;
    }
    string toString() override {
            return "EndStmtNode";
    }
    ~EndStmtNode() override = default;
};

class PrintStmtNode: public ASTNode {
    ASTNode* expr;
public:
    explicit PrintStmtNode(ASTNode* expr): expr(expr) {}
    ASTNodeType type() override {
        return ASTNodeType::PrintStmt;
    }
    string toString() override {
            return fmt::format("PrintStmtNode: PRINT {}", expr->toString());
    }
    ~PrintStmtNode() override = default;
    // getter setter
    ASTNode* getExpr() {
            return expr;
    }
    void setExpr(ASTNode* e) {
        this->expr = e;
    }
};


class InputStmtNode: public ASTNode {
    VarNode* var;
public:
    explicit InputStmtNode(VarNode* v): var(v) {}
    ~InputStmtNode() override = default;
    string toString() override {
            return fmt::format("InputStmtNode: INPUT {}", var->toString());
    }
    // getter setter
    ASTNodeType type() override {
            return ASTNodeType::InputStmt;
    }
    VarNode* getVar() {
        return var;
    }
    void setVar(VarNode* v) {
        this->var = v;
    }

};


class IFStmtNode: public ASTNode {
private:
    ASTNode* cond;
    int next_if_match = -1;
public:
    explicit IFStmtNode(ASTNode* expr, int next): cond(expr) , next_if_match(next) {};
    ~IFStmtNode() override = default;
    string toString() override {
            return fmt::format("IFStmtNode: IF {} THEN {}", cond->toString(), next_if_match);
    }
    ASTNode* getCond() {
        return cond;
    }
    ASTNode* setCond() {
        return cond;
    }
    ASTNodeType type() override {
            return ASTNodeType::IFStmt;
    }

    [[nodiscard]] int getNext() const {
        return next_if_match;
    }

    void setNext(int next_if_match) {
        this->next_if_match = next_if_match;
    }

};
inline bool belongsDataNode(ASTNodeType type) {
    return type == ASTNodeType::Num || type == ASTNodeType::String |
        type == ASTNodeType::Var | type == ASTNodeType::Data;
}
class NodeVisitor {
public:
    virtual ~NodeVisitor() = default;
    virtual void visit(ASTNode* node) = 0;
};

class Parser {
private:
    Token::Tokenizer* tokenizer;
    SymbolTable* sym_table;
    std::map<int, ASTNode*> stmts; // based on line_no
public:
    explicit Parser(Token::Tokenizer* tokenizer, SymbolTable *table): tokenizer(tokenizer), sym_table(table) {
    }
    NumNode* parseNum();
    StringNode* parseString();
    ASTNode* factor();
    ASTNode* term_p1();
    ASTNode* term_p2();
    ASTNode* term_p3();
    ASTNode* term_p4();
    ASTNode* expr();
    VarNode* parseVar();
    AssignStmtNode* parseAssignStmt();
    GOTOStmtNode* parseGOTOStmt();
    EndStmtNode* parseEndStmt();
    PrintStmtNode* parsePrintStmt();
    InputStmtNode* parseInputStmt();
    IFStmtNode* parseIFStmt();
    void parseProgram();
    void parseSingleLine();
    [[nodiscard]] auto getStmts() {
        return stmts;
    }
    void printAST(int line_no) const;
    void printAST(ASTNode* root) const;
    ~Parser() {
        delete tokenizer;
    }
};
class Env {
public:
    SymbolTable symbol_table {};
    Env() = default;

};
using ProgramStatus = struct ProgramStatus {
    int current_line = -1;
    int next_line = 0;
    std::filesystem::path current_file;
    bool running = false;
    std::optional<std::string> err_msg;
    std::string input_str{};
    std::string output_str{};
};
class Interpreter: public NodeVisitor {
private:
    Parser* parser{};
    Env* env{};
    ProgramStatus status{};

public:
    explicit Interpreter(Parser* p): parser(p) {
        env = new Env();
    };
    ~Interpreter() override = default;
    void interpret();
    void interpret_SingleStep();
    void visit(ASTNode *root) override;
    // TODO simulate IO
    void input(const std::string& input) {
        status.input_str += input;
    }
    string output() {
        auto out_s = status.output_str;
        status.output_str.clear();
        return out_s;
    }
    Parser *getParser() const {
        return parser;
    }
    Env *getEnv() const {
        return env;
    }
    ProgramStatus getStatus() const {
        return status;
    }


    /*
     * 从ASTNode中获取值
     * 需要区分VarNode和其他Node
     */
    template<typename T>
    T getNodeVal(ASTNode* node) {
        if(node->type() == ASTNodeType::Var) {
            string var_name = dynamic_cast<VarNode*>(node)->getName();
            auto v = env->symbol_table.get<T>(var_name);
            if(!v.has_value()) {
                throw std::runtime_error(fmt::format("var {} not found", var_name));
            }
            return v.value();
        }
        if constexpr (std::is_same_v<T, std::any>) {
            return node->getVal(); // any_cast is exactly equal
        } else {
            return std::any_cast<T>(node->getVal());
        }
    }
    void visit_BinOp(BinOpNode* node) {
        auto left_node = node->getLeft();
        auto right_node = node->getRight();
        if(left_node == nullptr || right_node == nullptr) {
            throw std::runtime_error("BinOpNode: left or right is nullptr");
        }
        visit(left_node);
        visit(right_node);

        auto left_v = getNodeVal<std::any>(left_node);
        auto right_v = getNodeVal<std::any>(right_node);
        std::optional<std::any> result;
        // HINT: 不尝试做类型转换，直接报错
        if(left_v.type() != right_v.type()) {
            throw std::runtime_error("BinOpNode: value type Unmatched");
        }
        // print("value type: {}\n",left_v.type().name());
        if(left_v.type() == typeid(int) || left_v.type() == typeid(double)) {
            if(util::ConvAny<int>(left_v)) {
                int left_i = std::any_cast<int>(left_v);
                int right_i = std::any_cast<int>(right_v);
                switch (node->getOp()) {
                case Token::TokenType::OP_ADD:
                    result = left_i + right_i;
                    break;
                case Token::TokenType::OP_SUB:
                    result = left_i - right_i;
                    break;
                case Token::TokenType::OP_MUL:
                    result = left_i * right_i;
                    break;
                case Token::TokenType::OP_DIV:
                    if (right_i == 0) {
                        throw std::runtime_error("BinOpNode: Division by zero");
                    }
                    result = left_i / right_i;
                    break;
                case Token::TokenType::OP_MOD:
                    if (right_i == 0) {
                        throw std::runtime_error("BinOpNode: MOD by zero");
                    }
                    result = left_i % right_i;
                    break;
                case Token::TokenType::OP_POW:
                    result = static_cast<int>(std::pow(left_i, right_i));
                    break;
                case Token::TokenType::OP_GT:
                    result = left_i > right_i;
                    break;
                case Token::TokenType::OP_LT:
                    result = left_i < right_i;
                    break;
                case Token::TokenType::OP_GE:
                    result = left_i >= right_i;
                    break;
                case Token::TokenType::OP_LE:
                    result = left_i <= right_i;
                    break;
                case Token::TokenType::OP_EQ:
                    result = left_i == right_i;
                    break;
                case Token::TokenType::OP_NE:
                    result = left_i != right_i;
                    break;
                default:
                    throw std::runtime_error(fmt::format("BinOpNode Add: Invalid binary operator {}",
                    tk2Str(node->getOp())));
                }
            } else if (util::ConvAny<double>(left_v)) {
                double left_d = std::any_cast<double>(left_v);
                double right_d = std::any_cast<double>(right_v);
                switch (node->getOp()) {
                case Token::TokenType::OP_ADD:
                    result = left_d + right_d;
                    break;
                case Token::TokenType::OP_SUB:
                    result = left_d - right_d;
                    break;
                case Token::TokenType::OP_MUL:
                    result = left_d * right_d;
                    break;
                case Token::TokenType::OP_DIV:
                    if (right_d == 0) {
                        throw std::runtime_error("BinOpNode: Division by zero");
                    }
                    result = left_d / right_d;
                    break;
                case Token::TokenType::OP_POW:
                    result = std::pow(left_d, right_d);
                    break;
                case Token::TokenType::OP_GT:
                    result = left_d > right_d;
                    break;
                case Token::TokenType::OP_LT:
                    result = left_d < right_d;
                    break;
                case Token::TokenType::OP_GE:
                    result = left_d >= right_d;
                    break;
                case Token::TokenType::OP_LE:
                    result = left_d <= right_d;
                    break;
                case Token::TokenType::OP_EQ:
                    result = left_d == right_d;
                    break;
                case Token::TokenType::OP_NE:
                    result = left_d != right_d;
                    break;
                default:
                    auto s = fmt::format("BinOpNode Add: Invalid binary operator {}",
                        tk2Str(node->getOp()));
                    throw std::runtime_error(s);
                }
            }
        }
        if(left_v.type() == typeid(string)) {
            switch (node->getOp()) {
            case Token::TokenType::OP_ADD:
                result = std::any_cast<string>(left_v) + std::any_cast<string>(right_v);
                break;
            case Token::TokenType::OP_GT:
                result = std::any_cast<string>(left_v) > std::any_cast<string>(right_v);
                break;
            case Token::TokenType::OP_LT:
                result = std::any_cast<string>(left_v) < std::any_cast<string>(right_v);
                break;
            case Token::TokenType::OP_GE:
                result = std::any_cast<string>(left_v) >= std::any_cast<string>(right_v);
                break;
            case Token::TokenType::OP_LE:
                result = std::any_cast<string>(left_v) <= std::any_cast<string>(right_v);
                break;
            case Token::TokenType::OP_EQ:
                result = std::any_cast<string>(left_v) == std::any_cast<string>(right_v);
                break;
            case Token::TokenType::OP_NE:
                result = std::any_cast<string>(left_v) != std::any_cast<string>(right_v);
                break;
            default:
                string s = fmt::format("BinOpNode Add: Invalid binary operator {}",
                tk2Str(node->getOp()));
                throw std::runtime_error(s);
            }
        }
        if (!result.has_value()) {
            string s = fmt::format("BinOpNode: Invalid value type {}", ast2Str(node->type()));
            throw std::runtime_error(s);
        }
        node->setValue(result.value());
    }
    void visit_UnaryOp(UnaryOpNode* node) {
        auto op = node->getOp();
        auto expr_node = node->getExpr();
        visit(expr_node);
        auto expr_v = getNodeVal<std::any>(expr_node);
        if(op == Token::TokenType::OP_ADD) {
            node->setValue(expr_v);
            return;
        }
        if (op == Token::TokenType::OP_SUB) {
            if(expr_v.type() == typeid(int)) {
                node->setValue(-std::any_cast<int>(expr_v));
                return;
            }
            if(expr_v.type() == typeid(double)) {
                node->setValue(-std::any_cast<double>(expr_v));
                return;
            }
            string s = fmt::format("UnaryOpNode: Invalid value type {}", ast2Str(node->type()));
            throw std::runtime_error(s);

        }
        string s = fmt::format("UnaryOpNode: Invalid unary operator {}", tk2Str(op));
        throw std::runtime_error(s);
    }
    void visit_Expr(ASTNode* node) {
        // node 要么是Op要么是Data
        if (node->type() == ASTNodeType::BinOp) {
            visit_BinOp(dynamic_cast<BinOpNode*>(node));
            return;
        }
        if (node->type() == ASTNodeType::UnaryOp) {
            visit_UnaryOp(dynamic_cast<UnaryOpNode*>(node));
            return;
        }
        if (node->type() == ASTNodeType::Num) {
            node->setValue(dynamic_cast<NumNode*>(node)->getVal());
            return;
        }
        if (node->type() == ASTNodeType::Var) {
            string var_name = dynamic_cast<VarNode*>(node)->getName();
            auto v = env->symbol_table.get<std::any>(var_name);
            if(!v.has_value()) {
                throw std::runtime_error(fmt::format("var {} not found", var_name));
            }
            // var never set value(lookup)
            return;
        }
        if (node->type() == ASTNodeType::String) {
            node->setValue(dynamic_cast<StringNode*>(node)->getString());
            return;
        }
        string s = fmt::format("Expr: Invalid type {}", ast2Str(node->type()));
        throw std::runtime_error(s);
    }
    void visit_IFStmtNode(IFStmtNode* node) {
        auto cond = node->getCond();
        visit_Expr(cond);
        auto cond_v = getNodeVal<std::any>(cond);
        bool cond_result;
        if(cond_v.type() == typeid(int) || cond_v.type() == typeid(double)) {
            cond_result = std::any_cast<int>(cond_v) != 0;
        }
        else if(cond_v.type() == typeid(string)) {
            cond_result = !std::any_cast<string>(cond_v).empty();
        }
        else {
            string s = fmt::format("IFStmtNode: Invalid value type {}", ast2Str(node->type()));
            throw std::runtime_error(s);
        }
        if(cond_result) {
            status.next_line = node->getNext();
        }
    }
    void visit_AssignStmtNode(AssignStmtNode* node) {
        auto left = node->getLeft();
        auto right = node->getRight();

        auto var_name = left->getName();
        visit_Expr(right);
        auto right_v = getNodeVal<std::any>(right);
        env->symbol_table.set(var_name, right_v);
        node->setValue(right_v);
    }
    void visit_GOTOStmtNode(GOTOStmtNode* node) {
        status.next_line = node->getLineNo();
        node->setValue(status.next_line);
    }
    void visit_EndStmtNode(EndStmtNode* node) {
        status.running = false;
    }
    void visit_InputStmtNode(InputStmtNode* node) {
        // 默认是string, 如果可以转换成数字就转换成数字
        string input;
        std::cin >> input;
        // HINT: INPUT n 定义n
        try {
            auto num = str2Number(input);
            if(std::holds_alternative<int>(num)) {
                int num_i = std::get<int>(num);
                env->symbol_table.set<int>(node->getVar()->getName(), num_i);
                node->setValue(num_i);
            } else {
                double num_d = std::get<double>(num);
                env->symbol_table.set<double>(node->getVar()->getName(), num_d);
                node->setValue(num_d);
            }
        } catch (std::exception& e) {
            env->symbol_table.set<string>(node->getVar()->getName(), input);
            node->setValue(input);
        }
    }
    void visit_PrintStmtNode(PrintStmtNode* node) {
        auto expr = node->getExpr();
        visit_Expr(expr);
        auto expr_v = getNodeVal<std::any>(expr);
        if(expr_v.type() == typeid(int)) {
            print("{}", std::any_cast<int>(expr_v));
        }
        if(expr_v.type() == typeid(double)) {
            print("{}", std::any_cast<double>(expr_v));
        }
        if(expr_v.type() == typeid(string)) {
            print("{}", std::any_cast<string>(expr_v));
        }
    }
};




#endif //PARSER_H
