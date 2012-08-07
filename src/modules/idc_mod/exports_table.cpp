#include "exports_table_v1_interface.h"
#include "exports_table_v1_impl.h"
#include "hashtables.h"

struct table_entry_t
{
	types::any* interface;
	exports_table_v1::handle handle;
};

DECLARE_MAP(offer_table, idc_offer_v1::closure_t*, table_entry_t);

struct exports_table_v1::state_t
{
	exports_table_v1::closure_t client_closure;
	heap_v1::closure_t* heap;
	offer_table_t offers;
};
