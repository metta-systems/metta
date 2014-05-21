Metta architecture description
------------------------------

Metta is an exocortex OS: it is your external memory and social interaction agent. This is the primary task aside from the additional multimedia and mobile assistant abilities. As an exocortex, OS's task is to provide means and ends to sharing and gathering relevant information for you.

As an information gathering assistant, the OS must support receiving, filtering, classifying and presenting relevant information in a suitable form.

As an information sharing assistant, the OS must support specifying privacy levels, distributing the secure storage, publishing directly to your peers and to other media.

As an evolving complex construct, the OS must be reflective, self-aware and extensible.

To provide this level of service, the key implementation principles are:

 * It must be possible to distribute work and storage seamlessly between trusted nodes on the user's network.
 * It must be possible to easily add and remove nodes from the trusted network.
 * Information should be synchronized between nodes whenever possible.
 * It should not be possible to bring down the node by overwhelming it with work: QoS guarantees must be agreed upon.
 * Storage should be associative, not hierarchical. Cross-node links must be possible with full semantic relation data.
 * Your data must be redundant, highly available and securely stored.
 * Processing of multimedia and semantically meaningful data must be fast and lean.
 * System must be comprised of components which can be composed and overridden in comprehensive ways.

Principles lead to technical requirements for the OS:

 * QoS management on all levels in the system. Agents must be able to negotiate necessary resources and OS must maintain promised resource guarantees.
 * Highly componentised and sandboxed system. When executing agents on behalf of other users it is crucial to provide data security for your own and for their data.
 * Low system management overhead. Single address space provides ways to avoid expensive data copying and most operations are performed in userspace at maximum possible speed.


General architecture
--------------------

Metta is implemented as a Single Address Space OS, for QoS management it uses technologies found in Nemesis OS, part of Pegasus project from the University of Cambridge.

Implementation consists of kickstart, glue code, trusted computing base components, library components and applications. Applications are vertically integrated - they perform most of the management themselves with high adaptivity and maintain QoS guarantees by removing contention in services usually provided by the OS.

Common functionality is implemented as library components, which give standard tested implementations of many functions needed by applications in Metta environment, and also save memory by sharing code and data between applications.


### Single Address Space ###

Virtual address space in Metta is single, shared between all processes. This means that virtual to physical mapping is equivalent in all domains.

One or more domains may be part of the same protection domain. This means they have the same access rights to the same set of virtual address ranges.

Domain may be part of more than one protection domains. This allows easy sharing by specifying protection domains at the stretch level and by adding domains using the same stretch into the same protection domain.

Implementation detail: on x86, for efficiency there is only one page directory and access rights for the memory frame correspond to global stretch access rights. Once the domain is activated and tries to perform operation not allowed by the global rights, it will fault and the fault handler will then check, if protection domain has more rights actually, in this case the page directory will be updated with new rights and the stretch these frames belong to will be added to a "altered stretches" list for the domain.

Any domain may request a stretch of virtual addresses from a stretch allocator, specifying the desired size and (optionally) a starting address and attributes. Should the request be successful, a new stretch will be created and returned to the caller. The caller is now the owner of the stretch. The starting address and length of the returned stretch may then be queried; these will always be a multiple of the machine's page size. The stretch is a part of protection domain.

Memory protection operations are carried out by the application through the stretch interface. This talks directly to the low-level translation system via simple system calls; it is not necessary to communicate with the system domain. Protection can be carried out in this way due to the protection model chosen which includes explicit rights for "change permissions" operations. A light-weight validation process checks if the caller is authorised to perform an operation.

It is also necessary that the frame which is being used for mapping (or which is being unmapped) is validated. This involves ensuring that the calling domain owns the frame, and that the frame is not currently mapped or nailed. These conditions are checked by using the RamTab, which is a simple enough structure to be used by low-level code.

Protection is carried out at stretch granularity -- every protection domain provides a mapping from the set of valid stretches to a subset of `read, write, execute, meta`. A domain which holds the `meta` right is authorised to modify protections and mappings on the relevant stretch. Domain may grant `read/write/execute` to another domain, if needed.

The accessibility of a stretch is determined by a combination of two things: the permissions for the stretch in the current protection domain and the global permissions of the stretch. The global permissions specify a minimum level of access that all domains share.

The translation system deals with inserting, retrieving or deleting mappings between virtual and physical addresses. As such it may be considered an interface to a table of information held about these mappings; the actual mapping will typically be performed as necessary by whatever memory management hardware or software is present.

The translation system is divided into two parts: a high-level management module, and the low-level trap handlers and system calls. The high-level part is private to the system domain, and handles the following:

 * Bootstrapping the MMU (in hardware or software), and setting up initial mappings.
 * Adding, modifying or deleting ranges of virtual addresses, and performing the associated page table management.
 * Creating and deleting protection domains.
 * Initialising and partly maintaining the RamTab; this is a simple data structure maintaining information about the current use of physical memory frames.

The high-level translation system is used by both the stretch allocator and the frames allocator. The stretch allocator uses it to setup initial entries in the page table for stretches it has created, or to remove such entries when a stretch is destroyed. These entries contain protection information but are by default invalid: i.e. addresses within the range will cause a page fault if accessed.

Placing this functionality within the system domain means that the low-level translation system does not need to be concerned with the allocation of page-table memory. It also allows protection faults, page faults and "unallocated address" faults to be distinguished and dispatched to the faulting application. The frames allocator, on the other hand, uses the RamTab to record the owner and logical frame width of allocated frames of main memory.

Stretch driver is located inside application space, provided by the shared library code or implemented by the application itself. It interfaces with frame allocator to provide backing RAM storage for stretches it manages.

User domain calls `map/unmap` from the stretch driver. Either mapping or unmapping a virtual address "va" requires that the calling domain is executing in a protection domain which holds a meta right for the stretch containing "va". A consequence of this is that it is not possible to map a virtual address which is not part of some stretch.


### Kickstart ###

Kickstart does all pre-initialisation work needed to get system going - it creates a system privileged domain, initialises the MMU and boots processors, then gives control to the boot modules loader.

Module loader will resolve module dependencies, determine load order and load the TCB modules — these are the most important system modules, always trusted by other components.

After initialisation control is transferred to root domain to run primary application.


### Glue code ###

Glue code performs only a minimally necessary subset of operations that require privileged CPU mode. This includes manipulating MMU tables and switching protection domains. This code is therefore not preemptible.

Code exists in glue layer for a number of reasons:
 * code is privileged and therefore needs to execute with supervisor permissions,
 * code is executed because of exception or interrupt and therefore needs to be both privileged and uninterruptible,
 * code is generally used and needs to run atomically/uninterruptible.

Interrupts and exception handlers are also implemented as stubs in glue code, due to their privileged nature.

Some glue code syscalls are privileged and can be used only by members of the TCB, others are used by application processes to request work from other domains.

To construct portable kernel, at least 3 different architectures should be supported by the glue code.

 * x86
 * x86_64
 * arm
 * hosted
 * mips?

> Pistachio has an excellent c++ implementation of the kernel - use it as reference.
(e.g. OSdev/L4/l4ka-pistachio/kernel/src/generic)

Glue code provides "syscalls interface" to other layers.

Available Nemesis syscalls to glue code:

 * privileged:
   * `ntsc_swpipl` Change interrupt priority level.
   * `ntsc_entkern` Enter kernel mode.
   * `ntsc_leavekern` Leave kernel mode.
   * `ntsc_kevent` Send an event from an interrupt stub.
   * `ntsc_rti` Return from an interrupt stub.

Syscalls for metta glue:

 * privileged:
   * ??`sc_activate(vcpu_t*)`  Activate a process

 * unprivileged:
   * `sc_return()` Return from activation, reentrancy reenabled and code resumed from the next instruction after call.
   * `sc_return_resume(context_t*)` Return from activation, resuming passed context.
   * `sc_return_block()` Return from activation and block.
   * `sc_block()` Block awaiting an event.
   * `sc_yield()` Relinquish CPU allocation for this period.
   * `sc_send(int n, uint64_t event)` Send an event.


### Trusted Computing Base ###

TCB components implement features absolutely necessary for application functioning and therefore define the OS kernel.

Kernel components have almost no private data, on which contention could arise. Most of the data for kernel calls is provided by the process engaged in the syscall, therefore not affecting service of other processes. This also helps API atomicity.

Components export functionality through one or more interfaces. Kernel and userspace components are accessed via interfaces alike.

All interfaces are strongly typed, and these types are defined in an interface definition language. It is clearly important, therefore, to start with a good type system, and [Evers93] presents a good discussion of the issues of typing in a systems environment. The type system used in Metta is a hybrid: it includes notions both of the abstract types of interfaces and of concrete data types. It represents a compromise between the conceptual elegance and software engineering benefits of purely abstract type systems such as that used in Emerald [Raj91], and the requirements of efficiency and inter-operability: the goal is to implement an operating system with few restrictions on programming language.

Concrete types are data types whose structure is explicit. They can be predefined (such as booleans, strings, and integers of various sizes) or constructed (as with records, sequences, etc). The space of concrete types also includes typed references to interfaces.

Interfaces are instances of ADTs. Interfaces are rarely static: they can be dynamically created and references to them passed around freely. The type system includes a simple concept of subtyping. An interface type can be a subtype of another ADT, in which case it supports all the operations of the supertype, and an instance of the subtype can be used where an instance of the supertype is required.

The operations supported by interfaces are like procedure calls: they take a number of arguments and normally return a number of results. They can also raise exceptions, which themselves can take arguments.

An [interface definition language][meddle] is used to specify the types, exceptions and methods of an interface, and a run-time typesystem allows the narrowing of types and the marshaling of parameters for non-local procedure invocations.

A namespace scheme (based on Plan-9 contexts) allows implementations of interfaces to be published and a trader component from the TCB may be used to find component(s) exporting a particular interface type or instance.

There are few restrictions on how the name space is structured. The model followed is that of [Saltzer79]: a name is a textual string, a binding is an association of a name with some value, and a context is a collection of bindings. Resolving a name is the process of locating the value bound to it. Name resolution requires that a context be specified. Context can bind names to other contexts, therefore providing a recursive tree-like structure.

Contexts allow apps to search for services. A string like `net.atta.metta` defines a context consisting of three subcontexts.

Requests for finding other interfaces go through security server (implicitly via trader) which decides what applications should see what. It is not uncommon for application to request, e.g. a memory_manager interface while in reality all its memory requests will go through logging_debugging_memory_manager and the application is actually being debugged without knowing about it. This can happen for all application interactions with its environment, which effectively puts every application into a highly controlled sandbox.

A component model is used to design, implement, deploy and reconfigure systems and applications. The component model has the following important features:

 * recursivity: components can be nested in composite components.
 * reflectivity: components have full introspection and intercession capabilities.
 * component sharing: a given component instance can be included (or shared) by more than one component. This is useful to model shared resources such as memory manager or device drivers for instance.
 * binding components: a single abstraction for components connections that is called bindings. Bindings can embed any communication semantics from synchronous method calls to remote procedure calls via marshalling.
 * execution model independence: no execution model is imposed. In that, components can be run within other execution models than the classical thread-based model such as event-based models and so on.
 * open: extra-functional services associated to a component can be customized through the notion of a control membrane.
 * access restrictions apply at bind time (fine grained interfaces).

This component model is derived from [Fractal middleware][fractal].

  [meddle]: https://github.com/berkus/metta/blob/develop/src/tools/meddler/grammar.txt
  [fractal]: http://fractal.ow2.org


### Library Components ###

Library components define the base substrate upon which the whole applications are built. Library components are real components and they export typed interfaces just like any other component does. Most library components are colocated into the same protection domain as the application using them.

Dynamic loader (Sjofn), similar to OMOS server, is used to perform component relocation and linking. Employed memory and loading models allow to share code and static data between all domains efficiently.

Meta-objects (in OMOS sense) are used to create generator interfaces which instantiate modules, used by application.

Runtime type system is a basis for reflectivity support and allows components to introspect their runtime environment, add new runtime types and resolve arising conflicts.

Together with Sjofn, type system allows service-oriented component combination and relatively easy component adaptation, similar to Scala language.


### Applications ###

Components are pieces of code and static data (which has no mutable state). (Clemens Szypersky defines a software component as a unit of composition with contractually specified interfaces and explicit context dependencies only.)

Applications are built from interconnected components.

Applications consist of standard and custom components, which provide most of the functionality and main driver code. Applications service themselves - i.e. they service their own page faults or CPU scheduling decisions, often using standard components that provide necessary functionality already.

After startup application receives it's own domain, protection domain, initially not shared with any other domains a set of pervasives, among them a naming context which it can use to find other necessary components, a virtual CPU interface which it can use to make scheduling decisions (and also a scheduling domain, that represents this VCPU), a stretch and heap interfaces which it can use to allocate memory.

Applications run in domains. The kernel's view of a domain is limited to a single data structure called the Domain Control Block. Kernel uses DCBs to schedule domains' CPU time and other resources.


### Interfaces ###

Interfaces are defined in an IDL language called Meddle, a compiler named meddler will generate proper stubs from these IDL declarations. Interface compiler is based on LLVM framework together with the rest of JIT system.


### VCPU ###

Virtual CPU interface allows domains to receive notifications when they are granted or revoked a CPU time slice or when a page fault or some other exception occurs.

VCPUs indeed perform inheritance scheduling - activation handler usually calls a scheduler function, which chooses next thread to run, by giving it the rest of CPU tick.


### IDC ###

The only default mean of inter-domain communication is Event. Sending an event is a relatively lightweight operation, upon which many other syncronization and communication primitives may be built.

A networked IDC is also possible, given that component interfaces are generated from the IDL files, and therefore can provide marshalling and unmarshalling information for instantiating component interface stubs.


High-level architecture
-----------------------

High-level functionality is provided by standard components and applications comprising Metta backbone.

### Exocortex ###

Exocortex is your external memory and your world representation. As user application, exocortex is a coordinator of agent swarm, doing tasks on your behalf in all reaches of the internets. You can instruct agents to aggregate information from other sources into your exocortex and to publish your information to other media.
This swarm implements centralised-decentralised model — whatever you want to publish is generated centrally by you, in your favourite application, in your home environment, then agents can publish, transfer, upload, and share your data across the net to blogs, video services, additional shadow storage etc. In the same manner, whatever agents find out, can be brought to one of your devices and then seamlessly synced between all your devices forming the exocortex.
This way you do not have to go for the news, the news come to you.
At the same time, things you create are bound to you centrally, as they are emitted from your exocortex and maintain your identity integrity — once you update something, you do not have to go out and update that bit everywhere, it is synced much like p2p networks share data.

..To be continued..
