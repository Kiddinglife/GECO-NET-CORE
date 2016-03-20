/*
* Copyright (c) 2016
* Geco Gaming Company
*
* Permission to use, copy, modify, distribute and sell this software
* and its documentation for GECO purpose is hereby granted without fee,
* provided that the above copyright notice appear in all copies and
* that both that copyright notice and this permission notice appear
* in supporting documentation. Geco Gaming makes no
* representations about the suitability of this software for GECO
* purpose.  It is provided "as is" without express or implied warranty.
*
*/

// created on 20-March-2016 by Jackie Zhang


/// \file
///


#include "geco-secure-hand-shake.h"

#if ENABLE_SECURE_HAND_SHAKE==1

// If building a RakNet DLL, be sure to tweak the CAT_EXPORT macro meaning
#if !defined(JACIE_HAS_STATIC_LIB) && defined(JACIE_HAS_DYNAMIC_LIB)
# define CAT_BUILD_DLL
#else
# define CAT_NEUTER_EXPORT
#endif

#include "cat/src/port/EndianNeutral.cpp"
#include "cat/src/port/AlignedAlloc.cpp"
#include "cat/src/time/Clock.cpp"
#include "cat/src/threads/Mutex.cpp"
#include "cat/src/threads/Thread.cpp"
#include "cat/src/threads/WaitableFlag.cpp"
#include "cat/src/hash/MurmurHash2.cpp"
#include "cat/src/lang/Strings.cpp"

#include "cat/src/math/BigRTL.cpp"
#include "cat/src/math/BigPseudoMersenne.cpp"
#include "cat/src/math/BigTwistedEdwards.cpp"

#include "cat/src/crypt/SecureCompare.cpp"
#include "cat/src/crypt/cookie/CookieJar.cpp"
#include "cat/src/crypt/hash/HMAC_MD5.cpp"
#include "cat/src/crypt/privatekey/ChaCha.cpp"
#include "cat/src/crypt/hash/Skein.cpp"
#include "cat/src/crypt/hash/Skein256.cpp"
#include "cat/src/crypt/hash/Skein512.cpp"
#include "cat/src/crypt/pass/Passwords.cpp"

#include "cat/src/crypt/rand/EntropyWindows.cpp"
#include "cat/src/crypt/rand/EntropyLinux.cpp"
#include "cat/src/crypt/rand/EntropyWindowsCE.cpp"
#include "cat/src/crypt/rand/EntropyGeneric.cpp"
#include "cat/src/crypt/rand/Fortuna.cpp"

#include "cat/src/crypt/tunnel/KeyAgreement.cpp"
#include "cat/src/crypt/tunnel/AuthenticatedEncryption.cpp"
#include "cat/src/crypt/tunnel/KeyAgreementInitiator.cpp"
#include "cat/src/crypt/tunnel/KeyAgreementResponder.cpp"
#include "cat/src/crypt/tunnel/KeyMaker.cpp"

#include "cat/src/crypt/tunnel/EasyHandshake.cpp"

#endif // LIBCAT_SECURITY

