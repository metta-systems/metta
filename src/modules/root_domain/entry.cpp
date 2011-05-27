#include "default_console.h"
#include "frames_module_v1_interface.h"
#include "mmu_module_v1_interface.h"
//#include "heap_module_v1_interface.h"
#include "macros.h"
#include "c++ctors.h"
#include "root_domain.h"
#include "bootinfo.h"
#include "new.h"
#include "elf_parser.h"
#include "debugger.h"
#include "module_loader.h"

// bootimage contains modules and namespaces
// each module has an associated namespace which defines some module attributes/parameters.
// startup module from which root_domain starts also has a namespace called "default_namespace"
// it defines general system attributes and startup configuration.

#include "mmu_module_impl.h" // for debug

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

    kconsole << "Found module " << module_name << " at address " << addr.start << " of size " << addr.size << endl;

    bootinfo_t* bi = new(BOOTINFO_PAGE) bootinfo_t(false);
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

//======================================================================================================================
// setup page mapping
//======================================================================================================================

#if 0
static void map_identity(const char* caption, address_t start, address_t end)
{
    #if MEMORY_DEBUG
    kconsole << "Mapping " << caption << endl;
    #endif
    end = page_align_up<address_t>(end); // one past end
    for (uint32_t k = start/PAGE_SIZE; k < end/PAGE_SIZE; k++)
        protection_domain_t::privileged().map(k * PAGE_SIZE, reinterpret_cast<void*>(k * PAGE_SIZE),
        page_t::kernel_mode | page_t::writable);
}
#endif


static system_frame_allocator_v1_closure* init_phys_mem(frames_module_v1_closure* frames_mod/*, ramtab_t& rtab, void* free*/)
{
// allmem comes from BOOTINFO_PAGE
// memmap is used memory - what needs to be mapped

    kconsole << "init_phys_mem" << endl;

    // We need to abstract frames module from the format of bootinfo page, so we create a local copy of
    // memory map and pass it to frames_mod as a parameter.
//     bootinfo_t* bi = new(BOOTINFO_PAGE) bootinfo_t;
//     std::vector<PMemDesc, stack_allocator(32)> memmap;
// 
//     std::for_each(bi->mmap_begin(), bi->mmap_end(), [](const multiboot_t::mmap_entry_t e)
//     {
//         kconsole << "mmap entry @ " << e.address() << " is " << e.size() << " bytes of type " << e.type() << endl;
//         memmap.push_back(PMemDesc(e.address(), e.size(), e.type()));
//     });

	system_frame_allocator_v1_closure* frames = frames_mod->create(0/*allmem, used, rtab, free*/);
    return frames;
}

static void init_mem(bootimage_t& bootimg)
{
    kconsole << "init_mem" << endl;

    // request necessary space for frames allocator
    frames_module_v1_closure* frames_mod;
    frames_mod = load_module<frames_module_v1_closure>(bootimg, "frames_mod", "exported_frames_module_rootdom");
    ASSERT(frames_mod);

    mmu_module_v1_closure* mmu_mod;
    mmu_mod = load_module<mmu_module_v1_closure>(bootimg, "mmu_mod", "exported_mmu_module_rootdom");
    ASSERT(mmu_mod);

    int required = frames_mod->required_size();
    int initial_heap_size = 64*1024;

    kconsole << "Init memory region size " << required + initial_heap_size << " bytes." << endl;
    mmu_v1_closure* mmu = mmu_mod->create(required + initial_heap_size/*, &rtab, &free*/);
    UNUSED(mmu);

    //BREAK();

    kconsole << "Creating frame allocator" << endl;
/*    system_frame_allocator_v1_closure* frames =*/ init_phys_mem(frames_mod/*, rtab, free*/);

    kconsole << "Creating heap" << endl;
//    auto heap_mod = load_module<heap_module_v1_closure>(bootimg, "heap_mod", "exported_heap_module_rootdom");
//    ASSERT(heap_mod);
//    heap = heap_mod->create_raw(free + required, initial_heap_size);

//    frames_mod->finished(frames, heap);

    // create virtual memory allocator
    // create stretch allocator
    // assign stretches to address ranges
    kconsole << "Creating stretch allocator" << endl;
//    salloc_mod = load_module<stretch_allocator_module_v1_closure>(bootimg, "stretchalloc_mod", "exported_stretch_allocator_module_rootdom");
//    ASSERT(salloc_mod);
/*
    salloc = init_virt_mem(salloc_mod, memmap, heap, mmu);
    Pervasives(strech_allocator) = salloc;

    sysalloc = salloc->create_nailed(frames, heap);
    mmu_mod->finished(mmu, frames, heap, sysalloc);

    StretchTblMod = lookup("StretchTblModCl");
    strtab        = StretchTblMod->New(StretchTblMod, Pvs(heap));
*/
    /* XXX SMH: create an initial default stretch driver */
//    SDriverMod    = lookup("SDriverModCl");
//    Pvs(sdriver)  = SDriverMod->NewNULL(SDriverMod, heap, strtab);
//     stretch_driver_t::default_driver().initialise();

    /* Create the initial address space; returns a pdom for Nemesis domain */
    kconsole << " + creating addr space." << endl;
//    nemesis_pdid = CreateAddressSpace(frames, mmu, salloc, nexusp);
//    MapInitialHeap(HeapMod, heap, iheap_size*sizeof(word_t), nemesis_pdid);
}

static void init_type_system(bootimage_t& bootimg)
{
    /* Get an Exception System */
    kconsole << " + Bringing up exceptions" << endl;
    exceptions_module_v1_closure* xcp_mod;
    xcp_mod = load_module<exceptions_module_v1_closure>(bootimg, "exceptions_mod", "exported_exceptions_module_v1_rootdom");
    ASSERT(xcp_mod);

	exceptions = xcp_mod->create();
	Pervasives(xcp) = exceptions;

    kconsole <<  " + Bringing up REAL type system" << endl;
    kconsole <<  " +-- getting safelongcardtable_mod..." << endl;
    lctmod = load_module<longcardtable_module_v1_closure>(bootimg, "longcardtable_mod", "exported_longcardtable_module_v1_rootdom");
    kconsole <<  " +-- getting stringtable_mod..." << endl;
    strmod = load_module<stringtable_module_v1_closure>(bootimg, "stringtable_mod", "exported_stringtable_module_v1_rootdom");
    kconsole <<  " +-- getting typesystem_mod..." << endl;
    tsmod = load_module<typesystem_module_v1_closure>(bootimg, "typesystem_mod", "exported_typesystem_module_v1_rootdom");
    kconsole <<  " +-- creating a new type system..." << endl;
    ts = tsmod->create(Pvs(heap), lctmod, strmod);
    kconsole <<  " +-- done: ts is at " << ts << endl;
    Pvs(types) = (TypeSystem_clp)ts;

    /* Preload any types in the boot image */
/*    {
        TypeSystemF_IntfInfo *info;

        kconsole << " +++ registering interfaces\n"));
        info = (TypeSystemF_IntfInfo)lookup("Types");
        while(*info) {
            TypeSystemF->RegisterIntf(ts, *info);
            info++;
        }
    }*/
}

static void init_namespaces(bootimage_t& /*bm*/)
{
    /* Build initial name space */
    kconsole <<  " + Building initial name space: ";

    /* Build root context */
    kconsole <<  "<root>, ";

    context_module_v1_closure* context_mod;
    context_mod = load_module<context_module_v1_closure>(bootimg, "context_mod", "exported_context_module_rootdom");
    ASSERT(context_mod);

	root = context_mod->create_context(heap, Pvs(types));

/*    ContextMod = lookup("ContextModCl");
    root = ContextMod$NewContext(ContextMod, heap, Pvs(types) );
    Pvs(root)  = root;

    kconsole <<  "modules, ";
    {
        Context_clp mods = ContextMod$NewContext(ContextMod, heap, Pvs(types));
        ANY_DECL(mods_any, Context_clp, mods);
        char *cur_name;
        addr_t cur_val;

        Context$Add(root,"modules",&mods_any);

        set_namespace(nexusprimal->namespc, "modules");
        while((cur_name=lookup_next(&cur_val))!=NULL)
        {
            TRC_CTX(eprintf("\n\tadding %p with name %s", cur_val, cur_name));
            Context$Add(mods, cur_name, cur_val);
        }
        TRC_CTX(eprintf("\n"));
    }

    kconsole <<  "blob, ";
    {
        Context_clp blobs= ContextMod$NewContext(ContextMod, heap, Pvs(types));
        ANY_DECL(blobs_any, Context_clp, blobs);
        Type_Any b_any;
        char *cur_name;
        addr_t cur_val;

        Context$Add(root, "blob", &blobs_any);

        set_namespace(nexusprimal->namespc, "blob");
        while((cur_name=lookup_next(&cur_val))!=NULL)
        {
            TRC_CTX(eprintf("\n\tadding %p with name %s", cur_val, cur_name));
            ANY_INIT(&b_any, RdWrMod_Blob, cur_val);
            Context$Add(blobs, cur_name, &b_any);
        }
        TRC_CTX(eprintf("\n"));
    }

    kconsole <<  "proc, ";
    {
        Context_clp proc = ContextMod$NewContext(ContextMod, heap, Pvs(types));
        Context_clp domains = ContextMod$NewContext(ContextMod, heap, Pvs(types));
        Context_clp cmdline = ContextMod$NewContext(ContextMod, heap, Pvs(types));
        ANY_DECL(tmpany, Context_clp, proc);
        ANY_DECL(domany, Context_clp, domains);
        ANY_DECL(cmdlineany,Context_clp,cmdline);
        ANY_DECL(kstany, addr_t, kst);
        string_t ident=strduph(k_ident, heap);
        ANY_DECL(identany, string_t, ident);
        Context$Add(root, "proc", &tmpany);
        Context$Add(proc, "domains", &domany);
        Context$Add(proc, "kst", &kstany);
        Context$Add(proc, "cmdline", &cmdlineany);
        Context$Add(proc, "k_ident", &identany);
#ifdef INTEL
        parse_cmdline(cmdline, ((kernel_st *)kst)->command_line);
#endif
    }

    kconsole <<  "pvs, ";
    {
        Type_Any any;
        if (Context$Get(root,"modules>PvsContext",&any)) {
            Context$Add(root,"pvs",&any);
        } else {
            eprintf ("NemesisPrimal: WARNING: >pvs not created\n");
        }
    }

    kconsole <<  "symbols, ";
    {
        Context_clp tmp = ContextMod$NewContext(ContextMod, heap, Pvs(types) );
        ANY_DECL(tmpany,Context_clp,tmp);
        Context$Add(root,"symbols",&tmpany);
    }
*/
    /* Build system services context */
/*    kconsole <<  "sys, ";
    {
        Context_clp sys = ContextMod$NewContext(ContextMod, heap, Pvs(types) );
        ANY_DECL(sys_any,Context_clp,sys);
        ANY_DECL(salloc_any, StretchAllocator_clp, salloc);
        ANY_DECL(sysalloc_any, StretchAllocator_clp, sysalloc);
        ANY_DECL(ts_any, TypeSystem_clp, Pvs(types));
        ANY_DECL(ffany, FramesF_clp, framesF);
        ANY_DECL(strtab_any, StretchTbl_clp, strtab);
        Context$Add(root, "sys", &sys_any);
        Context$Add(sys, "StretchAllocator", &salloc_any);
        Context$Add(sys, "SysAlloc", &sysalloc_any);
        Context$Add(sys, "TypeSystem", &ts_any);
        Context$Add(sys, "FramesF", &ffany);
        Context$Add(sys, "StretchTable", &strtab_any);
    }
*/
    /* IDC stub context */
/*    kconsole <<  "IDC stubs, ";
    {
        Closure_clp stubs_register;
        Context_clp stubs =
        ContextMod$NewContext(ContextMod, heap, Pvs(types) );

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
        Context_clp progs= ContextMod$NewContext(ContextMod, heap, Pvs(types));
        ANY_DECL(progs_any,Context_clp,progs);
        ProtectionDomain_ID prog_pdid;
        BootDomain_InfoSeq *boot_seq;
        BootDomain_Info *cur_info;
        nexus_prog *cur_prog;
        char *name;
        Type_Any *any, boot_seq_any, blob_any;

        Context$Add(root,"progs",&progs_any);

        boot_seq = SEQ_NEW(BootDomain_InfoSeq, 0, Pvs(heap));
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
/*                nemesis_info= Heap$Malloc(Pvs(heap), sizeof(BootDomain_Info));
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
                cur_info->priv_root = ContextMod$NewContext(ContextMod, heap, Pvs(types));

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
}

static NEVER_RETURNS void start_root_domain(bootimage_t& /*bm*/)
{
    /* Find the Virtual Processor module */
/*    vp = NAME_FIND("modules>VP", VP_clp);
    kconsole << " + got VP   at %p\n", vp));

    Time = NAME_FIND("modules>Time", Time_clp);
    kconsole << " + got Time at %p\n", Time));
    Pvs(time)= Time;
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

    Pvs(vp) = vp = DomainMgr$NewDomain(dommgr, Nemesis, &nemesis_pdid,
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
*/
    kconsole << " + did NewDomain." << endl;

    /* register our vp and pdom with the stretch allocators */
/*    SAllocMod$Done(SAllocMod, salloc, vp, nemesis_pdid);
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

#if defined(__i386__) || defined(__x86_64)
    kconsole << "*************** ENGAGING PROTECTION ******************\n"));
    MMU$Engage(mmu, VP$ProtDomID(vp));
#else
    // install page fault handler
    // Identity map currently executing code.
    // page 0 is not mapped to catch null pointers
    //     map_identity("bottom 4Mb", PAGE_SIZE, 4*MiB - PAGE_SIZE);
    // enable paging
    //     static_cast<x86_protection_domain_t&>(protection_domain_t::privileged()).enable_paging();
    //     kconsole << "Enabled paging." << endl;
    #warning Need some protection for your architecture.
#endif

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

extern "C" void entry()
{
    run_global_ctors(); // remember, we don't have proper crt0 yet.

    kconsole << " + image bootup entry!" << endl;

    bootinfo_t* bi = new(BOOTINFO_PAGE) bootinfo_t(false);
    address_t start, end;
    const char* name;
    if (!bi->get_module(1, start, end, name))
    {
        PANIC("Bootimage not found! in image bootup");
    }

    bootimage_t bootimage(name, start, end);

    init_mem(bootimage);
    init_type_system(bootimage);
    init_namespaces(bootimage);
// Load the modules.
// Module "boot" depends on all modules that must be probed at startup.
// Dependency resolution will bring up modules in an appropriate order.
//    load_modules(bootimage, "boot");
    start_root_domain(bootimage);
}
