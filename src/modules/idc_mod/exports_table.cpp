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
	//binder_callback_closure;
	heap_v1::closure_t* heap;
	//binder;
	//entry_factory;

	//server_state;
	//callback_connection;
	//callback_entry;

	//mutex;
	bool registered;
	offer_table_t offers;
	//event_count;
};
