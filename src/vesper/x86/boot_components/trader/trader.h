//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus+metta@madfire.net>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

/*!
Trader allows applications to find appropriate portals for communication.

Trader exclusively allocates portal IDs.

Predefined system portal IDs:
0 - get kernel info page
1 - create_portal
2 -
- portal_manager access
- trader access
- vm_server access
- security_server access
- scheduler access
- interrupt_dispatcher? access
- thread? access
*/
class trader_t
{
    trader_t();
private:
};

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
