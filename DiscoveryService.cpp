#include "DiscoveryService.h"

#include <arpa/inet.h>
#include <iostream>
#include <netinet/in.h>
#include <regex>
#include <string>
#include <sys/socket.h>
#include <sys/stat.h>

#include "Utils.h"

namespace {

}//namespace

//-------------------------------------------------------------------------------
DiscoveryService::DiscoveryService(const std::string &device_id_file)
    : m_udp_socket(SERVER_PORT,
                   sizeof(DiscoveryData),
                   MAX_QUEUE_SIZE,
                   "DiscoveryService",
                   false,
                   [this](const uint8_t *dataFromClient, size_t dataLen, const sockaddr_in &sa){
        HandleUdpDatagram(dataFromClient, dataLen, sa);
    })
{
    Utils::ReadDeviceId(m_discover_message, device_id_file);
    std::cout << "Device Name: " << m_discover_message.m_name << ", Device Serial Number: " << m_discover_message.m_serial_number << "\n";
}
//-------------------------------------------------------------------------------
DiscoveryService::~DiscoveryService()
{
    m_udp_socket.Stop();
    std::cout << "dtor\n";
}
//-------------------------------------------------------------------------------
void DiscoveryService::HandleUdpDatagram(const uint8_t *dataFromClient, size_t dataLen, const sockaddr_in &sa)
{
    auto msg = reinterpret_cast<const DiscoveryData*>(dataFromClient);
//    std::cout << "Received discovery datagram from " <<
//                                 inet_ntoa(sa.sin_addr) << ":" << ntohs(sa.sin_port) << "\n";
    if(msg->m_version != DiscoveryData::DISCOVERY_VERSION)
    {
        std::cout << "Failed: Incompatible Discovery message received\n";
        return;
    }
    Utils::SaveServerIpAddress(inet_ntoa(sa.sin_addr));
    m_udp_socket.SendToClient(sa, reinterpret_cast<char*>(&m_discover_message));
}
