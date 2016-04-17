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
    network_application_t* client = network_application_t::get_instance();
    network_plugin_t plugin;
    client->set_plugin(&plugin);

    // default use wild address and random port and blobking mode
    socket_binding_params_t socketDescriptor;
    if (client->startup(&socketDescriptor) == startup_result_t::START_SUCCEED)
    {

#if ENABLE_SECURE_HAND_SHAKE == 1
        {
            key_pair_t shsKeys;
            cat::TunnelPublicKey serverPublicKey;
            serverPublicKey.LoadFile("../../server_key.bin");
            shsKeys.remoteServerPublicKey = serverPublicKey;
            shsKeys.publicKeyMode = secure_connection_mode_t::USE_KNOWN_PUBLIC_KEY;
            char uname[] = "admin";
            connection_attempt_result_t connectResult = client->Connect("127.0.0.1", 38000, uname, sizeof(uname), &shsKeys);
            //ConnectionAttemptResult connectResult = client->Connect("127.0.0.1", 38000, uname, sizeof(uname));
            assert(connectResult == connection_attempt_result_t::CONNECTION_ATTEMPT_POSTED);
        }
#else
        assert(client->Connect("127.0.0.1", 38000, "root", strlen("root")) == ConnectionAttemptResult::CONNECTION_ATTEMPT_POSTED);
#endif

        unsigned int i;
        for (i = 0; i < client->GetLocalIPAddrCount(); i++)
        {
            std::cout << i + 1 << ". " << client->GetLocalIPAddr(i) << std::endl;
        }

        std::cout << " My GUID " << client->GetGuidFromSystemAddress(JACKIE_NULL_ADDRESS).g <<  std::endl;

        network_packet_t* packet = 0;

        //// Loop for input
        while (1)
        {
            for (packet = client->fetch_packet(); packet != 0;
                client->reclaim_packet(packet), packet = client->fetch_packet())
            {
                /// user logics goes here
                //Command* c = app->AllocCommand();
                //c->command = Command::BCS_SEND;
                //app->ExecuteComand(c);

            }
        }

        client->stop_recv_thread();
        client->stop_network_update_thread();
        network_application_t::reclaim_instance(client);
    }


    //#if ENABLE_SECURE_HAND_SHAKE==1
    //    {
    //        //std::cout << "Generating server public and private keys...\n";
    //        cat::TunnelKeyPair key_pair;
    //        bool ret = key_pair.LoadFile("../../server_key.bin");
    //        while (!ret)
    //        {
    //            //std::cout << "generrate keys bin file\n";
    //            //// you must call this first to generte jeys otherwise will fail to gernerate keys
    //            if (!cat::EasyHandshake::ref()->GetTLS()->Valid())
    //                cat::EasyHandshake::ref()->GetTLS()->OnInitialize();
    //
    //            if (!cat::EasyHandshake::ref()->GenerateServerKey(key_pair))
    //            {
    //                std::cerr << "FAILURE: Unable to generate key pair\n";
    //            }
    //            key_pair.SaveFile("../../server_key.bin");
    //            ret = key_pair.LoadFile("../../server_key.bin");
    //        }
    //        server->enable_secure_inbound_connections(key_pair, false);
    //    }
    //#endif
    //    /// default blobking
    //    socket_binding_params_t socketDescriptor("localhost", 38000);
    //    server->startup(&socketDescriptor);
    //
    //    network_packet_t* packet;
    //    cmd_t* c;
    //    while (1)
    //    {
    //        // This sleep keeps RakNet responsive
    //        for (packet = server->fetch_packet(); packet != 0;
    //            server->reclaim_packet(packet), packet = server->fetch_packet())
    //        {
    //            
    //            /// user logics goes here
    //            c = server->alloc_cmd();
    //            c->commandID = cmd_t::BCS_SEND;
    //           server->run_cmd(c);
    //        }
    //    }
    //
    //    server->stop_recv_thread();
    //    server->stop_network_update_thread();
    //    network_application_t::reclaim_instance(server);
    return 0;
}