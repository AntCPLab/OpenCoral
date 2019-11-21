import mpc_math, math

from Compiler.types import *
from Compiler.types import _unreduced_squant
from Compiler.library import *
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
    return sigmoid_from_e_x(x, exp(-x))

def sigmoid_from_e_x(x, e_x):
    return sanitize(x, 1 / (1 + e_x), 0, 1)

def sigmoid_prime(x):
    sx = sigmoid(x)
    return sx * (1 - sx)

def lse_0_from_e_x(x, e_x):
    return sanitize(-x, log_e(1 + e_x), x + 2 ** -x.f, 0)

def lse_0(x):
    return lse_0_from_e_x(x, exp(x))

def relu_prime(x):
    return (0 <= x)

def relu(x):
    return (0 < x).if_else(x, 0)

def progress(x):
    return
    print_ln(x)
    time()

def set_n_threads(n_threads):
    Layer.n_threads = n_threads
    Optimizer.n_threads = n_threads

class Layer:
    n_threads = 1

class Output(Layer):
    def __init__(self, N, debug=False):
        self.N = N
        self.X = sfix.Array(N)
        self.Y = sfix.Array(N)
        self.nabla_X = sfix.Array(N)
        self.l = MemValue(sfix(-1))
        self.e_x = sfix.Array(N)
        self.debug = debug
        self.weights = cint.Array(N)
        self.weights.assign_all(1)
        self.weight_total = N

    nablas = lambda self: ()
    thetas = lambda self: ()
    reset = lambda self: None

    def divisor(self, divisor, size):
        return cfix(1.0 / divisor, size=size)

    def forward(self, N=None):
        N = N or self.N
        lse = sfix.Array(N)
        @multithread(self.n_threads, N)
        def _(base, size):
            x = self.X.get_vector(base, size)
            y = self.Y.get_vector(base, size)
            e_x = exp(-x)
            self.e_x.assign(e_x, base)
            lse.assign(lse_0_from_e_x(-x, e_x) + x * (1 - y), base)
        e_x = self.e_x.get_vector(0, N)
        self.l.write(sum(lse) * \
                     self.divisor(self.N, 1))

    def backward(self):
        @multithread(self.n_threads, self.N)
        def _(base, size):
            diff = sigmoid_from_e_x(self.X.get_vector(base, size),
                                    self.e_x.get_vector(base, size)) - \
                                self.Y.get_vector(base, size)
            assert sfix.f == cfix.f
            diff *= self.weights.get_vector(base, size)
            self.nabla_X.assign(diff * self.divisor(self.weight_total, size), \
                                base)
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
        self.weights.assign(weights)
        self.weight_total = sum(weights)

class DenseBase(Layer):
    thetas = lambda self: (self.W, self.b)
    nablas = lambda self: (self.nabla_W, self.nabla_b)

    def backward_params(self, f_schur_Y):
        N = self.N
        tmp = Matrix(self.d_in, self.d_out, unreduced_sfix)

        @for_range_opt_multithread(self.n_threads, [self.d_in, self.d_out])
        def _(j, k):
            assert self.d == 1
            a = [f_schur_Y[i][0][k] for i in range(N)]
            b = [self.X[i][0][j] for i in range(N)]
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

        self.reset()

        self.nabla_Y = MultiArray([N, d, d_out], sfix)
        self.nabla_X = MultiArray([N, d, d_in], sfix)
        self.nabla_W = sfix.Matrix(d_in, d_out)
        self.nabla_W.assign_all(0)
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

    def compute_f_input(self):
        prod = MultiArray([self.N, self.d, self.d_out], sfix)
        @for_range_opt_multithread(self.n_threads, self.N)
        def _(i):
            self.X[i].plain_mul(self.W, res=prod[i])

        @for_range_opt_multithread(self.n_threads, self.N)
        def _(i):
            @for_range_opt(self.d)
            def _(j):
                v = prod[i][j].get_vector() + self.b.get_vector()
                self.f_input[i][j].assign(v)
        progress('f input')

    def forward(self):
        self.compute_f_input()
        self.Y.assign_vector(self.f(self.f_input.get_vector()))

    def backward(self, compute_nabla_X=True):
        N = self.N
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
                f_schur_Y[i] = nabla_Y[i].schur(f_prime_bit[i])

            progress('f prime schur Y')

        if compute_nabla_X:
            @for_range_opt(N)
            def _(i):
                if self.activation == 'id':
                    nabla_X[i] = nabla_Y[i].mul_trans(W)
                else:
                    nabla_X[i] = nabla_Y[i].schur(f_prime_bit[i]).mul_trans(W)

            progress('nabla X')

        self.backward_params(f_schur_Y)

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

class QuantBase(object):
    n_threads = 1

    @staticmethod
    def new_squant():
        class _(squant):
            @classmethod
            def get_input_from(cls, player, size=None):
                return cls._new(sint.get_input_from(player, size=size))
        return _

    def __init__(self, input_shape, output_shape):
        self.input_shape = input_shape
        self.output_shape = output_shape

        self.input_squant = self.new_squant()
        self.output_squant = self.new_squant()

        self.X = MultiArray(input_shape, self.input_squant)
        self.Y = MultiArray(output_shape, self.output_squant)

    def temp_shape(self):
        return [0]

class QuantConvBase(QuantBase):
    fewer_rounds = True
    temp_weights = None
    temp_inputs = None

    @classmethod
    def init_temp(cls, layers):
        size = 0
        for layer in layers:
            size = max(size, reduce(operator.mul, layer.temp_shape()))
        cls.temp_weights = sfix.Array(size)
        cls.temp_inputs = sfix.Array(size)

    def __init__(self, input_shape, weight_shape, bias_shape, output_shape, stride):
        super(QuantConvBase, self).__init__(input_shape, output_shape)

        self.weight_shape = weight_shape
        self.bias_shape = bias_shape
        self.stride = stride

        self.weight_squant = self.new_squant()
        self.bias_squant = self.new_squant()

        self.weights = MultiArray(weight_shape, self.weight_squant)
        self.bias = Array(output_shape[-1], self.bias_squant)

        self.unreduced = MultiArray(self.output_shape, sint,
                                    address=self.Y.address)

        assert(weight_shape[-1] == input_shape[-1])
        assert(bias_shape[0] == output_shape[-1])
        assert(len(bias_shape) == 1)
        assert(len(input_shape) == 4)
        assert(len(output_shape) == 4)
        assert(len(weight_shape) == 4)

    def input_from(self, player):
        for s in self.input_squant, self.weight_squant, self.bias_squant, self.output_squant:
            s.set_params(sfloat.get_input_from(player), sint.get_input_from(player))
        self.weights.input_from(player, budget=100000)
        self.bias.input_from(player)
        print('WARNING: assuming that bias quantization parameters are correct')

        self.output_squant.params.precompute(self.input_squant.params, self.weight_squant.params)

    def dot_product(self, iv, wv, out_y, out_x, out_c):
        bias = self.bias[out_c]
        acc = squant.unreduced_dot_product(iv, wv)
        acc.v += bias.v
        acc.res_params = self.output_squant.params
        #self.Y[0][out_y][out_x][out_c] = acc.reduce_after_mul()
        self.unreduced[0][out_y][out_x][out_c] = acc.v

    def reduction(self):
        unreduced = self.unreduced
        n_summands = self.n_summands()
        start_timer(2)
        n_outputs = reduce(operator.mul, self.output_shape)
        if n_outputs % self.n_threads == 0:
            n_per_thread = n_outputs // self.n_threads
            @for_range_opt_multithread(self.n_threads, self.n_threads)
            def _(i):
                res = _unreduced_squant(
                    sint.load_mem(unreduced.address + i * n_per_thread,
                                  size=n_per_thread),
                    (self.input_squant.params, self.weight_squant.params),
                    self.output_squant.params,
                    n_summands).reduce_after_mul()
                res.store_in_mem(self.Y.address + i * n_per_thread)
        else:
            @for_range_opt_multithread(self.n_threads, self.output_shape[1])
            def _(out_y):
                self.Y[0][out_y].assign_vector(_unreduced_squant(
                    unreduced[0][out_y].get_vector(),
                    (self.input_squant.params, self.weight_squant.params),
                    self.output_squant.params,
                    n_summands).reduce_after_mul())
        stop_timer(2)

    def temp_shape(self):
        return list(self.output_shape[1:]) + [self.n_summands()]

    def prepare_temp(self):
        shape = self.temp_shape()
        inputs = MultiArray(shape, self.input_squant,
                            address=self.temp_inputs)
        weights = MultiArray(shape, self.weight_squant,
                             address=self.temp_weights)
        return inputs, weights

class QuantConv2d(QuantConvBase):
    def n_summands(self):
        _, weights_h, weights_w, _ = self.weight_shape
        _, inputs_h, inputs_w, n_channels_in = self.input_shape
        return weights_h * weights_w * n_channels_in

    def forward(self, N=1):
        assert(N == 1)
        assert(self.weight_shape[0] == self.output_shape[-1])

        _, weights_h, weights_w, _ = self.weight_shape
        _, inputs_h, inputs_w, n_channels_in = self.input_shape
        _, output_h, output_w, n_channels_out = self.output_shape

        stride_h, stride_w = self.stride
        padding_h, padding_w = (weights_h // 2, weights_w // 2)

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
                            if inside is 0:
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

class QuantDepthwiseConv2d(QuantConvBase):
    def n_summands(self):
        _, weights_h, weights_w, _ = self.weight_shape
        return weights_h * weights_w

    def forward(self, N=1):
        assert(N == 1)
        assert(self.weight_shape[-1] == self.output_shape[-1])
        assert(self.input_shape[-1] == self.output_shape[-1])

        _, weights_h, weights_w, _ = self.weight_shape
        _, inputs_h, inputs_w, n_channels_in = self.input_shape
        _, output_h, output_w, n_channels_out = self.output_shape

        stride_h, stride_w = self.stride
        padding_h, padding_w = (weights_h // 2, weights_w // 2)

        depth_multiplier = 1

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
                                if inside is 0:
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

class QuantAveragePool2d(QuantBase):
    def __init__(self, input_shape, output_shape, filter_size):
        super(QuantAveragePool2d, self).__init__(input_shape, output_shape)
        self.filter_size = filter_size

    def input_from(self, player):
        print('WARNING: assuming that input and output quantization parameters are the same')
        for s in self.input_squant, self.output_squant:
            s.set_params(sfloat.get_input_from(player), sint.get_input_from(player))

    def forward(self, N=1):
        assert(N == 1)

        _, input_h, input_w, n_channels_in = self.input_shape
        _, output_h, output_w, n_channels_out = self.output_shape

        n = input_h * input_w
        print('divisor: ', n)

        assert output_h == output_w == 1
        assert n_channels_in == n_channels_out

        padding_h, padding_w = (0, 0)
        stride_h, stride_w = (2, 2)
        filter_h, filter_w = self.filter_size

        @for_range_opt(output_h)
        def _(out_y):
            @for_range_opt(output_w)
            def _(out_x):
                @for_range_opt(n_channels_in)
                def _(c):
                    in_x_origin = (out_x * stride_w) - padding_w
                    in_y_origin = (out_y * stride_h) - padding_h
                    fxs = (-in_x_origin).max(0)
                    #fxe = min(filter_w, input_w - in_x_origin)
                    fys = (-in_y_origin).max(0)
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
                    logn = int(math.log(n, 2))
                    acc = (acc + n // 2)
                    if 2 ** logn == n:
                        acc = acc.round(self.output_squant.params.k + logn,
                                        logn, nearest=True)
                    else:
                        acc = acc.int_div(sint(n),
                                          self.output_squant.params.k + logn)
                    #acc = min(255, max(0, acc))
                    self.Y[0][out_y][out_x][c] = self.output_squant._new(acc)

class QuantReshape(QuantBase):
    def __init__(self, input_shape, _, output_shape):
        super(QuantReshape, self).__init__(input_shape, output_shape)

    def input_from(self, player):
        print('WARNING: assuming that input and output quantization parameters are the same')
        _ = self.new_squant()
        for s in self.input_squant, _, self.output_squant:
            s.set_params(sfloat.get_input_from(player), sint.get_input_from(player))
        for i in range(2):
            sint.get_input_from(player)

    def forward(self, N=1):
        assert(N == 1)
        # reshaping is implicit
        self.Y.assign(self.X)

class QuantSoftmax(QuantBase):
    def input_from(self, player):
        print('WARNING: assuming that input and output quantization parameters are the same')
        for s in self.input_squant, self.output_squant:
            s.set_params(sfloat.get_input_from(player), sint.get_input_from(player))

    def forward(self, N=1):
        assert(N == 1)
        assert(len(self.input_shape) == 2)

        # just print the best
        def comp(left, right):
            c = left[1].v.greater_than(right[1].v, self.input_squant.params.k)
            #print_ln('comp %s %s %s', c.reveal(), left[1].v.reveal(), right[1].v.reveal())
            return [c.if_else(x, y) for x, y in zip(left, right)]
        print_ln('guess: %s', util.tree_reduce(comp, list(enumerate(self.X[0])))[0].reveal())

class Optimizer:
    n_threads = Layer.n_threads

    def forward(self, N):
        for j in range(len(self.layers) - 1):
            self.layers[j].forward()
            self.layers[j + 1].X.assign(self.layers[j].Y)
        self.layers[-1].forward(N)

    def backward(self):
        for j in range(1, len(self.layers)):
            self.layers[-j].backward()
            self.layers[-j - 1].nabla_Y.assign(self.layers[-j].nabla_X)
        self.layers[0].backward(compute_nabla_X=False)

    def run(self):
        i = MemValue(0)
        @do_while
        def _():
            if self.X_by_label is not None:
                N = self.layers[0].N
                assert self.layers[-1].N == N
                assert N % 2 == 0
                n = N // 2
                @for_range(n)
                def _(i):
                    self.layers[-1].Y[i] = 0
                    self.layers[-1].Y[i + n] = 1
                n_per_epoch = int(math.ceil(1. * max(len(X) for X in
                                                     self.X_by_label) / n))
                print('%d runs per epoch' % n_per_epoch)
                indices_by_label = []
                for label, X in enumerate(self.X_by_label):
                    indices = regint.Array(n * n_per_epoch)
                    indices_by_label.append(indices)
                    indices.assign(i % len(X) for i in range(len(indices)))
                    indices.shuffle()
                @for_range(n_per_epoch)
                def _(j):
                    j = MemValue(j)
                    for label, X in enumerate(self.X_by_label):
                        indices = indices_by_label[label]
                        @for_range_multithread(self.n_threads, 1, n)
                        def _(i):
                            idx = indices[i + j * n_per_epoch]
                            self.layers[0].X[i + label * n] = X[idx]
                    self.forward(None)
                    self.backward()
                    self.update(i)
            else:
                self.forward(None)
                self.backward()
                self.update(i)
            loss = self.layers[-1].l
            if self.report_loss:
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
        self.X_by_label = X_by_label
        for y in self.delta_thetas:
            y.assign_all(0)
        for layer in self.layers:
            layer.reset()

    def update(self, i_epoch):
        for nabla, theta, delta_theta in zip(self.nablas, self.thetas,
                                             self.delta_thetas):
            @for_range_opt_multithread(self.n_threads, len(nabla))
            def _(k):
                old = delta_theta[k]
                if isinstance(old, Array):
                    old = old.get_vector()
                red_old = self.momentum * old
                new = self.gamma * nabla[k]
                diff = red_old - new
                delta_theta[k] = diff
                theta[k] = theta[k] + delta_theta[k]
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
