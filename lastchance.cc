#include "ns3/command-line.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/end-device-lora-phy.h"
#include "ns3/end-device-lorawan-mac.h"
#include "ns3/gateway-lora-phy.h"
#include "ns3/gateway-lorawan-mac.h"
#include "ns3/log.h"
#include "ns3/lora-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/node-container.h"
#include "ns3/periodic-sender-helper.h"
#include "ns3/position-allocator.h"
#include "ns3/simulator.h"
#include "ns3/core-module.h"
#include "ns3/mobility-module.h"
#include "ns3/ns2-mobility-helper.h"
#include <algorithm>
#include <ctime>
#include <map>
#include <vector>

using namespace ns3;
using namespace lorawan;

NS_LOG_COMPONENT_DEFINE("SimpleLorawanNetworkExample");

std::vector<int> packetsSent(1, 0);
std::vector<int> packetsReceived(1, 0);
std::map<uint64_t, Time> transmissionTimes;  // Store transmission times
std::vector<Time> delays;  // Store delays

void OnTransmissionCallback(Ptr<const Packet> packet, uint32_t senderNodeId)
{
    NS_LOG_FUNCTION(packet << senderNodeId);
    LoraTag tag;
    packet->PeekPacketTag(tag);
    
    packetsSent[0]++;
    
    // Record the transmission time
    uint64_t packetId = packet->GetUid();
    transmissionTimes[packetId] = Simulator::Now();
}

void OnPacketReceptionCallback(Ptr<const Packet> packet, uint32_t receiverNodeId)
{
    NS_LOG_FUNCTION(packet << receiverNodeId);
    LoraTag tag;
    packet->PeekPacketTag(tag);
    
    packetsReceived[0]++;
    
    // Calculate the delay
    uint64_t packetId = packet->GetUid();
    if (transmissionTimes.find(packetId) != transmissionTimes.end())
    {
        Time transmissionTime = transmissionTimes[packetId];
        Time receptionTime = Simulator::Now();
        Time delay = receptionTime - transmissionTime;
        delays.push_back(delay);  // Store the delay
        NS_LOG_INFO("Packet " << packetId << " delay: " << delay.GetSeconds() << " seconds");
    }

  
}


CommandLine cmd;
int nDevices = 50; // nDevices değişkenini tanımlıyoruz
std::string OutputFolder = "";
int nGateways =1;
int appPeriodSeconds = 50; 
int main(int argc, char* argv[])
{
    // Set up logging
    /*LogComponentEnable("SimpleLorawanNetworkExample", LOG_LEVEL_ALL);
    LogComponentEnable("LoraChannel", LOG_LEVEL_INFO);
    LogComponentEnable("LoraPhy", LOG_LEVEL_ALL);
    LogComponentEnable("EndDeviceLoraPhy", LOG_LEVEL_ALL);
    LogComponentEnable("GatewayLoraPhy", LOG_LEVEL_ALL);
    LogComponentEnable("LoraInterferenceHelper", LOG_LEVEL_ALL);
    LogComponentEnable("LorawanMac", LOG_LEVEL_ALL);
    LogComponentEnable("EndDeviceLorawanMac", LOG_LEVEL_ALL);
    LogComponentEnable("ClassAEndDeviceLorawanMac", LOG_LEVEL_ALL);
    LogComponentEnable("GatewayLorawanMac", LOG_LEVEL_ALL);
    LogComponentEnable("LogicalLoraChannelHelper", LOG_LEVEL_ALL);
    LogComponentEnable("LogicalLoraChannel", LOG_LEVEL_ALL);
    LogComponentEnable("LoraHelper", LOG_LEVEL_ALL);
    LogComponentEnable("LoraPhyHelper", LOG_LEVEL_ALL);
    LogComponentEnable("LorawanMacHelper", LOG_LEVEL_ALL);
    LogComponentEnable("PeriodicSenderHelper", LOG_LEVEL_ALL);*/
    //LogComponentEnable ("Ns2MobilityHelper",LOG_LEVEL_ALL);
    // Create the channel
    NS_LOG_INFO("Creating the channel...");

    Ptr<LogDistancePropagationLossModel> loss = CreateObject<LogDistancePropagationLossModel>();
    loss->SetPathLossExponent(3.76);
    loss->SetReference(1, 7.7);

    Ptr<PropagationDelayModel> delay = CreateObject<ConstantSpeedPropagationDelayModel>();

    Ptr<LoraChannel> channel = CreateObject<LoraChannel>(loss, delay);

    // Create the helpers
    NS_LOG_INFO("Setting up helpers...");

    MobilityHelper mobility;
    Ptr<ListPositionAllocator> allocator = CreateObject<ListPositionAllocator>();
    allocator->Add(Vector(1000,0,0));
    mobility.SetPositionAllocator(allocator);
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");

    LoraPhyHelper phyHelper = LoraPhyHelper();
    phyHelper.SetChannel(channel);

    LorawanMacHelper macHelper = LorawanMacHelper();

    LoraHelper helper = LoraHelper();
   
    // Create end devices
    NS_LOG_INFO("Creating the end devices...");
   
    cmd.AddValue("nDevices", "Number of devices to simulate", nDevices);
    cmd.AddValue("nGateways","Number of Gateways",nGateways);
    cmd.AddValue("appPeriodSeconds","AppPeriodSeconds ",appPeriodSeconds);
    // Diğer komut satırı argümanlarını ekleyin
    cmd.AddValue("OutputFolder", "OutputFolder create", OutputFolder);
    cmd.Parse(argc, argv);
    
    std::string traceFile = "/home/yucel/ns-allinone-3.41/ns-3.41/scratch/ns2mobility.tcl";
    
    
    double simulationTimeSeconds = 2171.0;
    Ns2MobilityHelper ns2 = Ns2MobilityHelper(traceFile);
    NodeContainer endDevices;
    endDevices.Create(nDevices);
    ns2.Install();
    phyHelper.SetDeviceType(LoraPhyHelper::ED);
    macHelper.SetDeviceType(LorawanMacHelper::ED_A);
    helper.Install(phyHelper, macHelper, endDevices);

    // Create gateways
    NS_LOG_INFO("Creating the gateway...");

    NodeContainer gateways;
    gateways.Create(nGateways);

    mobility.Install(gateways);

    phyHelper.SetDeviceType(LoraPhyHelper::GW);
    macHelper.SetDeviceType(LorawanMacHelper::GW);
    helper.Install(phyHelper, macHelper, gateways);

    // Install applications in end devices
    
    PeriodicSenderHelper appHelper = PeriodicSenderHelper();
    appHelper.SetPeriod(Seconds(appPeriodSeconds));
    ApplicationContainer appContainer = appHelper.Install(endDevices);

    // Set Data Rates
    std::vector<int> sfQuantity(1);
    sfQuantity = LorawanMacHelper::SetSpreadingFactorsUp(endDevices, gateways, channel);

    // Install trace sources for packet transmission and reception
    for (auto node = endDevices.Begin(); node != endDevices.End(); ++node)
    {
        (*node)->GetDevice(0)->GetObject<LoraNetDevice>()->GetPhy()->TraceConnectWithoutContext(
            "StartSending",
            MakeCallback(OnTransmissionCallback));
    }

    for (auto node = gateways.Begin(); node != gateways.End(); ++node)
    {
        (*node)->GetDevice(0)->GetObject<LoraNetDevice>()->GetPhy()->TraceConnectWithoutContext(
            "ReceivedPacket",
            MakeCallback(OnPacketReceptionCallback));
    }
    
    /*Time stateSamplePeriod = Seconds(appPeriodSeconds);
    helper.EnablePeriodicDeviceStatusPrinting(endDevices,
                                              gateways,
                                              "nodeData.txt",
                                              stateSamplePeriod);
    helper.EnablePeriodicPhyPerformancePrinting(gateways, "phyPerformance.txt", stateSamplePeriod);
    helper.EnablePeriodicGlobalPerformancePrinting("globalPerformance.txt", stateSamplePeriod);*/
    
    // Simulation
    Simulator::Stop(Seconds(simulationTimeSeconds));
    Simulator::Run();
    Simulator::Destroy();
   
    // Calculate packet delivery ratio
    NS_LOG_INFO("Calculating performance metrics...");

    float totalPacketsSent = packetsSent[0];
    float totalPacketsReceived = packetsReceived[0];
    float totalPacketsLost = totalPacketsSent - totalPacketsReceived;
    float percantage = totalPacketsReceived / totalPacketsSent;
    std::cout << "Veri Orani: "
              << "Gonderilen: " << totalPacketsSent << ", "
              << "Ulaşan: " << totalPacketsReceived << ", "
              << "Kaybolan: " << totalPacketsLost << ", "
              << "Basari orani: " << percantage << std::endl;
    
    // Calculate average delay
    Time totalDelay = Seconds(0);
    for (Time delay : delays)
    {
        totalDelay += delay;
    }
    double averageDelay = delays.empty() ? 0 : totalDelay.GetSeconds() / delays.size();
    std::cout << "Ortalama Gecikme: " << averageDelay << " saniye" << std::endl;

    // Print each packet delay
    std::cout << "Packet Delays:" << std::endl;
    for (size_t i = 0; i < delays.size(); ++i)
    {
        std::cout << "Packet " << i + 1 << ": " << delays[i].GetSeconds() << " saniye" << std::endl;
    }

    return 0;
}
