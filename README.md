
# ZAMAI actor model challenge


## Overview

Use an Actor Framework with threads pinned on CPU cores.


## Build instructions


## What alternatives I would have considered with more time

* Chris Kohlhoff's [executors](https://github.com/executors/executors), have been postponed from C++20
* Mohamed B's [qb](https://github.com/isndev/qb), which is faster & more modern than Simplx

* balancing CPU core load... but since data hopping cores means cash flushes/resyncs, it's a hard problem with computational nodes whose cost can't be quatified ahead of time


## Misc Notes

IDag's virtual interface *may* have a cost, but I won't fiddle with it until I get profiler reports.

