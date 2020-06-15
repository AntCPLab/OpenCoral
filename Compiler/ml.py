"""
This module contains machine learning functionality. It is work in
progress, so you must expect things to change. The only tested
functionality for training is logistic regression. It can be run as
follows::

    sgd = ml.SGD([ml.Dense(n_examples, n_features, 1),
                  ml.Output(n_examples, approx=True)], n_epochs,
                 report_loss=True)
    sgd.layers[0].X.input_from(0)
    sgd.layers[1].Y.input_from(1)
    sgd.reset()
    sgd.run()

This loads measurements from party 0 and labels (0/1) from party
1. After running, the model is stored in :py:obj:`sgd.layers[0].W` and
:py:obj:`sgd.layers[1].b`. The :py:obj:`approx` parameter determines
whether to use an approximate sigmoid function. Setting it to 5 uses
a five-piece approximation instead of a three-piece one.
Inference can be run as follows::

    data = sfix.Matrix(n_test, n_features)
    data.input_from(0)
    res = sgd.eval(data)
    print_ln('Results: %s', [x.reveal() for x in res])

For inference/classification, this module offers the layers necessary
for neural networks such as DenseNet, ResNet, and SqueezeNet. A
minimal example using input from player 0 and model from player 1
looks as follows::

    graph = Optimizer()
    graph.layers = layers
    layers[0].X.input_from(0)
    for layer in layers:
        layer.input_from(1)
    graph.forward(1)
    res = layers[-1].Y

See the `readme <https://github.com/data61/MP-SPDZ/#tensorflow-inference>`_ for
an example of how to run MP-SPDZ on TensorFlow graphs.
"""

import math

from Compiler import mpc_math, util
from Compiler.types import *
from Compiler.types import _unreduced_squant
from Compiler.library import *
from Compiler.util import is_zero, tree_reduce
from Compiler.comparison import CarryOutRawLE
from Compiler.GC.types import sbitint
from functools import reduce

def log_e(x):
    return mpc_math.log_fx(x, math.e)

def exp(x):
    return mpc_math.pow_fx(math.e, x)

def sanitize(x, raw, lower, upper):
    exp_limit = 2 ** (x.k - x.f - 1)
    limit = math.log(exp_limit)
    if get_program().options.ring:
        res = raw
    else:
        res = (x > limit).if_else(upper, raw)
    return (x < -limit).if_else(lower, res)

def sigmoid(x):
    """ Sigmoid function.

    :param x: sfix """
    return sigmoid_from_e_x(x, exp(-x))

def sigmoid_from_e_x(x, e_x):
    return sanitize(x, 1 / (1 + e_x), 0, 1)

def sigmoid_prime(x):
    """ Sigmoid derivative.

    :param x: sfix """
    sx = sigmoid(x)
    return sx * (1 - sx)

@vectorize
def approx_sigmoid(x, n=3):
    """ Piece-wise approximate sigmoid as in
    `Dahl et al. <https://arxiv.org/abs/1810.08130>`_

    :param x: input
    :param n: number of pieces, 3 (default) or 5
    """
    if n == 5:
        cuts = [-5, -2.5, 2.5, 5]
        le = [0] + [x <= cut for cut in cuts] + [1]
        select = [le[i + 1] - le[i] for i in range(5)]
        outputs = [cfix(10 ** -4),
                   0.02776 * x + 0.145,
                   0.17 * x + 0.5,
                   0.02776 * x + 0.85498,
                   cfix(1 - 10 ** -4)]
        return sum(a * b for a, b in zip(select, outputs))
    else:
        a = x < -0.5
        b = x > 0.5
        return a.if_else(0, b.if_else(1, 0.5 + x))

def lse_0_from_e_x(x, e_x):
    return sanitize(-x, log_e(1 + e_x), x + 2 ** -x.f, 0)

def lse_0(x):
    return lse_0_from_e_x(x, exp(x))

def relu_prime(x):
    """ ReLU derivative. """
    return (0 <= x)

def relu(x):
    """ ReLU function (maximum of input and zero). """
    return (0 < x).if_else(x, 0)

def argmax(x):
    """ Compute index of maximum element.

    :param x: iterable
    :returns: sint
    """
    def op(a, b):
        comp = (a[1] > b[1])
        return comp.if_else(a[0], b[0]), comp.if_else(a[1], b[1])
    return tree_reduce(op, enumerate(x))[0]

def progress(x):
    return
    print_ln(x)
    time()

def set_n_threads(n_threads):
    Layer.n_threads = n_threads
    Optimizer.n_threads = n_threads

class Tensor(MultiArray):
    def __init__(self, *args, **kwargs):
        kwargs['alloc'] = False
        super(Tensor, self).__init__(*args, **kwargs)

class Layer:
    n_threads = 1
    inputs = []
    input_bias = True

    @property
    def shape(self):
        return list(self._Y.sizes)

    @property
    def X(self):
        self._X.alloc()
        return self._X

    @X.setter
    def X(self, value):
        self._X = value

    @property
    def Y(self):
        self._Y.alloc()
        return self._Y

    @Y.setter
    def Y(self, value):
        self._Y = value

class NoVariableLayer(Layer):
    input_from = lambda *args, **kwargs: None

class Output(Layer):
    """ Fixed-point logistic regression output layer.

    :param N: number of examples
    :param approx: :py:obj:`False` (default) or parameter for :py:obj:`approx_sigmoid`
    """
    def __init__(self, N, debug=False, approx=False):
        self.N = N
        self.X = sfix.Array(N)
        self.Y = sfix.Array(N)
        self.nabla_X = sfix.Array(N)
        self.l = MemValue(sfix(-1))
        self.e_x = sfix.Array(N)
        self.debug = debug
        self.weights = None
        self.approx = approx

    nablas = lambda self: ()
    thetas = lambda self: ()
    reset = lambda self: None

    def divisor(self, divisor, size):
        return cfix(1.0 / divisor, size=size)

    def forward(self, batch):
        if self.approx:
            self.l.write(999)
            return
        N = len(batch)
        lse = sfix.Array(N)
        @multithread(self.n_threads, N)
        def _(base, size):
            x = self.X.get_vector(base, size)
            y = self.Y.get(batch.get_vector(base, size))
            e_x = exp(-x)
            self.e_x.assign(e_x, base)
            lse.assign(lse_0_from_e_x(-x, e_x) + x * (1 - y), base)
        e_x = self.e_x.get_vector(0, N)
        self.l.write(sum(lse) * \
                     self.divisor(N, 1))

    def eval(self, size, base=0):
        if self.approx:
            return approx_sigmoid(self.X.get_vector(base, size), self.approx)
        else:
            return sigmoid_from_e_x(self.X.get_vector(base, size),
                                    self.e_x.get_vector(base, size))

    def backward(self, batch):
        N = len(batch)
        @multithread(self.n_threads, N)
        def _(base, size):
            diff = self.eval(size, base) - \
                   self.Y.get(batch.get_vector(base, size))
            assert sfix.f == cfix.f
            if self.weights is None:
                diff *= self.divisor(N, size)
            else:
                assert N == len(self.weights)
                diff *= self.weights.get_vector(base, size)
                if self.weight_total != 1:
                    diff *= self.divisor(self.weight_total, size)
            self.nabla_X.assign(diff, base)
        # @for_range_opt(len(diff))
        # def _(i):
        #     self.nabla_X[i] = self.nabla_X[i] * self.weights[i]
        if self.debug:
            a = cfix.Array(len(diff))
            a.assign(diff.reveal())
            @for_range(len(diff))
            def _(i):
                x = a[i]
                print_ln_if((x < -1.001) + (x > 1.001), 'sigmoid')
                #print_ln('%s', x)

    def set_weights(self, weights):
        self.weights = cfix.Array(len(weights))
        self.weights.assign(weights)
        self.weight_total = sum(weights)

class DenseBase(Layer):
    thetas = lambda self: (self.W, self.b)
    nablas = lambda self: (self.nabla_W, self.nabla_b)

    def backward_params(self, f_schur_Y, batch):
        N = len(batch)
        tmp = Matrix(self.d_in, self.d_out, unreduced_sfix)

        assert self.d == 1
        if self.d_out == 1:
            @multithread(self.n_threads, self.d_in)
            def _(base, size):
                A = sfix.Matrix(1, self.N, address=f_schur_Y.address)
                B = sfix.Matrix(self.N, self.d_in, address=self.X.address)
                mp = A.direct_mul(B, reduce=False,
                                  indices=(regint(0, size=1),
                                           regint.inc(N),
                                           batch.get_vector(),
                                           regint.inc(size, base)))
                tmp.assign_vector(mp, base)
        else:
            @for_range_opt_multithread(self.n_threads, [self.d_in, self.d_out])
            def _(j, k):
                a = [f_schur_Y[i][0][k] for i in range(N)]
                b = [self.X[i][0][j] for i in batch]
                tmp[j][k] = sfix.unreduced_dot_product(a, b)

        if self.d_in * self.d_out < 100000:
            print('reduce at once')
            @multithread(self.n_threads, self.d_in * self.d_out)
            def _(base, size):
                self.nabla_W.assign_vector(
                    tmp.get_vector(base, size).reduce_after_mul(), base=base)
        else:
            @for_range_opt(self.d_in)
            def _(i):
                self.nabla_W[i] = tmp[i].get_vector().reduce_after_mul()

        self.nabla_b.assign(sum(sum(f_schur_Y[k][j][i] for k in range(N))
                           for j in range(self.d)) for i in range(self.d_out))

        progress('nabla W/b')

class Dense(DenseBase):
    """ Fixed-point dense (matrix multiplication) layer.

    :param N: number of examples
    :param d_in: input dimension
    :param d_out: output dimension
    """
    def __init__(self, N, d_in, d_out, d=1, activation='id'):
        self.activation = activation
        if activation == 'id':
            self.f = lambda x: x
        elif activation == 'relu':
            self.f = relu
            self.f_prime = relu_prime
        elif activation == 'sigmoid':
            self.f = sigmoid
            self.f_prime = sigmoid_prime

        self.N = N
        self.d_in = d_in
        self.d_out = d_out
        self.d = d

        self.X = MultiArray([N, d, d_in], sfix)
        self.Y = MultiArray([N, d, d_out], sfix)
        self.W = sfix.Matrix(d_in, d_out)
        self.b = sfix.Array(d_out)

        self.nabla_Y = MultiArray([N, d, d_out], sfix)
        self.nabla_X = MultiArray([N, d, d_in], sfix)
        self.nabla_W = sfix.Matrix(d_in, d_out)
        self.nabla_b = sfix.Array(d_out)

        self.f_input = MultiArray([N, d, d_out], sfix)

    def reset(self):
        d_in = self.d_in
        d_out = self.d_out
        r = math.sqrt(6.0 / (d_in + d_out))
        @for_range(d_in)
        def _(i):
            @for_range(d_out)
            def _(j):
                self.W[i][j] = sfix.get_random(-r, r)
        self.b.assign_all(0)

    def input_from(self, player, raw=False):
        self.W.input_from(player, raw=raw)
        if self.input_bias:
            self.b.input_from(player, raw=raw)

    def compute_f_input(self, batch):
        N = len(batch)
        assert self.d == 1
        if self.input_bias:
            prod = MultiArray([N, self.d, self.d_out], sfix)
        else:
            prod = self.f_input
        @multithread(self.n_threads, N)
        def _(base, size):
            X_sub = sfix.Matrix(self.N, self.d_in, address=self.X.address)
            prod.assign_vector(
                X_sub.direct_mul(self.W, indices=(batch.get_vector(base, size),
                                                  regint.inc(self.d_in),
                                                  regint.inc(self.d_in),
                                                  regint.inc(self.d_out))),
                base)

        if self.input_bias:
            if self.d_out == 1:
                @multithread(self.n_threads, N)
                def _(base, size):
                    v = prod.get_vector(base, size) + self.b.expand_to_vector(0, size)
                    self.f_input.assign_vector(v, base)
            else:
                @for_range_opt_multithread(self.n_threads, N)
                def _(i):
                    v = prod[i].get_vector() + self.b.get_vector()
                    self.f_input[i].assign_vector(v)
        progress('f input')

    def forward(self, batch=None):
        self.compute_f_input(batch=batch)
        self.Y.assign_vector(self.f(
            self.f_input.get_part_vector(0, len(batch))))

    def backward(self, compute_nabla_X=True, batch=None):
        N = len(batch)
        d = self.d
        d_out = self.d_out
        X = self.X
        Y = self.Y
        W = self.W
        b = self.b
        nabla_X = self.nabla_X
        nabla_Y = self.nabla_Y
        nabla_W = self.nabla_W
        nabla_b = self.nabla_b

        if self.activation == 'id':
            f_schur_Y = nabla_Y
        else:
            f_prime_bit = MultiArray([N, d, d_out], sint)
            f_schur_Y = MultiArray([N, d, d_out], sfix)

            self.compute_f_input()
            f_prime_bit.assign_vector(self.f_prime(self.f_input.get_vector()))

            progress('f prime')

            @for_range_opt(N)
            def _(i):
                i = batch[i]
                f_schur_Y[i] = nabla_Y[i].schur(f_prime_bit[i])

            progress('f prime schur Y')

        if compute_nabla_X:
            @for_range_opt(N)
            def _(i):
                i = batch[i]
                if self.activation == 'id':
                    nabla_X[i] = nabla_Y[i].mul_trans(W)
                else:
                    nabla_X[i] = nabla_Y[i].schur(f_prime_bit[i]).mul_trans(W)

            progress('nabla X')

        self.backward_params(f_schur_Y, batch=batch)

class QuantizedDense(DenseBase):
    def __init__(self, N, d_in, d_out):
        self.N = N
        self.d_in = d_in
        self.d_out = d_out
        self.d = 1
        self.H = math.sqrt(1.5 / (d_in + d_out))

        self.W = sfix.Matrix(d_in, d_out)
        self.nabla_W = self.W.same_shape()
        self.T = sint.Matrix(d_in, d_out)
        self.b = sfix.Array(d_out)
        self.nabla_b = self.b.same_shape()

        self.X = MultiArray([N, 1, d_in], sfix)
        self.Y = MultiArray([N, 1, d_out], sfix)
        self.nabla_Y = self.Y.same_shape()

    def reset(self):
        @for_range(self.d_in)
        def _(i):
            @for_range(self.d_out)
            def _(j):
                self.W[i][j] = sfix.get_random(-1, 1)
        self.b.assign_all(0)

    def forward(self):
        @for_range_opt(self.d_in)
        def _(i):
            @for_range_opt(self.d_out)
            def _(j):
                over = self.W[i][j] > 0.5
                under = self.W[i][j] < -0.5
                self.T[i][j] = over.if_else(1, under.if_else(-1, 0))
                over = self.W[i][j] > 1
                under = self.W[i][j] < -1
                self.W[i][j] = over.if_else(1, under.if_else(-1, self.W[i][j]))
        @for_range_opt(self.N)
        def _(i):
            assert self.d_out == 1
            self.Y[i][0][0] = self.b[0] + self.H * sfix._new(
                sint.dot_product([self.T[j][0] for j in range(self.d_in)],
                                 [self.X[i][0][j].v for j in range(self.d_in)]))

    def backward(self, compute_nabla_X=False):
        assert not compute_nabla_X
        self.backward_params(self.nabla_Y)

class Dropout:
    def __init__(self, N, d1, d2=1):
        self.N = N
        self.d1 = d1
        self.d2 = d2
        self.X = MultiArray([N, d1, d2], sfix)
        self.Y = MultiArray([N, d1, d2], sfix)
        self.nabla_Y = MultiArray([N, d1, d2], sfix)
        self.nabla_X = MultiArray([N, d1, d2], sfix)
        self.alpha = 0.5
        self.B = MultiArray([N, d1, d2], sint)

    def forward(self):
        assert self.alpha == 0.5
        @for_range(self.N)
        def _(i):
            @for_range(self.d1)
            def _(j):
                @for_range(self.d2)
                def _(k):
                    self.B[i][j][k] = sint.get_random_bit()
        self.Y = self.X.schur(self.B)

    def backward(self):
        self.nabla_X = self.nabla_Y.schur(self.B)

class Relu(NoVariableLayer):
    """ Fixed-point ReLU layer.

    :param shape: input/output shape (tuple/list of int)
    """
    def __init__(self, shape, inputs=None):
        self.X = Tensor(shape, sfix)
        self.Y = Tensor(shape, sfix)
        self.inputs = inputs

    def forward(self, batch=[0]):
        assert len(batch) == 1
        @multithread(self.n_threads, self.X[batch[0]].total_size())
        def _(base, size):
            tmp = relu(self.X[batch[0]].get_vector(base, size))
            self.Y[batch[0]].assign_vector(tmp, base)

class Square(NoVariableLayer):
    """ Fixed-point square layer.

    :param shape: input/output shape (tuple/list of int)
    """
    def __init__(self, shape):
        self.X = MultiArray(shape, sfix)
        self.Y = MultiArray(shape, sfix)

    def forward(self, batch=[0]):
        assert len(batch) == 1
        self.Y.assign_vector(self.X.get_part_vector(batch[0]) ** 2)

class MaxPool(NoVariableLayer):
    """ Fixed-point MaxPool layer.

    :param shape: input shape (tuple/list of four int)
    :param strides: strides (tuple/list of four int, first and last must be 1)
    :param ksize: kernel size (tuple/list of four int, first and last must be 1)
    :param padding: :py:obj:`'VALID'` (default) or :py:obj:`'SAME'`
    """
    def __init__(self, shape, strides=(1, 2, 2, 1), ksize=(1, 2, 2, 1),
                 padding='VALID'):
        assert len(shape) == 4
        for x in strides, ksize:
            for i in 0, 3:
                assert x[i] == 1
        self.X = MultiArray(shape, sfix)
        if padding == 'SAME':
            output_shape = [int(math.ceil(shape[i] / strides[i])) for i in range(4)]
        else:
            output_shape = [(shape[i] - ksize[i]) // strides[i] + 1 for i in range(4)]
        self.Y = MultiArray(output_shape, sfix)
        self.strides = strides
        self.ksize = ksize

    def forward(self, batch=[0]):
        assert len(batch) == 1
        bi = MemValue(batch[0])
        need_padding = [self.strides[i] * (self.Y.sizes[i] - 1) + self.ksize[i] >
                        self.X.sizes[i] for i in range(4)]
        @for_range_opt_multithread(self.n_threads, self.X.sizes[3])
        def _(k):
            @for_range_opt(self.Y.sizes[1])
            def _(i):
                h_base = self.strides[1] * i
                @for_range_opt(self.Y.sizes[2])
                def _(j):
                    w_base = self.strides[2] * j
                    pool = []
                    for ii in range(self.ksize[1]):
                        h = h_base + ii
                        if need_padding[1]:
                            h_in = h < self.X.sizes[1]
                        else:
                            h_in = True
                        for jj in range(self.ksize[2]):
                            w = w_base + jj
                            if need_padding[2]:
                                w_in = w < self.X.sizes[2]
                            else:
                                w_in = True
                            if not is_zero(h_in * w_in):
                                pool.append(h_in * w_in * self.X[bi][h_in * h]
                                            [w_in * w][k])
                    self.Y[bi][i][j][k] = util.tree_reduce(
                        lambda a, b: a.max(b), pool)

class Argmax(NoVariableLayer):
    """ Fixed-point Argmax layer.

    :param shape: input shape (tuple/list of two int)
    """
    def __init__(self, shape):
        assert len(shape) == 2
        self.X = MultiArray(shape, sfix)
        self.Y = Array(shape[0], sint)

    def forward(self, batch=[0]):
        assert len(batch) == 1
        self.Y[batch[0]] = argmax(self.X[batch[0]])

class Concat(NoVariableLayer):
    """ Fixed-point concatentation layer.

    :param inputs: two input layers (tuple/list)
    :param dimension: dimension for concatenation (must be 3)
    """
    def __init__(self, inputs, dimension):
        self.inputs = inputs
        self.dimension = dimension
        shapes = [inp.shape for inp in inputs]
        assert dimension == 3
        assert len(shapes) == 2
        assert len(shapes[0]) == len(shapes[1])
        shape = []
        for i in range(len(shapes[0])):
            if i == dimension:
                shape.append(shapes[0][i] + shapes[1][i])
            else:
                assert shapes[0][i] == shapes[1][i]
                shape.append(shapes[0][i])
        self.Y = Tensor(shape, sfix)

    def forward(self, batch=[0]):
        assert len(batch) == 1
        @for_range_multithread(self.n_threads, 1, self.Y.sizes[1:3])
        def _(i, j):
            X = [x.Y[batch[0]] for x in self.inputs]
            self.Y[batch[0]][i][j].assign_vector(X[0][i][j].get_vector())
            self.Y[batch[0]][i][j].assign_part_vector(
                X[1][i][j].get_vector(),
                len(X[0][i][j]))

class Add(NoVariableLayer):
    """ Fixed-point addition layer.

    :param inputs: two input layers with same shape (tuple/list)
    """
    def __init__(self, inputs):
        assert len(inputs) > 1
        shape = inputs[0].shape
        for inp in inputs:
            assert inp.shape == shape
        self.Y = Tensor(shape, sfix)
        self.inputs = inputs

    def forward(self, batch=[0]):
        assert len(batch) == 1
        @multithread(self.n_threads, self.Y[0].total_size())
        def _(base, size):
            tmp = sum(inp.Y[batch[0]].get_vector(base, size)
                      for inp in self.inputs)
            self.Y[batch[0]].assign_vector(tmp, base)

class FusedBatchNorm(Layer):
    """ Fixed-point fused batch normalization layer.

    :param shape: input/output shape (tuple/list of four int)
    """
    def __init__(self, shape, inputs=None):
        assert len(shape) == 4
        self.X = Tensor(shape, sfix)
        self.Y = Tensor(shape, sfix)
        self.weights = sfix.Array(shape[3])
        self.bias = sfix.Array(shape[3])
        self.inputs = inputs

    def input_from(self, player, raw=False):
        self.weights.input_from(player, raw=raw)
        self.bias.input_from(player, raw=raw)
        tmp = sfix.Array(len(self.bias))
        tmp.input_from(player, raw=raw)
        tmp.input_from(player, raw=raw)

    def forward(self, batch=[0]):
        assert len(batch) == 1
        @for_range_opt_multithread(self.n_threads, self.X.sizes[1:3])
        def _(i, j):
            self.Y[batch[0]][i][j].assign_vector(
                self.X[batch[0]][i][j].get_vector() * self.weights.get_vector()
                + self.bias.get_vector())

class QuantBase(object):
    bias_before_reduction = True

    @staticmethod
    def new_squant():
        class _(squant):
            @classmethod
            def get_params_from(cls, player):
                cls.set_params(sfloat.get_input_from(player),
                               sint.get_input_from(player))
            @classmethod
            def get_input_from(cls, player, size=None):
                return cls._new(sint.get_input_from(player, size=size))
        return _

    def const_div(self, acc, n):
        logn = int(math.log(n, 2))
        acc = (acc + n // 2)
        if 2 ** logn == n:
            acc = acc.round(self.output_squant.params.k + logn, logn, nearest=True)
        else:
            acc = acc.int_div(sint(n), self.output_squant.params.k + logn)
        return acc

class FixBase:
    bias_before_reduction = False

    @staticmethod
    def new_squant():
        class _(sfix):
            params = None
        return _

    def input_params_from(self, player):
        pass

    def const_div(self, acc, n):
        return (sfix._new(acc) * self.output_squant(1 / n)).v

class BaseLayer(Layer):
    def __init__(self, input_shape, output_shape, inputs=None):
        self.input_shape = input_shape
        self.output_shape = output_shape

        self.input_squant = self.new_squant()
        self.output_squant = self.new_squant()

        self.X = Tensor(input_shape, self.input_squant)
        self.Y = Tensor(output_shape, self.output_squant)
        self.inputs = inputs

    def temp_shape(self):
        return [0]

class ConvBase(BaseLayer):
    fewer_rounds = True
    use_conv2ds = False
    temp_weights = None
    temp_inputs = None

    @classmethod
    def init_temp(cls, layers):
        size = 0
        for layer in layers:
            size = max(size, reduce(operator.mul, layer.temp_shape()))
        cls.temp_weights = sfix.Array(size)
        cls.temp_inputs = sfix.Array(size)

    def __init__(self, input_shape, weight_shape, bias_shape, output_shape, stride,
                 padding='SAME', tf_weight_format=False, inputs=None):
        super(ConvBase, self).__init__(input_shape, output_shape, inputs=inputs)

        self.weight_shape = weight_shape
        self.bias_shape = bias_shape
        self.stride = stride
        self.tf_weight_format = tf_weight_format
        if padding == 'SAME':
            # https://web.archive.org/web/20171223022012/https://www.tensorflow.org/api_guides/python/nn
            self.padding = []
            for i in 1, 2:
                s = stride[i - 1]
                if tf_weight_format:
                    w = weight_shape[i - 1]
                else:
                    w = weight_shape[i]
                if (input_shape[i] % stride[1] == 0):
                    pad_total = max(w - s, 0)
                else:
                    pad_total = max(w - (input_shape[i] % s), 0)
                self.padding.append(pad_total // 2)
        elif padding == 'VALID':
            self.padding = [0, 0]
        else:
            self.padding = padding

        self.weight_squant = self.new_squant()
        self.bias_squant = self.new_squant()

        self.weights = MultiArray(weight_shape, self.weight_squant)
        self.bias = Array(output_shape[-1], self.bias_squant)

        self.unreduced = Tensor(self.output_shape, sint)

        if tf_weight_format:
            weight_in = weight_shape[2]
        else:
            weight_in = weight_shape[3]
        assert(weight_in == input_shape[-1])
        assert(bias_shape[0] == output_shape[-1])
        assert(len(bias_shape) == 1)
        assert(len(input_shape) == 4)
        assert(len(output_shape) == 4)
        assert(len(weight_shape) == 4)

    def input_from(self, player, raw=False):
        self.input_params_from(player)
        self.weights.input_from(player, budget=100000, raw=raw)
        if self.input_bias:
            self.bias.input_from(player, raw=raw)

    def dot_product(self, iv, wv, out_y, out_x, out_c):
        bias = self.bias[out_c]
        acc = self.output_squant.unreduced_dot_product(iv, wv)
        acc.v += bias.v
        acc.res_params = self.output_squant.params
        #self.Y[0][out_y][out_x][out_c] = acc.reduce_after_mul()
        self.unreduced[0][out_y][out_x][out_c] = acc.v

    def reduction(self):
        unreduced = self.unreduced
        n_summands = self.n_summands()
        start_timer(2)
        n_outputs = reduce(operator.mul, self.output_shape)
        @multithread(self.n_threads, n_outputs)
        def _(base, n_per_thread):
            res = self.input_squant().unreduced(
                sint.load_mem(unreduced.address + base,
                              size=n_per_thread),
                self.weight_squant(),
                self.output_squant.params,
                n_summands).reduce_after_mul()
            res.store_in_mem(self.Y.address + base)
        stop_timer(2)
        unreduced.delete()

    def temp_shape(self):
        return list(self.output_shape[1:]) + [self.n_summands()]

    def prepare_temp(self):
        shape = self.temp_shape()
        inputs = MultiArray(shape, self.input_squant,
                            address=self.temp_inputs)
        weights = MultiArray(shape, self.weight_squant,
                             address=self.temp_weights)
        return inputs, weights

class Conv2d(ConvBase):
    def n_summands(self):
        _, weights_h, weights_w, _ = self.weight_shape
        _, inputs_h, inputs_w, n_channels_in = self.input_shape
        return weights_h * weights_w * n_channels_in

    def forward(self, batch=[None]):
        assert len(batch) == 1

        if self.tf_weight_format:
            assert(self.weight_shape[3] == self.output_shape[-1])
            weights_h, weights_w, _, _ = self.weight_shape
        else:
            assert(self.weight_shape[0] == self.output_shape[-1])
            _, weights_h, weights_w, _ = self.weight_shape
        _, inputs_h, inputs_w, n_channels_in = self.input_shape
        _, output_h, output_w, n_channels_out = self.output_shape

        stride_h, stride_w = self.stride
        padding_h, padding_w = self.padding

        self.unreduced.alloc()

        if self.use_conv2ds:
            @for_range_opt_multithread(self.n_threads, n_channels_out)
            def _(j):
                inputs = self.X.get_part_vector(0)
                if self.tf_weight_format:
                    weights = self.weights.get_vector_by_indices(None, None, None, j)
                else:
                    weights = self.weights.get_part_vector(j)
                inputs = inputs.pre_mul()
                weights = weights.pre_mul()
                res = sint(size = output_h * output_w)
                conv2ds(res, inputs, weights, output_h, output_w,
                        inputs_h, inputs_w, weights_h, weights_w,
                        stride_h, stride_w, n_channels_in, padding_h, padding_w)
                if self.bias_before_reduction:
                    res += self.bias.expand_to_vector(j, res.size).v
                self.unreduced.assign_vector_by_indices(res, 0, None, None, j)
            self.reduction()
            if not self.bias_before_reduction:
                @for_range_multithread(self.n_threads, 1,
                                       [self.output_shape[1],
                                        self.output_shape[2]])
                def _(i, j):
                    self.Y[0][i][j].assign_vector(self.Y[0][i][j].get_vector() +
                                                  self.bias.get_vector())
            return
        else:
            if self.fewer_rounds:
                inputs, weights = self.prepare_temp()

        @for_range_opt_multithread(self.n_threads,
                                   [output_h, output_w, n_channels_out])
        def _(out_y, out_x, out_c):
                    in_x_origin = (out_x * stride_w) - padding_w
                    in_y_origin = (out_y * stride_h) - padding_h
                    iv = []
                    wv = []
                    for filter_y in range(weights_h):
                        in_y = in_y_origin + filter_y
                        inside_y = (0 <= in_y) * (in_y < inputs_h)
                        for filter_x in range(weights_w):
                            in_x = in_x_origin + filter_x
                            inside_x = (0 <= in_x) * (in_x < inputs_w)
                            inside = inside_y * inside_x
                            if is_zero(inside):
                                continue
                            for in_c in range(n_channels_in):
                                iv += [self.X[0][in_y * inside_y]
                                       [in_x * inside_x][in_c]]
                                wv += [self.weights[out_c][filter_y][filter_x][in_c]]
                                wv[-1] *= inside
                    if self.fewer_rounds:
                        inputs[out_y][out_x][out_c].assign(iv)
                        weights[out_y][out_x][out_c].assign(wv)
                    else:
                        self.dot_product(iv, wv, out_y, out_x, out_c)

        if self.fewer_rounds:
            @for_range_opt_multithread(self.n_threads,
                                       list(self.output_shape[1:]))
            def _(out_y, out_x, out_c):
                self.dot_product(inputs[out_y][out_x][out_c],
                                 weights[out_y][out_x][out_c],
                                 out_y, out_x, out_c)

        self.reduction()

class QuantConvBase(QuantBase):
    def input_params_from(self, player):
        for s in self.input_squant, self.weight_squant, self.bias_squant, self.output_squant:
            s.get_params_from(player)
        print('WARNING: assuming that bias quantization parameters are correct')
        self.output_squant.params.precompute(self.input_squant.params, self.weight_squant.params)

class QuantConv2d(QuantConvBase, Conv2d):
    pass

class FixConv2d(Conv2d, FixBase):
    """ Fixed-point 2D convolution layer.

    :param input_shape: input shape (tuple/list of four int)
    :param weight_shape: weight shape (tuple/list of four int)
    :param bias_shape: bias shape (tuple/list of one int)
    :param output_shape: output shape (tuple/list of four int)
    :param stride: stride (tuple/list of two int)
    :param padding: :py:obj:`'SAME'` (default), :py:obj:`'VALID'`, or tuple/list of two int
    :param tf_weight_format: weight shape format is (height, width, input channels, output channels) instead of the default (output channels, height, widght, input channels)
    """

class QuantDepthwiseConv2d(QuantConvBase, Conv2d):
    def n_summands(self):
        _, weights_h, weights_w, _ = self.weight_shape
        return weights_h * weights_w

    def forward(self, batch):
        assert len(batch) == 1
        assert(self.weight_shape[-1] == self.output_shape[-1])
        assert(self.input_shape[-1] == self.output_shape[-1])

        _, weights_h, weights_w, _ = self.weight_shape
        _, inputs_h, inputs_w, n_channels_in = self.input_shape
        _, output_h, output_w, n_channels_out = self.output_shape

        stride_h, stride_w = self.stride
        padding_h, padding_w = self.padding

        depth_multiplier = 1

        self.unreduced.alloc()

        if self.use_conv2ds:
            assert depth_multiplier == 1
            assert self.weight_shape[0] == 1
            @for_range_opt_multithread(self.n_threads, n_channels_in)
            def _(j):
                inputs = self.X.get_vector_by_indices(0, None, None, j)
                assert not self.tf_weight_format
                weights = self.weights.get_vector_by_indices(0, None, None,
                                                             j)
                inputs = inputs.pre_mul()
                weights = weights.pre_mul()
                res = sint(size = output_h * output_w)
                conv2ds(res, inputs, weights, output_h, output_w,
                        inputs_h, inputs_w, weights_h, weights_w,
                        stride_h, stride_w, 1, padding_h, padding_w)
                res += self.bias.expand_to_vector(j, res.size).v
                self.unreduced.assign_vector_by_indices(res, 0, None, None, j)
            self.reduction()
            return
        else:
            if self.fewer_rounds:
                inputs, weights = self.prepare_temp()

        @for_range_opt_multithread(self.n_threads,
                                   [output_h, output_w, n_channels_in])
        def _(out_y, out_x, in_c):
                    for m in range(depth_multiplier):
                        oc = m + in_c * depth_multiplier
                        in_x_origin = (out_x * stride_w) - padding_w
                        in_y_origin = (out_y * stride_h) - padding_h
                        iv = []
                        wv = []
                        for filter_y in range(weights_h):
                            for filter_x in range(weights_w):
                                in_x = in_x_origin + filter_x
                                in_y = in_y_origin + filter_y
                                inside = (0 <= in_x) * (in_x < inputs_w) * \
                                         (0 <= in_y) * (in_y < inputs_h)
                                if is_zero(inside):
                                    continue
                                iv += [self.X[0][in_y][in_x][in_c]]
                                wv += [self.weights[0][filter_y][filter_x][oc]]
                                wv[-1] *= inside
                        if self.fewer_rounds:
                            inputs[out_y][out_x][oc].assign(iv)
                            weights[out_y][out_x][oc].assign(wv)
                        else:
                            self.dot_product(iv, wv, out_y, out_x, oc)

        if self.fewer_rounds:
            @for_range_opt_multithread(self.n_threads,
                                       list(self.output_shape[1:]))
            def _(out_y, out_x, out_c):
                self.dot_product(inputs[out_y][out_x][out_c],
                                 weights[out_y][out_x][out_c],
                                 out_y, out_x, out_c)

        self.reduction()

class AveragePool2d(BaseLayer):
    def __init__(self, input_shape, output_shape, filter_size, strides=(1, 1)):
        super(AveragePool2d, self).__init__(input_shape, output_shape)
        self.filter_size = filter_size
        self.strides = strides
        for i in (0, 1):
            if strides[i] == 1:
                assert output_shape[1+i] == 1
                assert filter_size[i] == input_shape[1+i]
            else:
                assert strides[i] == filter_size[i]
                assert output_shape[1+i] * strides[i] == input_shape[1+i]

    def input_from(self, player, raw=False):
        self.input_params_from(player)

    def forward(self, batch=[0]):
        assert len(batch) == 1

        _, input_h, input_w, n_channels_in = self.input_shape
        _, output_h, output_w, n_channels_out = self.output_shape

        assert n_channels_in == n_channels_out

        padding_h, padding_w = (0, 0)
        stride_h, stride_w = self.strides
        filter_h, filter_w = self.filter_size
        n = filter_h * filter_w
        print('divisor: ', n)

        @for_range_opt_multithread(self.n_threads,
                                   [output_h, output_w, n_channels_in])
        def _(out_y, out_x, c):
            in_x_origin = (out_x * stride_w) - padding_w
            in_y_origin = (out_y * stride_h) - padding_h
            fxs = util.max(-in_x_origin, 0)
            #fxe = min(filter_w, input_w - in_x_origin)
            fys = util.max(-in_y_origin, 0)
            #fye = min(filter_h, input_h - in_y_origin)
            acc = 0
            #fc = 0
            for i in range(filter_h):
                filter_y = fys + i
                for j in range(filter_w):
                    filter_x = fxs + j
                    in_x = in_x_origin + filter_x
                    in_y = in_y_origin + filter_y
                    acc += self.X[0][in_y][in_x][c].v
                    #fc += 1
            acc = self.const_div(acc, n)
            self.Y[0][out_y][out_x][c] = self.output_squant._new(acc)

class QuantAveragePool2d(QuantBase, AveragePool2d):
    def input_params_from(self, player):
        print('WARNING: assuming that input and output quantization parameters are the same')
        for s in self.input_squant, self.output_squant:
            s.get_params_from(player)

class FixAveragePool2d(FixBase, AveragePool2d):
    """ Fixed-point 2D AvgPool layer.

    :param input_shape: input shape (tuple/list of four int)
    :param output_shape: output shape (tuple/list of four int)
    :param filter_size: filter size (tuple/list of two int)
    :param strides: strides (tuple/list of two int)
    """

class QuantReshape(QuantBase, BaseLayer):
    def __init__(self, input_shape, _, output_shape):
        super(QuantReshape, self).__init__(input_shape, output_shape)

    def input_from(self, player):
        print('WARNING: assuming that input and output quantization parameters are the same')
        _ = self.new_squant()
        for s in self.input_squant, _, self.output_squant:
            s.set_params(sfloat.get_input_from(player), sint.get_input_from(player))
        for i in range(2):
            sint.get_input_from(player)

    def forward(self, batch):
        assert len(batch) == 1
        # reshaping is implicit
        self.Y.assign(self.X)

class QuantSoftmax(QuantBase, BaseLayer):
    def input_from(self, player):
        print('WARNING: assuming that input and output quantization parameters are the same')
        for s in self.input_squant, self.output_squant:
            s.set_params(sfloat.get_input_from(player), sint.get_input_from(player))

    def forward(self, batch):
        assert len(batch) == 1
        assert(len(self.input_shape) == 2)

        # just print the best
        def comp(left, right):
            c = left[1].v.greater_than(right[1].v, self.input_squant.params.k)
            #print_ln('comp %s %s %s', c.reveal(), left[1].v.reveal(), right[1].v.reveal())
            return [c.if_else(x, y) for x, y in zip(left, right)]
        print_ln('guess: %s', util.tree_reduce(comp, list(enumerate(self.X[0])))[0].reveal())

class Optimizer:
    """ Base class for graphs of layers. """
    n_threads = Layer.n_threads

    @property
    def layers(self):
        """ Get all layers. """
        return self._layers

    @layers.setter
    def layers(self, layers):
        """ Construct linear graph from list of layers. """
        self._layers = layers
        prev = None
        for layer in layers:
            if not layer.inputs and prev is not None:
                layer.inputs = [prev]
            prev = layer

    def set_layers_with_inputs(self, layers):
        """ Construct graph from :py:obj:`inputs` members of list of layers. """
        self._layers = layers
        used = set([None])
        for layer in reversed(layers):
            layer.last_used = list(filter(lambda x: x not in used, layer.inputs))
            used.update(layer.inputs)

    def forward(self, N=None, batch=None, keep_intermediate=True):
        """ Compute graph.

        :param N: batch size (used if batch not given)
        :param batch: indices for computation (:py:class:`Compiler.types.Array`. or list)
        :param keep_intermediate: do not free memory of intermediate results after use
        """
        if batch is None:
            batch = regint.Array(N)
            batch.assign(regint.inc(N))
        for layer in self.layers:
            if layer.inputs and len(layer.inputs) == 1 and layer.inputs[0] is not None:
                layer._X.address = layer.inputs[0].Y.address
            layer.Y.alloc()
            break_point()
            layer.forward(batch=batch)
            break_point()
            if not keep_intermediate:
                for l in layer.last_used:
                    l.Y.delete()

    def eval(self, data):
        """ Compute evaluation after training. """
        N = len(data)
        self.layers[0].X.assign(data)
        self.forward(N)
        return self.layers[-1].eval(N)

    def backward(self, batch):
        """ Compute backward propagation. """
        for layer in reversed(self.layers):
            if len(layer.inputs) == 0:
                layer.backward(compute_nabla_X=False, batch=batch)
            else:
                layer.backward(batch=batch)
                if len(layer.inputs) == 1:
                    layer.inputs[0].nabla_Y.assign_vector(
                        layer.nabla_X.get_part_vector(0, len(batch)))

    def run(self, batch_size=None):
        """ Run training.

        :param batch_size: batch size (defaults to example size of first layer)
        """
        if batch_size is not None:
            N = batch_size
        else:
            N = self.layers[0].N
        i = MemValue(0)
        @do_while
        def _():
            if self.X_by_label is None:
                self.X_by_label = [[None] * self.layers[0].N]
            assert len(self.X_by_label) in (1, 2)
            assert N % len(self.X_by_label) == 0
            n = N // len(self.X_by_label)
            n_per_epoch = int(math.ceil(1. * max(len(X) for X in
                                                 self.X_by_label) / n))
            print('%d runs per epoch' % n_per_epoch)
            indices_by_label = []
            for label, X in enumerate(self.X_by_label):
                indices = regint.Array(n * n_per_epoch)
                indices_by_label.append(indices)
                indices.assign(regint.inc(len(indices), 0, 1, 1, len(X)))
                indices.shuffle()
            @for_range(n_per_epoch)
            def _(j):
                batch = regint.Array(N)
                for label, X in enumerate(self.X_by_label):
                    indices = indices_by_label[label]
                    batch.assign(indices.get_vector(j * n, n) +
                                 regint(label * len(self.X_by_label[0]), size=n),
                                 label * n)
                self.forward(batch=batch)
                self.backward(batch=batch)
                self.update(i)
            loss = self.layers[-1].l
            if self.report_loss and not self.layers[-1].approx:
                print_ln('loss after epoch %s: %s', i, loss.reveal())
            else:
                print_ln('done with epoch %s', i)
            time()
            i.iadd(1)
            res = (i < self.n_epochs)
            if self.tol > 0:
                res *= (1 - (loss >= 0) * (loss < self.tol)).reveal()
            return res
        print_ln('finished after %s epochs', i)

class Adam(Optimizer):
    def __init__(self, layers, n_epochs):
        self.alpha = .001
        self.beta1 = 0.9
        self.beta2 = 0.999
        self.epsilon = 10 ** -8
        self.n_epochs = n_epochs

        self.layers = layers
        self.ms = []
        self.vs = []
        self.gs = []
        self.thetas = []
        for layer in layers:
            for nabla in layer.nablas():
                self.gs.append(nabla)
                for x in self.ms, self.vs:
                    x.append(nabla.same_shape())
            for theta in layer.thetas():
                self.thetas.append(theta)

        self.mhat_factors = Array(n_epochs, sfix)
        self.vhat_factors = Array(n_epochs, sfix)

        for i in range(n_epochs):
            for factors, beta in ((self.mhat_factors, self.beta1),
                                  (self.vhat_factors, self.beta2)):
                factors[i] = 1. / (1 - beta ** (i + 1))

    def update(self, i_epoch):
        for m, v, g, theta in zip(self.ms, self.vs, self.gs, self.thetas):
            @for_range_opt(len(m))
            def _(k):
                m[k] = self.beta1 * m[k] + (1 - self.beta1) * g[k]
                v[k] = self.beta2 * v[k] + (1 - self.beta2) * g[k] ** 2
                mhat = m[k] * self.mhat_factors[i_epoch]
                vhat = v[k] * self.vhat_factors[i_epoch]
                theta[k] = theta[k] - self.alpha * mhat / \
                           mpc_math.sqrt(vhat) + self.epsilon

class SGD(Optimizer):
    """ Stochastic gradient descent.

    :param layers: layers of linear graph
    :param n_epochs: number of epochs for training
    :param report_loss: disclose and print loss
    """
    def __init__(self, layers, n_epochs, debug=False, report_loss=False):
        self.momentum = 0.9
        self.layers = layers
        self.n_epochs = n_epochs
        self.thetas = []
        self.nablas = []
        self.delta_thetas = []
        for layer in layers:
            self.nablas.extend(layer.nablas())
            self.thetas.extend(layer.thetas())
            for theta in layer.thetas():
                self.delta_thetas.append(theta.same_shape())
        self.gamma = MemValue(sfix(0.01))
        self.debug = debug
        self.report_loss = report_loss
        self.tol = 0.000
        self.X_by_label = None

    def reset(self, X_by_label=None):
        """ Reset layer parameters.

        :param X_by_label: if given, set training data by public labels for balancing
        """
        self.X_by_label = X_by_label
        if X_by_label is not None:
            for label, X in enumerate(X_by_label):
                @for_range_multithread(self.n_threads, 1, len(X))
                def _(i):
                    j = i + label * len(X_by_label[0])
                    self.layers[0].X[j] = X[i]
                    self.layers[-1].Y[j] = label
        for y in self.delta_thetas:
            y.assign_all(0)
        for layer in self.layers:
            layer.reset()

    def update(self, i_epoch):
        for nabla, theta, delta_theta in zip(self.nablas, self.thetas,
                                             self.delta_thetas):
            @multithread(self.n_threads, len(nabla))
            def _(base, size):
                old = delta_theta.get_vector(base, size)
                red_old = self.momentum * old
                new = self.gamma * nabla.get_vector(base, size)
                diff = red_old - new
                delta_theta.assign_vector(diff, base)
                theta.assign_vector(theta.get_vector(base, size) +
                                    delta_theta.get_vector(base, size), base)
                if self.debug:
                    for x, name in (old, 'old'), (red_old, 'red_old'), \
                        (new, 'new'), (diff, 'diff'): 
                        x = x.reveal()
                        print_ln_if((x > 1000) + (x < -1000),
                                    name + ': %s %s %s %s',
                                    *[y.v.reveal() for y in (old, red_old, \
                                      new, diff)])
            if self.debug:
                d = delta_theta.get_vector().reveal()
                a = cfix.Array(len(d.v))
                a.assign(d)
                @for_range(len(a))
                def _(i):
                    x = a[i]
                    print_ln_if((x > 1000) + (x < -1000),
                                'update len=%d' % len(nabla))
                a.assign(nabla.get_vector().reveal())
                @for_range(len(a))
                def _(i):
                    x = a[i]
                    print_ln_if((x > 1000) + (x < -1000),
                                'nabla len=%d' % len(nabla))
        self.gamma.imul(1 - 10 ** - 6)
