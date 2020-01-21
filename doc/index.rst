Welcome to MP-SPDZ's documentation!
===================================

This documentation provides a reference to the most important
high-level functionality provided by the MP-SPDZ compiler. For a
tutorial and documentation on how to compile and run programs, the
implemented protocols etc. see https://github.com/data61/MP-SPDZ.

Compilation process
-------------------

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
