
# ZAMAI actor model challenge


## Overview

I use the [simplex](https://github.com/kluete/simplex) [Actor Model](https://en.wikipedia.org/wiki/Actor_model) framework: threads are ideally pinned on CPU cores (with said core masked out from the kernel config), i.e. one core = one thread.


# Rationale

Having more threads than CPU cores (without accounting for hyperthreading) means more context switches... which can become very expensive depending on the amount of data that has to be flushed/reloaded from CPU caches.

There is a cost to running an event loop on each thread, to be sure, but cache flushes are way more costly. Note that, in an Actor Model, involved CPU cores always run at 100% -- even when there is NO event to be processed, so CPU usage (say with htop) is not a relevant performance indicator. The correct way to benchmark such a system would be to count the amount of computations / time it can perform (after disabling all unnecessary I/O, of course).


# Caveats

A (freely) randomized DAG will generate unpredicatable and unbalanced CPU loads across the different CPU cores, especially wrt to node fanning and merging. A efficient design would seek to control such a graph so as to balance CPU core load, as well as to minimize cross-core hopping (requiring L1/L2 cache flushes).


# To do

* conductor service
* unique_ptr::get() ugliness
* install log functionality
  * prevent cout chopping
* how to wait for all results?
* build instruction will be pain in the ass?


## Build instructions


## What alternatives I would have considered with more time

* Chris Kohlhoff's [executors](https://github.com/executors/executors), whcse integration into ISO C++ has been postponed to after C++ 20. Kohlhoff wrote ASIO, so he knows his stuff.
* Mohamed B's [qb](https://github.com/isndev/qb), his actor framework is faster & more modern than Simplx but he's lacking some features like computer clustering.

* balancing CPU core load... but since hopping cores (with a significan payload) means cash flushes/resyncs, it's a tough problem if nodes' computational cost can't be quatified ahead of time


## Misc Notes

* the randomized DAG generation is simplified so downstream nodes are only DAG descendents. It'd be possible to generate upstream DAG nodes while preveting cycles but it seems beyond the scope of this project
* IDag's virtual interface no doubt has a runtime cost, but I generally don't speculate about performance without profiler reports at hand

