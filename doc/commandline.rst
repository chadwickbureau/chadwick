.. _cwtools.commandline:

Command-line options
=====================

All six Chadwick tools share a common set of command-line options
controlling their behavior. These are detailed in the following
table. Options which are not available for every tool are noted in
their descriptions; see the documentation for the individual tool for
its full set of options.

.. list-table:: Common command-line options and their effects
   :header-rows: 1
   :widths: 10,40

   * - Switch
     - Description
   * - ``-a``
     - Generate ASCII comma-delimited files (default). This option
       does not affect :program:`cwbox`.
   * - ``-d``
     - Print a list of the available fields and descriptions (for use
       with ``-f``). Not available for :program:`cwbox`.
   * - ``-D dir``
     - Directory in which to find team and roster files (``TEAMyyyy``
       and ``aaayyyy.ROS``), instead of the current directory.
   * - ``-e mmdd``
     - The latest date to process (inclusive)
   * - ``-f flist``
     - List of fields to output. The default list can be viewed with
       ``-h``; the list of available fields can be viewed with ``-d``.
       Not available for :program:`cwbox`.
   * - ``-ft``
     - Generate FORTRAN format files. This option does not affect
       :program:`cwbox`.
   * - ``-h``
     - Prints description and usage information for the tool.
   * - ``-i *gameid*``
     - Only process the game with ID ``gameid``
   * - ``-n``
     - If in ASCII mode (the default), the first row of the output is
       a comma-separated list of column headers. Not available for
       :program:`cwbox`.
   * - ``-Q``
     - Operate quietly; do not print progress messages.
   * - ``-s mmdd``
     - The earliest date to process (inclusive)
   * - ``-y``
     - Specifies the year to use (four digits)
