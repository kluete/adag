
# ZAMAI DAG challenge using Actor Model


## Overview

For this challenge I use the [simplex framework](https://github.com/kluete/simplex), which is an [Actor Model](https://en.wikipedia.org/wiki/Actor_model): each thread should be pinned on a CPU core (with said cores masked out from the Linux kernel config), i.e. one core = one thread. Hyper-threading should be disabled as it just burdens L1/L2 caches and is only beneficial if a thread is stuck, say on an I/O operation.

Each thread runs an event loop whose templated event handlers have minimal overhead. All those threads communicate through a common (software) memory bus making efficent use of the L3 cache.


## Rationale

Having more threads than CPU cores means more context switches... which can become very expensive depending on the amount of data that has to be flushed/reloaded from CPU caches.

While there is a (small) CPU cost to running an event loop on each thread, it is negligible compared to the significant cost of cache flushes/reloads.

As an Actor Model thread is never interrupted (on a properly-configered system) and its data only shared via a safe memory bus, their application code is effectively running in *single-threaded* mode. That is, th

Note that in an Actor Model, involved CPU cores always run at 100% -- even when there is NO event to be processed, so CPU usage (measured by htop, say) is not a relevant performance indicator. The correct way to benchmark such a system would be to count the amount of computations / time it can perform (after disabling all unnecessary I/O, of course).

There is almost no cost for instantiated actors that aren't actually used, i.e. that aren't processing events, except for their memory footprint... which the OS will page out at a one-off cost.


## Test Case

A randomized DAG with:

* total nodes = 200
* entry points/CPU cores = 4
* random bucket size = 5 nodes
* random path fanning-out & remerging

Results:

* total path terminations = 5948900
* computing (linear/single-threaded) exit points = 26 secs
* execution of its multithreaded compute paths = 1351 millisecs


## On DAG Randomization

A randomized DAG will generate unpredictable and unbalanced CPU loads across the different CPU cores, especially wrt to *node fanning and merging*. An efficient design would massage DAG topography so as to balance hardware processing resources (whether CPU core, GPU or grid computing node), as well as to minimize cross-core hopping (which triggers cache flushes).

DAG branches starts at (E) *entry/root nodes*, needle through (B) *branch nodes* and terminate at (X) *exit nodes*. Said values depend on the degree of node fanning/merging or more generally the DAG's topography.

In this implementation, a branch's nodes are randomly selected from within an advancing *bucket* of child nodes.

If the DAG's random buckets are too tightly packed -- say one of the entry nodes points to another entry node, there can be exponential "waves" (going both up and down) of complexity that'll create many DAG "termination points" and make threaded execution very unpredictable. This is usually due to RANDOM_BUCKET_SIZE being too small.

On the other hand, if the DAG's random buckets are too sparse -- say the # of DAG nodes is too high and/or the random bucket factor is too high -- the DAG won't exhibit any node convergence/fanning so won't really behave like a DAG but just a bunch of linked lists. This is easily identified if ROOT_NODES == reported exit nodes.

Once the DAG is generated, but before it is executed, this program must first do a *synchronous*  (single-threaded and therfore slow) traversal of the DAG to count the total number of path terminations, so that the *asynchronous* (multithreaded) execution of the DAG knows when to stop. The time of taken by the synchronous execution is considered a one-off and thus not part of the benchmarking.

The randomized DAG generation is simplified so downstream nodes are only DAG descendants. It'd be possible to generate upstream DAG nodes while preventing cycles but it seems beyond the scope of this assignment.

I use C++11 PRNGs instead of */dev/udev/* randomization, so that each run is deterministic.


## To Do

* cap compute ops?
* threaded log functionality
  * show node compute parallelism
  * async-aware stdout without chopping/scrambling
* fix unique_ptr::get() uglines?


## Build instructions

* compiles with gcc 8.3 in C++14

```
git submodule update --init

./build.sh
```


## What Worked

* preventing DAG loops
* spreading out DAG nodes with random children picked from a fixed bucket size
* wiring the Actor Model logic
* a DAG of 200 nodes, on 4 cores, with a random bucket size of 5 nodes, took 168 secs to complete. Note that a DAG with N x more nodes and an N x larger random bucket size would behave approxmately the same.


## What Didn't Work

* preventing a DAG edge from being traversed more than once. With both fanning and merging, it is possible for a path (A) to split into two paths (B1 and B2), each traversing whatever unrelated nodes for a little while and then remerging into the same node (C). Whatever happens downstream of that remerged node (C) will be executed twice. On a large-enough DAG (with a million nodes, say), these "reconvergences" can happen multiple times -- leading to *exponential* complexity and CPU cost. Exponential isn't the same as infinite but can sure feel the same on a human scale!
* this [spreadsheet](https://github.com/kluete/zamai/blob/master/timings.xlsx) shows the complexity growth
* liming the length of a path's node number. With fanning, paths can effectively get concatenated. Only the DAG's max nodes is an effective cap.


## What I Wrote Off (and didn't try)

* on the fly core-hopping whenever a path fanned out. With large enough payloads, this would have resulted in major slowdowns due to cache flushes -- both on fanning out and remerging. Either way, idle CPU cores are mostly valuable in energy-sensitive environments like smartphones, not in back-end data-intensive calculations.


## What Was Tricky

* in general, randomizing a DAG that allows for fanning and merging, on multiple cores, with no structure or constraints on its topology, feels a lot like *asking for trouble* because both # of cores and total # of DAG nodes increase the probablility of paths fanning out
* finding out that DAG entropy/homogeneity depends on the random bucket size because it affects probabilities of edge re-entries. I.e a 1M-node DAG would perform faster than a 100K-node DAG if the random bucket size was large enough to have less edge duplications
* even with a (deterministic) PRNG, different distributions affect DAG complexity, which is why -- counterintuitively -- on a single core, a 400-node DAG is traversed faster than 500-node DAG. Conversely, a much larger DAG with a larger RANDOM_BUCKET_SIZE may perform much faster.


## What Alternatives I'd Have Considered If Given More Time

* Chris Kohlhoff's [executors](https://github.com/executors/executors), whose integration into ISO C++ has been postponed to after C++20... but Kohlhoff wrote ASIO, so he knows his stuff.
* my colleague Mohamed B's [qb](https://github.com/isndev/qb), his actor framework is faster & more modern than Simplex but it's still lacking some features like cross-computer/cluster messaging.
* entirely forgo trying to predicting the # of exit nodes, instead embedding a countdown of maximum # of nodes to travel
* generating different node/actor class types to account for different computations (with different CPU costs)
* balancing CPU core load... but since hopping cores (with a significant payload) means cash flushes/resyncs, it's a tough problem if nodes' computational cost can't be quantified ahead of time
* output to graphviz format for visual debugging




## Misc Notes

* IDag's virtual interface no doubt has a runtime cost, but I generally don't speculate about performance without profiler reports at hand



(c) copyright 2020 by Peter Laufenberg

