#pragma once

// TODO: component should implement com_iunknown interface
// FIXME: component dependencies should be resolved prior to instantiating it, so use another little C stub?

class component /*: public object*/
{
public:
    component(com_iparent *); //!< Component boot entry point.
    ~component(); //!< Destroy component, free all resources. Might cause chain reaction to destroy all dependants as well?

	void privileged_init(); // privileged init during startup
};
