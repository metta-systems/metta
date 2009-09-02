//
// Copyright 2007 - 2009, Renārs Lediņš <renars@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

#include "obj_destructor.h"
#include "obj_mover.h"

template <typename T>
struct obj_type_behavior
{
    typedef T                           value_type;
    typedef obj_destructor<value_type>  destructor;
    typedef obj_mover<value_type>       mover;
};

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
