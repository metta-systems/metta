#pragma once

class component /*: public object*/
{
public:
    component(com_iparent *); //!< Component boot entry point.
    ~component(); //!< Destroy component, free all resources. Might cause chain reaction to destroy all dependants as well?
};
