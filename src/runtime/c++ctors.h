inline void run_global_ctors()
{
    typedef void (*ctorfn)();
    extern ctorfn ctors_GLOBAL[]; // zero terminated constructors table

    for (unsigned int m = 0; ctors_GLOBAL[m]; m++)
        ctors_GLOBAL[m]();
}
