@minitorch.compile
def attention_head(q: Tensor[64, 128], k: Tensor[128, 64], v: Tensor[64, 128]) -> Tensor[64, 128]:
    scores = matmul(q, k)
    probs = softmax(scores)
    return matmul(probs, v)

