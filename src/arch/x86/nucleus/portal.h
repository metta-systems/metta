//
// Copyright 2007 - 2009, Stanislav Karchebnyy <berkus@exquance.com>
//
// Distributed under the Boost Software License, Version 1.0.
// (See file LICENSE_1_0.txt or a copy at http://www.boost.org/LICENSE_1_0.txt)
//
#pragma once

namespace nucleus
{

/*!
 * Portal is a way for threads to cross PD borders.
 */
template <const string_t& method_spec>
class portal_t
{
public:
    // create a portal that calls into target_asid at the address
    // target_loc.
    portal_t(asid_t target_asid, address_t target_loc);

    // TODO: make template method that expands into series of
    // append_param() calls with types from method_spec and also
    // typechecks actual args against formal list in method_spec.
    void operator()();

    void return_to_caller(); // a paired reverse portal to return from
    // call to this portal. can also have parameters!
private:
};

}

// kate: indent-width 4; replace-tabs on;
// vim: set et sw=4 ts=4 sts=4 cino=(4 :
