//
// Part of Metta OS. Check http://metta.exquance.com for latest version.
//
// Copyright 2007 - 2010, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//

inline void run_global_ctors()
{
    typedef void (*ctorfn)();
    extern ctorfn ctors_GLOBAL[]; // zero terminated constructors table

    for (unsigned int m = 0; ctors_GLOBAL[m]; m++)
        ctors_GLOBAL[m]();
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
