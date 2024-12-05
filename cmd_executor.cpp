//
// Created by ayanami on 12/5/24.
//

#include "cmd_executor.h"

void CmdExecutor::receiveCmd(Command cmd) {
    switch (cmd) {
        case Command::RUN:
            handleCmdRun();
            break;
        case Command::STOP:
            handleCmdStop();
            break;
        case Command::CLEAR:
            handleCmdClear();
            break;
        case Command::RESUME:
            handleCmdResume();
            break;
        case Command::DEBUG:
            handleCmdDebug();
            break;
        default:
            throw std::runtime_error("Invalid command");
    }
}

void CmdExecutor::handleCmdDebug() {

}
void CmdExecutor::handleCmdRun() {

}
void CmdExecutor::handleCmdClear() {

}
void CmdExecutor::handleCmdResume() {

}
void CmdExecutor::handleCmdStop() {
   // TODO: stop如何打断input之类状态？
}
void receiveInputs(QString inputs) {

}

void sendOutputs(QString outputs);
void sendError(QString error);