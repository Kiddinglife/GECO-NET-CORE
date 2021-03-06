/*
	Copyright (c) 2011-2012 Christopher A. Taylor.  All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

	* Redistributions of source code must retain the above copyright notice,
	  this list of conditions and the following disclaimer.
	* Redistributions in binary form must reproduce the above copyright notice,
	  this list of conditions and the following disclaimer in the documentation
	  and/or other materials provided with the distribution.
	* Neither the name of LibCat nor the names of its contributors may be used
	  to endorse or promote products derived from this software without
	  specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
	ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef CAT_TUNNEL_TLS_HPP
#define CAT_TUNNEL_TLS_HPP

#include <cat/math/BigTwistedEdwards.hpp>
#include <cat/crypt/rand/Fortuna.hpp>
#include <cat/threads/Thread.hpp>

namespace cat {


// Thread-local-storage (TLS) for Tunnel
class TunnelTLS : public ITLS
{
	BigTwistedEdwards *_math;
	FortunaOutput *_csprng;

public:
	TunnelTLS();
	CAT_INLINE virtual ~TunnelTLS() {}

	static const char *GetNameString() { return "TunnelTLS"; }

	CAT_INLINE bool Valid() { return _csprng && _math; }
	CAT_INLINE BigTwistedEdwards *Math() { return _math; }
	CAT_INLINE FortunaOutput *CSPRNG() { return _csprng; }

	bool OnInitialize();
	void OnFinalize();
};


} // namespace cat

#endif // CAT_TUNNEL_TLS_HPP
