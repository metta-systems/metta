#include "naming_context_v1_interface.h"
#include "naming_context_v1_impl.h"
#include "interface_v1_interface.h"
#include "exception_v1_impl.h"
#include "interface_v1_state.h"
#include "exceptions.h"

//=====================================================================================================================
// Method implementations.
// most methods are shared since records, enumerations,
// choices and exceptions are all are simple extensions of naming_context.
//=====================================================================================================================

static naming_context_v1::names
list(naming_context_v1::closure_t* self)
{
    return naming_context_v1::names();
  //     EnumRecState_t *st = (EnumRecState_t *)(self->st);
  // Context_Names  *seq;
  // NOCLOBBER int i;

  // TRC(pr("Enum$List: called\n"));
    
  // /* Get the result sequence */
  // seq = SEQ_CLEAR (SEQ_NEW (Context_Names, st->num, Pvs(heap)));
  
  // /* Run through all the types defined in the current interface */
  // TRY { 
  //   for( i=0; i < st->num; i++ ) {
  //     AddName( (st->elems)[i].name, Pvs(heap), seq );
  //   }
  // } CATCH_ALL {
  //   DB(pr("Enum$List: failed in Malloc: undoing.\n"));
  //   SEQ_FREE_ELEMS (seq);
  //   SEQ_FREE (seq);
  //   DB(pr("Enum$List: done undoing.\n"));
  //   RERAISE;
  // }
  // ENDTRY;

  // TRC(pr("Enum$List: done.\n"));
  // return seq;

}

static bool
get(naming_context_v1::closure_t* self, const char* name, types::any* obj)
{
    return false;
//   EnumRecState_t *st = (EnumRecState_t *)(self->st);
//   int i;

//   /* Straight linear string search for now. */
//   for( i = 0; i < st->num; i++ ) 
//     {
// #if 0 /* XXX PRB */
// 	/* lazily complete initialisation of Field_t val field */
// 	if ((st->elems)[i].val.type == TCODE_NONE) {
// 	    (st->elems)[i].val.val = (st->elems)[i].u.any.val;
// 	    (st->elems)[i].val.type = (st->elems)[i].u.any.type;
// 	}
// #endif 0
//       if (!(strcmp (name, (st->elems)[i].name ) ) )
// 	{
// 	  ANY_COPY(o,(Type_Any *)&((st->elems)[i].val));
// 	  return True;
// 	}
//     }
//   return False;
}

/*
 * Return base type of a choice; i.e the type of the discriminator
 */
// static types::code
// base(choice_v1::closure_t* self)
// {
// 	return self->d_state->disc;
// }

static const char*
info(exception_v1::closure_t* self, interface_v1::closure_t** i, uint32_t* n)
{
	*i = 0;//self->d_state->intf->rep.any.value;
	*n = 0;//self->d_state->params.num;
    return 0;//strdup(st->name);
}

//=====================================================================================================================
// add, remove, dup and destroy methods from naming_context are all nulls.
//=====================================================================================================================

static void
shared_add(naming_context_v1::closure_t*, const char*, types::any)
{
    OS_RAISE((exception_support_v1::id)"naming_context_v1.denied", 0);
    return;
}

static void
shared_remove(naming_context_v1::closure_t*, const char*)
{ 
    OS_RAISE((exception_support_v1::id)"naming_context_v1.denied", 0);
    return; 
}

// dup is not yet defined, but it raises denied as well

static void
shared_destroy(naming_context_v1::closure_t*)
{ 
    return; 
}

//=====================================================================================================================
// Method suites
//=====================================================================================================================

naming_context_v1::ops_t enum_ops = {
	list,
	get,
	shared_add,
	shared_remove,
	shared_destroy
};

naming_context_v1::ops_t record_ops = {
	list,
	get,
	shared_add,
	shared_remove,
	shared_destroy
};

// choice_v1::ops_t choice_ops = {
// 	list,
// 	get,
// 	shared_add,
// 	shared_remove,
// 	shared_destroy,
// 	base
// };

exception_v1::ops_t exception_ops = {
	list,
	get,
	shared_add,
	shared_remove,
	shared_destroy,
	info
};
