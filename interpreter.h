//
// Created by ayanami on 12/4/24.
//
#pragma once

#ifndef INTREPRETER_H
#define INTREPRETER_H

#include <QEventLoop>
#include <QObject>
#include <concepts>
#include "parser.h"
using std::string;
using std::vector;
// using fmt::print;
using fmt::format;
using std::map;
using std::unordered_map;


class Env {
public:
    std::shared_ptr<SymbolTable> symbol_table {};
    explicit Env(std::shared_ptr<SymbolTable> table) {
        symbol_table = table;
    }
    void print() {
        symbol_table->printSymbols();
    }
    auto getRepl() const {
        return symbol_table->getRepl();
    }

};
enum class ProgramMode {
    NORMAL,
    DEBUG,
    DEV,
};
using ProgramStatus = struct ProgramStatus {
    int current_line = -1;
    int next_line = 0;
    std::filesystem::path current_file;
    bool running = false;
    bool blocking = false;
    std::optional<std::string> err_msg;
    ProgramMode mode = ProgramMode::DEV;
    std::set<int> breakpoints;
    void reload() {
        current_line = -1;
        next_line = 0;
        running = false;
        err_msg = {};
    }
    void reset() {
        reload();
        breakpoints.clear();
    }
    void add_breakpoint(int line) {
        print("interpreter: add breakpoint: {}\n", line);
        breakpoints.insert(line);
    }
    void delete_breakpoint(int line) {
        for(auto it = breakpoints.begin(); it != breakpoints.end(); ++it) {
            if(*it == line) {
                breakpoints.erase(it);
                return;
            }
        }
    }
    [[nodiscard]] bool break_at(int line_no) const {
        return breakpoints.contains(line_no);
    }

};

template<typename T>
concept Streamable = requires(T& var) {
    { std::cin >> var } -> std::convertible_to<std::istream&>;
    { std::cout << var } -> std::convertible_to<std::ostream&>;
};

class MockInputStream : public QObject{
    Q_OBJECT
    std::deque<string> inputs;
    bool fail = false;
    bool bad = false;
    bool waiting = false;
    std::mutex mtx;
public slots:
    void receiveInput(std::string input) {
        const std::lock_guard<std::mutex> lock(mtx);
        inputs.push_back(input);
        emit inputReady();
    }
signals:
    void inputReady();
    void waitingForInput();
public:
    explicit MockInputStream() = default;
    template<Streamable T>
    void requireInput(T& var) {
        QEventLoop loop;
        connect(this, &MockInputStream::inputReady, &loop, &QEventLoop::quit);
        waiting = true;
        emit waitingForInput();
        loop.exec();
        {
            if(!inputs.empty()) {
                // in clear, may be empty
                const std::lock_guard<std::mutex> lock(mtx);
                std::istringstream iss(inputs.front());
                iss >> var;
                fail = iss.fail();
                bad = iss.bad();
                inputs.pop_front();
            }
            waiting = false;
        }
    }
    void clear() {
        // free possible blocked
        if(waiting) {
            emit inputReady();
        }
        const std::lock_guard<std::mutex> lock(mtx);
        inputs.clear();
    }
};

class MockOutputStream: public QObject{
    Q_OBJECT
    std::deque<string> outputs;
    std::mutex out_mtx;
signals:
    void sendOutput(std::string output);
public:
    explicit MockOutputStream() = default;
    template<Streamable T>
    void output(const T& output) {
        const std::lock_guard<std::mutex> lock(out_mtx);
        outputs.push_back(output);
        emit sendOutput(output);
    }
    auto lookOutput() {
        const std::lock_guard<std::mutex> lock(out_mtx);
        vector<string> outputs_copy;
        std::ranges::copy(outputs, std::back_inserter(outputs_copy));
        return outputs_copy;
    }
    auto getOutput() {
        const std::lock_guard<std::mutex> lock(out_mtx);
        if(outputs.empty()) {
            return string{};
        }
        std::string out_str;
        for (const auto& output: outputs) {
            out_str += output;
        }
        outputs = {};
        return out_str;
    }
    void clear() {
        const std::lock_guard<std::mutex> lock(out_mtx);
        outputs.clear();
    }
};
// Error 直接用异常传递，调用interpreter的时候捕获runtime_error
class Interpreter: public NodeVisitor {


private:
    std::shared_ptr<Parser> parser{};
    std::shared_ptr<Env> env{};
    ProgramStatus status{};
    MockInputStream inputStream{};
    MockOutputStream outputStream{};
    MockOutputStream astStream{};
public:
    explicit Interpreter(std::shared_ptr<Parser> p, std::shared_ptr<Env> e,
                         const ProgramMode mode = ProgramMode::DEV): parser(p), env(e) {
        reset();
        status.mode = mode;
    };
    [[nodiscard]] const MockInputStream* getInputStream() const {
        return &inputStream;
    }
    [[nodiscard]] const MockOutputStream* getOutputStream() const {
        return &outputStream;
    }
    [[nodiscard]] const MockOutputStream *getASTStream() const {
        return &astStream;
    }
    ~Interpreter() override = default;
    void interpret();
    void interpret_SingleStep();
    // line: total line including line_no
    void addLine(int line_no, string line) {

    }
    void input(std::string var) {
        print("[DEBUG] input {}\n", var);
        if(status.mode == ProgramMode::DEV) {
            static std::istringstream iss;
            iss.str(var);
            std::cin.rdbuf(iss.rdbuf());
        } else {
            inputStream.receiveInput(var);
        }
    }
    template<Streamable T>
    void requireInput(T& var) {
        status.blocking = true;
        print("[DEBUG] waiting for input ...\n");
        if(status.mode == ProgramMode::DEV) {
            std::cin >> var;
        } else {
            inputStream.requireInput(var);
        }
        status.blocking = false;
    }
    template<Streamable T>
    void output(T output) {
        if(status.mode == ProgramMode::DEV) {
            std::cout << output;
        } else {
            outputStream.output(output);
        }
    }
    template<Streamable T>
    void astOutput(T output) {
        if(status.mode == ProgramMode::DEV) {
        std::cout << output;
        } else {
        astStream.output(output);
        }
    }
    void visit(ASTNode *root) override;
    [[nodiscard]] std::shared_ptr<Parser> getParser() const {
        return parser;
    }
    [[nodiscard]] std::shared_ptr<Env> getEnv() const {
        return env;
    }
    [[nodiscard]] ProgramStatus getStatus() const {
        return status;
    }
    void reset(bool status_reload = false) {
        // clear status
        // clear env
        // clear IO
        if (status_reload) {
            status.reload();
        } else {
            status.reset();
        }
        env->symbol_table->clear();
        inputStream.clear();
        outputStream.clear();
        astStream.clear();
    }
    void setMode(ProgramMode mode) {
        status.mode = mode;
    }
    void setFile(std::filesystem::path file) {
        status.current_file = std::move(file);
    }
    void loadFile(const std::filesystem::path &file, ProgramMode m = ProgramMode::DEV) {
        reset(status.current_file == file);
        setFile(file);
        setMode(m);
        parser->reload(file);
    }
    void loadProgram(Token::BasicProgram&& program, ProgramMode m = ProgramMode::DEV) {
        reset();
        setMode(m);
        parser->reload(std::move(program));
    }
    void reload() {
        if (status.current_file.empty()) {
            throw std::runtime_error("No file loaded");
        }
        loadFile(status.current_file, status.mode);
    }
    void switchMode(ProgramMode m) {
        // 如果是自测，那debug模式还是自测
        this->reset(true);
        status.mode = m;
    }
    void addBreakpoint(int line) {
        status.add_breakpoint(line);
    }
    void deleteBreakpoint(int line) {
        status.delete_breakpoint(line);
    }

    /*
     * 从ASTNode中获取值
     * 需要区分VarNode和其他Node
     */
    template<typename T>
    T getNodeVal(ASTNode* node) {
        if(node->type() == ASTNodeType::Var) {
            string var_name = dynamic_cast<VarNode*>(node)->getName();
            auto v = env->symbol_table->get<T>(var_name);
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
        if (Token::isRightAssociative(node->getOp())) {
            visit(right_node);
            visit(left_node);
        } else {
            visit(left_node);
            visit(right_node);
        }

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
                    // HINT: basic mod is different from cpp
                    // 5 % -3 = -1(basic, sign decided by b in a % b)
                    // 5 % -3 = 2(cpp)
                    result = right_i > 0 ? left_i % right_i : ((left_i % right_i) + right_i);
                    break;
                case Token::TokenType::OP_POW:
                    //
                    result = static_cast<int>(std::pow(left_i, right_i));
                    break;
                case Token::TokenType::OP_GT:
                    result = left_i > right_i ? 1 : 0;
                    break;
                case Token::TokenType::OP_LT:
                    result = left_i < right_i ? 1 : 0;
                    break;
                case Token::TokenType::OP_GE:
                    result = left_i >= right_i ? 1 : 0;
                    break;
                case Token::TokenType::OP_LE:
                    result = left_i <= right_i ? 1 : 0;
                    break;
                case Token::TokenType::OP_EQ:
                    result = left_i == right_i ? 1 : 0;
                    break;
                case Token::TokenType::OP_NE:
                    result = left_i != right_i ? 1 : 0;
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
                    result = left_d > right_d ? 1 : 0;
                    break;
                case Token::TokenType::OP_LT:
                    result = left_d < right_d ? 1 : 0;
                    break;
                case Token::TokenType::OP_GE:
                    result = left_d >= right_d ? 1 : 0;
                    break;
                case Token::TokenType::OP_LE:
                    result = left_d <= right_d ? 1 : 0;
                    break;
                case Token::TokenType::OP_EQ:
                    result = left_d == right_d ? 1 : 0;
                    break;
                case Token::TokenType::OP_NE:
                    result = left_d != right_d ? 1 : 0;
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
                result = std::any_cast<string>(left_v) > std::any_cast<string>(right_v) ? 1 : 0;
                break;
            case Token::TokenType::OP_LT:
                result = std::any_cast<string>(left_v) < std::any_cast<string>(right_v) ? 1 : 0;
                break;
            case Token::TokenType::OP_GE:
                result = std::any_cast<string>(left_v) >= std::any_cast<string>(right_v) ? 1 : 0;
                break;
            case Token::TokenType::OP_LE:
                result = std::any_cast<string>(left_v) <= std::any_cast<string>(right_v) ? 1 : 0;
                break;
            case Token::TokenType::OP_EQ:
                result = std::any_cast<string>(left_v) == std::any_cast<string>(right_v) ? 1 : 0;
                break;
            case Token::TokenType::OP_NE:
                result = std::any_cast<string>(left_v) != std::any_cast<string>(right_v) ? 1 : 0;
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
            auto v = env->symbol_table->get<std::any>(var_name);
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
        env->symbol_table->set(var_name, right_v);
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
        string input = "undefined";
        requireInput(input);
        // HINT: INPUT n 定义n
        try {
            auto num = str2Number(input);
            if(std::holds_alternative<int>(num)) {
                int num_i = std::get<int>(num);
                env->symbol_table->set<int>(node->getVar()->getName(), num_i);
                node->setValue(num_i);
            } else {
                double num_d = std::get<double>(num);
                env->symbol_table->set<double>(node->getVar()->getName(), num_d);
                node->setValue(num_d);
            }
        } catch (std::exception& e) {
            env->symbol_table->set<string>(node->getVar()->getName(), input);
            node->setValue(input);
        }
    }
    void visit_PrintStmtNode(PrintStmtNode* node) {
        auto expr = node->getExpr();
        visit_Expr(expr);
        auto expr_v = getNodeVal<std::any>(expr);
        if(expr_v.type() == typeid(int)) {
            output(format("{}", std::any_cast<int>(expr_v)));
        }
        if(expr_v.type() == typeid(double)) {
            output(format("{}", std::any_cast<double>(expr_v)));
        }
        if(expr_v.type() == typeid(string)) {
            output(fmt::format("{}", std::any_cast<string>(expr_v)));
        }
    }
};




#endif //INTREPRETER_H
