/*
	ZHashMap.hpp
	Author: James Russell <jcrussell@762studios.com>
	Created: 12/24/2011

	Purpose: 

	Templated hash map implementation.

	License:

	This program is free software. It comes without any warranty, to
	the extent permitted by applicable law. You can redistribute it
	and/or modify it under the terms of the Do What The Fuck You Want
	To Public License, Version 2, as published by Sam Hocevar. See
	http://sam.zoy.org/wtfpl/COPYING for more details.

*/

#pragma once

#ifndef _ZHASHMAP_HPP
#define _ZHASHMAP_HPP

#include <zstl/include/ZPair.hpp>
#include <zstl/include/ZArray.hpp>
#include <zstl/include/ZList.hpp>

//Default number of buckets
#ifndef ZHASHMAP_DEFAULT_BUCKETS
#define ZHASHMAP_DEFAULT_BUCKETS (32)
#endif

//Default load factor for the HashMap
#ifndef ZHASHMAP_DEFAULT_LOADFACTOR
#define ZHASHMAP_DEFAULT_LOADFACTOR (0.75)
#endif

//Factor by which HashMap grows when loadfactor is exceeded
#ifndef ZHASHMAP_DEFAULT_GROWFACTOR
#define ZHASHMAP_DEFAULT_GROWFACTOR (1.5)
#endif

/*
Hasher class, which produces hash values of objects for use in ZHashMap.

The template parameter T is the type we will be hashing.
*/
template <typename T>
class ZHasher
{
public:
	//Comparator for use with equals and less than
	ZComparator<T> Comp;

	//Default hash function, which attempts to cast the object as a hash value
	ZHashValue Hash(const T& _obj) const
	{
		return (ZHashValue)_obj;
	}

	//Default equals function, which compares objects
	bool Equals(const T& _first, const T& _second) const
	{
		return Comp.Equals(_first, _second);
	}
};

/*
Templated dynamic hash map implementation.

Uses chained hashing using linked list to ensure it behaves gracefully even under 
heavy load and does not maintain order between keys.

The template parameter K is the 'key' type, which must be hashable.

The template parameter V is the 'value' type, which need not be hashable.

The template parameter H is the 'hasher' type, which defaults to the ZHasher for K.
*/
template <typename K, typename V, typename H = ZHasher<K> >
class ZHashMap
{
private:
	//The hasher for our keys
	H Hasher;

	//The element count
	size_t ElementCount;

	//Maximum Load Factor for the HashMap; 0 indicates that the HashMap should not resize
	double LoadFactor;

	//The map
	ZArray< ZList< ZPair<K, V> > > Map;//@todo fix allocator passing - needs to forward given allocator to constructed members

	//Resizes and copies the HashMap if we exceed LoadFactor
	inline void CheckLoadFactor()
	{
		double currentLoadFactor = (double)ElementCount / (double)Map.Size();

		if (LoadFactor > 0 && currentLoadFactor > LoadFactor)
		{
			SetBuckets((size_t)((double)Map.Size() * ZHASHMAP_DEFAULT_GROWFACTOR));
		}
	}

	//Gets a bucket given a hash code
	inline size_t GetBucket(ZHashValue _hash) const
	{
		return _hash % Map.Size();
	}

public:
	/*
	Default Constructor.
	*/
	ZHashMap();

    /**
     * Specify allocator to use.
     */
	ZHashMap(ZAllocator<ZPair<K,V>>* alloc);

	/*
	Parameterized Constructor.

	@param _buckets - the number of buckets the hashmap should use
	*/
	ZHashMap(size_t _buckets);

	/*
	Parameterized Constructor.

	@param _buckets - the number of buckets the hashmap should use
	@param _loadFactor - load factor at which the HashMap gets resized; -1 indicates the map should not auto-resize
	*/
	ZHashMap(size_t _buckets, double _loadFactor);

	/*
	public ZHashMap<K, V, H>::operator[]
	
	Functionally the same as ZHashMap<K, V, H>::Get.
	
	@param _key - the key to lookup
	@return (V&) - the value mapped to _key
	*/
	V& operator [] (const K& _key);

	/*
	public ZHashMap<K, V, H>::Clear

	Clears the hash map of all keys and values.
	
	@return (void)
	*/
	void Clear();

	/*
	public ZHashMap<K, V, H>::ContainsKey

	Determines if the hash map contains the given key.
	
	@param _key - the key to check
	@return (bool) - boolean indicating if the hash map contains the key
	@context (all)
	*/
	bool ContainsKey(const K& _key) const;

	/*
	public ZHashMap<K, V, H>::ContainsValue

	Determines if the hash map contains the given value.

	@param _value - the value to check for
	@return (bool) - boolean indicating if the hash map contains the value
	*/
	bool ContainsValue(const V& _value) const;

	/*
	public ZHashMap<K, V, H>::Empty

	Indicates whether or not the hash map is empty.

	@return (bool) - boolean indicating the hash map is empty (free of keys and values)
	*/
	bool Empty() const;

	/*
	public ZHashMap<K, V, H>::Get
	
	Gets the value mapped to the given key.  Will runtime assert if no mapped value is found,
	so use TryGet if you think the element might not be there.  If the runtime assert is
	ignored, the behavior is undefined.
	
	@param _key - the key to lookup
	@return (V&) - the value mapped to _key
	*/
	V& Get(const K& _key) const;

	/*
	public ZHashMap<K, V, H>::Keys

	Gets a list of keys in the map.

	@return - ZList containing all the keys in the map
	*/
	ZList<K> Keys() const;

	/*
	public ZHashMap<K, V, H>::Mappings

	Gets a list of all the key value mappings in the hash map.

	@return (ZList<ZPair<K, V>>)- list of key-value pairs
	*/
	ZList< ZPair<K, V> > Mappings() const;

	/*
	public ZHashMap<K, V, H>::Put

	Puts the given key and value combination into the hash map.

	@param _key - the key to associate with _value
	@param _value - the value to lookup using _key
	@return (V&) - returns _value
	*/
	V& Put(const K& _key, const V& _value);

	/*
	public ZHashMap<K, V, H>::Remove

	Removes the associated key and mapped value.

	@param _key - the key to lookup
	@return (void)
	*/
	void Remove(const K& _key);
	
	/*
	public ZHashMap<K, V, H>::SetBuckets
	
	Sets the number of buckets the hashmap will use.  Existing elements will be re-hashed into the map.
	
	@param _buckets - number of buckets to use
	@return (void)
	*/
	void SetBuckets(size_t _buckets);

	/*
	public ZHashMap<K, V, H>::Size

	Returns the number of mapped key-value pairs in the map.

	@return (size_t) - the number of key value pairs
	*/
	size_t Size() const;

	/*
	public ZHashMap<K, V, H>::TryGet

	Get function which attempts a lookup.

	@param _key - the key to lookup
	@param _val - 'out' parameter, value associated with _key
	@return (bool) - boolean indicating whether or not the value is found
	*/
	bool TryGet(const K& _key, V& _val) const;

	/*
	public ZHashMap<K, V, H>::TryGetPtr

	Get function which attempts a lookup.

	@param _key - the key to lookup
	@return (V*) - pointer to the contained value if found, NULL if not found
	*/
	V* TryGetPtr(const K& _key) const;

	/*
	public ZHashMap<K, V, H>::Values

	Gets a list of values in the map.

	@return (ZList<V>)- ZList containing all the values in the map
	*/
	ZList<V> Values() const;
};

template <typename K, typename V, typename H>
ZHashMap<K, V, H>::ZHashMap()
: ElementCount(0), LoadFactor(ZHASHMAP_DEFAULT_LOADFACTOR), Map(ZHASHMAP_DEFAULT_BUCKETS)
{
	Map.Resize(ZHASHMAP_DEFAULT_BUCKETS);
}

template <typename K, typename V, typename H>
ZHashMap<K, V, H>::ZHashMap(ZAllocator<ZPair<K,V>>* alloc)
: ElementCount(0), LoadFactor(ZHASHMAP_DEFAULT_LOADFACTOR), Map(ZHASHMAP_DEFAULT_BUCKETS, alloc)
{
	Map.Resize(ZHASHMAP_DEFAULT_BUCKETS);
}

template <typename K, typename V, typename H>
ZHashMap<K, V, H>::ZHashMap(size_t _buckets) 
: ElementCount(0), LoadFactor(ZHASHMAP_DEFAULT_LOADFACTOR), Map(_buckets)
{
	#if ZSTL_DISABLE_RUNTIME_CHECKS
	#else
	ZSTL_ASSERT(_buckets > 0, "Cannot make ZHashMap with no buckets!");
	#endif

	Map.Resize(_buckets);
}

template <typename K, typename V, typename H>
ZHashMap<K, V, H>::ZHashMap(size_t _buckets, double _loadFactor)
: ElementCount(0), LoadFactor(_loadFactor), Map(_buckets) 
{
	#if ZSTL_DISABLE_RUNTIME_CHECKS
	#else
	ZSTL_ASSERT(_buckets > 0, "Cannot make ZHashMap with no buckets!");
	#endif

	Map.Resize(_buckets);
}

template <typename K, typename V, typename H >
V& ZHashMap<K, V, H>::operator[]( const K& _key )
{
	return Get(_key);
}

template <typename K, typename V, typename H>
void ZHashMap<K, V, H>::Clear()
{
	size_t i;

	for (i = 0; i < Map.Size(); i++)
		Map[i].Clear();

	ElementCount = 0;
}

template <typename K, typename V, typename H>
bool ZHashMap<K, V, H>::ContainsKey(const K& _key) const
{
	size_t bucket;
	typename ZList< ZPair<K, V> >::Iterator itr;

	bucket = GetBucket(Hasher.Hash(_key));

	for (itr = Map[bucket].Begin(); itr != Map[bucket].End(); itr++)
	{
		if ( Hasher.Equals((*itr).First, _key) )
		{
			return true;
		}
	}

	return false;
}

template <typename K, typename V, typename H>
bool ZHashMap<K, V, H>::ContainsValue(const V& _value) const
{
	size_t bucket;
	typename ZList< ZPair<K, V> >::Iterator itr;

	for (bucket = 0; bucket < Map.Size(); bucket++)
	{
		for (itr = Map[bucket].Begin(); itr != Map[bucket].End(); itr++)
		{
			if ( (*itr).Second == _value )
				return true;
		}
	}

	return false;
}

template <typename K, typename V, typename H>
bool ZHashMap<K, V, H>::Empty() const
{
	return ElementCount == 0;
}

template <typename K, typename V, typename H >
V& ZHashMap<K, V, H>::Get( const K& _key ) const
{
	size_t bucket;
	typename ZList< ZPair<K, V> >::Iterator itr;

	bucket = GetBucket(Hasher.Hash(_key));

	for (itr = Map[bucket].Begin(); itr != Map[bucket].End(); itr++)
	{
		if ( Hasher.Equals((*itr).First, _key) )
		{
			return (*itr).Second;
		}
	}

	#if ZSTL_DISABLE_RUNTIME_CHECKS
	#else
	ZSTL_ERROR("ZHashMap::Get could not find value!");
	#endif

	//If the runtime assert is ignored and we continue, return ref to a static local variable
	//This is effectively 'undefined' behavior
	static V val;

	return val;
}

template <typename K, typename V, typename H>
ZList<K> ZHashMap<K, V, H>::Keys() const
{
	ZList<K> keys;

	size_t bucket;
	typename ZList< ZPair<K, V> >::Iterator itr;

	for (bucket = 0; bucket < Map.Size(); bucket++)
	{
		for (itr = Map[bucket].Begin(); itr != Map[bucket].End(); itr++)
		{
			keys.PushBack((*itr).First);
		}
	}

	return keys;
}

template <typename K, typename V, typename H>
ZList< ZPair<K, V> > ZHashMap<K, V, H>::Mappings() const
{
	ZList< ZPair<K, V> > mappings;

	size_t bucket;
	typename ZList< ZPair<K, V> >::Iterator itr;

	for (bucket = 0; bucket < Map.Size(); bucket++)
	{
		for (itr = Map[bucket].Begin(); itr != Map[bucket].End(); itr++)
		{
			mappings.PushBack((*itr));
		}
	}

	return mappings;
}

template <typename K, typename V, typename H>
V& ZHashMap<K, V, H>::Put(const K& _key, const V& _value)
{
	size_t bucket;
	typename ZList< ZPair<K, V> >::Iterator itr;

	CheckLoadFactor();
	
	bucket = GetBucket(Hasher.Hash(_key));

	for (itr = Map[bucket].Begin(); itr != Map[bucket].End(); itr++)
	{
		if ( Hasher.Equals((*itr).First, _key) )
		{
			(*itr).Second = _value;
			return (*itr).Second;
		}
	}

	Map[bucket].PushBack( ZPair<K, V>(_key, _value) );
	ElementCount++;

	return Map[bucket].Back().Second;
}

template <typename K, typename V, typename H>
void ZHashMap<K, V, H>::Remove(const K& _key)
{
	size_t bucket;
	typename ZList< ZPair<K, V> >::Iterator itr;
	
	bucket = GetBucket(Hasher.Hash(_key));

	for (itr = Map[bucket].Begin(); itr != Map[bucket].End(); itr++)
	{
		if ( Hasher.Equals((*itr).First, _key) )
		{
			Map[bucket].Erase(itr);
			ElementCount--;
			return;
		}
	}
}

template <typename K, typename V, typename H>
void ZHashMap<K, V, H>::SetBuckets( size_t _buckets )
{
	//Get a copy of the current map
	ZList< ZPair<K, V> > mappings = Mappings();

	//Clear the old map
	Clear();

	//Set our new bucket size
	Map.Resize(_buckets);

	//Hash all previous elements into the ZNew HashMap
	for (typename ZList< ZPair<K, V> >::Iterator itr = mappings.Begin(); itr != mappings.End(); itr ++)
		Put((*itr).First, (*itr).Second);
}

template <typename K, typename V, typename H>
size_t ZHashMap<K, V, H>::Size() const
{
	return ElementCount;
}

template <typename K, typename V, typename H>
bool ZHashMap<K, V, H>::TryGet(const K& _key, V& _val) const
{
	size_t bucket;
	typename ZList< ZPair<K, V> >::Iterator itr;

	bucket = GetBucket(Hasher.Hash(_key));

	for (itr = Map[bucket].Begin(); itr != Map[bucket].End(); itr++)
	{
		if ( Hasher.Equals((*itr).First, _key) )
		{
			_val = (*itr).Second;
			return true;
		}
	}

	return false;
}

template <typename K, typename V, typename H>
V* ZHashMap<K, V, H>::TryGetPtr(const K& _key) const
{
	size_t bucket;
	typename ZList< ZPair<K, V> >::Iterator itr;

	bucket = GetBucket(Hasher.Hash(_key));

	for (itr = Map[bucket].Begin(); itr != Map[bucket].End(); itr++)
	{
		if ( Hasher.Equals((*itr).First, _key) )
		{
			return &(*itr).Second;
		}
	}

	return NULL;
}

template <typename K, typename V, typename H>
ZList<V> ZHashMap<K, V, H>::Values() const
{
	ZList<V> values;

	size_t bucket;
	typename ZList< ZPair<K, V> >::Iterator itr;

	for (bucket = 0; bucket < Map.Size(); bucket++)
	{
		for (itr = Map[bucket].Begin(); itr != Map[bucket].End(); itr++)
		{
			values.PushBack((*itr).Second);
		}
	}

	return values;
}

#endif

