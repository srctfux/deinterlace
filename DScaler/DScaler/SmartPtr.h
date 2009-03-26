/////////////////////////////////////////////////////////////////////////////
// $Id$
/////////////////////////////////////////////////////////////////////////////
//
// DScaler changes:
// * changed static casts to dynamic to allow
//   more natural syntax with downcasting checks
// * Renamed from yasper::ptr to SmartPtr in line with coding style
//
/////////////////////////////////////////////////////////////////////////////


/*
 * yasper - A non-intrusive reference counted pointer.
 *	    Version: 1.04
 *			
 *  Many ideas borrowed from Yonat Sharon and
 *  Andrei Alexandrescu.
 *
 * (zlib license)
 * ----------------------------------------------------------------------------------	
 * Copyright (C) 2005-2007 Alex Rubinsteyn
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * -----------------------------------------------------------------------------------
 *
 * Send all questions, comments and bug reports to:
 * Alex Rubinsteyn (alex.rubinsteyn {at-nospam} gmail {dot} com)
 */


#ifndef _SMART_PTR_H
#define _SMART_PTR_H

#include <exception>

struct NullPointerException : public std::exception
{
    NullPointerException(): std::exception("[SmartPtr Exception] Attempted to use NULL pointer") {}
};

struct Counter
{
	Counter(unsigned c = 1) : count(c) {}
	unsigned count;
};

template <typename X>
class SmartPtr
{

public:
    typedef X element_type;

	/*
		SmartPtr needs to be its own friend so SmartPtr< X > and SmartPtr< Y > can access
		each other's private data members
	*/
	template <class Y> friend class SmartPtr;
	/*
		default constructor
			- don't create Counter
	*/
	SmartPtr() : rawPtr(0), counter(0) { }
	
	/*
		Construct from a raw pointer
	*/
	SmartPtr(X* raw, Counter* c = 0) : rawPtr(0), counter(0)
	{
		if (raw)
		{
			rawPtr = raw;
			if (c) acquire(c);
			else counter = new Counter;
		}
	}
	
	template <typename Y>
 	explicit SmartPtr(Y* raw, Counter* c = 0) : rawPtr(0), counter(0)
	{
		if (raw)
		{
			rawPtr = dynamic_cast<X*>( raw );
			if (c) acquire(c);
			else counter = new Counter;
		}
	}
	
	
	/*
		Copy constructor
	*/
	SmartPtr(const SmartPtr< X >& otherPtr)
	{
		acquire( otherPtr.counter );
		rawPtr = otherPtr.rawPtr;
	}
	
	template <typename Y>
	explicit SmartPtr(const SmartPtr< Y >& otherPtr) : rawPtr(0), counter(0)
	{
		acquire(otherPtr.counter);
		rawPtr = dynamic_cast<X*>( otherPtr.GetRawPointer());
	}
	

	/*
		Destructor
	*/
	~SmartPtr()
	{
		release();
	}

/*
	Assignment to another SmartPtr
*/

SmartPtr& operator=(const SmartPtr< X >& otherPtr)
{
	if (this != &otherPtr)
	{
		release();
		acquire(otherPtr.counter);
		rawPtr = otherPtr.rawPtr;
	}
	return *this;
}

template <typename Y>
SmartPtr& operator=(const SmartPtr< Y >& otherPtr)
{
	if ( this != (SmartPtr< X >*) &otherPtr )
	{
		release();
		acquire(otherPtr.counter);
		rawPtr = dynamic_cast<X*> (otherPtr.GetRawPointer());
	}
	return *this;
}

/*
	Assignment to raw pointers is really dangerous business.
	If the raw pointer is also being used elsewhere,
	we might prematurely delete it, causing much pain.
	Use sparingly/with caution.
*/

SmartPtr& operator=(X* raw)
{

	if (raw)
	{
		release();
		counter = new Counter;
		rawPtr = raw;
	}
	return *this;
}

template <typename Y>
SmartPtr& operator=(Y* raw)
{
	if (raw)
	{
		release();
		counter = new Counter;
		rawPtr = dynamic_cast<X*>(raw);
	}
	return *this;
}

/*
	assignment to long to allow SmartPtr< X > = NULL,
	also allows raw pointer assignment by conversion.
	Raw pointer assignment is really dangerous!
	If the raw pointer is being used elsewhere,
	it will get deleted prematurely.
*/
	SmartPtr& operator=(long num)
	{
		if (num == 0)  //pointer set to null
		{
			release();
		}

		else //assign raw pointer by conversion
		{
			release();
			counter = new Counter;
			rawPtr = reinterpret_cast<X*>(num);
		}	

		return *this;
	}

/*
	Member Access
*/
	X* operator->() const
	{
		return GetRawPointer();
	}


/*
	Dereference the pointer
*/
	X& operator* () const
	{
		return *GetRawPointer();
	}


/*
	Conversion/casting operators
*/


	operator BOOL() const
	{
		return IsValid();
	}

	
	/*
	   implicit casts to base types of the
	   the pointer we're storing
	*/
	
	template <typename Y>
	operator Y*() const
	{
		return dynamic_cast<Y*>(rawPtr);
	}

	template <typename Y>
	operator const Y*() const
	{
		return dynamic_cast<const Y*>(rawPtr);
	}

	template <typename Y>
	operator SmartPtr<Y>()
	{
		//new SmartPtr must also take our counter or else the reference counts
		//will go out of sync
		return SmartPtr<Y>(rawPtr, counter);
	}


/*
	Provide access to the raw pointer
*/

	X* GetRawPointer() const
	{
		if (rawPtr == 0) throw new NullPointerException;
		return rawPtr;
	}

	
/*
	Is there only one reference on the counter?
*/
	BOOL IsUnique() const
	{
		if (counter && counter->count == 1) return TRUE;
		return FALSE;
	}
	
	BOOL IsValid() const
	{
		if (counter && rawPtr) return TRUE;
		return FALSE;
	}

	unsigned GetCount() const
	{
		if (counter) return counter->count;
		return 0;
	}

private:
	X* rawPtr;

	Counter* counter;

	// increment the count
	void acquire(Counter* c)
	{
 		counter = c;
		if (c)
		{
			(c->count)++;
		}
	}

	// decrement the count, delete if it is 0
	void release()
	{
        if (counter)
		{			
			(counter->count)--; 	

			if (counter->count == 0)
			{
				delete counter;			
				delete rawPtr;
			}
		}
		counter = 0;
		rawPtr = 0;

	}
};


template <typename X, typename Y>
BOOL operator==(const SmartPtr< X >& lptr, const SmartPtr< Y >& rptr)
{
	return lptr.GetRawPointer() == rptr.GetRawPointer();
}

template <typename X, typename Y>
BOOL operator==(const SmartPtr< X >& lptr, Y* raw)
{
	return lptr.GetRawPointer() == raw ;
}

template <typename X>
BOOL operator==(const SmartPtr< X >& lptr, long num)
{
	if (num == 0 && !lptr.IsValid())  //both pointer and address are null
	{
		return TRUE;
	}

	else //convert num to a pointer, compare addresses
	{
		return lptr == reinterpret_cast<X*>(num);
	}
	
}

template <typename X, typename Y>
BOOL operator!=(const SmartPtr< X >& lptr, const SmartPtr< Y >& rptr)
{
	return ( !operator==(lptr, rptr) );
}

template <typename X, typename Y>
BOOL operator!=(const SmartPtr< X >& lptr, Y* raw)
{
	return ( !operator==(lptr, raw) );
}

template <typename X>
BOOL operator!=(const SmartPtr< X >& lptr, long num)
{
	return (!operator==(lptr, num) );
}

template <typename X>
BOOL operator!(const SmartPtr< X >& p)
{
	return (!p.IsValid());
}


/* less than comparisons for storage in containers */
template <typename X, typename Y>
BOOL operator< (const SmartPtr< X >& lptr, const SmartPtr < Y >& rptr)
{
	return lptr.GetRawPointer() < rptr.GetRawPointer();
}

template <typename X, typename Y>
BOOL operator< (const SmartPtr< X >& lptr, Y* raw)
{
	return lptr.GetRawPointer() < raw;
}

template <typename X, typename Y>
BOOL operator< (X* raw, const SmartPtr< Y >& rptr)
{
	return raw < rptr.GetRawPointer();
}

#endif

