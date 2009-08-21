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
