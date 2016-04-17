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
    network_application_t* server = network_application_t::get_instance();
    server->set_sleep_time(5);
    server->set_inbound_connection_pwd("admin", (int)strlen("admin"));
    server->ban_remote_system("202.168.1.123", 100);
    server->ban_remote_system("202.168.1", 10000);
    server->ban_remote_system("202.168.1", 1000);
    server->ban_remote_system("192.168.0.168", 0);
    server->set_plugin(&plugin);

#if ENABLE_SECURE_HAND_SHAKE==1
    {
        //std::cout << "Generating server public and private keys...\n";
        cat::TunnelKeyPair key_pair;
        bool ret = key_pair.LoadFile("../../server_key.bin");
        while (!ret)
        {
            //std::cout << "generrate keys bin file\n";
            //// you must call this first to generte jeys otherwise will fail to gernerate keys
            if (!cat::EasyHandshake::ref()->GetTLS()->Valid())
                cat::EasyHandshake::ref()->GetTLS()->OnInitialize();

            if (!cat::EasyHandshake::ref()->GenerateServerKey(key_pair))
            {
                std::cerr << "FAILURE: Unable to generate key pair\n";
            }
            key_pair.SaveFile("../../server_key.bin");
            ret = key_pair.LoadFile("../../server_key.bin");
        }
        server->enable_secure_inbound_connections(key_pair, false);
    }
#endif
    /// default blobking
    socket_binding_params_t socketDescriptor("localhost", 38000);
    server->startup(&socketDescriptor);

    network_packet_t* packet;
    cmd_t* c;
    while (1)
    {
        // This sleep keeps RakNet responsive
        for (packet = server->fetch_packet(); packet != 0;
            server->reclaim_packet(packet), packet = server->fetch_packet())
        {
            
            /// user logics goes here
            c = server->alloc_cmd();
            c->commandID = cmd_t::BCS_SEND;
           server->run_cmd(c);
        }
    }

    server->stop_recv_thread();
    server->stop_network_update_thread();
    network_application_t::reclaim_instance(server);
    return 0;
}