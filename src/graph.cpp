#include "graph.h"

#include <ostream>
#include <unordered_map>

namespace minitorch {

const char *opTypeName(OpType op) {
    switch (op) {
    case OpType::Input:
        return "INPUT";
    case OpType::Output:
        return "OUTPUT";
    case OpType::Matmul:
        return "MATMUL";
    case OpType::Add:
        return "ADD";
    case OpType::Relu:
        return "RELU";
    case OpType::Sigmoid:
        return "SIGMOID";
    case OpType::Softmax:
        return "SOFTMAX";
    case OpType::Transpose:
        return "TRANSPOSE";
    case OpType::Reshape:
        return "RESHAPE";
    case OpType::Const:
        return "CONST";
    case OpType::MatmulRelu:
        return "MATMUL_RELU";
    case OpType::MatmulSigmoid:
        return "MATMUL_SIGMOID";
    }
    return "UNKNOWN";
}

int Graph::addNode(OpType op, std::string name, std::vector<int> inputs, TensorType type) {
    const int id = static_cast<int>(nodes_.size());
    nodes_.push_back(GraphNode{id, op, std::move(name), std::move(inputs), std::move(type)});
    return id;
}

void Graph::print(std::ostream &out) const {
    for (const auto &node : nodes_) {
        out << "#" << node.id << " " << opTypeName(node.op);
        if (!node.name.empty()) {
            out << " " << node.name;
        }
        out << " inputs=[";
        for (std::size_t i = 0; i < node.inputs.size(); ++i) {
            if (i != 0) {
                out << ", ";
            }
            out << node.inputs[i];
        }
        out << "] shape=[";
        for (std::size_t i = 0; i < node.type.dims.size(); ++i) {
            if (i != 0) {
                out << ", ";
            }
            out << node.type.dims[i];
        }
        out << "]\n";
    }
}

Graph GraphBuilder::build(const ModuleNode &module) const {
    Graph graph;
    if (module.functions.empty()) {
        return graph;
    }

    const auto &function = *module.functions.front();
    std::unordered_map<std::string, int> symbols;

    for (const auto &param : function.params) {
        TensorType type{param.type.dims};
        symbols[param.name] = graph.addNode(OpType::Input, param.name, {}, std::move(type));
    }

    for (const auto &stmt : function.body) {
        if (stmt->kind == StmtKind::Assign) {
            const auto &assign = static_cast<const AssignStmtNode &>(*stmt);
            graph.addNode(OpType::Const, assign.target, {}, TensorType{});
            symbols[assign.target] = static_cast<int>(graph.nodes().size()) - 1;
        } else if (stmt->kind == StmtKind::Return) {
            graph.addNode(OpType::Output, "return", {}, TensorType{function.returnType.dims});
        }
    }

    return graph;
}

} // namespace minitorch

