@minitorch.compile
def linear(x: Tensor[128, 256], w: Tensor[256, 512], b: Tensor[128, 512]) -> Tensor[128, 512]:
    y = matmul(x, w)
    return y + b

@minitorch.compile
def mlp_forward(x: Tensor[128, 256], w1: Tensor[256, 512], b1: Tensor[128, 512]) -> Tensor[128, 512]:
    h1 = linear(x, w1, b1)
    return relu(h1)

