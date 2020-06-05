
# ZAMAI DAG challenge using Actor Model


## Overview

For this challenge I use the [simplex framework](https://github.com/kluete/simplex), which is an [Actor Model](https://en.wikipedia.org/wiki/Actor_model): each thread should be pinned on a CPU core (with said cores masked out from the Linux kernel config), i.e. one core = one thread. Hyper-threading should be disabled as it just burdens L1/L2 caches and is only beneficial if a thread is stuck, say on an I/O operation.

Each thread runs an event loop whose templated event handlers have minimal overhead. All those threads communicate through a common (software) memory bus making efficent use of the L3 cache.


## Rationale

Having more threads than CPU cores means more context switches... which can become very expensive depending on the amount of data that has to be flushed/reloaded from CPU caches.

While there is a (small) CPU cost to running an event loop on each thread, it is negligible compared to the significant cost of cache flushes/reloads.

As an Actor Model thread is never interrupted (on a properly-configered system) and its data only shared via a safe memory bus, their application code is effectively running in *single-threaded* mode. That is, th

Note that in an Actor Model, involved CPU cores always run at 100% -- even when there is NO event to be processed, so CPU usage (measured by htop, say) is not a relevant performance indicator. The correct way to benchmark such a system would be to count the amount of computations / time it can perform (after disabling all unnecessary I/O, of course).

There is almost no cost for instantiated actors that aren't actually used, i.e. that aren't processing events, except for their memory layout... which the OS will page out at a one-off cost.


## On DAG Randomization

A randomized DAG will generate unpredictable and unbalanced CPU loads across the different CPU cores, especially wrt to *node fanning and merging*. An efficient design would massage DAG topography so as to balance hardware processing resources (whether CPU core, GPU or grid computing node), as well as to minimize cross-core hopping (which triggers cache flushes).

DAG branches starts at (E) *entry/root nodes*, needle through (B) *branch nodes* and terminate at (X) *exit nodes*. Said values depend on the degree of node fanning/merging or more generally the DAG's topography.

In this implementation, a branch's nodes are randomly selected from within an advancing *bucket* of child nodes.

If the DAG's random buckets are too tightly packed -- say one of the entry nodes points to another entry node, there can be exponential "waves" (going both up and down) of complexity that'll create many DAG "termination points" and make threaded execution very unpredictable.

On the other hand, if the DAG's random buckets are too sparse -- say the # of DAG nodes is too high and/or the random bucket factor is too high -- the DAG won't exhibit any node convergence/fanning so won't really behave like a DAG but just a bunch of linked lists. This is easily identified if ROOT_NODES == reported exit nodes.

Once the DAG is generated, but before it is executed, this program must first do a *synchronous*  (single-threaded and therfore slow) traversal of the DAG to count the total number of path terminations, so that the *asynchronous* (multithreaded) execution of the DAG knows when to stop.

The randomized DAG generation is simplified so downstream nodes are only DAG descendants. It'd be possible to generate upstream DAG nodes while preventing cycles but it seems beyond the scope of this assignment.

I use C++11 PRNGs instead of */dev/udev/* randomization, so that each run is deterministic.


## To Do

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

* IDag's virtual interface no doubt has a runtime cost, but I generally don't speculate about performance without profiler reports at hand


(c) copyright 2020 by Peter Laufenberg

