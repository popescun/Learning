# C++ design patterns for low-latency applications including high-frequency trading
Authors: Paul Bilokon, Burak Gunduz
___
## 2.3 Design patterns

Design patterns and programming strategies will be used interchangeably throughout this work. As mentioned earlier there are many ways to optimise C++ code to achieve lower latencies. The strategies that will
be examined in this work are as follows:
* `Cache Warming`: To minimize memory access time and boost program responsiveness, data is
preloaded into the CPU cache before it’s needed [50].
* `Compile-time Dispatch`: Through techniques like template specialization or function overloading,
optimised code paths are chosen at compile time based on type or value, avoiding runtime dispatch
and early optimisation decisions.
* `Constexpr`: Computations marked as constexpr are evaluated at compile time, enabling constant
folding and efficient code execution by eliminating runtime calculations [46].
* `Loop Unrolling`: Loop statements are expanded during compilation to reduce loop control overhead
and improve performance, especially for small loops with a known iteration count.
* `Short-circuiting`: Logical expressions cease evaluation when the final result is determined, reducing
unnecessary computations and potentially improving performance.
* `Signed vs Unsigned Comparisons`: Ensuring consistent signedness in comparisons avoids conversion-
related performance issues and maintains efficient code execution.
* `Avoid Mixing Float and Doubles`: Consistent use of float or double types in calculations prevents
implicit type conversions, potential loss of precision, and slower execution.
* `Branch Prediction/Reduction`: Accurate prediction of conditional branch outcomes allows specula-
tive code execution, reducing branch misprediction penalties and improving performance.
* `Slowpath Removal`: Optimisation technique aiming to minimize execution of rarely executed code
paths, enhancing overall performance.
* `SIMD`: Single Instruction, Multiple Data (SIMD) allows a single instruction to operate on multiple
data points simultaneously, significantly accelerating vector and matrix computations.
* `Prefetching`: Explicitly loading data into cache before it is needed can help in reducing data fetch
delays, particularly in memory-bound applications.
* `Lock-free Programming`: Utilises atomic operations to achieve concurrency without the use of locks,
thereby eliminating the overhead and potential deadlocks associated with lock-based synchroniza-
tion.
* `Inlining`: Incorporates the body of a function at each point the function is called, reducing function
call overhead and enabling further optimisation by the compiler.
___
## 2.4 LMAX Disruptor