#pragma once

#include <memory>

#include "DiscoveryData.h"
#include "SimpleUdpSocket.h"

class DiscoveryClient
{
public:
    DiscoveryClient(const std::string &device_id_file,
                    std::function<void (DiscoveryData *data, std::string ip, uint16_t port)> handleDiscoveryDatagram = nullptr);
    ~DiscoveryClient();
    void Discover();

protected:
    void ReadDatagrams(const uint8_t *dataFromClient, size_t dataLen, const sockaddr_in &sa);
    void HandleDiscoveryDatagram(DiscoveryData *data, std::string ip, uint16_t port) const;

    const uint32_t m_discovery_version = DiscoveryData::DISCOVERY_VERSION;
    std::function<void (DiscoveryData *data, std::string ip, uint16_t port)> HandleDiscoveryDatagramCb;
    SimpleUdpSocket m_udp_socket;
    DiscoveryData m_self_id;
};

