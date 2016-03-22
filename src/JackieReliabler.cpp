#include "JackieReliabler.h"
#include "JackieApplication.h"
#include <iostream>
using namespace geco::net;

JackieReliabler::JackieReliabler() { }
JackieReliabler::~JackieReliabler() { }

void JackieReliabler::ApplyNetworkSimulator(double _packetloss, unsigned short _minExtraPing, unsigned short _extraPingVariance)
{
    std::cout << " JackieReliabler::ApplyNetworkSimulator is not implemented.";
}

bool JackieReliabler::ProcessOneConnectedRecvParams(JackieApplication* serverApp, JISRecvParams* recvParams, unsigned mtuSize)
{
    std::cout << " JackieReliabler::ProcessOneConnectedRecvParams is not implemented.";
    return true;
}

void JackieReliabler::Reset(bool param1, int MTUSize, bool client_has_security)
{
    std::cout << " JackieReliabler::Reset is not implemented.";
}

void JackieReliabler::SetSplitMessageProgressInterval(int splitMessageProgressInterval)
{
    std::cout << " JackieReliabler::SetSplitMessageProgressInterval is not implemented.";
}

void JackieReliabler::SetUnreliableTimeout(TimeMS unreliableTimeout)
{
    std::cout << " JackieReliabler::SetUnreliableTimeout is not implemented.";
}

void JackieReliabler::SetTimeoutTime(TimeMS defaultTimeoutTime)
{
    std::cout << " JackieReliabler::SetTimeoutTime is not implemented.";
}

bool JackieReliabler::Send(ReliableSendParams& sendParams)
{
    //remoteSystemList[sendList[sendListIndex]].reliabilityLayer.Send(data, numberOfBitsToSend, priority, reliability, orderingChannel, useData == false, remoteSystemList[sendList[sendListIndex]].MTUSize, currentTime, receipt);
    std::cout <<
        "Not implemented JackieReliabler::Send(ReliableSendParams& sendParams)";
    return true;
}


