#ifndef GECO_INET_SOCKET_H_
#define GECO_INET_SOCKET_H_

#include "geco-export.h"
#include "JACKIE_Atomic.h"
#include "JACKIE_Thread.h"
#include "geco-time.h"
#include "geco-net-type.h"
#include "geco-globals.h" // only for JackieSleep() global function
#include "geco-malloc-interface.h"
#include "geco-net-config.h"
#include <cassert>

// #define TEST_NATIVE_CLIENT_ON_WINDOWS
#ifdef TEST_NATIVE_CLIENT_ON_WINDOWS
#define __native_client__
typedef int PP_Resource;
#endif

GECO_NET_BEGIN_NSPACE

/// JIS is short name for GECO_INet_Socket 
class  JackieINetSocket;
struct JIS_BerkleyBindParams;
struct JIS_SendParams;
class IServerApplication;
class JackieApplication;

typedef int JISSocket;
typedef int JISSendResult;
typedef int JISRecvResult;

enum  JISBindResult
{
    JISBindResult_SUCCESS = 0,
    JISBindResult_REQUIRES_NET_SUPPORT_IPV6_DEFINED,
    JISBindResult_FAILED_BIND_SOCKET,
    JISBindResult_FAILED_SEND_RECV_TEST,
};
enum  JISType
{
    JISType_WINDOWS_STORE_8 = 0,
    JISType_PS3,
    JISType_PS4,
    JISType_CHROME,
    JISType_VITA,
    JISType_XBOX_360,
    JISType_XBOX_720,
    JISType_WINDOWS,
    JISType_LINUX
};

GECO_EXPORT extern const char* JISTypeToString(JISType reason);
GECO_EXPORT extern const char* JISBindResultToString(JISBindResult reason);

struct GECO_EXPORT JISSendParams
{
    char *data;
    int length;
    JISSendResult bytesWritten; // use 0 to init
    InetAddress receiverINetAddress;
    int ttl;
};

struct GECO_EXPORT ReliableSendParams
{
    bool broadcast;
    union{ bool useCallerDataAllocation; bool makeDataCopy; };
    char *data;
    char orderingChannel;
    BitSize bitsSize; // bits
    PacketSendPriority sendPriority;
    PacketReliability packetReliability;
    JackieAddressGuidWrapper receiverAdress;
    TimeUS currentTime;
    UInt32 receipt;
    UInt16 mtu;
};

struct GECO_EXPORT JISRecvParams
{
    char data[MAXIMUM_MTU_SIZE];
    JISRecvResult bytesRead;
    InetAddress senderINetAddress;
    TimeUS timeRead;
    JackieINetSocket *localBoundSocket;
};

class GECO_EXPORT JISEventHandler
{
    public:
    virtual ~JISEventHandler() { }
    virtual void ReclaimOneJISRecvParams(JISRecvParams *s, UInt32 index) = 0;
    virtual JISRecvParams *AllocJISRecvParams(UInt32 deAlloclJISRecvParamsQIndex) = 0;
};

class GECO_EXPORT JISAllocator
{
    public:
    static JackieINetSocket* AllocJIS(void);
    static void DeallocJIS(JackieINetSocket *s);
};

class GECO_EXPORT JackieINetSocket
{
    protected:
    IServerApplication *eventHandler;
    InetAddress boundAddress;
    unsigned int userConnectionSocketIndex;
    JISType socketType;

    public:
    JackieINetSocket() : eventHandler(0) { }
    virtual ~JackieINetSocket() { }

    virtual  JISSendResult Send(JISSendParams *sendParameters,
        const char *file, unsigned int line) = 0;

    // In order for the handler to trigger, some platforms must call PollRecvFrom, 
    // some platforms this create an internal thread.
    inline void SetRecvEventHandler(IServerApplication *_eventHandler) { eventHandler = _eventHandler; }
    inline IServerApplication * GetEventHandler(void) const { return eventHandler; }

    inline JISType GetSocketType(void) const { return socketType; }
    inline void SetSocketType(JISType t) { socketType = t; }

    inline unsigned int GetUserConnectionSocketIndex(void) const { return userConnectionSocketIndex; }
    inline void SetUserConnectionSocketIndex(unsigned int i) { userConnectionSocketIndex = i; }

    inline InetAddress GetBoundAddress(void) const { return boundAddress; }

    inline bool IsBerkleySocket(void) const
    {
        return socketType != JISType_CHROME && socketType != JISType_WINDOWS_STORE_8;
    }
    static void GetMyIP(InetAddress addresses[MAX_COUNT_LOCAL_IP_ADDR]);

    inline static void DomainNameToIP(const char *domainName, char ip[65])
    {
        DomainNameToIP(domainName, ip);
    }

    /// Text-print the intenal memebers in this class
    virtual void Print(void);
};

#if defined (WINDOWS_STORE_RT) 	/// We are using WINDOWS_STORE_RT plateform
//@TODO
#elif defined (__native_client__)  	/// We are using NaCI plateform
//@TODO
#else 	/// We are using Wins or LINUX or Unix plateform
struct GECO_EXPORT JISBerkleyBindParams
{
    // Input parameters
    unsigned short port;
    char *hostAddress; // must be number ip adress, using 'localhost' is wrong
    unsigned short addressFamily; // AF_INET or AF_INET6
    int type; // SOCK_DGRAM
    int protocol; // 0
    bool isNonBlocking;
    int isBroadcast;
    int setIPHdrIncl;
    int doNotFragment;
    int pollingThreadPriority;
    JackieApplication *eventHandler;
    unsigned short remotePortJackieNetWasStartedOn_PS3_PS4_PSP2;
};

class GECO_EXPORT GECO_ISocketTransceiver
{
    public:

    GECO_ISocketTransceiver() { }
    virtual ~GECO_ISocketTransceiver() { }

    /// Called when SendTo would otherwise occur.
    virtual int JackieINetSendTo(const char *data, int length, const InetAddress &systemAddress) = 0;

    /// Called when RecvFrom would otherwise occur. 
    /// Return number of bytes read and Write data into dataOut
    /// Return -1 to use JackieNet's normal recvfrom, 0 to abort JackieNet's normal 
    /// recvfrom,and positive to return data
    virtual int JackieINetRecvFrom(char dataOut[MAXIMUM_MTU_SIZE], InetAddress *senderOut, bool calledFromMainThread) = 0;

    /// RakNet needs to know whether an address is a dummy override address, 
    /// so it won't be added as an external addresses
    virtual bool IsFork(const InetAddress &systemAddress) const = 0;
};

class GECO_EXPORT JISBerkley : public JackieINetSocket
{
    protected:

    JISBerkleyBindParams binding;
    GECO_ISocketTransceiver *jst;
    JackieAtomicLong isRecvFromLoopThreadActive;

    JISSocket rns2Socket;
#if defined(__APPLE__)
    // http://sourceforge.net/p/open-dis/discussion/683284/thread/0929d6a0
    CFSocketRef             _cfSocket;
#endif

    public:
    JISBerkley();
    virtual ~JISBerkley();

    const JISBerkleyBindParams *GetBindingParams(void) const { return &binding; }
    inline JISSocket GetSocket(void) const { return rns2Socket; }

    inline void SetSocketTransceiver(GECO_ISocketTransceiver *jst_) { this->jst = jst_; }
    inline GECO_ISocketTransceiver* GetSocketTransceiver(void) const { return this->jst; }

    //////////////////////////////////////////////////////////////////////////
    /// 1. Expecially, this function will internally test if the binding really works 
    /// 2. by sending a char to itself and receive it, only if send and receive both succeed, 
    /// 3. binded is finally considered as successful
    //////////////////////////////////////////////////////////////////////////
    JISBindResult Bind(JISBerkleyBindParams *bindParameters, const char *file, unsigned int line);
    JISBindResult BindShared(JISBerkleyBindParams *bindParameters, const char *file, unsigned int line);
    JISBindResult BindSharedIPV4(JISBerkleyBindParams *bindParameters, const char *file, unsigned int line);
    JISBindResult BindSharedIPV4And6(JISBerkleyBindParams *bindParameters, const char *file, unsigned int line);

    //////////////////////////////////////////////////////////////////////////
    /// 1. Used internally in @mtd JISBindResult Bind(...)
    /// 2. set nonblocking to 0 = blocking-socket; 
    /// 3. set nonblocking  to 1 = nonblocking-socket; 
    /// 4. setsockopt() will always return 0 if succeed otherwise return < 0
    /// 5. do not support reuse addr which means you cannot to a in-use port
    //////////////////////////////////////////////////////////////////////////
    inline void SetSocketOptions(void)
    {
        int returnVal = SOCKET_ERROR;

        /// do not support reuse addr which means you cannot to a in - use port， only in this way,
        /// detecting id a port is in use can be acheieved
        //returnVal = setsockopt(rns2Socket, SOL_SOCKET, SO_REUSEADDR, (const char*) &returnVal, sizeof(returnVal));
        //assert(returnVal == 0);

        // This doubles the max throughput rate
        returnVal = GECO_SO_REVBUF_SIZE;
        returnVal = setsockopt__(rns2Socket, SOL_SOCKET, SO_RCVBUF, (char *)& returnVal, sizeof(returnVal));
        assert(returnVal == 0);

        // This doesn't make much difference: 10% maybe Not supported on console 2
        returnVal = GECO_SO_SNDBUF_SIZE;
        returnVal = setsockopt__(rns2Socket, SOL_SOCKET, SO_SNDBUF, (char *)& returnVal, sizeof(returnVal));
        assert(returnVal == 0);

        // Immediate fore-close with ignoring the buffered sending data. 
        // voice, xbox and windows's SOCK_DGRAM does not support 
        // SO_DONTLINGER, SO_KEEPALIVE, SO_LINGER and SO_OOBINLINE
        returnVal = 0;
        returnVal = setsockopt__(rns2Socket, SOL_SOCKET, SO_LINGER,
            (char *)& returnVal, sizeof(returnVal));
        //assert(returnVal == 0);

    }
    inline void SetNonBlockingSocket(unsigned long nonblocking)
    {
        int res;
#ifdef _WIN32
        res = ioctlsocket__(rns2Socket, FIONBIO, &nonblocking);
#else
        if( nonblocking > 0 )
        {
            int flags = fcntl(rns2Socket, F_GETFL, 0);
            res = fcntl(rns2Socket, F_SETFL, flags | O_NONBLOCK); // setup to non-blocking
        }
        else
            res = fcntl(rns2Socket, F_SETFL, 0); // setup to blocking
#endif
        assert(res == 0);
    }
    inline void SetBroadcastSocket(int broadcast)
    {
        assert(setsockopt__(rns2Socket, SOL_SOCKET, SO_BROADCAST,
            (char *)& broadcast, sizeof(broadcast)) == 0);
    }
    inline void SetIPHdrIncl(int ipHdrIncl)
    {
        int val = setsockopt__(rns2Socket, IPPROTO_IP, IP_HDRINCL,
            (char*)&ipHdrIncl, sizeof(ipHdrIncl));
        //        val = setsockopt__(rns2Socket, IPPROTO_IP, SO_DONTLINGER,
        //            (const char*)&ipHdrIncl, sizeof(ipHdrIncl));
        /// this assert always fail maybe need admin permission
        /// assert(val == 0);
    }
    inline void SetDoNotFragment(int opt)
    {
#if defined( IP_DONTFRAGMENT )
#if defined(_WIN32) && !defined(_DEBUG)
        // If this assert hit you improperly linked against WSock32.h
        assert(IP_DONTFRAGMENT == 14);
#endif
        opt = setsockopt__(rns2Socket, boundAddress.GetIPProtocol(), IP_DONTFRAGMENT,
            (char *)& opt, sizeof(opt));
        assert(opt == 0);
#endif
    }

    //////////////////////////////////////////////////////////////////////////
    /// 1. return value == 0 - sender sends us an msg with length 0 for some reason
    /// 2. return value < 0 - other error occurs (EWOUDBLOCK exlusive) in recvform()
    /// 3. returnvalue  > 0 - size of read bytes
    /// 4. RecvFromLoopInt() will autimatically use the block mode setup in @mem binding
    //////////////////////////////////////////////////////////////////////////
    unsigned int RecvFromLoopInt(void);
    JISRecvResult RecvFrom(JISRecvParams *recvFromStruct);
    JISRecvResult RecvFromIPV4(JISRecvParams *recvFromStruct);
    JISRecvResult RecvFromIPV4And6(JISRecvParams *recvFromStruct);

    //////////////////////////////////////////////////////////////////////////
    /// 1. send by jst if not null, otherwise by @mtd SendWithoutVDP(...)
    /// 2. Returns value is either > 0(send succeeds) or <0 (send error)
    /// 3. ! WILL NEVER return 0 because we never send empty msg to the receiver
    //////////////////////////////////////////////////////////////////////////
    virtual JISSendResult Send(JISSendParams *sendParameters, const char *file, unsigned int line) override;
    JISSendResult SendWithoutVDP(JISSocket rns2Socket, JISSendParams *sendParameters, const char *file, unsigned int line);

    /// Constructor not called at this monment !
    //friend GECO_THREAD_DECLARATION(RunRecvCycleLoop);
    //friend GECO_THREAD_DECLARATION(RunSendCycleLoop);

    static bool IsPortInUse(unsigned short port, const char *hostAddress, unsigned short addressFamily, int type);

    //////////////////////////////////////////////////////////////////////////
    /// 1. use gethostname(...) to get host name like mengdizhang,
    /// 2. use this host name in gethostbyname(...) to het a list of avaiable ip addrs
    /// 3. if addrs num < MAX_COUNT_LOCAL_IP_ADDR, the rest of them will be filled with empty addr
    //////////////////////////////////////////////////////////////////////////
    static void GetMyIPBerkley(InetAddress addresses[MAX_COUNT_LOCAL_IP_ADDR]);
    static void GetMyIPBerkleyV4V6(InetAddress addresses[MAX_COUNT_LOCAL_IP_ADDR]);
    static void GetMyIPBerkleyV4(InetAddress addresses[MAX_COUNT_LOCAL_IP_ADDR]);

    virtual void Print(void);

    protected:
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// 1. Notice that you can only call this function after calling Bind(...) to associate @mem rns2Socket with @mem systemAddressOut
    /// 2. Individually call this function will get unkonwn error, so they are marked as protected functions only for internal use
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    static void GetSystemAddressViaJISSocket(JISSocket rns2Socket, InetAddress *systemAddressOut);
    static void GetSystemAddressViaJISSocketIPV4(JISSocket rns2Socket, InetAddress *systemAddressOut);
    static void GetSystemAddressViaJISSocketIPV4And6(JISSocket rns2Socket, InetAddress *systemAddressOut);
};
#endif

GECO_NET_END_NSPACE
#endif

