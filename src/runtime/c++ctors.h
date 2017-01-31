//
// Part of Metta OS. Check https://atta-metta.net for latest version.
//
// Copyright 2007 - 2017, Stanislav Karchebnyy <berkus@atta-metta.net>
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
