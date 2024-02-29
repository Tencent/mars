boost.context
=============

boost.context is a foundational library that provides a sort of cooperative multitasking on a single thread.
By providing an abstraction of the current execution state in the current thread, including the stack (with 
local variables) and stack pointer, all registers and CPU flags, and the instruction pointer, a execution_context 
instance represents a specific point in the application's execution path. This is useful for building 
higher-level abstractions, like coroutines, cooperative threads (userland threads) or an equivalent to 
C# keyword yield in C++.

A fiber provides the means to suspend the current execution path and to transfer execution control, 
thereby permitting another fiber to run on the current thread. This state full transfer mechanism 
enables a fiber to suspend execution from within nested functions and, later, to resume from where it 
was suspended. While the execution path represented by a fiber only runs on a single thread, it can be 
migrated to another thread at any given time.

A context switch between threads requires system calls (involving the OS kernel), which can cost more than 
thousand CPU cycles on x86 CPUs. By contrast, transferring control among fibers requires only fewer than 
hundred CPU cycles because it does not involve system calls as it is done within a single thread.

boost.context requires C++11! 
