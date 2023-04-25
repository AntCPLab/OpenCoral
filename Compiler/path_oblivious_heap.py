"""This module contains a basic implementation of the Path Oblivious Heap'
oblivious priority queue as proposed by 
`Shi <https://eprint.iacr.org/2019/274.pdf>`.
"""

from abc import ABC, abstractmethod
from enum import Enum
from typing import Generic, Tuple, TypeVar

from Compiler import library as lib, oram, util
from Compiler.circuit_oram import CircuitORAM
from Compiler.dijkstra import HeapEntry
from Compiler.path_oram import PathORAM
from Compiler.types import sint, Array, _secret


# TODO:
# - How to represent subtree-min (separate RAM?)
# - Implement update_min
# - Implement/modify evict
# - Implement queue operations


def noop(*args, **kwargs):
    pass


### SETTINGS ###

# If enabled, compile-time debugging info is printed.
COMPILE_DEBUG = True

# If enabled, high-level debugging messages are printed at runtime.
DEBUG = True

dprint_ln = lib.print_ln if DEBUG else noop
cprint = print if COMPILE_DEBUG else noop

### IMPLEMENTATION ###

T = TypeVar("T")  # TODO: Should be more restrictive


class AbstractMinPriorityQueue(ABC, Generic[T]):
    """An abstract class defining the basic behavior
    of a min priority queue.
    """

    @abstractmethod
    def insert(self, value: T, priority: T) -> None:
        """Insert a value with a priority into the queue."""
        pass

    @abstractmethod
    def extract_min(self) -> T:
        """Remove the minimal element in the queue and return it."""
        pass


class EmptyIndexStructure:
    """Since Path Oblivious Heap does not need to
    maintain a position map, we use an empty index structure
    for compatibility.
    """

    def __init__(*args, **kwargs):
        pass

    def noop(*args, **kwargs):
        return None

    def __getattr__(self, _):
        return self.noop


class NoIndexORAM:
    index_structure = EmptyIndexStructure


class BasicMinTree(NoIndexORAM):
    """Basic Min tree data structure behavior."""

    def __init__(self):
        subtree_min_size = self.entry_size + (self.D,)  # (prio, value, leaf)
        # Maintain subtree-mins in a separate RAM
        # (some of the attributes we access are defined in the ORAM classes,
        # so no meta information is available when accessed in this constructor.)
        self.subtree_mins = oram.RAM(
            2 ** (self.D + 1),
            oram.Entry.get_empty(self.value_type, subtree_min_size).types(),
            self.get_array,
        )

    @lib.method_block
    def update_min(self, leaf: int):
        # TODO: Update min (leaf to root)
        raise NotImplementedError


class CircuitMinTree(CircuitORAM, BasicMinTree):
    """Binary Bucket Tree data structure
    using Circuit ORAM as underlying data structure.
    """

    def __init__(
        self,
        capacity: int,
        int_type: _secret = sint,
        entry_size: Tuple[int] | None = None,
        bucket_size: int = 3,
        stash_size: int | None = None,
        init_rounds: int = -1,
    ):
        CircuitORAM.__init__(
            self,
            capacity,
            value_type=int_type,
            entry_size=entry_size,
            bucket_size=bucket_size,
            stash_size=stash_size,
            init_rounds=init_rounds,
        )
        BasicMinTree.__init__(self)


class PathMinTree(PathORAM, BasicMinTree):
    """Binary Bucket Tree data structure
    using Path ORAM as underlying data structure.
    """

    def __init__(
        self,
        capacity: int,
        int_type: _secret = sint,
        entry_size: Tuple[int] | None = None,
        bucket_oram: oram.AbstractORAM = oram.TrivialORAM,
        bucket_size: int = 2,
        stash_size: int | None = None,
        init_rounds: int = -1,
    ):
        PathORAM.__init__(
            self,
            capacity,
            value_type=int_type,
            entry_size=entry_size,
            bucket_oram=bucket_oram,
            bucket_size=bucket_size,
            stash_size=stash_size,
            init_rounds=init_rounds,
        )
        # For compatibility with inherited __repr__
        self.ram = self.buckets
        self.root = oram.RefBucket(1, self)

        BasicMinTree.__init__(self)


class POHVariant(Enum):
    """Constants representing Path and Circuit variants
    and utility functions to map the variants to defaults.
    """

    PATH = 0
    CIRCUIT = 1

    def get_tree_class(self):
        return PathMinTree if self == self.PATH else CircuitMinTree

    def get_default_bucket_size(self):
        return 2 if self == self.PATH else 3

    def __repr__(self):
        return "Path" if self == self.PATH else "Circuit"


class PathObliviousHeap(AbstractMinPriorityQueue[_secret]):
    """A basic Path Oblivious Heap implementation supporting
    insert, extract_min, and find_min.
    """

    def __init__(
        self,
        capacity: int,
        security: int | None = None,
        type_hiding_security: bool = False,
        int_type: _secret = sint,
        entry_size: Tuple[int] | None = None,
        variant: POHVariant = POHVariant.PATH,
        bucket_oram: oram.AbstractORAM = oram.TrivialORAM,
        bucket_size: int | None = None,
        stash_size: int | None = None,
        init_rounds: int = -1,
    ):
        # Check inputs
        if int_type != sint:
            raise lib.CompilerError("POH: Only sint is supported as int_type.")

        if variant is not POHVariant.PATH:
            raise lib.CompilerError("POH: Only the PATH variant is supported.")

        # Initialize basic class fields
        self.type_hiding_security = type_hiding_security

        # TODO: Figure out what default should be (capacity = poly(security))
        if security is None:
            security = capacity

        # Use default entry size (for Dijkstra) if not specified (distance, node)
        if entry_size is None:
            entry_size = (32, util.log2(capacity))  # TODO: Why 32?

        # Use default bucket size if not specified
        if bucket_size is None:
            bucket_size = variant.get_default_bucket_size()

        # TODO: Base stash size on security parameter if not specified
        if stash_size is None:
            stash_size = util.log2(security) ** 2  # TODO: ???

        # Print debug messages
        dprint_ln(
            "POH: Initializing a queue with a capacity of %s and security parameter %s",
            capacity,
            security,
        )
        dprint_ln(
            f"POH: Type hiding security is {'en' if self.type_hiding_security else 'dis'}abled",
        )
        dprint_ln("POH: Variant is %s", variant)

        # Initialize data structure with dummy elements
        self.tree = variant.get_tree_class()(
            capacity,
            int_type=int_type,
            entry_size=entry_size,
            bucket_oram=bucket_oram,
            bucket_size=bucket_size,
            stash_size=stash_size,
            init_rounds=init_rounds,
        )

    def insert(self, value: _secret, priority: _secret, fake: bool = False) -> None:
        """Insert an element with a priority into the queue."""
        if fake:
            self._fake_insert()
        elif self.type_hiding_security:
            # Important: Fixed order of operations
            self._insert(value, priority)
            self._fake_extract_min()
            self._fake_find_min()
        else:
            self._insert(value, priority)

    def extract_min(self, fake: bool = False) -> _secret | None:
        """Extract the element with the smallest (ie. highest)
        priority from the queue.
        """
        if fake:
            self._fake_extract_min()
        elif self.type_hiding_security:
            # Important: Fixed order of operations
            self._fake_insert()
            res = self._extract_min()
            self._fake_find_min()
            return res
        else:
            return self._extract_min()

    def find_min(self, fake: bool = False) -> _secret | None:
        """Find the element with the smallest (ie. highest)
        priority in the queue and return its value and priority."""
        if fake:
            self._fake_find_min()
        elif self.type_hiding_security:
            # Important: Fixed order of operations
            self._fake_insert()
            self._fake_extract_min()
            return self._find_min()
        else:
            return self._find_min()

    def _insert(self, value: _secret, priority: _secret) -> None:
        # TODO
        # O(log n)
        # Insert entry into root bucket
        # Evict
        # UpdateMin
        raise NotImplementedError

    def _extract_min(self) -> _secret:
        # TODO
        # O(log n)
        # Get label of min entry from root bucket
        # Scan path and remove element
        # Evict
        # UpdateMin
        raise NotImplementedError

    def _find_min(self) -> _secret:
        # TODO
        # O(1)
        # Get and return value of min entry from root bucket.
        return self.tree.subtree_mins[0]

    def _fake_insert(self) -> None:
        raise NotImplementedError

    def _fake_extract_min(self) -> None:
        raise NotImplementedError

    def _fake_find_min(self) -> None:
        raise NotImplementedError


class POHToHeapQAdapter(PathObliviousHeap):
    """
    Adapts Path Oblivious Heap to the HeapQ interface,
    allowing plug-and-play replacement in the Dijkstra
    implementation.
    """

    def __init__(self, max_size, *args, **kwargs):
        """Initialize a POH with the required capacity
        and disregard all other parameters.
        """
        super().__init__(max_size)  # TODO: Check parameters

    def update(self, value, priority, for_real=True):
        """Call insert instead of update.
        Warning: When using this adapter, duplicate values are
        allowed to be inserted, and no values are ever updated.
        """
        self.insert(value, priority, fake=not for_real)

    def pop(self, for_real=True):
        """Renaming of pop to extract_min."""
        return self.extract_min(fake=not for_real)
