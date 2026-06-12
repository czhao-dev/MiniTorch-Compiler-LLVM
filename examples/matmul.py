@minitorch.compile
def matmul_example(x: Tensor[4, 4], w: Tensor[4, 4]) -> Tensor[4, 4]:
    return matmul(x, w)

