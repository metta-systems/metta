//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "string.h"

namespace metta {
namespace common {

const string::data string::shared_null = {};
const string::data string::shared_empty = {};

string::string() : d(&shared_null)
{
    d->ref++;
}

}
}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
