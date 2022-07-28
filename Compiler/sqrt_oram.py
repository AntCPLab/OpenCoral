from __future__ import annotations
from abc import abstractmethod
from typing import Callable, Generic, Iterable, Literal, Type, Any, TypeVar
from Compiler import library as lib
from Compiler.GC.types import cbit, sbit, sbitint, sbits
from Compiler.oram import AbstractORAM, get_n_threads
from Compiler.types import MultiArray, sgf2n, sint, _secret, MemValue, Array, _clear, sintbit, cint
import numpy as np

debug = True
reveal = True
n_parallel = 1024

def swap(array: Array | MultiArray, pos_a: int | cint, pos_b: int | cint, cond: sintbit | sbit):
    if isinstance(array, MultiArray):
        temp = array[pos_b][:]
        array[pos_b].assign(cond.if_else(array[pos_a][:], array[pos_b][:]))
        array[pos_a].assign(cond.if_else(temp, array[pos_a][:]))
    if isinstance(array, Array):
        temp = array[pos_b]
        array[pos_b] = cond.if_else(array[pos_a], array[pos_b])
        array[pos_a] = cond.if_else(temp, array[pos_a])

T = TypeVar("T", sint, sbitint)
B = TypeVar("B", sintbit, sbit)

class SqrtOram(Generic[T, B]):
    # TODO: Preferably this is an Array of vectors, but this is currently not supported
    # One should regard these structures as Arrays where an entry may hold more
    # than one value (which is a nice property to have when using the ORAM in
    # practise).
    shuffle: MultiArray
    stash: MultiArray
    # A block has an index and data
    # `shuffle` and `stash` store the data,
    # `shufflei` and `stashi` store the index
    shufflei: Array
    stashi: Array

    shuffle_used: Array
    position_map: PositionMap

    # The size of the ORAM, i.e. how many elements it stores
    n: int
    # The period, i.e. how many calls can be made to the ORAM before it needs to be refreshed
    T: int
    # Keep track of how far we are in the period, and coincidentally how large
    # the stash is (each access results in a fake or real block being put on
    # the stash)
    t: cint

    def __init__(self, data: MultiArray, entry_length: int = 1, value_type: Type[T] = sint, k: int = 0, period: int | None = None) -> None:
        """Initialize a new Oblivious RAM using the "Square-Root" algorithm.

        Args:
            data (MultiArray): The data with which to initialize the ORAM. For all intents and purposes, data is regarded as a one-dimensional Array. However, one may provide a MultiArray such that every "block" can hold multiple elements (an Array).
            value_type (sint): The secret type to use, defaults to sint.
            k (int): Leave at 0, this parameter is used to recursively pass down the depth of this ORAM.
            period (int): Leave at None, this parameter is used to recursively pass down the top-level period.
        """
        self.n = len(data)

        self.value_type = value_type
        if value_type != sint and value_type != sbitint:
            raise Exception("The value_type must be either sint or sbitint")
        self.bit_type: Type[B] = value_type.bit_type
        self.index_type = value_type.get_type(int(np.ceil(np.log2(self.n)) ))
        self.entry_length = entry_length

        if debug:
            lib.print_ln('Initializing SqrtORAM of size %s at depth %s', self.n, k)
        self.shuffle_used = cint.Array(self.n)
        # Random permutation on the data
        self.shuffle = data
        self.shufflei = Array.create_from([self.index_type(i) for i in range(self.n)])
        permutation = Array.create_from(self.shuffle_the_shuffle())
        # Calculate the period if not given
        # upon recursion, the period should stay the same ("in sync"),
        # therefore it can be passed as a constructor parameter
        self.T = int(np.ceil(np.sqrt(self.n * np.log2(self.n) - self.n + 1))
                         ) if not period else period
        if debug and not period:
            lib.print_ln('Period set to %s', self.T)
        # Initialize position map (recursive oram)
        self.position_map = PositionMap.create(permutation, k + 1, self.T)

        # Initialize stash
        self.stash = MultiArray((self.T, data.sizes[1]), value_type=value_type)
        self.stashi = Array(self.T, value_type=value_type)
        self.t = MemValue(cint(0))


    def read(self, index: T):
        data = self.value_type.Array(self.entry_length)
        return self.access(index, self.bit_type(False), data)

    def write(self, index: T, value: Array):
        self.access(index, self.bit_type(True), value)

    __getitem__ = read
    __setitem__ = write

    def access(self, index: T, write: B, value: Array):
        if len(value) != self.entry_length:
            raise Exception("A block must be of size entry_length={}".format(self.entry_length))
        # Method Blocks do not accepts arrays as arguments
        # workaround by temporarily storing it as a class field
        # arrays are stored in memory so this is fine
        index = MemValue(index)
        return Array.create_from(self._access(index, write, value[:]))

    @lib.method_block
    def _access(self, index: T, write: B, *value: list[T]):
        item: T = self.value_type(*value)

        if debug:
            @lib.if_e(write.reveal() == 1)
            def _():
                lib.print_ln('Writing to secret index %s', index.reveal())
            @lib.else_
            def __():
                lib.print_ln('Reading from secret index %s', index.reveal())

        # Refresh if we have performed T (period) accesses
        @lib.if_(self.t == self.T)
        def _():
            self.refresh()

        found: B = MemValue(self.bit_type(False))

        # Scan through the stash
        @lib.if_(self.t > 0)
        def _():
            nonlocal found
            found |= index == self.stashi[0]
        # We ensure that if the item is found in stash, it ends up in the first
        # position (more importantly, a fixed position) of the stash
        # This allows us to keep track of it in an oblivious manner
        @lib.for_range_opt(self.t)
        def _(i):
            nonlocal found
            found_: B =  index == self.stashi[i + 1]
            swap(self.stash, 0, i, found_)
            swap(self.stashi, 0, i, found_)
            found |= found_
            #  found = self.bit_type(found.bit_or(found_))
        # If the item was not in the stash, we move the unknown and unimportant
        # stash[0] out of the way (to the end of the stash)
        swap(self.stash, self.t, 0, sintbit(found.bit_not().bit_and(self.t > 0)))
        swap(self.stashi, self.t, 0, sintbit(found.bit_not().bit_and(self.t > 0)))

        if debug:
            @lib.if_e(found.reveal() == 1)
            def _():
                lib.print_ln('    Found item in stash')
            @lib.else_
            def __():
                lib.print_ln('    Item not in stash')
                lib.print_ln('    Moved stash[0]=(%s: %s) to stash[t=%s]=(%s: %s)', self.stashi[0].reveal(), self.stash[0].reveal(), self.t, self.stashi[self.t].reveal(), self.stash[self.t].reveal())

        # Possible fake lookup of the item in the shuffle,
        # depending on whether we already found the item in the stash
        physical_address = self.position_map.get_position(index, found)
        self.shuffle_used[physical_address] = cbit(True)

        # If the item was in the stash (thus currently residing in stash[0]),
        # we place the random item retrieved from the shuffle at the end of the stash
        self.stash[self.t].assign(found.if_else(
            self.shuffle[physical_address][:],
            self.stash[self.t][:]))
        self.stashi[self.t] = found.if_else(
                self.shufflei[physical_address],
                self.stashi[self.t])
        # If the item was not found in the stash,
        # we place the item retrieved from the shuffle in stash[0]
        self.stash[0].assign(found.bit_not().if_else(
            self.shuffle[physical_address][:],
            self.stash[0][:]))
        self.stashi[0] = found.bit_not().if_else(
                self.shufflei[physical_address],
                self.stashi[0])
        if debug:
            @lib.if_e(found.reveal() == 1)
            def _():
                lib.print_ln('\tMoved shuffle[%s]=(%s: %s) to stash[t]', physical_address, self.shufflei[physical_address].reveal(), self.shuffle[physical_address].reveal())
            @lib.else_
            def __():
                lib.print_ln('\tMoved shuffle[%s]=(%s: %s) to stash[0]', physical_address, self.shufflei[physical_address].reveal(), self.shuffle[physical_address].reveal())


        # Increase the "time" (i.e. access count in current period)
        self.t.iadd(1)

        self.stash[0].assign(write.if_else(item, self.stash[0][:]))
        item=write.bit_not().if_else(self.stash[0][:], item)
        return item


    @lib.method_block
    def shuffle_the_shuffle(self):
        """Permute the memory using a newly generated permutation and return
        the permutation that would generate this particular shuffling.

        This permutation is needed to know how to map logical addresses to
        physical addresses, and is used as such by the postition map."""

        # Random permutation on n elements
        random_shuffle = sint.get_secure_shuffle(self.n)
        # Apply the random permutation
        lib.print_ln('\tGenerated shuffle')
        self.shuffle.secure_permute(random_shuffle)
        lib.print_ln('\tShuffled shuffle')
        self.shufflei.secure_permute(random_shuffle)
        lib.print_ln('\tShuffled shuffle indexes')
        # Calculate the permutation that would have produced the newly produced
        # shuffle order. This can be calculated by regarding the logical
        # indexes (shufflei) as a permutation and calculating its inverse,
        # i.e. find P such that P([1,2,3,...]) = shufflei.
        # this is not necessarily equal to the inverse of the above generated
        # random_shuffle, as the shuffle may already be out of order (e.g. when
        # refreshing).
        permutation = MemValue(self.shufflei[:].inverse_permutation())
        lib.print_ln('\tCalculated inverse permutation')
        return permutation

    @lib.method_block
    def refresh(self):
        """Refresh the ORAM by reinserting the stash back into the shuffle, and
        reshuffling the shuffle.

        This must happen after T (period) accesses to the ORAM."""
        lib.print_ln('Refreshing SqrtORAM')

        # Shuffle and emtpy the stash, and store elements back into shuffle
        j = MemValue(cint(0,size=1))
        @lib.for_range_opt(self.n)
        def _(i):
            @lib.if_(self.shuffle_used[i])
            def _():
                nonlocal j
                self.shuffle[i] = self.stash[j]
                self.shufflei[i] = self.stashi[j]
                j += 1

        # Reset the clock
        self.t.write(0)
        # Reset shuffle_used
        self.shuffle_used.assign_all(0)

        # Reinitialize position map
        permutation = self.shuffle_the_shuffle()
        # Note that we skip here the step of "packing" the permutation.
        # Since the underlying memory of the position map is already aligned in
        # this packed structure, we can simply overwrite the memory while
        # maintaining the structure.
        self.position_map.reinitialize(*permutation)

    @lib.method_block
    def reinitialize(self, *data: T):
        # Note that this method is only used during refresh, and as such is
        # only called with a permutation as data.

        # The logical addresses of some previous permutation are irrelevant and must be reset
        self.shufflei.assign([self.index_type(i) for i in range(self.n)])
        # Reset the clock
        self.t.write(0)
        # Reset shuffle_used
        self.shuffle_used.assign_all(0)

        # Note that the self.shuffle is actually a MultiArray
        # This structure is preserved while overwriting the values using
        # assign_vector
        self.shuffle.assign_vector(self.value_type(data, size=self.n * self.entry_length))
        permutation = self.shuffle_the_shuffle()
        self.position_map.reinitialize(*permutation)


class PositionMap(Generic[T, B]):
    PACK_LOG: int = 2
    PACK: int = 1 << PACK_LOG

    n: int # n in the paper
    depth: int # k in the paper
    value_type: Type[T]

    def __init__(self, n: int, value_type: Type[T] = sint, k:int = -1) -> None:
        self.n = n
        self.depth=MemValue(cint(k))
        self.value_type = value_type
        self.bit_type = value_type.bit_type
        self.index_type = self.value_type.get_type(int(np.ceil(np.log2(n))))

    @abstractmethod
    def get_position(self, logical_address: _secret, fake: B) -> Any:
        """Retrieve the block at the given (secret) logical address."""
        if debug:
            lib.print_ln('\t%s Scanning %s for logical address %s (fake=%s)', self.depth, self.__class__.__name__, logical_address.reveal(), sintbit(fake).reveal())

    def reinitialize(self, *permutation: T):
        """Reinitialize this PositionMap.

        Since the reinitialization occurs at runtime (`on SqrtORAM.refresh()`),
        we cannot simply call __init__ on self. Instead, we must take care to
        reuse and overwrite the same memory.
        """
        ...

    @classmethod
    def create(cls, permutation: Array, k: int, period: int, value_type: Type[T] = sint) -> PositionMap:
        """Creates a new PositionMap. This is the method one should call when
        needing a new position map. Depending on the size of the given data, it
        will either instantiate a RecursivePositionMap or
        a LinearPositionMap."""
        n = len(permutation)

        if n / PositionMap.PACK <= period:
            if debug:
                lib.print_ln('Initializing LinearPositionMap at depth %s of size %s', k, n)
            res = LinearPositionMap(permutation, value_type, k=k)
        else:
            if debug:
                lib.print_ln('Initializing RecursivePositionMap at depth %s of size %s', k, n)
            res = RecursivePositionMap(permutation, period, value_type, k=k)

        return res


class RecursivePositionMap(PositionMap[T, B], SqrtOram[T, B]):

    def __init__(self, permutation: Array, period: int, value_type: Type[T] = sint, k:int=-1) -> None:
        PositionMap.__init__(self, len(permutation), k=k)
        pack = PositionMap.PACK

        # We pack the permutation into a smaller structure, index with a new permutation
        packed_size = int(np.ceil(self.n / pack))
        packed_structure = MultiArray(
            (packed_size, pack), value_type=value_type)
        for i in range(packed_size):
            packed_structure[i] = Array.create_from(
                permutation[i*pack:(i+1)*pack])

        # TODO: Should this be n or packed_size?
        SqrtOram.__init__(self, packed_structure, value_type=value_type, period=period, entry_length=pack, k=self.depth)

    @lib.method_block
    def get_position(self, logical_address: T, fake: B) -> _clear:
        super().get_position(logical_address, fake)

        pack = PositionMap.PACK
        pack_log = PositionMap.PACK_LOG

        # The item at logical_address
        # will be in block with index h (block.<h>)
        # at position l in block.data (block.data<l>)
        h = MemValue(self.value_type.bit_compose(sbits.get_type(program.bit_length)(logical_address).right_shift(pack_log, program.bit_length)))
        l = self.value_type.bit_compose(sbits(logical_address) & (pack - 1))

        # The resulting physical address
        p = MemValue(self.index_type(0))
        found: B = MemValue(self.bit_type(False))

        # First we try and retrieve the item from the stash

        # We retrieve stash[h]
        # Since h is secret, we do this by scanning the entire stash
        @lib.for_range(self.t)
        def _(j):
            nonlocal found
            condition = self.stashi[j] == h
            found |= condition
            # block = stash[h]
            # block is itself an array (it holds a permutation)
            # we need to grab block[l]
            @lib.for_range(pack)
            def _(i):
                nonlocal condition
                condition &= l == i
                p.write(condition.if_else(self.stash[j][i], p))

            if debug:
                @lib.if_(condition.reveal() == 1)
                def _():
                    lib.print_ln('\t%s Found position in stash[%s]=(%s: %s)', self.depth, j, self.stashi[j].reveal(), self.stash[j].reveal())

        # Then we try and retrieve the item from the shuffle (the actual memory)

        if debug:
            @lib.if_(found.reveal() == 0)
            def _():
                lib.print_ln('\t%s Position not in stash', self.depth)


        p_prime = self.position_map.get_position(h, found)
        self.shuffle_used[p_prime] = cbit(True)
        # The block retrieved from the shuffle
        # Depending on whether the block has already been `found`, this block
        # is either the desired block (found=False) or a random block
        # (found=True)
        block_p_prime: Array = self.shuffle[p_prime]

        if debug:
            @lib.if_e(found.reveal() == 0)
            def _():
                lib.print_ln('\t%s Retrieved stash[%s]=(%s: %s)', self.depth, p_prime.reveal(), self.shufflei[p_prime.reveal()].reveal(), self.shuffle[p_prime.reveal()].reveal())
            @lib.else_
            def __():
                lib.print_ln('\t%s Retrieved dummy stash[%s]=(%s: %s)',self.depth, p_prime.reveal(), self.shufflei[p_prime.reveal()].reveal(), self.shuffle[p_prime.reveal()].reveal())

        # We add the retrieved block from the shuffle to the stash
        self.stash[self.t].assign(block_p_prime[:])
        self.stashi[self.t] = self.shufflei[p_prime]
        # Increase t
        self.t += 1

        # if found or not fake
        condition = self.bit_type(fake.bit_or(found.bit_not()))
        # Retrieve l'th item from block
        # l is secret, so we must use linear scan
        @lib.for_range_opt(pack)
        def _(i):
            hit: B = self.bit_type(i == l)
            p.write((condition & hit).if_else(block_p_prime[i], p))

        return p.reveal()

    @lib.method_block
    def reinitialize(self, *permutation: T):
        SqrtOram.reinitialize(self, *permutation)

class LinearPositionMap(PositionMap):
    physical: Array
    used: Array

    def __init__(self, data: Array, value_type: Type[T] = sint, k:int =-1) -> None:
        PositionMap.__init__(self, len(data), value_type, k=k)
        self.physical = data
        self.used = self.bit_type.Array(self.n)

    @lib.method_block
    def get_position(self, logical_address: T, fake: B) -> _clear:
        """
        This method corresponds to GetPosBase in the paper.
        """
        super().get_position(logical_address, fake)
        fake = self.bit_type(fake)

        # In order to get an address at secret logical_address,
        # we need to perform a linear scan.
        linear_scan = self.bit_type.Array(self.n)
        @lib.for_range_opt(self.n)
        def _(i):
            linear_scan[i] = logical_address == i

        p: MemValue = MemValue(self.index_type(-1))
        done: B = self.bit_type(False)

        @lib.for_range_opt(self.n)
        def _(j):
            nonlocal done, fake
            condition: B = (self.bit_type(fake.bit_not()) & linear_scan[j]) \
                .bit_or(fake & self.bit_type((self.used[j]).bit_not()) & done.bit_not())
            p.write(condition.if_else(self.physical[j], p))
            self.used[j] = condition.if_else(self.bit_type(True), self.used[j])
            done = self.bit_type(condition.if_else(self.bit_type(True), done))

        if debug:
            @lib.if_((p.reveal() < 0).bit_or(p.reveal() > len(self.physical)))
            def _():
                lib.runtime_error('%s Did not find requested logical_address in shuffle, something went wrong.', self.depth)

        return p.reveal()

    @lib.method_block
    def reinitialize(self, *data: T):
        self.physical.assign_vector(data)
        self.used.assign_all(False)
