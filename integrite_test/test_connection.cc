#include <iostream>
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "geco_application.h"
#include "geco-net-plugin.h"

#if GTEST_OS_WINDOWS_MOBILE
# include <tchar.h>  // NOLINT
GTEST_API_ int _tmain(int argc, TCHAR** argv) {
#else
GTEST_API_ int main(int argc, char** argv)
{
#endif  // GTEST_OS_WINDOWS_MOBILE

    network_plugin_t plugin;
    network_application_t* server = network_application_t::GetInstance();
    server->SetSleepTime(5);
    server->SetIncomingConnectionsPasswd("admin", (int)strlen("admin"));
    server->SetBannedRemoteSystem("202.168.1.123", 100);
    server->SetBannedRemoteSystem("202.168.1", 10000);
    server->SetBannedRemoteSystem("202.168.1", 1000);
    server->SetBannedRemoteSystem("192.168.0.168", 0);
    server->SetPlugin(&plugin);

#if ENABLE_SECURE_HAND_SHAKE==1
    {
        std::cout << "Generating server public and private keys...\n";
        //cat::TunnelTLS* m_tls;
        //cat::SlowTLS::ref()->Find(m_tls);
        cat::TunnelKeyPair key_pair;
        //key_pair.Generate(m_tls);
        bool ret = key_pair.LoadFile("server_key.bin");
        while (!ret)
        {
            cat::EasyHandshake* handshake = cat::EasyHandshake::ref();
            if (!handshake->GenerateServerKey(key_pair) && handshake->GetTLS()->Valid())
            {
                std::cout << "FAILURE: Unable to generate key pair\n";
                exit(1);
            }
            key_pair.SaveFile("server_key.bin");
            ret = key_pair.LoadFile("server_key.bin");
        }
        server->EnableSecureIncomingConnections(key_pair, false);
    }
#endif
    /// default blobking
    socket_binding_params_t socketDescriptor("localhost", 38000);
    server->Start(&socketDescriptor);

    network_packet_t* packet;
    Command* c;
    while (1)
    {
        // This sleep keeps RakNet responsive
        for (packet = server->GetPacketOnce(); packet != 0;
            server->ReclaimPacket(packet), packet = server->GetPacketOnce())
        {
            /// user logics goes here
            c = server->AllocCommand();
            c->commandID = Command::BCS_SEND;
            server->PostComand(c);
        }
    }

    server->StopRecvThread();
    server->StopNetworkUpdateThread();
    network_application_t::DestroyInstance(server);
    return 0;
}