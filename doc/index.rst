Welcome to MP-SPDZ's documentation!
===================================

This documentation provides a reference to the most important
high-level functionality provided by the MP-SPDZ compiler. For a
tutorial and documentation on how to run programs, the
implemented protocols etc. see https://github.com/data61/MP-SPDZ.

Compilation process
-------------------

After putting your code in ``Program/Source/<progname>.mpc``, run the
compiler from the root directory as follows::

  ./compile.py [options] <progname> [args]

The arguments ``<progname> [args]`` are accessible as list under
``program.args`` within ``progname.mpc``, with ``<progname>`` as
``program.args[0]``.

The following options influence the computation domain:

.. cmdoption:: -F <integer length>
	       --field=<integer length>

   Compile for computation modulo a prime and the default integer
   length. This means that secret integers are assumed to have at most
   said length unless explicitely told otherwise. The compiled output
   will communicate the minimum length of the prime number to the
   virtual machine, which will fail if this is not met. This is the
   default with an *integer length* set to 64.

.. cmdoption:: -R <ring size>
	       --ring=<ring size>

   Compile for computation modulo 2^(*ring size*). This will set the
   assumed length of secret integers to one less because many
   operations require this. The exact ring size will be communicated
   to the virtual machine, which will fail if it is run for another
   length.

.. cmdoption:: -B <integer length>
	       --binary=<integer length>

   Compile for binary computation using *integer length* as default.

For arithmetic computation (``-F`` and ``-R``) you can set the bit
length during execution using ``program.set_bit_length(length)``. For
binary computation you can do so with ``sint =
sbitint.get_type(length)``.

The following option switches from a single computation domain to
mixed computation when using in conjunction with arithmetic
computation:

.. cmdoption:: -X
	       --mixed

   Enables mixed computation using daBits.

.. cmdoption:: -Y
	       --edabit

   Enables mixed computation using edaBits.

The implementation of both daBits and edaBits are explained in this paper_.

.. _paper: https://eprint.iacr.org/2020/338

The following options change less fundamental aspects of the
computation:

.. cmdoption:: -D
	       --dead-code-elimination

   Eliminates unused code. This currently means computation that isn't
   used for input or output or written to the so-called memory (e.g.,
   :py:class:`Compiler.types.Array`; see :py:mod:`Compiler.types`).

.. cmdoption:: -b <budget>
	       --budget=<budget>

   Set the budget for loop unrolling with
   :py:func:`Compiler.library.for_range_opt` and similar. This means
   that loops are unrolled up to *budget* instructions. Default is
   100,000 instructions.

.. cmdoption:: -C
	       --CISC

   Speed up the compilation of repetitive code at the expense of a
   potentially higher number of communication rounds. For example, the
   compiler by default will try to compute a division and a logarithm
   in parallel if possible. Using this option complex operations such
   as these will be seperated and only multiple divisions or
   logarithms will be computed in parallel. This speeds up the
   compilation because of reduced complexity.

Compilation vs run time
~~~~~~~~~~~~~~~~~~~~~~~

The most important thing to keep in mind is that the Python code is
executed at compile-time. This means that Python data structures such
as :py:class:`list` and :py:class:`dict` only exist at compile-time
and that all Python loops are unrolled. For run-time loops and lists,
you can use :py:func:`Compiler.library.for_range` (or the more
optimizing :py:func:`Compiler.library.for_range_opt`) and
:py:class:`Compiler.types.Array`. For convenient multithreading you
can use :py:func:`Compiler.library.for_range_opt_multithread`, which
automatically distributes the computation on the requested number of
threads.

This reference uses the term 'compile-time' to indicate Python types
(which are inherently known when compiling). If the term 'public' is
used, this means both compile-time values as well as public run-time
types such as :py:class:`Compiler.types.regint`.

.. toctree::
   :maxdepth: 4
   :caption: Contents:

   Compiler


Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
