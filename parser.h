//
// Created by ayanami on 12/3/24.
//

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
        if (it != symbols.end()) {
            return std::nullopt;
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
    BinOp = "BinOp",
    UnaryOp = "UnaryOp",
    AssignStmt = "Assign",
    Var = "Var",
    Num = "Num",
    String = "String",
    Data = "Data",
    GOTOStmt = "GOTO",
    EndStmt = "END",
    PrintStmt = "Print",
    InputStmt = "Input",
    IFStmt = "IF",
    NoOp = "NoOp",
};
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

};
// throws std::runtime_error
inline std::variant<int, double> str2Number(const std::string& str) {
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

class NumNode: public ASTNode {
public:
    explicit NumNode(std::variant<int, double> num) {
        ASTNode::setValue(num);
    }
    ASTNodeType type() override {
        return ASTNodeType::Num;
    }
    [[nodiscard]] int getInt() const {
        auto num = std::any_cast<std::variant<int, double>>(getVal());
        int num_i;
        double num_d;
        if(std::holds_alternative<int>(num)) {
            num_i = std::get<int>(num);
            return num_i;
        } else {
            num_d = std::get<double>(num);
            return static_cast<int>(num_d);
        }
    }
    [[nodiscard]] double getDouble() const {
        auto num = std::any_cast<std::variant<int, double>>(getVal());
        int num_i;
        double num_d;
        if(std::holds_alternative<int>(num)) {
            num_i = std::get<int>(num);
            return num_i;
        } else {
            num_d = std::get<double>(num);
            return num_d;
        }
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
};
class UnaryOpNode: public ASTNode {
    ASTNode* expr;
    Token::TokenType op;
public:
    UnaryOpNode(ASTNode* expr, Token::TokenType op): expr(expr), op(op) {
        if(!Token::isUnaryOp(op)) {
            throw std::runtime_error(fmt::format("UnaryOpNode: Invalid unary operator {}", tk2Str(op)));
        }
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
    ~EndStmtNode() override = default;
};

class PrintStmtNode: public ASTNode {
    ASTNode* expr;
public:
    explicit PrintStmtNode(ASTNode* expr): expr(expr) {}
    ASTNodeType type() override {
        return ASTNodeType::PrintStmt;
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
    [[nodiscard]] auto getStmts() {
        return stmts;
    }
    void printAST(int line_no) const;
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
};
class Interpreter: public NodeVisitor {
private:
    Parser* parser{};
    Env* env{};
    ProgramStatus status{};

public:
    Interpreter() = default;
    ~Interpreter() override = default;
    void interpret();
    void visit(ASTNode *root) override;


    /*
     * 从ASTNode中获取值
     * 需要区分VarNode和其他Node
     */
    template<typename T>
    T getNodeVal(ASTNode* node) {
        if(node->type() != ASTNodeType::Var) {
            string var_name = dynamic_cast<VarNode*>(node)->getName();
            auto v = env->symbol_table.get<T>(var_name);
            if(!v.has_value()) {
                throw std::runtime_error(fmt::format("var {} not found", var_name));
            }
            return v.value();
        }
        return std::any_cast<T>(node->getVal());
    }

    void visit_BinOp(BinOpNode* node) {
        auto left_node = node->getLeft();
        auto right_node = node->getRight();

        auto left_v = getNodeVal<std::any>(left_node);
        auto right_v = getNodeVal<std::any>(right_node);
        std::any result;
        if(left_v.type() != right_v.type()) {
            throw std::runtime_error(fmt::format("BinOpNode: type {},{} Unmatched", left_v.type(), right_v.type()));
        }
        if(left_v.type() == typeid(int)) {
            switch (node->getOp()) {
                case Token::TokenType::OP_ADD:
                    result = std::any_cast<int>(left_v) + std::any_cast<int>(right_v);
                    break;
                case Token::TokenType::OP_SUB:
                    result = std::any_cast<int>(left_v) - std::any_cast<int>(right_v);
                    break;
                case Token::TokenType::OP_MUL:
                    result = std::any_cast<int>(left_v) * std::any_cast<int>(right_v);
                    break;
                case Token::TokenType::OP_DIV:
                    result = std::any_cast<int>(left_v) / std::any_cast<int>(right_v);
                    break;
                case Token::TokenType::OP_MOD:
                    result = std::any_cast<int>(left_v) % std::any_cast<int>(right_v);
                    break;
                case Token::TokenType::OP_POW:
                    result = std::pow(std::any_cast<int>(left_v), std::any_cast<int>(right_v));
                    break;
                case Token::TokenType::OP_GT:
                    result = std::any_cast<int>(left_v) > std::any_cast<int>(right_v);
                    break;
                case Token::TokenType::OP_LT:
                    result = std::any_cast<int>(left_v) < std::any_cast<int>(right_v);
                    break;
                case Token::TokenType::OP_GE:
                    result = std::any_cast<int>(left_v) >= std::any_cast<int>(right_v);
                    break;
                case Token::TokenType::OP_LE:
                    result = std::any_cast<int>(left_v) <= std::any_cast<int>(right_v);
                    break;
                case Token::TokenType::OP_EQ:
                    result = std::any_cast<int>(left_v) == std::any_cast<int>(right_v);
                    break;
                case Token::TokenType::OP_NE:
                    result = std::any_cast<int>(left_v) != std::any_cast<int>(right_v);
                    break;
                default:
                    throw std::runtime_error(fmt::format("BinOpNode Add {}: Invalid binary operator {}",
                        left_v.type(), tk2Str(node->getOp())));
            }
        }
        if(left_v.type() == typeid(double)) {
            switch (node->getOp()) {
                case Token::TokenType::OP_ADD:
                    result = std::any_cast<double>(left_v) + std::any_cast<double>(right_v);
                    break;
                case Token::TokenType::OP_SUB:
                    result = std::any_cast<double>(left_v) - std::any_cast<double>(right_v);
                    break;
                case Token::TokenType::OP_MUL:
                    result = std::any_cast<double>(left_v) * std::any_cast<double>(right_v);
                    break;
                case Token::TokenType::OP_DIV:
                    result = std::any_cast<double>(left_v) / std::any_cast<double>(right_v);
                    break;
                case Token::TokenType::OP_GT:
                    result = std::any_cast<double>(left_v) > std::any_cast<double>(right_v);
                    break;
                case Token::TokenType::OP_LT:
                    result = std::any_cast<double>(left_v) < std::any_cast<double>(right_v);
                    break;
                case Token::TokenType::OP_GE:
                    result = std::any_cast<double>(left_v) >= std::any_cast<double>(right_v);
                    break;
                case Token::TokenType::OP_LE:
                    result = std::any_cast<double>(left_v) <= std::any_cast<double>(right_v);
                    break;
                case Token::TokenType::OP_EQ:
                    result = std::any_cast<double>(left_v) == std::any_cast<double>(right_v);
                    break;
                case Token::TokenType::OP_NE:
                    result = std::any_cast<double>(left_v) != std::any_cast<double>(right_v);
                    break;
                default:
                    throw std::runtime_error(fmt::format("BinOpNode Add {}: Invalid binary operator {}", left_v.type(),tk2Str(node->getOp())));
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
                throw std::runtime_error(fmt::format("BinOpNode Add {}: Invalid binary operator {}", left_v.type(),tk2Str(node->getOp())));
            }
        }
        node->setValue(result);
    }
    void visit_UnaryOp(UnaryOpNode* node) {
        auto op = node->getOp();
        auto expr_node = node->getExpr();
        auto expr_v = getNodeVal<std::any>(expr_node);
        if(op == Token::TokenType::OP_ADD) {
            node->setValue(expr_v);
            return;
        }
        if (op == Token::TokenType::OP_SUB) {
            if(expr_v.type() == typeid(int)) {
                node->setValue(-std::any_cast<int>(expr_v));
            }
            if(expr_v.type() == typeid(double)) {
                node->setValue(-std::any_cast<double>(expr_v));
            }
            throw std::runtime_error(fmt::format("UnaryOpNode: Invalid type {}", expr_v.type()));
        }
        throw std::runtime_error(fmt::format("UnaryOpNode: Invalid unary operator {}", tk2Str(op)));
    }
    void visit_IFStmtNode(IFStmtNode* node) {
        auto cond = node->getCond();
        auto cond_v = getNodeVal<std::any>(cond);
        bool cond_result;
        if(cond_v.type() == typeid(int) || cond_v.type() == typeid(double)) {
            cond_result = std::any_cast<int>(cond_v) != 0;
        }
        else if(cond_v.type() == typeid(string)) {
            cond_result = !std::any_cast<string>(cond_v).empty();
        }
        else {
            throw std::runtime_error(fmt::format("IFStmtNode: Invalid type {}", cond_v.type()));
        }
        if(cond_result) {
            status.next_line = node->getNext();
        }
    }
    void visit_AssignStmtNode(AssignStmtNode* node) {
        auto left = node->getLeft();
        auto right = node->getRight();

        auto var_name = left->getName();
        if (!env->symbol_table.contains(var_name)) {
            throw std::runtime_error(fmt::format("AssignStmtNode: var {} not found", var_name));
        }
        // x = y
        if(right->type() == ASTNodeType::Var) {
            // ATTENTION: 值传递
            auto right_name = dynamic_cast<VarNode*>(right)->getName();
            auto right_v = env->symbol_table.get<std::any>(right_name);
            if(!right_v.has_value()) {
                throw std::runtime_error(fmt::format("AssignStmtNode: var {} not found", right_name));
            }
            env->symbol_table.set(var_name, right_v.value());
        }
        // x = 1
        env->symbol_table.set(var_name, right->getVal());
    }
    void visit_GOTOStmtNode(GOTOStmtNode* node) {
        status.next_line = node->getLineNo();
    }
    void visit_EndStmtNode(EndStmtNode* node) {
        status.running = false;
    }
    void visit_InputStmtNode(InputStmtNode* node) {
        // 默认是string, 如果可以转换成数字就转换成数字
        string input;
        std::cin >> input;
        auto num = str2Number(input);
        bool success = env->symbol_table.setIfExist(node->getVar()->getName(), num);
        if(!success) {
            throw std::runtime_error(fmt::format("InputStmtNode: var {} not found", node->getVar()->getName()));
        }
    }
    void visit_PrintStmtNode(PrintStmtNode* node) {
        auto expr = node->getExpr();
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
