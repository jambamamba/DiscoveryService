#pragma once

#include <memory>

#include "DiscoveryData.h"
#include "SimpleUdpSocket.h"

class DiscoveryClient
{
public:
    DiscoveryClient();
    ~DiscoveryClient();
    void Discover();

protected:
    void HandleDiscoveryDatagram(DiscoveryData *data, std::string ip, uint16_t port) const;
    void ReadDatagrams(const uint8_t *dataFromClient, size_t dataLen, const sockaddr_in &sa);

protected:
    const uint32_t m_discovery_version = DiscoveryData::DISCOVERY_VERSION;
    SimpleUdpSocket m_udp_socket;
    DiscoveryData m_self_id;
};

