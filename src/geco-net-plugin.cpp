#include "geco-net-plugin.h"

GECO_NET_BEGIN_NSPACE
JackieIPlugin::JackieIPlugin()
{
    serverApplication = 0;
#if JackieNet_SUPPORT_PacketizedTCP==1 && JackieNet_SUPPORT_TCPInterface==1
    tcpInterface = 0;
#endif
}
GECO_NET_END_NSPACE