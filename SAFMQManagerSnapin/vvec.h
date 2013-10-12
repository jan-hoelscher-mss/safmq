/*
 Copyright 2005 Matthew J. Battey

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software distributed
	under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
	CONDITIONS OF ANY KIND, either express or implied. See the License for the
	specific language governing permissions and limitations under the License.




This software implements a platform independent Store and Forward Message Queue.
*/
#if !defined(_VVEC_H_)
#define _VVEC_H_

template<class _type, int _vt_type>
class VariantVector : public VARIANT
{
private:
	_type	*ptr;
	bool	release;
	bool	locked;

public:
	VariantVector(int size, bool autoRelease = true) {
		this->vt = _vt_type | VT_ARRAY;

		SAFEARRAYBOUND	bnds;

		bnds.cElements = size;
		bnds.lLbound = 0;

		parray = ::SafeArrayCreate(_vt_type, 1, &bnds);
		release = autoRelease;
		locked = false;
		ptr = NULL;		
	}

	VariantVector(VARIANT& src, bool takeOwnership = false) {
		if (!(src.vt & VT_ARRAY)) 
			throw -1;

		vt = src.vt;
		parray = src.parray;
		locked = false;
		release = takeOwnership;
	}

	virtual ~VariantVector() {
		if (locked)
			unlock();

		if (release)
			::SafeArrayDestroy(parray);
	}

	long getBounds() {
		long	bounds;
		::SafeArrayGetUBound(parray, 1, &bounds);
		return bounds + 1; // add 1 for the zero based array
	}

	void lock() {
		::SafeArrayAccessData(parray, (void HUGEP**)&ptr);
		locked = true;
	}

	void unlock() {
		::SafeArrayUnaccessData(parray);
		locked = false;
		ptr = NULL;
	}

	const _type& operator[](int idx) const {
		return ptr[idx];
	}

	_type& operator[](int idx) {
		return ptr[idx];
	}

	_type* data() {
		return ptr;
	}
};

#endif
