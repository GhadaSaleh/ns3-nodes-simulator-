#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"




#include "ns3/point-to-point-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/wifi-module.h"

#include "ns3/command-line.h"
#include "ns3/string.h"
#include "ns3/ssid.h"
#include "ns3/mobility-helper.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/on-off-helper.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/qos-txop.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/olsr-helper.h"
#include "ns3/csma-helper.h"
#include "ns3/animation-interface.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("CognitiveRadioAdHocNetwork");

// DGSA Algorithm Function
int DynamicGenerationSizeAdjustment(int cwnd, int GS, int GRTT, int RTTThreshold) {
    int EGS;

    if (GS >= cwnd) {
        GS = cwnd;
        EGS = GS;
    } else {
        if (GRTT <= RTTThreshold) {
            EGS = GS + 4;
        } else if (GRTT > RTTThreshold) {
            EGS = GS - 4;
        } else {
            EGS = GS;
        }
    }

    if (EGS >= cwnd) {
        EGS = GS;
    } else {
        EGS = EGS;
    }

    return EGS;
}

// Custom application to simulate packet transfer
class MyPacketTransfer : public Application {
public:
    MyPacketTransfer();
    virtual ~MyPacketTransfer();

    void Setup(Ipv4Address address, uint32_t packetSize, uint32_t numPackets, Time packetInterval);

private:
    virtual void StartApplication(void);
    virtual void StopApplication(void);

    void SendPacket(void);

    Ipv4Address m_peerAddress;
    uint32_t m_packetSize;
    uint32_t m_numPackets;
    Time m_packetInterval;
    uint32_t m_packetsSent;
    Ptr<Socket> m_socket;
    EventId m_sendEvent;
    bool m_running;
};

MyPacketTransfer::MyPacketTransfer()
    : m_packetSize(0),
      m_numPackets(0),
      m_packetInterval(Seconds(0)),
      m_packetsSent(0),
      m_socket(0),
      m_sendEvent(),
      m_running(false)
{
}

MyPacketTransfer::~MyPacketTransfer()
{
    m_socket = 0;
}

void MyPacketTransfer::Setup(Ipv4Address address, uint32_t packetSize, uint32_t numPackets, Time packetInterval)
{
    m_peerAddress = address;
    m_packetSize = packetSize;
    m_numPackets = numPackets;
    m_packetInterval = packetInterval;
}

void MyPacketTransfer::StartApplication(void)
{
    m_running = true;
    m_packetsSent = 0;
    m_socket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
    m_socket->Bind();
    m_socket->Connect(InetSocketAddress(m_peerAddress, 9));
    SendPacket();
}

void MyPacketTransfer::StopApplication(void)
{
    m_running = false;

    if (m_sendEvent.IsRunning())
    {
        Simulator::Cancel(m_sendEvent);
    }

    if (m_socket)
    {
        m_socket->Close();
    }
}

void MyPacketTransfer::SendPacket(void)
{
    Ptr<Packet> packet = Create<Packet>(m_packetSize);
    m_socket->Send(packet);

    if (++m_packetsSent < m_numPackets)
    {
        Time nextPacketTime = m_packetInterval + Simulator::Now();
        m_sendEvent = Simulator::Schedule(nextPacketTime, &MyPacketTransfer::SendPacket, this);
    }
}

int main(int argc, char *argv[])
{
    // Network Parameters
    int cwnd = 25;
    int GS = 6;
    int GRTT = 80; // Placeholder value for
}

