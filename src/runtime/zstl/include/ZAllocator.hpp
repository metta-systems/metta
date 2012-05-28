//////////////////////
/* ZList Allocators */
//////////////////////

/*
Allocator.  Handles allocation of objects from byte storage.
*/
template <typename T>
class ZAllocator
{
public:
	//Virtual Destructor
	virtual ~ZAllocator() { }

	/*
	Allocate @c bytes from storage and return as type T.
	*/
	virtual T* Allocate(size_t bytes) = 0;

	/*
	virtual public ZListAllocator<T>::Clone

	Clone function, which is required to allocate and return a ZNew instance of this
	type of allocator.

	@return - allocated instance of this type of allocator
	*/
	virtual ZAllocator<T>* Clone() = 0;

	/*
	Deallocate previously allocated storage.
	*/
	virtual void Deallocate(T* _node) = 0;

	/*
	Destroy allocator.
	Heap allocated allocators should delete themselves (suicide).
	*/
	virtual void Destroy() = 0;
};

