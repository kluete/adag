
# ZAMAI actor model challenge


## Overview

I use the [simplex](https://github.com/kluete/simplex) framework, which is an [Actor Model](https://en.wikipedia.org/wiki/Actor_model): threads are ideally pinned on CPU cores (with said cores masked out from the kernel config), i.e. one core = one thread. Hyper-threading should be *disabled* as it just increases cache flushes and is only beneficial if a thread is stuck, say on an I/O operation.

Each thread runs an event loop and they all communicate through a common L3 cache memory bus.


# Rationale

Having more threads than CPU cores means more context switches... which can become very expensive depending on the amount of data that has to be flushed/reloaded from CPU caches.

While there is a cost to running an event loop on each thread, cache flushes are way more costly. Note that in an Actor Model, involved CPU cores always run at 100% -- even when there is NO event to be processed, so CPU usage (say with htop) is not a relevant performance indicator. The correct way to benchmark such a system would be to count the amount of computations / time it can perform (after disabling all unnecessary I/O, of course).

There is almost no cost for instantiated actors that aren't used (not processing any event), except very little memory.


# Caveats

I use C++'s PRNGs in lieux of /dev/udev/ randomization, so a run is deterministic.

A randomized DAG will generate unpredictable and unbalanced CPU loads across the different CPU cores, especially wrt to node fanning and merging. An efficient design would seek to control such a graph so as to balance load per CPU core, as well as to minimize cross-core hopping (which trigger L1/L2 cache flushes).

If the random buckets are too tight -- say one of the root nodes (a DAG entry node) points -- to another root node, there can be exponential "waves" (going both up and down) of complexity that'll create many DAG "termination points" and make threaded execution very unpredictable.

If the random buckets are too sparse -- say the # of DAG nodes is too high and/or the random bucket factor is too high -- the DAG won't exhibit any node convergence/fanning so won't really look/behave like a DAG but just a bunch of linked lists. This can be identified if ROOT_NODES == reported exit nodes.

Once the DAG is generated, but before it is executed, my program must first do a *synchronous* traversal of the DAG to count the total number of path terminations, so that the *asynchronous* execution of the DAG (on multiple CPU cores) knows when to stop.


# To Do

* threaded log functionality
  * template to build thread-local sprintf string
  * async-aware stdout without chopping/scrambling
* fix unique_ptr::get() ugliness
* build instruction


## Build instructions


## What alternatives I would have considered if given more time

* Chris Kohlhoff's [executors](https://github.com/executors/executors), whose integration into ISO C++ has been postponed to after C++ 20. Kohlhoff wrote ASIO, so he knows his stuff.
* Mohamed B's [qb](https://github.com/isndev/qb), his actor framework is faster & more modern than Simplx but he's lacking some features like computer clustering.

* balancing CPU core load... but since hopping cores (with a significant payload) means cash flushes/resyncs, it's a tough problem if nodes' computational cost can't be quantified ahead of time
* generating different node/actor class types to account for different computations


## Misc Notes

* the randomized DAG generation is simplified so downstream nodes are only DAG descendants. It'd be possible to generate upstream DAG nodes while preventing cycles but it seems beyond the scope of this project
* IDag's virtual interface no doubt has a runtime cost, but I generally don't speculate about performance without profiler reports at hand


copyright Peter Laufenberg 2020

