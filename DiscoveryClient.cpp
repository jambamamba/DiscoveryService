#include <arpa/inet.h>
#include <ifaddrs.h>
#include <iostream>
#include <cstring>

#include "DiscoveryClient.h"
#include "Utils.h"

namespace  {

auto BroadcastIps()
{
    std::cout << "BroadcastIps";

    std::vector<sockaddr_in> addresses;
    struct ifaddrs *addrs,*tmp;

    int res = getifaddrs(&addrs);
    if(res == -1)
    {
        std::cout << "getifaddrs FAILED, errno: " << errno << ", " << strerror(errno);
        return addresses;
    }
    tmp = addrs;

    while (tmp)
    {
        if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_INET)
        {
            std::cout << "Interface name: " << tmp->ifa_name;
            struct sockaddr_in *ifa_addr = reinterpret_cast <sockaddr_in *>(tmp->ifa_addr);
            struct sockaddr_in *ifa_netmask = reinterpret_cast <sockaddr_in *>(tmp->ifa_netmask);

            if(ifa_addr && ifa_netmask)
            {
                uint32_t broadcastIp = (ifa_addr->sin_addr.s_addr | ~ ifa_netmask->sin_addr.s_addr);
                struct sockaddr_in ifa_broadcast;
                ifa_broadcast.sin_addr.s_addr = broadcastIp;
                std::cout << "Network Inteface: " << tmp->ifa_name
                                          << ", Address: " << inet_ntoa(ifa_addr->sin_addr)
                                          << ", Netmast: " << inet_ntoa(ifa_netmask->sin_addr)
                                          << ", Broadcast Address: " << inet_ntoa(ifa_broadcast.sin_addr);
                ifa_broadcast.sin_port = htons(SERVER_PORT);
                ifa_broadcast.sin_family = AF_INET;
                addresses.emplace_back(ifa_broadcast);
            }
        }

        tmp = tmp->ifa_next;
    }

    freeifaddrs(addrs);
    return addresses;
}
}//namespace

DiscoveryClient::DiscoveryClient(std::function<void (DiscoveryData *, std::string, uint16_t)> handleDiscoveryDatagram)
    : HandleDiscoveryDatagramCb(handleDiscoveryDatagram)
    , m_udp_socket(INADDR_ANY,
                sizeof(DiscoveryData),
                MAX_QUEUE_SIZE,
                "DiscoveryService",
                true,
                [this](const uint8_t *dataFromClient, size_t dataLen, const sockaddr_in &sa){
            ReadDatagrams(dataFromClient, dataLen, sa);
        })
{
    std::cout << "ctor ";
    Utils::ReadDeviceId(m_self_id);
}

DiscoveryClient::~DiscoveryClient()
{
    m_udp_socket.Stop();
}

void DiscoveryClient::Discover()
{
    std::cout << "Discover";
    for(auto client : BroadcastIps())
    {
        DiscoveryData data;
        m_udp_socket.SendToClient(client, reinterpret_cast<const char*>(&data));
        std::cout << "Broadcast message to " << inet_ntoa(client.sin_addr)
                                  << ":" << ntohs(client.sin_port) << "\n";
    }
}

void DiscoveryClient::ReadDatagrams(const uint8_t *data, size_t dataLen, const sockaddr_in &sa)
{
    while(dataLen >= sizeof(DiscoveryData))
    {
        DiscoveryData discoveryData;
        discoveryData = *reinterpret_cast<const DiscoveryData*>(data);
        HandleDiscoveryDatagram(&discoveryData, inet_ntoa(sa.sin_addr), sa.sin_port);

        dataLen -= sizeof(DiscoveryData);
        data = data + sizeof(DiscoveryData);
    }
}

void DiscoveryClient::HandleDiscoveryDatagram(DiscoveryData *data, std::string ip, uint16_t port) const
{
    if(data->m_version != m_discovery_version)
    {
        std::cout << "Incompatible discovery message received";
        return;
    }
    if(m_self_id.m_name[0] &&
            strcmp(data->m_name, m_self_id.m_name) == 0 &&
            m_self_id.m_serial_number[0] &&
            strcmp(data->m_serial_number, m_self_id.m_serial_number) == 0)
    {
        //message from self to be ignored
        return;
    }
    std::cout << "Received discovery datagram from "
             << ip
             << ":"
             << port
             << ", version:"
             << data->m_version;
    if(HandleDiscoveryDatagramCb)
    {
        HandleDiscoveryDatagramCb(data, ip, port);
    }
}
