Contexts allow apps to search for services.
Contexts are syntactically just strings. A string like com.exquance.metta defines a context consisting of three subcontexts.



Fractal is a modular, extensible and programming language agnostic component model that can be used to design, implement, deploy and reconfigure systems and applications, from operating systems to middleware platforms and to graphical user interfaces. The goal of Fractal is to reduce the development, deployment and maintenance costs of software systems in general, and of ObjectWeb projects in particular.

The Fractal component model has the following important features:

 * recursivity : components can be nested in composite components (hence the "Fractal" name).
 * reflectivity : components have full introspection and intercession capabilities.
 * component sharing : a given component instance can be included (or shared) by more than one component. This is useful to model shared resources such as memory manager or device drivers for instance.
 * binding components : a single abstraction for components connections that is called bindings . Bindings can embed any communication semantics from synchronous method calls to remote procedure calls
 * execution model independence : no execution model is imposed. In that, components can be run within other execution models than the classical thread-based model such as event-based models and so on.
 * open : extra-functional services associated to a component can be customized through the notion of a control membrane.



* components bind together (@sa THINK/KORTEX)
* access restrictions apply at bind time (fine grained interfaces)
