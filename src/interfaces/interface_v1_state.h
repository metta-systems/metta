#pragma once

#include "operation_v1_interface.h"
#include "interface_v1_interface.h"

//=====================================================================================================================
// Interface representation data structures.
// These structures must be statically-constructible (i.e. only PODs and pointers).
//=====================================================================================================================

/**
 * Parameter type: used in operations.
 */
struct param_t 
{
    operation_v1::parameter  rep;
    const char*              name;
};

/**
 * Field type: used in Records, Choices, Enumerations and Exceptions.
 */
struct field_t {
    types::any   value;
    const char*  name;
};

/**
 * Enumeration and record type state.
 */
struct enum_rec_state_t
{
    size_t    num;
    field_t*  elems;     /* val = enum. value or record type code */
};

/*
 * Choice type state
 */
struct ChoiceState_t {
    enum_rec_state_t  ctx;
    types::code       disc;      /* discriminator    */
};

struct ExcRef_t {
    types::code    intf; /* Interface where this exception is defined. */
    uint32_t       num;  /* Index of the exception in the interface.   */
};

/* Exceptions */
struct Exc_t {
    enum_rec_state_t         params;     /* Parameters - must be first  */
    exception_v1::closure_t  closure;    /* Closure for exception       */
    interface_v1::state_t*   intf;       /* Defining interface          */
    const char*              name;       /* Name of exception           */
};

/**
 * Operations
 */
struct operation_t
{
    const char*               name;        /**< Name of Operation      */
    operation_v1::kind        kind;        /**< Type of Operation      */
    param_t*                  params;      /**< Array of parameters    */
    uint32_t                  num_args;    /**< How many there are     */
    uint32_t                  num_res;     /**< How many there are     */
    uint32_t                  proc_num;    /**< Procedure number       */
    ExcRef_t*                 exns;        /**< Array of exceptions    */       
    uint32_t                  num_excepts; /**< Number of exceptions   */
    operation_v1::closure_t*  closure;     /**< Closure for operation  */
};


/**
 * Type within an interface
 */
struct type_representation_t
{
    types::any                any;        /**< Class-specific stuff           */
    types::any                code;       /**< Type code of type, as an any   */
    types::name               name;       /**< Name of type                   */
    interface_v1::state_t*    interface;  /**< Pointer to defining interface  */
    memory_v1::size           size;       /**< Size of an instance            */
};

/**
 * Interface itself (also a type, obviously)
 */
struct interface_v1::state_t
{
    type_representation_t          rep;           /**< Type description             */
    types::code const*             needs;         /**< List of needed interfaces    */
    size_t                         num_needs;     /**< How many of them are there   */
    type_representation_t* const*  types;         /**< List of defined types        */
    size_t                         num_types;     /**< How many of them are there   */
    bool                           local;         /**< Is this interface local?     */
    types::code                    supertype;     /**< Supertype, if any            */
    operation_t* const*            methods;       /**< Table of methods             */
    size_t                         num_methods;   /**< No. of methods we define     */
    Exc_t* const*                  exns;          /**< Table of exceptions          */
    size_t                         num_exns;      /**< No. of exceptions we define  */
};

/*
 * Macros for manipulating type codes
 */
#define TCODE_META_NAME     "meta_interface"
#define TCODE_MASK      ((types::code) 0xffffull)
#define TCODE_INTF_CODE(_tc)    ((_tc) & ~TCODE_MASK)
#define TCODE_IS_INTERFACE(_tc) (!((_tc) & TCODE_MASK))

#define TCODE_VALID_TYPE(_tc,_b) \
    ( ((_tc) & TCODE_MASK) <= (_b)->num_types)

#define TCODE_WHICH_TYPE(_tc,_b) \
    (((_b)->types)[ ((_tc) & TCODE_MASK) - 1 ])

#define TCODE_NONE      0xffffffffffffffffULL
#define meta_interface_type_code 0

