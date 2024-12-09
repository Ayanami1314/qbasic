//
// Created by ayanami on 12/5/24.
//

#include "cmd_executor.h"

void CmdExecutor::runCmd(Command cmd, const std::vector<std::string>& argv) {
    switch (cmd) {
        case Command::CHANGE_MODE:
            handleCmdChangeMode(argv);
            break;
        case Command::LOAD:
            handleCmdLoad(argv);
            break;
        case Command::RUN:
            handleCmdRun(argv);
            break;
        case Command::STOP:
            handleCmdStop(argv);
            break;
        case Command::CLEAR:
            handleCmdClear(argv);
            break;
        case Command::RESUME:
            handleCmdResume(argv);
            break;
        case Command::DEBUG:
            handleCmdDebug(argv);
            break;
        case Command::ADD_BREAKPOINT:
            handleCmdAddBreakpoint(argv);
            break;
        case Command::REMOVE_BREAKPOINT:
            handleCmdRemoveBreakpoint(argv);
            break;
        default:
            throw std::runtime_error("Invalid command");
    }
}
void CmdExecutor::handleCmdChangeMode(const vector<std::string>& argv) {
    if (argv.size() != 1) {
        throw std::runtime_error("Invalid arguments");
    }
    static std::set<std::string> modes = {"DEBUG", "NORMAL", "DEV"};
    if (modes.find(argv[0]) == modes.end()) {
            throw std::runtime_error("Invalid mode");
    }
    if (argv[0] == "DEBUG") {
        mode = ProgramMode::DEBUG;
    } else if (argv[0] == "NORMAL") {
        mode = ProgramMode::NORMAL;
    } else if (argv[0] == "DEV") {
        mode = ProgramMode::DEV;
    }
}
void CmdExecutor::handleCmdLoad(const vector<std::string>& argv) {
    if (argv.size() != 1) {
        throw std::runtime_error("Invalid arguments");
    }
    if(!std::filesystem::exists(argv[0])) {
        throw std::runtime_error("File not found");
    }
    choosed_file = argv[0];
    interpreter->loadFile(choosed_file, mode);
}
void CmdExecutor::handleCmdDebug(const vector<std::string>& argv) {
    try {
        mode = ProgramMode::DEBUG;
        interpreter->setMode(ProgramMode::DEBUG);
        interpreter->reload();
    } catch (std::exception& e) {
        print("Failed to debug program: {}\n", e.what());
        emit sendError(QString::fromStdString(e.what()));
    }

}
void CmdExecutor::handleCmdRun(const vector<std::string>& argv) {
    try {
        interpreter->setMode(mode);
        interpreter->reload();
        interpreter->interpret();
    } catch (std::exception& e) {
        print("Failed to run program: {}\n", e.what());
        emit sendError(QString::fromStdString(e.what()));
    }

}
void CmdExecutor::handleCmdClear(const vector<std::string>& argv) {
    interpreter->reset();
    // clear ui
}
void CmdExecutor::handleCmdResume(const vector<std::string>& argv) {
    try {
        interpreter->interpret();
    } catch (std::exception& e) {
        print("Failed to debug program: {}\n", e.what());
        emit sendError(QString::fromStdString(e.what()));
    }
}
void CmdExecutor::handleCmdStop(const vector<std::string>& argv) {
   // TODO: stop如何打断input之类状态？
    interpreter->reset();
}
// addbreakpoint [line_no]
void CmdExecutor::handleCmdAddBreakpoint(const vector<std::string>& argv) {
    if (argv.size() != 1) {
        throw std::runtime_error("Invalid arguments");
    }
    int line_no = std::stoi(argv[0]);
    print("add breakpoint: {}", line_no);
    interpreter->addBreakpoint(line_no);
    emit breakpointChanged();
}
void CmdExecutor::handleCmdRemoveBreakpoint(const vector<std::string>& argv) {
    if (argv.size() != 1) {
        throw std::runtime_error("Invalid arguments");
    }
    int line_no = std::stoi(argv[0]);
    interpreter->deleteBreakpoint(line_no);
    emit breakpointChanged();
}