#pragma once
#ifndef TOKENIZER_H
#define TOKENIZER_H
#include <string>
#include <fmt/core.h>
namespace Msg {
using std::string;
using MsgTemplate = const string;
static inline MsgTemplate EmptyMsg = "意外为空: {}";
static inline MsgTemplate UnmatchType = "类型不匹配: {}, {}";

} // namespace Msg

#endif