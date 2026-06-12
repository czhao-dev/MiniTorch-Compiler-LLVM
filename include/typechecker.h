#pragma once

#include "ast.h"

#include <stdexcept>

namespace minitorch {

class TypeError final : public std::runtime_error {
public:
    explicit TypeError(const std::string &message) : std::runtime_error(message) {}
};

class TypeChecker {
public:
    void check(const ModuleNode &module) const;
};

} // namespace minitorch

