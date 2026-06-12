#pragma once

#include "ast.h"

#include <iosfwd>
#include <string>
#include <vector>

namespace minitorch {

enum class OpType {
    Input,
    Output,
    Matmul,
    Add,
    Relu,
    Sigmoid,
    Softmax,
    Transpose,
    Reshape,
    Const,
    MatmulRelu,
    MatmulSigmoid,
};

struct TensorType {
    std::vector<int> dims;
};

struct GraphNode {
    int id = -1;
    OpType op = OpType::Const;
    std::string name;
    std::vector<int> inputs;
    TensorType type;
};

class Graph {
public:
    int addNode(OpType op, std::string name, std::vector<int> inputs, TensorType type);
    const std::vector<GraphNode> &nodes() const { return nodes_; }
    void print(std::ostream &out) const;

private:
    std::vector<GraphNode> nodes_;
};

class GraphBuilder {
public:
    Graph build(const ModuleNode &module) const;
};

const char *opTypeName(OpType op);

} // namespace minitorch

