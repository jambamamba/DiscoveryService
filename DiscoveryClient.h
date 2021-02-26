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
    const uint32_t DiscoveryVersion = DiscoveryData::DISCOVERY_VERSION;
    SimpleUdpSocket UdpSocket;
};

