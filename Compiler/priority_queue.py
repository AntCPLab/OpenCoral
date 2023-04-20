"""This module contains a rough implementation of the Path Oblivious Heap'
oblivious priority queue as proposed by 
`Shi <https://eprint.iacr.org/2019/274.pdf>`.
"""

from abc import ABC, abstractmethod
from enum import Enum
from typing import Generic, Tuple, TypeVar
from Compiler import library as lib, oram, util
from Compiler.program import Program
from Compiler.types import sint, Array, _secret
from Compiler.dijkstra import HeapEntry

# If enabled, high-level debugging messages are printed at runtime.
DEBUG = True

T = TypeVar("T")


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


class BinaryBucketTree:
    """Binary Bucket Tree Data Structure
    using RAM as underlying data structure.
    """

    def __init__(
        self,
        capacity: int,
        security: int,
        bucket_oram: oram.AbstractORAM,
        bucket_size: int,
    ):
        # Initialize basic class fields
        self.height = util.log2(capacity)
        self.security = security
        self.bucket_oram = bucket_oram
        self.bucket_size = bucket_size

        # Initialize underlying RAM
        self.ram = oram.RAM(capacity, (sint,), lambda x, y: None)


class POHVariant(Enum):
    """Constants representing Path and Circuit variants."""

    PATH = 0
    CIRCUIT = 1


class PathObliviousHeap(AbstractMinPriorityQueue[_secret]):
    """A basic Path Oblivious Heap implementation."""

    def __init__(
        self,
        capacity: int,
        security: int = 0,  # TODO: Figure out how to parametrize
        int_type: _secret = sint,
        variant: POHVariant = POHVariant.PATH,
        bucket_oram: oram.AbstractORAM = oram.TrivialORAM,
        bucket_size: int = 2,
        type_hiding_security: bool = False,
    ):
        if int_type != sint:
            raise lib.CompilerError("POH: Only sint is supported as int_type.")

        if variant is not POHVariant.PATH:
            raise lib.CompilerError(
                "POH: Only the PATH variant is supported."
            )

        if DEBUG:
            lib.print_ln(
                "POH: Initializing a queue with a capacity of %s and security parameter %s.",
                capacity,
                security,
            )
            lib.print_ln(
                f"POH: Type hiding security is {'en' if type_hiding_security else 'dis'}abled.",
            )

        # Initialize basic class fields
        self.capacity = capacity
        self.security = security
        self.int_type = int_type
        self.variant = variant
        self.type_hiding_security = type_hiding_security

        # Initialize data structure with dummy elements
        self.tree = BinaryBucketTree(
            capacity, security, bucket_oram, bucket_size
        )

    def insert(self, value: _secret, priority: _secret, is_fake: bool = False) -> None:
        # TODO
        # O(log n)
        # Insert entry into root bucket
        # Evict
        raise NotImplementedError

    def find_min(self) -> _secret:
        # TODO
        # O(1)
        # Get and return value of min entry from root bucket.
        raise NotImplementedError

    def extract_min(self) -> _secret:
        # TODO
        # O(log n)
        # Get label of min entry from root bucket
        # Scan path and remove element
        # Evict
        raise NotImplementedError


class POHToHeapQAdapter(PathObliviousHeap):
    """
    Adapts Path Oblivious Heap to the HeapQ interface,
    allowing plug-and-play replacement in the Dijkstra
    implementation.
    """

    def __init__(self, max_size, oram_type=None, init_rounds=None, int_type=None):
        """Initialize a POH with the required capacity
        and disregard all other parameters.
        """
        super().__init__(max_size)

    def update(self, value, priority, for_real=True):
        """Call insert instead of update.
        Warning: When using this adapter, duplicate values are
        allowed to be inserted, and no values are ever updated.
        """
        self.insert(value, priority, is_fake=not for_real)

    def pop(self, for_real=True):
        """Renaming of pop to extract_min."""
        return self.extract_min(is_fake=not for_real)
