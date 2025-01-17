# Copyright 2020-2021 Huawei Technologies Co., Ltd
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ============================================================================
"""Sparse related tools."""
from mindspore.ops import operations as P
from ..cell import Cell


class SparseToDense(Cell):
    """
    Converts a sparse tensor into dense.

    Not yet supported by any backend at the moment.

    Inputs:
        - **sparse_tensor** (SparseTensor): the sparse tensor to convert.

    Outputs:
        Tensor, converted from sparse tensor.

    Raises:
        TypeError: If the`sparse_tensor.indices` data type is neither int32 nor int64.
        TypeError: If the 'sparse_tensor.values' data type is not a Number or bool.
        TypeError: If 'sparse_tensor.dense_shape' is not a tuple.

    Supported Platforms:
        ``CPU``

    Examples:
        >>> import mindspore as ms
        >>> from mindspore import Tensor, SparseTensor
        >>> import mindspore.nn as nn
        >>> indices = Tensor([[0, 1], [1, 2]])
        >>> values = Tensor([1, 2], dtype=ms.int32)
        >>> dense_shape = (3, 4)
        >>> sparse_tensor = SparseTensor(indices, values, dense_shape)
        >>> sparse_to_dense = nn.SparseToDense()
        >>> result = sparse_to_dense(sparse_tensor)
        >>> print(result)
        [[0 1 0 0]
         [0 0 2 0]
         [0 0 0 0]]
    """

    def __init__(self):
        super(SparseToDense, self).__init__()
        self.sparse_to_dense = P.SparseToDense()

    def construct(self, sparse_tensor):
        return self.sparse_to_dense(sparse_tensor.indices,
                                    sparse_tensor.values,
                                    sparse_tensor.dense_shape)


class SparseTensorDenseMatmul(Cell):
    """
    Multiplies sparse matrix `a` and dense matrix `b`.
    The rank of sparse matrix and dense matrix must equal to `2`.

    Args:
        - *adjoint_st** (bool) - If true, sparse tensor is transposed before multiplication. Default: False.
        - *adjoint_dt** (bool) - If true, dense tensor is transposed before multiplication. Default: False.

    Inputs:
        - **indices** (Tensor) - A 2-D Tensor, represents the position of the element in the sparse tensor.
            Support int32, int64, each element value should be non-negative. The shape is :math:`(n, 2)`.
        - **values** (Tensor) - A 1-D Tensor, represents the value corresponding to the position in the `indices`.
            Support float16, float32, float64, int32, int64. The shape should be :math:`(n,).
        - **sparse_shape** (tuple) - A positive int tuple which specifies the shape of sparse tensor,
            should have 2 elements, represent sparse tensor shape is :math:`(N, C)`.
        - **dense** (Tensor) - A 2-D Tensor, the dtype is same as `values`.
            If `adjoint_st` is False and `adjoint_dt` is False, the shape must be :math:`(C, M)`.
            If `adjoint_st` is False and `adjoint_dt` is True, the shape must be :math:`(M, C)`.
            If `adjoint_st` is True and `adjoint_dt` is False, the shape must be :math:`(N, M)`.
            If `adjoint_st` is True and `adjoint_dt` is True, the shape must be :math:`(M, N)`.

    Outputs:
        Tensor, the dtype is the same as `values`.
        If `adjoint_st` is False, the shape is :math:`(N, M)`.
        If `adjoint_st` is True, the shape is :math:`(C, M)`.

    Raises:
        TypeError: If the type of `adjoint_st` or `adjoint_dt` is not bool, or the dtype of `indices`,
            dtype of `values` and dtype of `dense` don't meet the parameter description.
        ValueError: If `sparse_shape`, shape of `indices, shape of `values`,
            and shape of `dense` don't meet the parameter description.

    Supported Platforms:
        ``CPU``

    Examples:
        >>> import mindspore as ms
        >>> from mindspore import Tensor
        >>> from mindspore import nn
        >>> indices = Tensor([[0, 1], [1, 2]], dtype=ms.int32)
        >>> values = Tensor([1, 2], dtype=ms.float32)
        >>> sparse_shape = (3, 4)
        >>> dense = Tensor([[1, 1], [2, 2], [3, 3], [4, 4]], dtype=ms.float32)
        >>> sparse_dense_matmul = nn.SparseTensorDenseMatmul()
        >>> out = sparse_dense_matmul(indices, values, sparse_shape, dense)
        >>> print(out)
        [[2 2]
         [0 6]
         [6 0]]
    """

    def __init__(self, adjoint_st=False, adjoint_dt=False):
        """Initialize SparseTensorDenseMatmul"""
        super(SparseTensorDenseMatmul, self).__init__()
        self.adj_st = adjoint_st
        self.adj_dt = adjoint_dt
        self.sparse_dense_matmul = P.SparseTensorDenseMatmul(adjoint_st=self.adj_st, adjoint_dt=self.adj_dt)

    def construct(self, indices, values, sparse_shape, dense):
        return self.sparse_dense_matmul(indices, values, sparse_shape, dense)
