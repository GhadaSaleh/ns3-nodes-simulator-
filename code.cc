// Include necessary NS3 modules and libraries
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/csma-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

// Define the logging component for this code

NS_LOG_COMPONENT_DEFINE("AdHocNetwork");

// DGSA Algorithm Function: Dynamic Generation Size Adjustment
// This function adjusts the generation size based on congestion window (cwnd),
// generation size (GS), round-trip time (GRTT), and round-trip time threshold (RTTThreshold).
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

// Constructor for MyPacketTransfer application
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

// Destructor for MyPacketTransfer application
MyPacketTransfer::~MyPacketTransfer()
{
    m_socket = 0;
}

// Setup function to configure the MyPacketTransfer application
void MyPacketTransfer::Setup(Ipv4Address address, uint32_t packetSize, uint32_t numPackets, Time packetInterval)
{
    m_peerAddress = address;
    m_packetSize = packetSize;
    m_numPackets = numPackets;
    m_packetInterval = packetInterval;
}

// StartApplication function for MyPacketTransfer application
void MyPacketTransfer::StartApplication(void)
{
    m_running = true;
    m_packetsSent = 0;
    m_socket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
    m_socket->Bind();
    m_socket->Connect(InetSocketAddress(m_peerAddress, 9));
    SendPacket();
}

// StopApplication function for MyPacketTransfer application
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

// SendPacket function to send packets in MyPacketTransfer application
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

// The main function
int main(int argc, char *argv[])
{
    // Network Parameters
    int cwnd = 25;
    int GS = 6;
    int GRTT = 80; // Placeholder value for GRTT (in milliseconds)
    int RTTThreshold = 100;

    // Initialize NS3 and enable logging for UdpEchoClientApplication and UdpEchoServerApplication
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    // Create nodes and set up the CSMA network
    NodeContainer nodes;
    nodes.Create(7);

    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", DataRateValue(DataRate("2Mbps")));
    csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(50)));

    NetDeviceContainer devices;
    devices = csma.Install(nodes);

    // Install Internet stack on the nodes
    InternetStackHelper stack;
    stack.Install(nodes);

    // Assign IP addresses to devices
    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = address.Assign(devices);

    // Set up an echo server application
    UdpEchoServerHelper echoServer(9);
    ApplicationContainer serverApps = echoServer.Install(nodes.Get(0));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));

    // Set up a custom application for packet transfer
    MyPacketTransfer myApp;
    myApp.Setup(interfaces.GetAddress(0), 536, 10, Seconds(1.0));
    nodes.Get(6)->AddApplication(&myApp);
    myApp.SetStartTime(Seconds(2.0));
    myApp.SetStopTime(Seconds(10.0));

    // Call DGSA Algorithm to adjust the estimated generation size (EGS)
    int EGS = DynamicGenerationSizeAdjustment(cwnd, GS, GRTT, RTTThreshold);

    // Output the estimated generation size
    NS_LOG_INFO("Estimated Generation Size: " << EGS);

    // Calculate throughput for different packet sizes and numbers of packets
    std::cout << "TABLE I Throughput Vs No.of packets" << std::endl;
    std::cout << "No. of packets\tThroughput (kbps) with Packet Size = 590 bytes\tThroughput (kbps) with Packet Size = 1000 bytes" << std::endl;

    // Array of number of packets for throughput calculation
    int numPackets[] = {100, 200, 300, 400, 450, 500, 600, 700, 800};
    for (int i = 0; i < 9; i++)
    {
        double throughput590 = 0.0, throughput1000 = 0.0;
        if (numPackets[i] > 0)
        {
            // Calculate throughput with packet size = 590 bytes
            throughput590 = (numPackets[i] * 590 * 8.0) / (10.0 * 1000.0);

            // Calculate throughput with packet size = 1000 bytes
            throughput1000 = (numPackets[i] * 1000 * 8.0) / (10.0 * 1000.0);
        }
        std::cout << numPackets[i] << "\t" << throughput590 << "\t\t\t" << throughput1000 << std::endl;
    }

    // Calculate throughput for different packet sizes and numbers of packets
    std::cout << std::endl;
    std::cout << "TABLE II Throughput Vs Packet Size" << std::endl;
    std::cout << "Packet Size (bytes)\tThroughput (bps) with No.of Packets = 5\tThroughput (bps) with No.of Packets = 10" << std::endl;

    // Array of packet sizes for throughput calculation
    int packetSizes[] = {1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000};
    int numPackets1 = 5, numPackets2 = 10;
    for (int i = 0; i < 8; i++)
    {
        double throughput5 = 0.0, throughput10 = 0.0;
        if (packetSizes[i] > 0)
        {
            // Calculate throughput with 5 packets
            throughput5 = (numPackets1 * packetSizes[i] * 8.0) / (10.0);

            // Calculate throughput with 10 packets
            throughput10 = (numPackets2 * packetSizes[i] * 8.0) / (10.0);
        }
        std::cout << packetSizes[i] << "\t\t\t" << throughput5 << "\t\t\t" << throughput10 << std::endl;
    }

    // Set up animation interface for visualization (if required)
    AnimationInterface anim("/home/ghada/animation.xml");
    anim.EnablePacketMetadata(true);

    // Run the simulation
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}

