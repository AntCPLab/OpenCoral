"""This module contains a basic implementation of the Path Oblivious Heap'
oblivious priority queue as proposed by 
`Shi <https://eprint.iacr.org/2019/274.pdf>`.
"""

from abc import ABC, abstractmethod
from enum import Enum
from typing import Generic, List, Tuple, TypedDict, TypeVar

from Compiler import library as lib, oram, util
from Compiler.circuit_oram import CircuitORAM
from Compiler.dijkstra import HeapEntry
from Compiler.path_oram import PathORAM
from Compiler.types import cint, regint, _secret, sint


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

# Types
T = TypeVar("T")  # TODO: Should be more restrictive
SubtreeMinEntry = TypedDict(
    "SubtreeMinEntry",
    {"empty": _secret, "prio": _secret, "value": _secret, "leaf": _secret},
)


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

    def __init__(self, init_rounds=-1):
        self.subtree_min_entry_size = self.entry_size + (
            self.D + 1,
        )  # (prio, value, leaf)

        empty_min_entry = self._get_empty_entry()

        # Maintain subtree-mins in a separate RAM
        # (some of the attributes we access are defined in the ORAM classes,
        # so no meta information is available when accessed in this constructor.)
        self.subtree_mins = oram.RAM(
            2 ** (self.D + 1) + 1,  # +1 to make space for stash min (index -1)
            empty_min_entry.types(),
            self.get_array,
        )
        if init_rounds != -1:
            lib.stop_timer()
            lib.start_timer(1)
        self.subtree_mins.init_mem(empty_min_entry)
        if init_rounds != -1:
            lib.stop_timer(1)
            lib.start_timer()

    def update_min(self, leaf_label: int) -> None:
        """Update subtree_min entries on the path from the specified leaf
        to the root bucket (and stash).
        """
        indices = self._get_reversed_min_indices_and_children_on_path_to(leaf_label)

        # Degenerate case (leaf): no children to consider if we are at a leaf.
        # However, we must remember to set the leaf label of the entry.
        leaf_ram_index = indices[0][0]
        leaf_min = self._get_bucket_min(leaf_ram_index, leaf_label)
        self._set_subtree_min(leaf_min, leaf_ram_index)

        # Iterate through internal path nodes and root
        for c, l, r in indices[1:]:
            current = self._get_bucket_min(c, leaf_label)
            left, right = map(self.get_subtree_min, [l, r])

            # TODO: Is the following oblivious?

            # Compare pairs
            cmp_l_c = self._compare_entries(left, current)
            cmp_c_r = self._compare_entries(current, right)
            cmp_l_r = self._compare_entries(left, right)

            # Only one of the three has the highest priority
            c_min = cmp_l_c * (1 - cmp_c_r)
            l_min = (1 - cmp_l_c) * (1 - cmp_l_r)
            r_min = cmp_c_r * cmp_l_r

            # entry = min(current, left, right)
            entry = dict()
            for key in current.keys():
                entry[key] = (
                    c_min * current[key] + l_min * left[key] + r_min * right[key]
                )
            self._set_subtree_min(entry, c)

        # Degenerate case (stash): the only child of stash is the root
        # so only compare those two
        stash_min = self._get_stash_min(leaf_label)
        root_min = self.get_subtree_min(0)

        cmp_s_r = self._compare_entries(stash_min, root_min)

        # entry = min(stash_min, root_min)
        entry = dict()
        for key in stash_min.keys():
            entry[key] = (1 - cmp_s_r) * stash_min[key] + cmp_s_r * root_min[key]
        self._set_subtree_min(entry)

    def _get_empty_entry(self) -> oram.Entry:
        return oram.Entry.get_empty(self.value_type, self.subtree_min_entry_size)

    def _compare_entries(
        self, first: SubtreeMinEntry, second: SubtreeMinEntry
    ) -> _secret:
        """Entries are never equal unless they are empty.
        First, compare on emptyness.
        Next, compare on priority.
        Finally, tie break on value.
        Returns 0 if first has highest priority,
        and 1 if second has highest priority.
        """
        # TODO: Tie break is probably not secure if there are duplicates.
        # Can be fixed with unique ids

        one_empty = first["empty"].bit_xor(second["empty"])
        empty_cmp = first["empty"].greater_than(second["empty"])
        prio_equal = first["prio"].equal(second["prio"])
        prio_cmp = first["prio"].greater_than(second["prio"])
        value_cmp = first["value"].greater_equal(second["value"])
        return one_empty.if_else(
            empty_cmp,
            prio_equal.if_else(
                value_cmp,
                prio_cmp,
            ),
        )

    def get_subtree_min(self, index: int = -1) -> SubtreeMinEntry:
        """Returns a dictionary representing the subtree-min
        of the bucket with the specified index. If index is not specified,
        it returns the subtree-min of the stash (index -1),
        which is the subtree-min of the complete tree."""
        entry = iter(self.subtree_mins[index])
        return self._entry_to_dict(entry)

    def _set_subtree_min(self, entry: SubtreeMinEntry, index: int = -1) -> None:
        self.subtree_mins[index] = self._dict_to_entry(entry)

    def _entry_to_dict(self, entry: oram.Entry) -> SubtreeMinEntry:
        d = dict()
        entry = iter(entry)
        d["empty"] = next(entry)
        next(entry)  # Disregard index
        d["prio"] = next(entry)
        d["value"] = next(entry)
        d["leaf"] = next(entry)
        return d

    def _dict_to_entry(self, d: SubtreeMinEntry) -> oram.Entry:
        return oram.Entry(
            0,  # index isn't used
            (d["prio"], d["value"], d["leaf"]),
            empty=d["empty"],
            value_type=self.value_type,
        )

    def _get_bucket_min(self, index: int, leaf_label: int) -> SubtreeMinEntry:
        return self._get_ram_min(
            self.buckets, index, index + self.bucket_size, leaf_label
        )

    def _get_stash_min(self, leaf_label: int) -> SubtreeMinEntry:
        # TODO: Is this secure? We touch every entry so probably.
        return self._get_ram_min(self.stash.ram, 0, self.stash.size, leaf_label)

    def _get_ram_min(
        self, ram: oram.RAM, start: int, end: int, leaf_label: int
    ) -> SubtreeMinEntry:
        """Scan through bucket, finding the entry with highest priority."""
        leaf_label = self.value_type(leaf_label)
        start = start * self.bucket_size
        res = self._entry_to_dict(ram[start])
        res["leaf"] = self.value_type(leaf_label)
        if self.bucket_size == 1:
            return res
        end = start + self.bucket_size
        for i in range(start + 1, end):
            entry = self._entry_to_dict(ram[i])
            cmp_res_entry = self._compare_entries(res, entry)
            for key in entry.keys() - {"leaf"}:
                res[key] = (1 - cmp_res_entry) * res[key] + cmp_res_entry * entry[key]
        return res

    def _get_reversed_min_indices_and_children_on_path_to(
        self, leaf_label: int
    ) -> List[Tuple[int, int, int]]:
        """Returns a list from leaf to root of tuples of (index, left_child, right_child).
        Used for update_min.
        """
        indices = [(0, 1, 2)]
        index = 0
        for _ in range(self.D):
            index = 2 * index + 1 + (leaf_label & 1)
            leaf_label >>= 1
            indices += [(index,) + self._get_child_indices(index)]
        return list(reversed(indices))

    def _get_child_indices(self, i) -> Tuple[int, int]:
        """This is how a binary tree works."""
        return 2 * i + 1, 2 * i + 2


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

        BasicMinTree.__init__(self, init_rounds)


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

    :ivar type_hiding_security: A boolean indicating whether
        type hiding security is enabled. Enabling this
        makes the cost of every operation equal to the
        sum of the costs of all operations. This is initially
        set by passing an argument to the class constructor.
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
        """
        Initializes a Path Oblivious Heap priority queue.

        :param capacity: The max capacity of the queue.
        :param security: A security parameter, used for determining the stash size
            in order to make the error probability negligible in this parameter.
            Defaults to be equal to the capacity.
        :param type_hiding_security: (Currently not supported) True if the types of
            executed operations should be oblivious, False otherwise. Defaults to False.
        :param int_type: The data type of the queue, used for both key and value.
            Defaults to `sint`.
        :param entry_size: A tuple containing an integer per entry value that specifies
            the bit length of that value. Defaults to `(32, util.log2(capacity))`.
        :param variant: A `POHVariant` enum class member specifying the variant (either
            `PATH` or `CIRCUIT`). Defaults to `PATH`.
        :param bucket_oram: The ORAM used in every bucket. Defaults to `oram.TrivialORAM`.
        :param bucket_size: The size of every bucket. Defaults to
            `variant.get_default_bucket_size()`.
        :param stash_size: The size of the stash. Defaults to the squared base 2 logarithm
            of the security parameter.
        :param init_rounds: If not equal to -1, initialization is timed in isolation.
            Defaults to -1.
        """
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

        # TODO: How to do superlogarithmic?
        # TODO: Experiment with constant stash size as in Path ORAM
        if stash_size is None:
            stash_size = util.log2(security) ** 2

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
        # Return -1 if empty
        entry = self.tree.get_subtree_min()
        return entry["empty"].if_else(sint(-1), entry["value"])

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
