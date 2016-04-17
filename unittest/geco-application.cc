/*
 * geco-application.cc
 *
 *  Created on: 12 Apr 2016
 *      Author: jakez
 */

#include "gtest/gtest.h"
#include "geco-msg-ids.h"
#include "geco-secure-hand-shake.h"
#include "network_socket_t.h"
#include "geco-net-type.h"
using namespace geco::net;
static const unsigned char OFFLINE_MESSAGE_DATA_ID[16] =
{ 0x00, 0xFF, 0xFF, 0x00, 0xFE, 0xFE, 0xFE, 0xFE, 0xFD, 0xFD, 0xFD, 0xFD, 0x12,
0x34, 0x56, 0x78 };
TEST(IsDomainIPAddrTest, when_given_numberic_addr_return_false)
{
    const char* host_Num = "192.168.1.168";
    EXPECT_FALSE(isDomainIPAddr(host_Num));
}

TEST(IsDomainIPAddrTest, when_given_domain_addr_return_true)
{
    const char* host_domain = "www.baidu.com";
    EXPECT_TRUE(isDomainIPAddr(host_domain));
}

TEST(ItoaTest, when_given_positive_and_nagative_integers_return_correct_string)
{
    char result[8];

    Itoa(12, result, 10);
    EXPECT_STREQ("12", result);

    Itoa(-12, result, 10);
    EXPECT_STREQ("-12", result);

    Itoa(-12.5, result, 10);
    EXPECT_STREQ("-12", result);
}

TEST(DomainNameToIPTest, when_given_localhost_string_return_127001)
{
    char result[65] = { 0 };
    DomainNameToIP("localhost", result);
    EXPECT_STREQ("127.0.0.1", result);
    printf("localhost ip addr ('%s')\n", result);
}

TEST(DomainNameToIPTest, when_given_hostname_return_bound_ip_for_that_nic)
{
    char result[65] = { 0 };
    DomainNameToIP("DESKTOP-E2KL25B", result);
    //EXPECT_STREQ("192.168.56.1", result);
    printf("hostname ip addr ('%s')\n", result);
}

TEST(DomainNameToIPTest, when_given_numberic_addr_return_same_ip_addr)
{
    char result[65] = { 0 };
    DomainNameToIP("192.168.2.5", result);
    EXPECT_STREQ("192.168.2.5", result);
}

TEST(DomainNameToIPTest, when_given_external_domain_return_correct_ip_addr)
{
    char result[65] = { 0 };
    DomainNameToIP("www.baidu.com", result);
    printf("baidu ip addr ('%s')\n", result);
}

TEST(NetworkAddressTests, test_NetworkAddress_size_equals_7)
{
    EXPECT_EQ(7, network_address_t::size());
}

TEST(NetworkAddressTests, TestToHashCode)
{
    network_address_t addr3("localhost", 32000);
    printf("hash code for addr '%s' is %ld\n'", addr3.ToString(), network_address_t::ToHashCode(addr3));

    network_address_t addr4("192.168.56.1", 32000);
    printf("hash code for addr '%s' is %ld\n'", addr4.ToString(), network_address_t::ToHashCode(addr4));
}

/// usually seprate the ip addr and port number and you will ne fine
TEST(NetworkAddressTests, TestCtorToStringFromString)
{
    network_address_t default_ctor_addr;
    const char* str1 = default_ctor_addr.ToString();

    network_address_t param_ctor_addr_localhost("localhost", 12345);
    const char* str2 = param_ctor_addr_localhost.ToString();
    EXPECT_STREQ("127.0.0.1|12345", str2);

    // THIS IS WRONG, so when you use domain name, you have to seprate two-params ctor
    //JACKIE_INET_Address param_ctor_addr_domain("ZMD-SERVER:1234");

    // If you have multiple ip address bound on hostname, this will return the first one,
    // so sometimes, it will not be the one you want to use, so better way is to assign the ip address
    // manually.
    network_address_t param_ctor_addr_domain("DESKTOP-E2KL25B", 1234);
    const char* str3 = param_ctor_addr_domain.ToString();
    //EXPECT_STREQ("192.168.56.1|1234", str3);
}

static void test_superfastfunction_func()
{
    std::cout << "\nGlobalFunctions_h::test_superfastfunction_func() starts...\n";
    char* name = "jackie";
    std::cout << "name hash code = " << (name, strlen(name) + 1, strlen(name) + 1);
}


TEST(NetworkAddressTests, SetToLoopBack_when_given_ip4_return_ip4_loopback)
{
    network_address_t addr("192.168.1.108", 12345);
    addr.SetToLoopBack(4);
    EXPECT_STREQ("127.0.0.1|12345", addr.ToString());
}

// this function will not work if you do not define NET_SUPPORT_IP6 MACRO
TEST(NetworkAddressTests, SetToLoopBack_when_given_ip6_return_ip6_loopback)
{
    network_address_t addr("192.168.1.108", 12345);
    addr.SetToLoopBack(6);
    EXPECT_STREQ("192.168.1.108|12345", addr.ToString());
}
TEST(NetworkAddressTests, IsLANAddress_when_given_non_localhost_return_false)
{
    network_address_t addr("192.168.1.108", 12345);
    EXPECT_TRUE(addr.IsLANAddress()) << " 192.168.1.108";
}
TEST(JackieGUIDTests, ToUInt32_)
{
    guid_t gui(12);
    EXPECT_STREQ("12", gui.ToString());
    EXPECT_EQ(12, guid_t::ToUInt32(gui));
}
TEST(JackieGUIDTests, TestToString)
{
    EXPECT_STREQ("JACKIE_INet_GUID_Null", JACKIE_NULL_GUID.ToString());
    guid_t gui(12);
    EXPECT_STREQ("12", gui.ToString());
}
TEST(JackieGUIDTests, TestToHashCode)
{
    guid_address_wrapper_t wrapper;
    EXPECT_STREQ("204.204.204.204|52428", wrapper.ToString());
    EXPECT_EQ(-395420190, guid_address_wrapper_t::ToHashCode(wrapper));

    guid_t gui(12);
    guid_address_wrapper_t wrapper1(gui);
    EXPECT_STREQ("12", wrapper1.ToString());
    EXPECT_EQ(12, guid_address_wrapper_t::ToHashCode(wrapper1));

    network_address_t adrr("localhost", 12345);
    guid_address_wrapper_t wrapper2(adrr);
    EXPECT_STREQ("127.0.0.1|12345", wrapper2.ToString());
    EXPECT_EQ(network_address_t::ToHashCode(adrr), guid_address_wrapper_t::ToHashCode(wrapper2));
}
TEST(NetTimeTests, test_gettimeofday)
{
    time_t rawtime;
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
    rawtime = tv.tv_sec;

    char buffer[128];
    struct tm * timeinfo;
    timeinfo = localtime(&rawtime);
    strftime(buffer, 128, "%x %X", timeinfo);

    char buff[32];
    sprintf(buff, ":%i", tv.tv_usec / 1000);
    strcat(buffer, buff);

    printf("JackieGettimeofday(%s)\n", buffer);

    printf("GetTime(%i)\n", GetTimeMS());
    GecoSleep(1000);
    printf("GetTime(%i)\n", GetTimeMS());
}
TEST(JISBerkleyTests, test_GetMyIPBerkley)
{
    network_address_t addr[MAX_COUNT_LOCAL_IP_ADDR];
    berkley_socket_t::GetMyIPBerkley(addr);
    for (int i = 0; i < MAX_COUNT_LOCAL_IP_ADDR; i++)
    {
        printf("(%s)\n", addr[i].ToString());
    }
}

