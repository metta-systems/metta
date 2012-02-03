//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2011, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "default_console.h"
#include "any.h"
#include "macros.h"
#include "c++ctors.h"
#include "root_domain.h"
#include "bootinfo.h"
#include "new.h"
#include "elf_parser.h"
#include "debugger.h"
#include "module_loader.h"
#include "infopage.h"
#include "frames_module_v1_interface.h"
#include "mmu_v1_interface.h"
#include "mmu_module_v1_interface.h"
#include "mmu_module_v1_impl.h" // for debug
#include "heap_v1_interface.h"
#include "heap_module_v1_interface.h"
#include "pervasives_v1_interface.h"
#include "system_frame_allocator_v1_interface.h"
#include "system_stretch_allocator_v1_interface.h"
#include "stretch_driver_module_v1_interface.h"
#include "stretch_table_module_v1_interface.h"
#include "stretch_table_v1_interface.h"
#include "stretch_allocator_module_v1_interface.h"
#include "map_card64_address_v1_interface.h"
#include "map_card64_address_factory_v1_interface.h"
#include "map_string_address_factory_v1_interface.h"
#include "type_system_factory_v1_interface.h"
#include "type_system_f_v1_interface.h"
#include "nemesis/exception_system_v1_interface.h"
#include "exceptions.h"

// bootimage contains modules and namespaces
// each module has an associated namespace which defines some module attributes/parameters.
// startup module from which root_domain starts also has a namespace called "default_namespace"
// it defines general system attributes and startup configuration.

static pervasives_v1::rec pervasives;

//======================================================================================================================
// Look up in root_domain's namespace and load a module by given name, satisfying its dependencies, if possible.
//======================================================================================================================

/// tagged_t namesp.find(string key)

// with module_loader_t loading any modules twice is safe, and if we track module dependencies then only what is needed
// will be loaded.
static void* load_module(bootimage_t& bootimg, const char* module_name, const char* clos)
{
    bootimage_t::modinfo_t addr = bootimg.find_module(module_name);
    if (!addr.start)
        return 0;

    kconsole << " + Found module " << module_name << " at address " << addr.start << " of size " << addr.size << endl;

    bootinfo_t* bi = new(bootinfo_t::ADDRESS) bootinfo_t;
    elf_parser_t loader(addr.start);
    return bi->get_module_loader().load_module(module_name, loader, clos);
    /* FIXME: Skip dependencies for now */
}

template <class closure_type>
static inline closure_type* load_module(bootimage_t& bootimg, const char* module_name, const char* clos)
{
    return static_cast<closure_type*>(load_module(bootimg, module_name, clos));
}

//======================================================================================================================
// setup MMU and frame allocator
//======================================================================================================================

static protection_domain_v1::id create_address_space(system_frame_allocator_v1::closure_t* frames, mmu_v1::closure_t* mmu)
{
    auto pdom = mmu->create_domain();

    memory_v1::physmem_desc null_pmem; // FIXME: we pass pmems by value in the interface atm... it's not even used!

    // First we need to map the PIP globally read-only.
    auto str = PVS(stretch_allocator)->create_over(PAGE_SIZE,
            stretch_v1::rights(stretch_v1::right_read).add(stretch_v1::right_global),
            information_page_t::ADDRESS, memory_v1::attrs_regular, PAGE_WIDTH, null_pmem);

    /* Map stretches over the boot image */

    // map nucleus glue code

    // map over loaded modules
    // FIXME: 
    // should use module_loader module map and map with appropriate rights for text (RX), rodata(R), and bss (RW)...
                // TRC_MEM(eprintf("MOD:  T=%06lx:%06lx\n",
                //                      mod->addr, mod->size));
                // str = StretchAllocatorF$NewOver(sallocF, mod->size, AXS_GE,
                //                              (addr_t)mod->addr,
                //                              0, PAGE_WIDTH, NULL);
                // ASSERT_ADDRESS(str, mod->addr);
#if 0
    /* Intialise the pdom map to zero */
    for (map_index=0; map_index < MAP_SIZE; map_index++) {
        addr_map[map_index].address= NULL;
        addr_map[map_index].str    = (Stretch_cl *)NULL;
    }
    map_index= 0;

    /* Map stretches over the boot image */
	  case nexus_namespace:
	    name = nexus_ptr.nu_name++;
	    nexus_ptr.generic = (char *)nexus_ptr.generic +
		name->nmods * sizeof(addr_t);
	    TRC_MEM(eprintf("NAME: N=%06lx:%06lx\n",
			    name->naddr, name->nsize));
	    str = StretchAllocatorF$NewOver(sallocF, name->nsize,
					    AXS_GR, (addr_t)name->naddr,
					    0, PAGE_WIDTH, NULL);
	    ASSERT_ADDRESS(str, name->naddr);
	    break;

	  case nexus_program:
	    prog= nexus_ptr.nu_prog++;
	    TRC_MEM(eprintf("PROG: T=%06lx:%06lx D=%06lx:%06lx "
			    "B=%06lx:%06lx  \"%s\"\n",
			    prog->taddr, prog->tsize,
			    prog->daddr, prog->dsize,
			    prog->baddr, prog->bsize,
			    prog->program_name));

	    str = StretchAllocatorF$NewOver(sallocF, prog->tsize,
					    AXS_NONE, (addr_t)prog->taddr,
					    0, PAGE_WIDTH, NULL);
	    ASSERT_ADDRESS(str, prog->taddr);

	    /* Keep record of the stretch for later mapping into pdom */
	    addr_map[map_index].address= (addr_t)prog->taddr;
	    addr_map[map_index++].str  = str;

	    if (prog->dsize + prog->bsize) {
		str = StretchAllocatorF$NewOver(
		    sallocF, ROUNDUP((prog->dsize+prog->bsize), FRAME_WIDTH),
		    AXS_NONE, (addr_t)prog->daddr, 0, PAGE_WIDTH, NULL);
		ASSERT_ADDRESS(str, prog->daddr);
		/* Keep record of the stretch for later mapping into pdom */
		addr_map[map_index].address= (addr_t)prog->daddr;
		addr_map[map_index++].str  = str;
	    }

	    break;

	case nexus_blob:
	    blob = nexus_ptr.nu_blob++;
	    TRC_MEM(eprintf("BLOB: B=%06lx:%06lx\n",
			    blob->base, blob->len));

	    /* slap a stretch over it */
	    str = StretchAllocatorF$NewOver(sallocF, blob->len,
					    AXS_GR, (addr_t)blob->base,
					    0, PAGE_WIDTH, NULL);
	    ASSERT_ADDRESS(str, blob->base);
	    break;

#endif
    return pdom;
}

/*!
 * At startup we created a physical heap; while this is fine, the idea
 * of protection is closely tied to that of stretches. Hence this function
 * maps a stretch over the existing heap.
 * This allows us to map it read/write for us, and read-only to everyone else.
 */
static void map_initial_heap(heap_module_v1::closure_t* heap_mod, heap_v1::closure_t* heap, size_t initial_heap_size, protection_domain_v1::id root_domain_pdid)
{
    kconsole << "Mapping stretch over heap: " << int(initial_heap_size) << " bytes at " << heap << endl;
    memory_v1::physmem_desc null_pmem; // FIXME: we pass pmems by value in the interface atm... it's not even used!

    auto str = PVS(stretch_allocator)->create_over(initial_heap_size, stretch_v1::rights(stretch_v1::right_read), memory_v1::address(heap), memory_v1::attrs_regular, PAGE_WIDTH, null_pmem);

    auto real_heap = heap_mod->realize(heap, str);

    if (real_heap != heap)
    {
        kconsole << WARNING << __FUNCTION__ << ": realize changed heap address from " << heap << " to " << real_heap << endl;
    }

    // Map our heap as local read/write
    str->set_rights(root_domain_pdid, stretch_v1::rights(stretch_v1::right_read).add(stretch_v1::right_write));
}

static void init_mem(bootimage_t& bootimg)
{
    kconsole << " + init_mem" << endl;
    bootinfo_t* bi = new(bootinfo_t::ADDRESS) bootinfo_t;

    // For address space randomization we should load modules as we go, for simplicity we load them all here.
    auto frames_factory = load_module<frames_module_v1::closure_t>(bootimg, "frames_mod", "exported_frames_module_rootdom");
    ASSERT(frames_factory);

    auto mmu_factory = load_module<mmu_module_v1::closure_t>(bootimg, "mmu_mod", "exported_mmu_module_rootdom");
    ASSERT(mmu_factory); // mmu_factory

    auto heap_factory = load_module<heap_module_v1::closure_t>(bootimg, "heap_mod", "exported_heap_module_rootdom");
    ASSERT(heap_factory);

    auto stretch_allocator_mod = load_module<stretch_allocator_module_v1::closure_t>(bootimg, "stretch_allocator_mod", "exported_stretch_allocator_module_rootdom"); //stretch_allocator_factory
    ASSERT(stretch_allocator_mod);

    auto stretch_table_mod = load_module<stretch_table_module_v1::closure_t>(bootimg, "stretch_table_mod", "exported_stretch_table_module_rootdom"); // stretch_table_factory
    ASSERT(stretch_table_mod);

    auto stretch_driver_mod = load_module<stretch_driver_module_v1::closure_t>(bootimg, "stretch_driver_mod", "exported_stretch_driver_module_rootdom"); // stretch_driver_factory
    ASSERT(stretch_driver_mod);

    // === WORKAROUND ===
    // To avoid overwriting memory with further loaded modules, preload them here.
    // This will go away after a proper module loader mod is implemented.
    load_module<exception_system_v1::closure_t>(bootimg, "exceptions_mod", "exported_exception_system_rootdom");
    load_module<map_card64_address_factory_v1::closure_t>(bootimg, "hashtables_mod", "exported_map_card64_address_factory_rootdom");
    load_module<type_system_factory_v1::closure_t>(bootimg, "typesystem_mod", "exported_type_system_factory_rootdom");
    //load_module<context_module_v1::closure_t>(bootimg, "context_mod", "exported_context_module_rootdom");
    // === END WORKAROUND ===

    size_t modules_size;
    address_t modules_base = bi->used_modules_memory(&modules_size);
    bi->use_memory(modules_base, modules_size); // TODO: use_memory as we load the modules, so no need for used_modules_memory()

// FIXME: point of initial reservation is so that MMU_mod would configure enough pagetables to accomodate initial v2p mappings!
    // request necessary space for frames allocator
    int required = frames_factory->required_size();
    int initial_heap_size = 128*KiB;

    ramtab_v1::closure_t* rtab;
    memory_v1::address next_free;

    kconsole << " + Init memory region size " << int(required + initial_heap_size) << " bytes." << endl;
    auto mmu = mmu_factory->create(required + initial_heap_size, &rtab, &next_free);

    kconsole << " + Obtained ramtab closure @ " << rtab << ", next free " << next_free << endl;

    kconsole << " + Creating frame allocator" << endl;
    auto frames = frames_factory->create(rtab, next_free);

    kconsole << " + Creating heap" << endl;
    auto heap = heap_factory->create_raw(next_free + required, initial_heap_size);
    PVS(heap) = heap;

    frames_factory->finish_init(frames, heap);

#if HEAP_DEBUG
    kconsole << " + Heap alloc test:";
    for (size_t counter = 0; counter < 127*KiB; ++counter)
    {
        address_t p = heap->allocate(counter);
        if (!p)
        {
            kconsole << "Allocation of " << counter << " bytes failed! Dumping heap state:" << endl;
            heap->check(true);
        }
        heap->free(p);
    }
    heap->check(true);
    kconsole << " done." << endl;
#endif

    kconsole << " + Creating system stretch allocator" << endl;
    auto system_stretch_allocator = stretch_allocator_mod->create(heap, mmu);
    PVS(stretch_allocator) = system_stretch_allocator;

    /*
     * We create a 'special' stretch allocator which produces stretches
     * for page tables, protection domains, DCBs, and so forth.
     * What 'special' means will vary from architecture to architecture,
     * but it will typcially imply at least that the stretches will
     * be backed by phyiscal memory on creation.
     */
    kconsole << " + Creating nailed stretch allocator" << endl;
    auto sysalloc = system_stretch_allocator->create_nailed(reinterpret_cast<frame_allocator_v1::closure_t*>(frames), heap);//yikes!

    mmu_factory->finish_init(mmu, reinterpret_cast<frame_allocator_v1::closure_t*>(frames), heap, sysalloc); //yikes again!

    kconsole << " + Creating stretch table" << endl;
    auto strtab = stretch_table_mod->create(heap);

    kconsole << " + Creating null stretch driver" << endl;
    PVS(stretch_driver) = stretch_driver_mod->create_null(heap, strtab);

    // Create the initial address space; returns a pdom for root domain.
    kconsole << " + Creating initial address space." << endl;
    auto root_domain_pdid = create_address_space(frames, mmu);
    map_initial_heap(heap_factory, heap, initial_heap_size, root_domain_pdid);
}

static void init_type_system(bootimage_t& bootimg)
{
    //TODO: type system, meta-interface

    /* Get an Exception System */
    kconsole << " + Bringing up exceptions" << endl;
    auto xcp_factory = load_module<exception_system_v1::closure_t>(bootimg, "exceptions_mod", "exported_exception_system_rootdom");
    ASSERT(xcp_factory);
	PVS(exceptions) = xcp_factory->create();
    ASSERT(PVS(exceptions));
    // Exceptions are used by further modules, which make extensive use of heap and its exceptions.

    // Check exception handling via too big heap allocation (easiest)
    kconsole << "__ Testing exceptions" << endl;
    OS_TRY {
        auto res = PVS(heap)->allocate(128*1024*1024);
        ASSERT(res);
    }
    // OS_CATCH("heap_v1.no_memory") {
    OS_CATCH_ALL {
        kconsole << "__ Handled heap_v1.no_memory exception, yippie!" << endl;
    }
    OS_ENDTRY

    kconsole <<  " + Bringing up type system" << endl;
    kconsole <<  " +-- getting safe_card64table_mod..." << endl;
    auto lctmod = load_module<map_card64_address_factory_v1::closure_t>(bootimg, "hashtables_mod", "exported_map_card64_address_factory_rootdom");
    ASSERT(lctmod);
    kconsole <<  " +-- getting stringtable_mod..." << endl;
    auto strmod = load_module<map_string_address_factory_v1::closure_t>(bootimg, "hashtables_mod", "exported_map_string_address_factory_rootdom");
    ASSERT(strmod);
    kconsole <<  " +-- getting typesystem_mod..." << endl;
    auto ts_factory = load_module<type_system_factory_v1::closure_t>(bootimg, "typesystem_mod", "exported_type_system_factory_rootdom");
    ASSERT(ts_factory);
    kconsole <<  " +-- creating a new type system..." << endl;
    auto lct = lctmod->create(PVS(heap));
    ASSERT(lct);
    auto str = strmod->create(PVS(heap));
    ASSERT(str);

    // lct test
    // for (int i = 0; i < 1000; ++i)
    // {
    //     if (!lct->put(i, i))
    //         kconsole << "Putting " << i << " into lct failed!" << endl;
    // }

    auto ts = ts_factory->create(PVS(heap), lct, str);
    ASSERT(ts);
    kconsole <<  " +-- done: ts is at " << ts << endl;
    PVS(types) = ts;

    /* Preload any types in the boot image */
    {
        type_system_f_v1::interface_info* info;

        kconsole << " +++ registering interfaces" << endl;
        info = (type_system_f_v1::interface_info*)0;//lookup("Types");
        while(*info) {
            ts->register_interface(*info);
            info++;
        }
    }
}

static void init_namespaces(bootimage_t& bootimg)
{
    //TODO: module namespaces
    //TODO: context
    //TODO: IDC stubs

    /* Build initial name space */
    kconsole <<  " + Building initial name space: ";

    /* Build root context */
    kconsole <<  "Root, ";
#if 0
    auto context_factory = load_module<context_module_v1::closure_t>(bootimg, "context_mod", "exported_context_module_rootdom");
    ASSERT(context_factory);
	auto root = context_factory->create_context(heap, PVS(types));
    ASSERT(root);
    PVS(root)  = root;

    kconsole <<  "modules, ";
    {
        auto mods = context_factory->create_context(heap, PVS(types));
        DECLARE_ANY(mods_any, context_v1_closure, mods);
        char *cur_name;
        addr_t cur_val;

        root->add("modules", &mods_any);

        // TODO: make namespace iterator, init named namespace from bootimg and run over all of its entries.

        set_namespace(nexusprimal->namespc, "modules");//set_namespace affects what lookup_next will return below...
        while((cur_name=lookup_next(&cur_val))!=NULL)
        {
            D(kconsole << endl << "adding " << cur_val << " with name " << cur_name);
            mods->add(cur_name, cur_val);
        }
        D(kconsole << endl);
    }

    kconsole <<  "blob, ";
    {
        auto blobs = context_factory->create_context(heap, PVS(types));
        DECLARE_ANY(blobs_any, context_v1_closure, blobs);
        Type_Any b_any;
        char *cur_name;
        addr_t cur_val;

        root->add("blob", &blobs_any);

        set_namespace(nexusprimal->namespc, "blob");
        while((cur_name=lookup_next(&cur_val))!=NULL)
        {
            D(kconsole << endl << "adding " << cur_val << " with name " << cur_name);
            ANY_INIT(&b_any, RdWrMod_Blob, cur_val);
            blobs->add(cur_name, &b_any);
        }
        D(kconsole << endl);
    }

    kconsole <<  "proc, ";
    {
        auto proc = context_factory->create_context(heap, PVS(types));
        auto domains = context_factory->create_context(heap, PVS(types));
        auto cmdline = context_factory->create_context(heap, PVS(types));
        ANY_DECL(tmpany, Context_clp, proc);
        ANY_DECL(domany, Context_clp, domains);
        ANY_DECL(cmdlineany,Context_clp,cmdline);
        ANY_DECL(kstany, addr_t, kst);
        string_t ident=strduph(k_ident, heap);
        ANY_DECL(identany, string_t, ident);
        Context$Add(root, "proc", &tmpany);
        Context$Add(proc, "domains", &domany);
        Context$Add(proc, "kernel_state", &kstany);
        Context$Add(proc, "cmdline", &cmdlineany);
        Context$Add(proc, "kernel_ident", &identany);
#ifdef INTEL
        parse_cmdline(cmdline, ((kernel_st *)kst)->command_line);
#endif
    }

    kconsole <<  "pervasives, ";
    {
        Type_Any any;
        if (root->get("modules.PvsContext",&any)) {
            root->add("pervasives",&any);
        } else {
            eprintf ("NemesisPrimal: WARNING: >pvs not created\n");
        }
    }

    kconsole <<  "symbols, ";
    {
        auto syms = context_factory->create_context(heap, PVS(types));
        DECLARE_ANY(syms_any, context_v1_closure, syms);
        root->add("symbols", syms_any);
    }

    /* Build system services context */
    kconsole <<  "sys, ";
    {
        auto sys = context_factory->create_context(heap, PVS(types));
        DECLARE_ANY(sys_any, context_v1_closure, sys);
        DECLARE_ANY(salloc_any, stretch_allocator_v1_closure, salloc);
        DECLARE_ANY(sysalloc_any, system_stretch_allocator_v1_closure, sysalloc);
        DECLARE_ANY(ts_any, typesystem_v1_closure, PVS(types));
        DECLARE_ANY(frames_any, frame_v1_closure, framesF);
        DECLARE_ANY(strtab_any, stretch_table_v1_closure, strtab);

        root->add("sys", &sys_any);
        sys->add("stretch_allocator", &salloc_any);
        sys->add("system_allocator", &sysalloc_any);
        sys->add("typesystem", &ts_any);
        sys->add("frames_allocator", &frames_any);
        sys->add("stretch_table", &strtab_any);
    }

    /* IDC stub context */
/*    kconsole <<  "IDC stubs, ";
    {
        Closure_clp stubs_register;
        Context_clp stubs =
        ContextMod$NewContext(ContextMod, heap, PVS(types) );

        // Create the stubs context
        CX_ADD("stubs", stubs, Context_clp);

        set_namespace(nexusprimal->namespc, "primal");

        // Dig up the stubs register closure
        stubs_register = * ((Closure_clp *) lookup("StubsRegisterCl"));
        Closure$Apply(stubs_register);

        TRC_CTX(eprintf("\n"));
    }
*/
    /*
     ** Next is the program information. Need to iterate though them and
     ** get the entry points, values, and namespaces of each of them
     ** and dump 'em into a context.
     */

    /* Boot domains/programs context */
/*    kconsole <<  "progs." << endl;
    {
        Context_clp progs= ContextMod$NewContext(ContextMod, heap, PVS(types));
        ANY_DECL(progs_any,Context_clp,progs);
        ProtectionDomain_ID prog_pdid;
        BootDomain_InfoSeq *boot_seq;
        BootDomain_Info *cur_info;
        nexus_prog *cur_prog;
        char *name;
        Type_Any *any, boot_seq_any, blob_any;

        Context$Add(root,"progs",&progs_any);

        boot_seq = SEQ_NEW(BootDomain_InfoSeq, 0, PVS(heap));
        ANY_INIT(&boot_seq_any, BootDomain_InfoSeq, boot_seq);
*/
        /* Iterate through progs in nexus and add appropriately... */
/*        while((cur_prog= find_next_prog())!=NULL)
        {
            TRC_PRG(eprintf("Found a program, %s,  at %p\n",
                            cur_prog->program_name, cur_prog));

            if(!strcmp("Primal", cur_prog->program_name))
            {
                TRC_PRG(eprintf(
                    "NemesisPrimal: found params for nemesis domain.\n"));
*/
                /*
                 * * Allocate some space for the nemesis info stuff, and then
                 ** add the info in the 'progs' context s.t. we can get at
                 ** it from the Nemesis domain (for Builder$NewThreaded)
                 */
/*                nemesis_info= Heap$Malloc(PVS(heap), sizeof(BootDomain_Info));
                if(!nemesis_info) {
                    eprintf("NemesisPrimal: out of memory. urk. death.\n");
                    ntsc_halt();
                }

                nemesis_info->name       = cur_prog->program_name;
                nemesis_info->stackWords = cur_prog->params.stack;
                nemesis_info->aHeapWords = cur_prog->params.astr;
                nemesis_info->pHeapWords = cur_prog->params.heap;
                nemesis_info->nctxts     = cur_prog->params.nctxts;
                nemesis_info->neps       = cur_prog->params.neps;
                nemesis_info->nframes    = cur_prog->params.nframes;
*/
                /* get the timing parameters from the nbf */
/*                nemesis_info->p = cur_prog->params.p;
                nemesis_info->s = cur_prog->params.s;
                nemesis_info->l = cur_prog->params.l;

                nemesis_info->x = (cur_prog->params.flags & BOOTFLAG_X) ?
                True : False;
                nemesis_info->k = (cur_prog->params.flags & BOOTFLAG_K) ?
                True : False;

                if(!nemesis_info->k)
                    eprintf("WARNING: Nemesis domain has no kernel priv!\n");

                CX_ADD_IN_CX(progs, "Nemesis", nemesis_info, BootDomain_InfoP);

                MapDomain((addr_t)cur_prog->taddr, (addr_t)cur_prog->daddr,
                          nemesis_pdid);
            } else {
                cur_info= Heap$Malloc(heap, sizeof(BootDomain_Info));
*/
                /* Each program has a closure at start of text... */
/*                cur_info->cl= (Closure_cl *)cur_prog->taddr;
*/
                /* Create a new pdom for this program */
/*                prog_pdid      = MMU$NewDomain(mmu);
                cur_info->pdid = prog_pdid;

                MapDomain((addr_t)cur_prog->taddr, (addr_t)cur_prog->daddr,
                          prog_pdid);

                cur_info->name       = cur_prog->program_name;
                cur_info->stackWords = cur_prog->params.stack;
                cur_info->aHeapWords = cur_prog->params.astr;
                cur_info->pHeapWords = cur_prog->params.heap;
                cur_info->nctxts     = cur_prog->params.nctxts;
                cur_info->neps       = cur_prog->params.neps;
                cur_info->nframes    = cur_prog->params.nframes;
*/
                /* get the timing parameters from the nbf */
/*                cur_info->p = cur_prog->params.p;
                cur_info->s = cur_prog->params.s;
                cur_info->l = cur_prog->params.l;

                cur_info->x = (cur_prog->params.flags & BOOTFLAG_X) ?
                True : False;
                cur_info->k = (cur_prog->params.flags & BOOTFLAG_K) ?
                True : False;
  */              /*
                 ** We create a new context to hold the stuff passed in in
                 ** the .nbf as the namespace of this program.
                 ** This will be later be inserted as the first context of
                 ** the merged context the domain calls 'root', and hence
                 ** will override any entries in subsequent contexts.
                 ** This allows the use of custom modules (e.g. a debug heap
                 ** module, a new threads package, etc) for a particular
                 ** domain, without affecting any other domains.
                 */
/*                TRC_PRG(eprintf("Creating program's environment context.\n"));
                cur_info->priv_root = ContextMod$NewContext(ContextMod, heap, PVS(types));

                // XXX what are the other fields of cur_prog->name _for_ ??
                set_namespace((namespace_entry *)cur_prog->name->naddr, NULL);
                while((name=lookup_next((addr_t *)&any))!=(char *)NULL)
                {
                    NOCLOBBER bool_t added= False;

                    // XXX hack!!
                    if (!strncmp(name, "blob>", 5))
                    {
                        ANY_INIT(&blob_any, RdWrMod_Blob, (addr_t)any);
                        //
                        //            TRC_PRG(eprintf("  ++ adding blob '%s':"
                        //                    "base=%x, len= %x\n",
                        //                    name, any->type, any->val));
                         //
                        any = &blob_any;
                    }

                    TRC_PRG(eprintf("  ++ adding '%s': type= %qx, val= %qx\n",
                            name, any->type, any->val));
*/
                    /*
                    ** XXX SMH: the below is messy in order to deal with
                    ** compound names of the form X>Y. If we wanted to deal
                    ** with the more general case (A>B>...>Z), it would be
                    ** even messier.
                    ** Perhaps a 'grow' flag to the Context$Add method would
                    ** be a good move? Wait til after crackle though....
                    ** Also: if want to override something in e.g. >modules>,
                    ** won't work unless copy across rest of modules too.
                    ** If you don't understand why, don't do it.
                    ** If you do understand why, change Context.c
                    */
  /*                  added  = False;
                    TRY {
                        Context$Add(cur_info->priv_root, name, any);
                        added= True;
                    } CATCH_Context$NotFound(UNUSED name) {
                        TRC_PRG(eprintf(" notfound %s (need new cx)\n", name));
                        // do nothing; added is False
                    } CATCH_ALL {
                        TRC_PRG(eprintf("     (caught exception!)\n"));
                        // ff
                    } ENDTRY;

                    if(!added) { // need a subcontext
                        Context_clp new_cx;
                        char *first, *rest;

                        first= strdup(name);
                        rest = strchr(first, SEP);
                        *rest++ = '\0';
                        new_cx= CX_NEW_IN_CX(cur_info->priv_root, first);
                        Context$Add(new_cx, rest, any);
                    }
                }

                if (cur_prog->params.flags & BOOTFLAG_B)
                {
                    // Add to the end of the sequence
                    SEQ_ADDH(boot_seq, cur_info);
                }
                else
                {
                    // Not a boot domain, so just dump the info in a context
                    mk_prog_cx(progs, cur_info);
                }
            }
        }
        kconsole << " + Adding boot domain sequence to progs context...\n"));
        Context$Add(progs, "BootDomains", &boot_seq_any);
    }*/
#endif
}

static NEVER_RETURNS void start_root_domain(bootimage_t& /*bm*/)
{
    //TODO: domain manager
    //TODO: VCPU
    //TODO: nucleus syscalls
    /* Find the Virtual Processor module */
/*    vp = NAME_FIND("modules>VP", VP_clp);
    kconsole << " + got VP   at %p\n", vp));

    Time = NAME_FIND("modules>Time", Time_clp);
    kconsole << " + got Time at %p\n", Time));
    PVS(time)= Time;
*/
    /* IM: init the wall-clock time values of the PIP */
/*    INFO_PAGE.prev_sched_time = NOW();
    INFO_PAGE.ntp_time  = NOW();
    INFO_PAGE.NTPscaling_factor = 0x0000000100000000LL; // 1.0

    // DomainMgr
    kconsole << " + initialising domain manager.\n"));
    {
        DomainMgrMod_clp dmm;
        LongCardTblMod_clp LongCardTblMod;
        Type_Any dommgrany;

        dmm = NAME_FIND("modules>DomainMgrMod", DomainMgrMod_clp);
        LongCardTblMod = NAME_FIND("modules>LongCardTblMod", LongCardTblMod_clp);
        dommgr = DomainMgrMod$New(dmm, salloc, LongCardTblMod,
                                framesF, mmu, vp, Time, kst);

        ANY_INIT(&dommgrany,DomainMgr_clp,dommgr);
        Context$Add(root,"sys>DomainMgr",&dommgrany);
    }

    kconsole << " + creating Trace module.\n"));
    {
        Type_Any any;
        Trace_clp t;
        TraceMod_clp tm;

        tm = NAME_FIND("modules>TraceMod", TraceMod_clp);
        t = TraceMod$New(tm);
        kst->trace = t;
        ANY_INIT(&any, Trace_clp, t);
        Context$Add(root, "sys>Trace", &any);
    }
*/
    kconsole << " + creating first domain." << endl;

    /*
    * The Nemesis domain contains servers which look after all sorts
    * of kernel resources.  It it trusted to play with the kernel
    * state in a safe manner
    */
/*    Nemesis = NAME_FIND("modules>Nemesis", Activation_clp);
    Nemesis->st= kst;

    if(nemesis_info == (BootDomain_Info *)NULL) {
        // shafted
        eprintf("NemesisPrimal: didn't get nemesis params. Dying.\n");
        ntsc_halt();
    }

    PVS(vp) = vp = DomainMgr$NewDomain(dommgr, Nemesis, &nemesis_pdid,
                                    nemesis_info->nctxts,
                                    nemesis_info->neps,
                                    nemesis_info->nframes,
                                    0, // no act str reqd
                                    nemesis_info->k,
                                    "Nemesis",
                                    &did,
                                    &dummy_offer);
    // Turn off activations for now
    VP$ActivationsOff(vp);

#ifdef __IX86__
    // Frob the pervasives things. This is pretty nasty.
    RW(vp)->pvs      = &NemesisPVS;
    INFO_PAGE.pvsptr = &(RW(vp)->pvs);
#endif
    kconsole << " + did NewDomain." << endl;
*/

    /* register our vp and pdom with the stretch allocators */
/*
    //stretch_allocator_mod->finish_init():
SAllocMod$Done(SAllocMod, salloc, vp, nemesis_pdid);
    SAllocMod$Done(SAllocMod, (StretchAllocatorF_cl *)sysalloc,
                vp, nemesis_pdid);

    DomainMgr$AddContracted(dommgr, did,
                            nemesis_info->p,
                            nemesis_info->s,
                            nemesis_info->l,
                            nemesis_info->x);

    kconsole << "NemesisPrimal: done DomainMgr_AddContracted.\n"));
    kconsole << "      + domain ID      = %x\n", (word_t)did));
    kconsole << "      + activation clp = %p\n", (addr_t)Nemesis));
    kconsole << "      + vp closure     = %p\n", (addr_t)vp));
    kconsole << "      + rop            = %p\n", (addr_t)RO(vp)));

    kconsole << "*************** ENGAGING PROTECTION ******************\n"));
    MMU$Engage(mmu, VP$ProtDomID(vp));

    kconsole << "NemesisPrimal: Activating Nemesis domain" << endl;
    ntsc_actdom(RO(vp), Activation_Reason_Allocated);
*/
    PANIC("root_domain entry returned!");
}

//======================================================================================================================
// load all required modules (mostly drivers)
//======================================================================================================================

//static void load_modules(UNUSED_ARG bootimage_t& bm, UNUSED_ARG const char* root_module)
//{
    // if bootpage contains devtree, we use it in building modules deps
    // find modules corresponding to devtree entries and add them to deps list
    // if no devtree present (on x86) we add "probe devices later" entry to bootpage to force
    // module probing after initial startup.

//     module_loader_t ml;
//     ml.load_modules("boot");
    // each module has .modinfo section with entry point and other meta info
    // plus .modinfo.deps section with module dependencies graph data

    // Load components from bootimage.
//     kconsole << "opening initfs @ " << bootimage->mod_start << endl;
//     initfs_t initfs(bootcp->mod_start);
//     typedef void (*comp_entry)(bootinfo_t bi_page);
//}

//======================================================================================================================
// Image bootup entry point
//======================================================================================================================

/*!
 * Image bootup starts executing without paging and with full ring0 rights.
 */

extern "C" void _start()
{
    run_global_ctors(); // remember, we don't have proper crt0 yet.

    kconsole << " + image bootup entry!" << endl;

    bootinfo_t* bi = new(bootinfo_t::ADDRESS) bootinfo_t;
    address_t start, end;
    const char* name;
    if (!bi->get_module(1, start, end, name))
    {
        PANIC("Bootimage not found! in image bootup");
    }

    bi->use_memory(information_page_t::ADDRESS, PAGE_SIZE);
    bi->use_memory(bootinfo_t::ADDRESS, PAGE_SIZE);
    bi->use_memory(0xb8000, PAGE_SIZE);

    bootimage_t bootimage(name, start, end);

    INFO_PAGE.pervasives = &pervasives;

    init_mem(bootimage);
    init_type_system(bootimage);
    init_namespaces(bootimage);
    start_root_domain(bootimage);
}
// Load the modules.
// Module "boot" depends on all modules that must be probed at startup.
// Dependency resolution will bring up modules in an appropriate order.
//    load_modules(bootimage, "boot");

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
