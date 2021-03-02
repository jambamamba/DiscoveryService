#include "SimpleUdpSocket.h"

#include <arpa/inet.h>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <net/if.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

namespace
{
//-------------------------------------------------------------------------------
void InitFileDescriptorSet( timeval &tv, int socketfd, fd_set *set )
{
   FD_ZERO( set );
   tv.tv_sec = 1;
   tv.tv_usec = 0;
   FD_SET( socketfd, set );
}
//-------------------------------------------------------------------------------
bool InterfaceConnected(int socketfd)
{
    struct ifreq ifr;

    memset( &ifr, 0, sizeof(ifr) );
    strcpy( ifr.ifr_name, "wlan0" );

    if( ioctl( socketfd, SIOCGIFFLAGS, &ifr ) != -1 )
    {
        return (ifr.ifr_flags & ( IFF_UP | IFF_RUNNING )) == ( IFF_UP | IFF_RUNNING );
    }
    else
    {
//        std::cout << "error in ioctl call, errno: " << errno << ", " << strerror(errno) << "\n";
    }
    return false;
}
//-------------------------------------------------------------------------------
bool WaitForSocketIO(int socket, fd_set &socketSet)
{
    struct timeval tv;
    InitFileDescriptorSet( tv, socket, &socketSet );

    int ret = select( socket + 1, &socketSet, nullptr, nullptr, &tv );
    if( ret == -1 )
    {
        if(errno == EAGAIN)// try again
        {
            return false;
        }
        else if (errno == EBADF) // file descriptor closed while trying to read from socket
        {
            std::cout << "error in select call: EBADF, File Closed\n";
            return false;
        }

        std::cout << "error in select call, errno: " << errno << ", " << strerror(errno) << "\n";
        return false;
    }
    else if( ret == 0 )// timeout
    {
       return false;
    }
    return true;
}
//-------------------------------------------------------------------------------
bool ReadFromClient(std::atomic<bool> &killed,
                    int socket,
                    struct sockaddr_in &sa,
                    std::vector<unsigned char> &recvdData)
{
    ssize_t bytesRecvd = 0;
    constexpr int SOCKET_READ_BUFFER_SIZE = 1024;
    std::array<char, SOCKET_READ_BUFFER_SIZE> buffer;
    socklen_t fromlen = sizeof(sa);
    while(!killed)
    {
        ssize_t recsize = ::recvfrom(socket, (void*)buffer.data(), buffer.size(), 0, (struct sockaddr*)&sa, &fromlen);
        if (recsize < 0)
        {
          if(errno == EAGAIN)
          {
              break;
          }
          std::cout << std::strerror(errno) << "\n";
          return false;
        }
        else if(recsize == 0) //socket closed
        {
            return false;
        }
        recvdData.insert(recvdData.end(), buffer.data(), buffer.data() + recsize);
        bytesRecvd += recsize;
    }
//    std::cout << "Received " << bytesRecvd << " bytes of data\n";
    return true;
}
}//namespace

//-------------------------------------------------------------------------------
SimpleUdpSocket::SimpleUdpSocket(uint16_t port,
                                   uint32_t sizeofDatagram,
                                   uint32_t numDatagrams,
                                   const std::string &serviceName,
                                 bool enableBroadcast,
                                   ReceiveFromClientCallback_T receiveFromClientCallback)
   : m_socket(-1)
   , m_sizeof_datagram(sizeofDatagram)
   , m_service_name(serviceName)
   , m_killed(false)
   , m_receive_from_client_callback(receiveFromClientCallback)
{
   m_buffer.reserve(sizeofDatagram * numDatagrams);

   struct sockaddr_in sa = {};
   sa.sin_family = AF_INET;
   sa.sin_addr.s_addr = htonl(INADDR_ANY);
   sa.sin_port = htons(port);

   m_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
   if(m_socket < 1)
   {
       close(m_socket);
       std::cout << m_service_name << " failed to create socket\n";
       exit(-1);
   }
   else if (bind(m_socket, (struct sockaddr *)&sa, sizeof sa) == -1)
   {
     close(m_socket);
     std::cout << m_service_name << " failed to bind to port " << port << "\n";
     exit(-1);
   }
   fcntl(m_socket, F_SETFL, fcntl(m_socket, F_GETFL, 0) | O_NONBLOCK);

   if(enableBroadcast)
   {
       int broadcastEnable=1;
       int ret=setsockopt(m_socket, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));
       if(ret != 0)
       {
           std::cout << m_service_name << " failed to enable broadcast mode, errno: " << errno << ", " << strerror(errno);
       }
   }

   std::cout << m_service_name << " started on port " << port << "\n";

   m_thread = std::async([this](){
      SimpleUdpSocketThreadProc();
   });
}

//-------------------------------------------------------------------------------
SimpleUdpSocket::~SimpleUdpSocket()
{
    m_killed = true;
    m_thread.wait();
    if(m_socket != 0)
    {
        close(m_socket);
    }
    m_socket = 0;
    std::cout << "dtor\n";
}
//-------------------------------------------------------------------------------
void SimpleUdpSocket::SimpleUdpSocketThreadProc()
{
   pthread_setname_np( pthread_self(), m_service_name.c_str() );

   std::cout << "Waiting for messages in new thread\n";

   while(!m_killed)
   {
       if(!InterfaceConnected(m_socket))
       {
           //restart device
       }

       if(!WaitForSocketIO(m_socket, m_socket_set))
       {
           continue;
       }

       struct sockaddr_in sa;
       if(!ReadFromClient(m_killed, m_socket, sa, m_buffer))
       {
           continue;
       }

       while((uint32_t)m_buffer.size() > 4)
       {
           unsigned char byte = m_buffer.at(0);
           if(m_buffer.at(0) == 0xca &&
                   m_buffer.at(1) == 0xfe &&
                   m_buffer.at(2) == 0xba &&
                   m_buffer.at(3) == 0xbe)
           {
               break;
           }
           m_buffer.erase(m_buffer.begin(), m_buffer.begin() + 1);
       }
       while((uint32_t)m_buffer.size() >= m_sizeof_datagram)
       {
//           std::cout << "Received datagram from " << inet_ntoa(sa.sin_addr) << ":" << ntohs(sa.sin_port) << "\n";
           m_receive_from_client_callback(m_buffer.data(), m_sizeof_datagram, sa);
           m_buffer.erase(m_buffer.begin(), m_buffer.begin() + m_sizeof_datagram);
       }
   }
}

//-------------------------------------------------------------------------------
void SimpleUdpSocket::SendToClient(const sockaddr_in &sa, const char *dataToClient) const
{
    int bytesWritten = sendto(m_socket, dataToClient, m_sizeof_datagram, 0,(struct sockaddr*)&sa, sizeof sa);
    if(bytesWritten != m_sizeof_datagram)
    {
        std::cout << "Not all bytes could be be sent to "
                                    << inet_ntoa(sa.sin_addr) << ":" << ntohs(sa.sin_port)
                                    << ", bytes sent: " << bytesWritten
                                    << " of " << m_sizeof_datagram
                                    << "\n";
    }
    else
    {
//        std::cout << "sent datagram to " << inet_ntoa(sa.sin_addr) << ":" << ntohs(sa.sin_port) << "\n";
    }
}

//-------------------------------------------------------------------------------
void SimpleUdpSocket::Stop()
{
    m_killed = true;
}
