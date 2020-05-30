
# ZAMAI actor model challenge


## Overview

I use an Actor Framework: threads are ideally pinned on CPU cores (with said core masked out from the kernel config), i.e. one core = one thread.


## Build instructions


## What alternatives I would have considered with more time

* Chris Kohlhoff's [executors](https://github.com/executors/executors), which have been postponed to after C++ 20 (Kohlhoff wrote ASIO)
* Mohamed B's [qb](https://github.com/isndev/qb), which is faster & more modern than Simplx

* balancing CPU core load... but since hopping cores (with a significan payload) means cash flushes/resyncs, it's a tough problem if nodes' computational cost can't be quatified ahead of time


## Misc Notes

* the randomized DAG generation is simplified so downstream nodes are only DAG descendents. It'd be possible to generate upstream DAG nodes while preveting cycles but it seems beyond the scope of this project
* IDag's virtual interface no doubt has a runtime cost, but I generally don't speculate about performance without profiler reports at hand

