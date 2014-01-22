Use "this" pointer to mimic current c++ behaviour transparently.
 - at *this there is vtbl pointer to the list of methods for the interface instance -> hence, every instance carries with it the list of methods
 - at *(this+4) there is version integer - version of interface used to pick correct method offsets, might help JIT generators to make proper
calls to obtained interfaces.

```cpp
// base class for component interfaces
class component_interface_t
{
public:
    virtual ~component_interface_t() {}
    int interface_version;
};

class interface1 : public component_interface_t
{
public:
    void method1();
    void method2();
};
```

c++ client:

```cpp
interface1* intf = trader::get_interface("interface1");
intf->method1();
```

c client:

```c
interface1_wrap* meth = trader__get_interface("interface1");
meth->methods->method1(meth->instance);
```
