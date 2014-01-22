User part of the kernel objects is used to manipulate and access them. You pass a user-level object address to
syscall to have kernel initialize and fill up this structure.

fluke_thread_create is not used, fluke_thread_create_hash is in form of thread_t constructor.

thread_t* thread = new(UTCB_ADDRESS) thread_t(UTCB_ADDRESS, &hash);

thread_t::thread_t(address_t utcb_va, hash_t* hash)
{
    thread_create_syscall(utcb_va, hash);
}


We allow nesters to create sub-threads and sub-tasks, hence the thread_create_syscall does not specify address space for the new thread:
it's derived from caller's address space (nesters are allowed to create only immediate children).
To create a sub-task nester should call task_create_syscall.


1-1 thread mapping - but it will be possible for userspace code to schedule it's own threads using the same mechanisms?
Each thread has KTCB and UTCB areas: kernel- and userspace-accessible parts.

A User Thread Control Block is defined for every unique thread on the system, known shortly as the UTCB of the thread. This block of memory can be used as thread-local storage as well as a buffer for inter-thread communication.

UTCB memory regions are predefined in the virtual memory address space, but the task of allocating UTCBs to threads is left to the pagers as this is thought to add unnecessary policy to the microkernel.


Newly started thread is awaiting for IPC message containing IP and SP to start execution from.

Thread has a pager - master resource allocator for thread (@sa Parent interface?).
Threads have pager (allocation) and scheduler (time-slice management). Thread can give back unused cpu time to threads
for which it is a scheduler and return it back to its scheduler.

exchange_registers_syscall allows switching between threads or modifying a thread.


Virtual Registers are per-thread state structures giving easy access to thread state and allowing safe IPC.

Message Registers - could be used like BeOS' BMessage structured messages for high-level communications support.



L4 pager == L4 space id == nester == protection domain
L4 scheduler == parent thread (it gives it's children the timeslices, usually the same as nester?)
L4 task = nester + threads running in it's space

Things to do with a thread: create, destroy, schedule and run, preempt or yield.

{{from fluke:
fluke_thread_create
fluke_thread_create_hash
fluke_thread_destroy [cannot destroy itself]
fluke_thread_disable_exceptions [can only disable for itself]
fluke_thread_enable_exceptions [can only enable for itself]
fluke_thread_get_client [preliminary iface][itself]
fluke_thread_get_handlers [itself]
fluke_thread_get_saved_state [itself]
fluke_thread_get_server [preliminary iface][itself]
fluke_thread_get_state [cannot call on itself]
fluke_thread_interrupt [cannot call on itself]
fluke_thread_move [cannot call on itself]
fluke_thread_reference
fluke_thread_return_from_exception [itself]
fluke_thread_self [itself]
fluke_thread_set_client [preliminary iface][itself]
fluke_thread_set_handlers [itself]
fluke_thread_set_saved_state [itself]
fluke_thread_set_server [preliminary iface][itself]
fluke_thread_set_state [cannot call on itself]
fluke_thread_schedule  [cannot call on itself] // donate cpu to another thread
}}


thread_create(newTid, nester)
.. 
.. newly created thread blocks in IPC waiting for message containing IP and SP to start execution from.



thread_destroy(tid)
.. can be carried out only by thread's nester? or a privileged thread
..


1-1 thread mapping - but it will be possible for userspace code to schedule it's own threads using the same mechanisms?
Each thread has KTCB and UTCB areas: kernel- and userspace-accessible parts.
Thread may crawl through multiple protection domains maintaining a call stack.

Threads have pager (allocation) and scheduler (time-slice management). Thread can give back unused cpu time to threads
for which it is a scheduler and return it back to its scheduler.

exchange_registers_syscall allows switching between threads or modifying a thread.
Thread has a pager - master resource allocator for thread (@sa Parent interface?)
Virtual Registers are per-thread state structures giving easy access to thread state and allowing safe IPC.

A User Thread Control Block is defined for every unique thread on the system, known shortly as the UTCB of the thread. This block of memory can be used as thread-local storage as well as a buffer for inter-thread communication.

UTCB memory regions are predefined in the virtual memory address space, but the task of allocating UTCBs to threads is left to the pagers as this is thought to add unnecessary policy to the microkernel.



== Threads ==

  * `kern__thread_create` creates a thread in nester address space, inits thread stack to point to in-kernel thread booter. Activation is not created until first thread's upcall. Thread is added to runnable queue.

  * when scheduler donates cpu time to newly created thread, a new activation is created and thread boot upcalls into target space's thread entrypoint.

  * whenever thread returns from the entrypoint, it is about to exit – all resources are cleaned up, activations terminated and thread destroys itself. (same as calling `kern__thread_destroy`)


For modern OS a for-free thread distributability is crucial (@sa Alpha kernel).


=== Threads / Activations [migrating threads model] ===

Migrating threads implementation. Threads start living in kernel and make excursions to user tasks and above.
Mach style - thread/activation duality with similar semantics to Mach's.

Separation between "kernel" and "glue code".

Tasks make "bindings" which declares which interfaces and calls they can perform. Security checks are performed when binding. Token revocation can happen during runtime as well at interface granularity level (?).

atomic-osdi99:

scheduler activations ![2]

Quicksilver ![17] and Nonstop ![3] are transactional operating systems; in both of these systems, the kernel provides primitives for maintaining transactional semantics in a distributed system. In this way, transactional semantics are provided for high-level services such as ﬁle operations even though the basic kernel operations on which they are built may not be.

![31][[BR]]
![15] - flukeref?

=== IPC ===
Atomic "stateless" IPC - no continuations.
Purely atomic kernel API [Fluke, atomic-osdi99].

both process- and interrupt-models are possible for testing thanks to fluke architecture.
(process-model improves preemptibility, interrupt-model improves memory usage)

process-fully-preemptible model is good for low-latency desktop.
interrupt-partially-preemptible shall be fine for embedded/memory constrained devices and appliances.

""For example, Mach’s average per-thread kernel memory overhead was reduced by 85% when the kernel was changed to use a partial interrupt model [10, 11].""

=== Memory ===

sigma0 allocator, L4 style  + userspace suballocators and pagers on top, Fluke style.

Процесс в Mach - это, в первую очередь, адресное пространство и набор нитей, которые выполняются в этом адресном пространстве. Таким образом, выполнение связано с нитями, а процессы являются пассивными объектами и служат для сбора всех ресурсов, использующихся группой взаимосвязанных нитей, в удобные контейнеры.

process (task) - called "space" in L4

== Passive vs Active object model ==
'''[passive.pdf]'''

Passive objects - entities which do not have their own threads and client threads migrate to these objects to do useful work. (@sa thread-migrate)

""A passive object model provides enhanced performance and simplicity because it is more closely matched to the basic nature of microprocessors and the requirements of applications. It also provides more functionality by making the flow of control between objects a first-class abstraction which can be examined, manipulated, and used to carry information about the operation in progress.""

Presented benefits:

 1. Passive objects more naturally support synchronous object invocation, which is the common case.
 1. Passive objects more closely model the nature of the underlying hardware.
 1. The implementation of inter-object control transfer is simpler and faster with passive objects.
 1. The explicit nature of inter-object control transfer makes more optimized implementations possible.
 1. Passive objects can be smaller and more lightweight, because they involve less storage overhead.
 1. Passive objects more accurately model the requirements of real-time systems.
 1. Passive objects make accurate resource accounting easier.
 1. Interruption of operations in progress is more easily implemented with passive objects.
 1. Passive objects are easier to implement and manage in user code.
 1. In a passive model, it is easier for personality servers to control the execution environment of their subjects.

=== Q&A ===
Q: "A passive model provides less protection, because clients must trust servers with their threads."[[BR]]
A: Migration of a thread to a server object does not
necessarily grant the server any rights other than the
right to temporarily execute in its scheduling context.
With proper design, even this right can be revoked or
transferred back into the client object in a way that
fully maintains the protection of both the server and
the client. Our work on supporting migrating threads
on Mach ![7] demonstrates how this can be done.

Q: "It is more difficult to program in a passive model, because all objects must handle internal synchronization issues."[[BR]]
A: '''It is true that in an active object model''', a simple object can be created requiring no internal synchronization by creating just one thread in the object. However, in a full microkernel implementation of passive objects, it often turns out to be extremely easy to achieve the same effect, such as by creating only one "activation record" on which incoming threads can run, or by maintaining a global lock acquired and released automatically on entry and exit from the object.

![7] Bryan Ford and Jay Lepreau. Evolving Mach 3.0 to use migrating threads. Technical Report UUCS-93-022, University of Utah, August 1993. A portion of this paper will appear in Proc. of the Winter 1994 USENIX Conference. '''[thread-migrate.pdf]'''

![1] Brian N. Bershad, Thomas E. Anderson, Edward D. Lazowska, and Henry M. Levy. Lightweight remote procedure call. ACM Transactions on Computer Systems, 8(1):37-55, February 1990. (contains general-purpose migrating threads model) '''[lrpc.pdf]'''


'''[thread-migrate.pdf]'''

The key element of our design is decoupling of the thread abstraction into the execution context and the schedulable thread of control, consisting of a chain of contexts. A key element of our implementation is that threads are now "based" in the kernel, and temporarily make excursions into tasks via upcalls.

 * Thread - a sequential flow of control.
 * Single process - single kernel-provided thread (traditional Unix).
 * Single task - multiple kernel-provided threads (Mach).
 * User threads - provided by user-level libraries on top of kernel threads, by manipulation of PC and stack from user-space.

In many systems, a thread includes much more than the flow of control. In Mach, a thread also:
 * is a schedulable entity, with priority and scheduling policy attributes.
 * contains resource accounting statistics (e.g. accumulated CPU time).
 * contains execution context of a computation - state of registers, PC, SP, references to containing task and designated exception handler.
 * provides the point of thread control, visible to user programs through a thread control port.

During migrating threads RPC:
 * only partial context switch (PD and subset of registers, like stack pointer),
 * scheduler is never involved.
 * there's no server thread state (registers, stack) to restore at all.

Note that binding in this model can be very similar to that in thread switching (static) model [Section 6.2] <-#DFN

Changes to Mach model:

Thread decoupled into:
 * logical flow of control, represented by stack of activations in tasks; (per-space UTCB)
 * schedulable entity, with priority and resource accounting. (in-kernel KTCB)

Activation represents:
 * execution context of a computation, including a task whose code it is executing, its exception handler, PC, registers and SP
 * user-visible point of control.

Activation is now exported to user-code as "thread".[[BR]]
This is necessary to provide controllability and protection at the same time. "Thread" migrates between tasks and enters and leaves the kernel. "Activation", a user-mode part, remains permanently fixed to a particular task. Arbitrary control is permitted only on a specific activation, but not the thread as a whole.

Clouds ![16]

"Scheduler activations"![1] - kernel threads with special support for user-level scheduling (primarily concerned with behavior of kernel threads within a PD, orthogonal to this paper). Theoretically could be combined in the same system.

In an object-based environment, invocation of relatively fine-grained objects is prohibitively inefficient if all objects must be active.

'''A distinction should be made between the "kernel" and the "glue code".''' The kernel is conceptually a PD much like a user-level task, in which threads can execute, wait, migrate in and out, and so on; its primary distinction is that it is specially privileged and provides basic system control services. Glue code is the low-level, highly system-dependent code that enacts the transitions between all protection domains, both user and kernel.
This distinction between kernel and glue code is often overlooked because both types of code usually execute in supervisor mode and are often linked together in a single binary image. However, this does not necessarily have to be the case; in QNX the 7K nucleus consists of essentially nothing but glue code, while the "kernel proper" is placed in a specially privileged but otherwise ordinary process.
In the presence of migrating threads the distinction between them becomes extremely important.

Termination mechanism used (splicing up activations at the point of termination and letting the upper half continue work, while the original thread returns to the previous activation) required careful planning of kernel data structures and locking mechanisms, in particular, the line between the kernel and glue code had to be defined precisely. Once worked out, this technique not only added additional controllability, but considerably simplified the implementation of control mechanisms in the kernel.

temporary mapping ![23]

thread migrations to other nodes researched in depth in Alpha ![11]

![11] Raymond K. Clark, E. Douglas Jensen, and Franklin D. Reynolds. An architectural overview of the Alpha real-time distributed kernel. In Proc. of the USENIX Workshop on Micro-kernels and Other Kernel Architectures, pages 127-146, Seattle, WA, April 1992. '''[!AlphaRtDistributedKernel
