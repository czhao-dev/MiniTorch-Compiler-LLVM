#include "typechecker.h"

#include <unordered_set>

namespace minitorch {

void TypeChecker::check(const ModuleNode &module) const {
    if (module.functions.empty()) {
        throw TypeError("module must contain at least one function");
    }

    for (const auto &function : module.functions) {
        if (function->body.empty()) {
            throw TypeError("function '" + function->name + "' must contain at least one statement");
        }

        std::unordered_set<std::string> names;
        for (const auto &param : function->params) {
            if (param.type.dims.empty()) {
                throw TypeError("parameter '" + param.name + "' must have at least one tensor dimension");
            }
            if (!names.insert(param.name).second) {
                throw TypeError("duplicate parameter '" + param.name + "'");
            }
        }
    }
}

} // namespace minitorch

