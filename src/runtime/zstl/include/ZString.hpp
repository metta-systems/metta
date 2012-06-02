/*
	ZBasicString.hpp
	Author: James Russell <jcrussell@762studios.com>
	Created: 9/12/2011

	Purpose: 

	String header, which will include string implementations and typedef
	the standard string implementation.

	License:

	This program is free software. It comes without any warranty, to
	the extent permitted by applicable law. You can redistribute it
	and/or modify it under the terms of the Do What The Fuck You Want
	To Public License, Version 2, as published by Sam Hocevar. See
	http://sam.zoy.org/wtfpl/COPYING for more details.

*/

#pragma once

#ifndef _ZSTRING_HPP
#define _ZSTRING_HPP

#include <ZSTL/ZBasicString.hpp>	//Dynamic ASCII String

//This typedef indicates what our base string type will be
typedef ZBasicString<> ZString;

#endif

