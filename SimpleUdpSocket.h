#pragma once

#include <functional>
#include <future>
#include <memory>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <vector>

class NetworkServerInterface;
//! \addtogroup Services
//! \{
class SimpleUdpSocket
{
    using ReceiveFromClientCallback_T = std::function<void (const uint8_t *dataFromClient, size_t dataLen, const sockaddr_in &sa)>;
public:
    SimpleUdpSocket(uint16_t port,
                     uint32_t sizeofDatagram,
                     uint32_t numDatagrams,
                     const std::string &serviceName,
                     bool enableBroadcast,
                     ReceiveFromClientCallback_T receiveFromClientCallback);
    virtual ~SimpleUdpSocket();
    void SendToClient(const sockaddr_in &sa, const char *dataToClient) const;
    void Stop();

protected:
    void SimpleUdpSocketThreadProc();

    std::future<void> m_thread;
    int m_socket;
    fd_set m_socket_set;
    std::vector<unsigned char> m_buffer;
    uint32_t m_sizeof_datagram;
    std::string m_service_name;
    std::atomic<bool> m_killed;
    ReceiveFromClientCallback_T m_receive_from_client_callback;
};

//! \}
