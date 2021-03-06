#pragma once

#include <memory>

#include "DiscoveryData.h"
#include "SimpleUdpSocket.h"

class DiscoveryClient
{
public:
    DiscoveryClient(const std::string &device_id_file,
                    std::function<void (DiscoveryData *data, uint32_t ip, uint16_t port)> handleDiscoveryDatagram = nullptr);
    ~DiscoveryClient();
    void UpdateDeviceId(const std::string &device_id_file);
    void Discover();

protected:
    void ReadDatagrams(const uint8_t *dataFromClient, size_t dataLen, const sockaddr_in &sa);
    void HandleDiscoveryDatagram(DiscoveryData *data, uint32_t ip, uint16_t port) const;

    const uint32_t m_discovery_version = DiscoveryData::DISCOVERY_VERSION;
    std::function<void (DiscoveryData *data, uint32_t ip, uint16_t port)> HandleDiscoveryDatagramCb;
    SimpleUdpSocket m_udp_socket;
    DiscoveryData m_discover_message;
};

