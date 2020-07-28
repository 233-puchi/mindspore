# Copyright 2020 Huawei Technologies Co., Ltd
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
"""
Test nn.Distribution.Exponential.
"""
import pytest

import mindspore.nn as nn
from mindspore import dtype
from mindspore import Tensor


def test_arguments():
    """
    Args passing during initialization.
    """
    e = nn.Exponential()
    assert isinstance(e, nn.Distribution)
    e = nn.Exponential([0.1, 0.3, 0.5, 1.0], dtype=dtype.float32)
    assert isinstance(e, nn.Distribution)

def test_rate():
    """
    Invalid rate.
    """
    with pytest.raises(ValueError):
        nn.Exponential([-0.1], dtype=dtype.float32)
    with pytest.raises(ValueError):
        nn.Exponential([0.0], dtype=dtype.float32)

class ExponentialProb(nn.Cell):
    """
    Exponential distribution: initialize with rate.
    """
    def __init__(self):
        super(ExponentialProb, self).__init__()
        self.e = nn.Exponential(0.5, dtype=dtype.float32)

    def construct(self, value):
        prob = self.e('prob', value)
        log_prob = self.e('log_prob', value)
        cdf = self.e('cdf', value)
        log_cdf = self.e('log_cdf', value)
        sf = self.e('survival_function', value)
        log_sf = self.e('log_survival', value)
        return prob + log_prob + cdf + log_cdf + sf + log_sf

def test_exponential_prob():
    """
    Test probability functions: passing value through construct.
    """
    net = ExponentialProb()
    value = Tensor([0.2, 0.3, 5.0, 2, 3.9], dtype=dtype.float32)
    ans = net(value)
    assert isinstance(ans, Tensor)

class ExponentialProb1(nn.Cell):
    """
    Exponential distribution: initialize without rate.
    """
    def __init__(self):
        super(ExponentialProb1, self).__init__()
        self.e = nn.Exponential(dtype=dtype.float32)

    def construct(self, value, rate):
        prob = self.e('prob', value, rate)
        log_prob = self.e('log_prob', value, rate)
        cdf = self.e('cdf', value, rate)
        log_cdf = self.e('log_cdf', value, rate)
        sf = self.e('survival_function', value, rate)
        log_sf = self.e('log_survival', value, rate)
        return prob + log_prob + cdf + log_cdf + sf + log_sf

def test_exponential_prob1():
    """
    Test probability functions: passing value/rate through construct.
    """
    net = ExponentialProb1()
    value = Tensor([0.2, 0.9, 1, 2, 3], dtype=dtype.float32)
    rate = Tensor([0.5], dtype=dtype.float32)
    ans = net(value, rate)
    assert isinstance(ans, Tensor)

class ExponentialKl(nn.Cell):
    """
    Test class: kl_loss between Exponential distributions.
    """
    def __init__(self):
        super(ExponentialKl, self).__init__()
        self.e1 = nn.Exponential(0.7, dtype=dtype.float32)
        self.e2 = nn.Exponential(dtype=dtype.float32)

    def construct(self, rate_b, rate_a):
        kl1 = self.e1('kl_loss', 'Exponential', rate_b)
        kl2 = self.e2('kl_loss', 'Exponential', rate_b, rate_a)
        return kl1 + kl2

def test_kl():
    """
    Test kl_loss function.
    """
    net = ExponentialKl()
    rate_b = Tensor([0.3], dtype=dtype.float32)
    rate_a = Tensor([0.7], dtype=dtype.float32)
    ans = net(rate_b, rate_a)
    assert isinstance(ans, Tensor)

class ExponentialCrossEntropy(nn.Cell):
    """
    Test class: cross_entropy of Exponential distribution.
    """
    def __init__(self):
        super(ExponentialCrossEntropy, self).__init__()
        self.e1 = nn.Exponential(0.3, dtype=dtype.float32)
        self.e2 = nn.Exponential(dtype=dtype.float32)

    def construct(self, rate_b, rate_a):
        h1 = self.e1('cross_entropy', 'Exponential', rate_b)
        h2 = self.e2('cross_entropy', 'Exponential', rate_b, rate_a)
        return h1 + h2

def test_cross_entropy():
    """
    Test cross_entropy between Exponential distributions.
    """
    net = ExponentialCrossEntropy()
    rate_b = Tensor([0.3], dtype=dtype.float32)
    rate_a = Tensor([0.7], dtype=dtype.float32)
    ans = net(rate_b, rate_a)
    assert isinstance(ans, Tensor)

class ExponentialBasics(nn.Cell):
    """
    Test class: basic mean/sd/mode/entropy function.
    """
    def __init__(self):
        super(ExponentialBasics, self).__init__()
        self.e = nn.Exponential([0.3, 0.5], dtype=dtype.float32)

    def construct(self):
        mean = self.e('mean')
        sd = self.e('sd')
        var = self.e('var')
        mode = self.e('mode')
        entropy = self.e('entropy')
        return mean + sd + var + mode + entropy

def test_bascis():
    """
    Test mean/sd/var/mode/entropy functionality of Exponential distribution.
    """
    net = ExponentialBasics()
    ans = net()
    assert isinstance(ans, Tensor)
