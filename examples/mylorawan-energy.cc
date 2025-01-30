#include "ns3/end-device-lora-phy.h"
#include "ns3/forwarder-helper.h"
#include "ns3/network-server-helper.h"

#include "ns3/gateway-lora-phy.h"
#include "ns3/class-a-end-device-lorawan-mac.h"
#include "ns3/gateway-lorawan-mac.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/random-waypoint-mobility-model.h"
#include "ns3/lora-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/mobility-module.h"
#include "ns3/node-container.h"
#include "ns3/position-allocator.h"
#include "ns3/periodic-sender-helper.h"
#include "ns3/command-line.h"
#include "ns3/energy-module.h"
#include "ns3/basic-energy-harvester.h"
#include "ns3/basic-energy-source.h"
#include "ns3/basic-energy-source-helper.h"
#include "ns3/lora-radio-energy-model-helper.h"
#include <ns3/config-store-module.h>
#include "ns3/stats-module.h"
#include "ns3/file-helper.h"
#include "ns3/core-module.h"
#include "ns3/waypoint-mobility-model.h"
#include "ns3/names.h"
#include "ns3/lora-packet-tracker.h"
//#include "ns3/steady-state-random-waypoint-mobility-model.h"
#include "ns3/string.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/random-variable-stream.h"
#include "ns3/pointer.h"
#include <algorithm>
#include <ctime>
#include <unistd.h>


using namespace ns3;
using namespace lorawan;

void
RemainingEnergy (std::string context, double oldValue, double remainingEnergy)
{
  NS_LOG_UNCOND (Simulator::Now ().GetSeconds ()
                 << "s,"<<context<<"," << remainingEnergy << "J");
}

// this variables can be modified in the command line e.g --radio=20.0
double radio = 20.0;
int numendDevices = 1;

//+++++++++++++++++++++
int aesKey = 128;
int packetSize = 12;

unsigned char  msg[] = "messagesenv_hhhhmessagesenv_hhhhmessagesenv_hhhhmessagesenv_hhhh";
//---------------------

MobilityHelper createscenario(double radio); //random position around starcenter 
MobilityHelper createstarcenter();
void printposition(NodeContainer n);
CommandLine setupcmd();

int main (int argc, char *argv[]){
       

    CommandLine cmd =setupcmd();
    cmd.Parse (argc, argv);
  //  LogComponentEnable ("LorawanMac", LOG_LEVEL_ALL);
  //  LogComponentEnable ("LoraEnergyModelExample", LOG_LEVEL_ALL);
  //  LogComponentEnable ("LoraFrameHeader", LOG_LEVEL_ALL);
  //  LogComponentEnable ("LorawanMacHeader", LOG_LEVEL_ALL);
  //  LogComponentEnable ("LoraPacketTracker", LOG_LEVEL_ALL);
  //  LogComponentEnable ("LoraPhy", LOG_LEVEL_DEBUG);
      LogComponentEnable ("PeriodicSender", LOG_LEVEL_DEBUG);
    NodeContainer endDevices;
    endDevices.Create(numendDevices);
    MobilityHelper scenario = createscenario(radio);
    scenario.Install (endDevices);
    //put the hub in 0.0 0.0 0.0  center   gateway
    NodeContainer hub;
    hub.Create(1);
        
    MobilityHelper hubposition = createstarcenter();
    hubposition.Install (hub);

     // Create the lora channel object
    Ptr<LogDistancePropagationLossModel> loss = CreateObject<LogDistancePropagationLossModel> ();
    loss->SetPathLossExponent (3.76);
    loss->SetReference (1, 7.7);
    Ptr<PropagationDelayModel> delay = CreateObject<ConstantSpeedPropagationDelayModel> ();
    Ptr<LoraChannel> channel = CreateObject<LoraChannel> (loss, delay);

    // Create the LoraPhyHelper (Phy, Mac and Lora)
    LoraPhyHelper phyHelper = LoraPhyHelper ();


    phyHelper.SetChannel (channel);
    LorawanMacHelper macHelper = LorawanMacHelper ();
    LoraHelper helper = LoraHelper ();

    helper.EnablePacketTracking();

    phyHelper.SetDeviceType(LoraPhyHelper::ED);
    macHelper.SetDeviceType(LorawanMacHelper::ED_A);
    
    NetDeviceContainer  endDevicesNetDevices = helper.Install(phyHelper, macHelper, endDevices);
    

    LoraHelper helperHub = LoraHelper ();
 // helperHub.EnablePacketTracking();
    phyHelper.SetDeviceType (LoraPhyHelper::GW);
    macHelper.SetDeviceType (LorawanMacHelper::GW);

 // helperHub.Install (phyHelper, macHelper, hub);
    helper.Install (phyHelper, macHelper, hub);
    
    macHelper.SetSpreadingFactorsUp (endDevices, hub, channel);
    // Create NS
    ////////////

    NodeContainer networkServers;
    networkServers.Create(1);

    // Install the NetworkServer application on the network server
    NetworkServerHelper networkServerHelper;
    networkServerHelper.SetGateways(hub);
    networkServerHelper.SetEndDevices(endDevices);
    if (aesKey!=0)
    {
         networkServerHelper.SetDecrypt(aesKey);
    }
    networkServerHelper.Install(networkServers);

    // Install the Forwarder application on the gateways
    ForwarderHelper forwarderHelper;
    forwarderHelper.Install(hub);   


    PeriodicSenderHelper periodicSenderHelper;
    periodicSenderHelper.SetPeriod (Seconds (6));
    //+++++++++++++++++++++++++++++++++++++++++++++++++
    size_t input=0;
    if (aesKey!=0)
    {
      packetSize = sizeof(msg);
      input = packetSize;
      //printf("%d\n",packetSize );
    }
    periodicSenderHelper.SetPacketSize (packetSize);  
    periodicSenderHelper.EnableAes(aesKey,msg,input);
    //-------------------------------------------------    

    ApplicationContainer appContainer = periodicSenderHelper.Install (endDevices);
    double simulationTime = 3600;
    Time appStopTime = Seconds (simulationTime);
    appContainer.Start (Seconds (0));
    appContainer.Stop (appStopTime);
    /************************
   * Install Energy Model *
   ************************/

    BasicEnergySourceHelper basicSourceHelper;
    LoraRadioEnergyModelHelper radioEnergyHelper;
    
    // Bateria PD2032 200 mAh 3.7V
    basicSourceHelper.Set ("BasicEnergySourceInitialEnergyJ", DoubleValue (2664)); // Energy in J
    basicSourceHelper.Set ("BasicEnergySupplyVoltageV", DoubleValue (3.7));

    //Valores de LoraWan modem Semtech SX1276/77/78/79 datasheet, tabla de la pág 14. Rev7 May 2020
    radioEnergyHelper.Set ("StandbyCurrentA", DoubleValue (0.0016));
    radioEnergyHelper.Set ("TxCurrentA", DoubleValue (0.120)); //20 dbm
    radioEnergyHelper.Set ("SleepCurrentA", DoubleValue (0.0000002));
    radioEnergyHelper.Set ("RxCurrentA", DoubleValue (0.0115));

    radioEnergyHelper.SetTxCurrentModel ("ns3::ConstantLoraTxCurrentModel",
                                       "TxCurrent", DoubleValue (0.12)); // +20 dBm
  //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  if (aesKey !=0)
  {
    double aesCurrent;
    double supplyVoltage = 3.7;
    double txCurrentA = 0.120;
    switch (aesKey){
    case 128:
      aesCurrent = (0.002909501 / supplyVoltage) * packetSize;
      break;
    case 192:
      aesCurrent = (0.002850558 / supplyVoltage) * packetSize;
      break;
    case 256:
      aesCurrent = (0.003068377 / supplyVoltage) * packetSize;
      break;
    default:
      aesCurrent = 0;
    }

    radioEnergyHelper.Set ("TxCurrentA", DoubleValue (txCurrentA + aesCurrent));
    radioEnergyHelper.SetTxCurrentModel ("ns3::ConstantLoraTxCurrentModel",
                                       "TxCurrent", DoubleValue (txCurrentA + aesCurrent));
  }
  //---------------------------------------------------------

  // install source on EDs' endDevices
    EnergySourceContainer sources = basicSourceHelper.Install (endDevices);
    BasicEnergyHarvesterHelper basicHarvesterHelper;
    basicHarvesterHelper.Set ("PeriodicHarvestedPowerUpdateInterval", TimeValue (Seconds (1.0)));
    basicHarvesterHelper.Set ("HarvestablePower", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=0.009]"));
    EnergyHarvesterContainer harvesters = basicHarvesterHelper.Install (sources);
    
    Names::Add ("/Names/EnergySource", sources.Get (0));
    
    GnuplotHelper plotHelper;
    std::string fic = "gnuplot-energy-example" + std::string("_aes") + std::to_string(aesKey);
    plotHelper.ConfigurePlot (fic,
                             "Energy vs. Time ",
                             "Time (Seconds)",
                             "Energy(J)",
                             "png");

    plotHelper.PlotProbe ("ns3::DoubleProbe",
                         "/Names/EnergySource/RemainingEnergy",
                         "Output",
                         "EnergyRemaining",
                         GnuplotAggregator::KEY_INSIDE);                    
  // install device model
    DeviceEnergyModelContainer deviceModels = radioEnergyHelper.Install
      (endDevicesNetDevices, sources);

  /**************
   * Get output *
   **************/
    
    for(NodeContainer::Iterator n = endDevices.Begin (); n != endDevices.End (); n++){
         Ptr<Node> endDeviceobject = *n;
         Ptr<EnergySource> basicSourcePtr = DynamicCast<EnergySource> (sources.Get(endDeviceobject->GetId()));
    //Ptr<EnergySource> basicSourcePtr = DynamicCast<EnergySource> (sources.Get(1));
        basicSourcePtr->TraceConnect("RemainingEnergy",std::to_string(endDeviceobject->GetId()), MakeCallback(&RemainingEnergy));
    } 
    printposition(hub);
    printposition(endDevices);
  

 
   
 // Simulator::Stop (appStopTime + Hours (1)); 
   Simulator::Stop (appStopTime);
    Simulator::Run ();
 
  
   // GtkConfigStore config;
  // config.ConfigureAttributes ();
  
 
 
Simulator::Destroy ();
LoraPacketTracker &tracker = helper.GetPacketTracker ();
std::cout << tracker.CountMacPacketsGlobally (Seconds (0), Hours (1)) << std::endl;
//std::cout << tracker.CountMacPacketsGlobally (Seconds (0), appStopTime + Hours (1)) << std::endl;

  std::cout << tracker.CountAverageDelay (Seconds (0), Hours (1)) << std::endl;
  
    return 0;
}




MobilityHelper createscenario(double radio)
{
    MobilityHelper mobility;
    mobility.SetPositionAllocator ("ns3::UniformDiscPositionAllocator",
                                 "rho", DoubleValue (radio),
                                 "X", DoubleValue (0.0),
                                 "Y", DoubleValue (0.0),
                                 "Z", DoubleValue (0.0));
   //     mobility.SetMobilityModel ("ns3::GaussMarkovMobilityModel");
          mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
   
 
   
}





/*
MobilityHelper createscenario(double radio)
{
    MobilityHelper mobility;
    
ObjectFactory poss;
  poss.SetTypeId ("ns3::UniformDiscPositionAllocator");
  poss.Set("rho",DoubleValue (radio));
  poss.Set("X",DoubleValue (0.0));
  poss.Set("Y",DoubleValue (0.0));
  poss.Set("Z",DoubleValue (0.0));
  
  Ptr<PositionAllocator> taPositionAllocc = poss.Create ()->GetObject<PositionAllocator> ();
  mobility.SetMobilityModel ("ns3::RandomWaypointMobilityModel",
                        
                        "Speed", StringValue ("ns3::UniformRandomVariable[Min=20|Max=70]"),
                         "Pause", StringValue ("ns3::ConstantRandomVariable[Constant=2.0]"),
                        "PositionAllocator", PointerValue (taPositionAllocc));  
   return mobility; 

}

*/




void printposition(NodeContainer n){// iterate our endDevices and print their position.
    for (NodeContainer::Iterator j = n.Begin ();
           j != n.End (); ++j)
        {
        Ptr<Node> object = *j;
        Ptr<MobilityModel> position = object->GetObject<MobilityModel> ();
        NS_ASSERT (position != 0);
        Vector pos = position->GetPosition ();
        std::cout << "x=" << pos.x << ", y=" << pos.y << ", z=" << pos.z << std::endl;
    }
}
CommandLine setupcmd(){
    CommandLine cmd;
    cmd.AddValue ("radio", "Radio of the disc where random put the endDevices", radio);
    cmd.AddValue ("numendDevices", "Num. endDevices in the grid for simulating",numendDevices);
    return cmd;
}



MobilityHelper createstarcenter()
{
 MobilityHelper mobility;
 mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (1.0),
                                 "DeltaY", DoubleValue (1.0),
                                 "GridWidth", UintegerValue (1),
                                 "LayoutType", StringValue ("RowFirst"));
                                 
        mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
 
    return mobility;
                              
                                 


/*
MobilityHelper createstarcenter()
{
  MobilityHelper mobility;
  
  ObjectFactory pos;
  pos.SetTypeId ("ns3::GridPositionAllocator");
  pos.Set("MinX",DoubleValue (0.0));
  pos.Set("MinY",DoubleValue (0.0));
  pos.Set("DeltaX",DoubleValue (1.0));
  pos.Set("DeltaY",DoubleValue (1.0));
  pos.Set("GridWidth",UintegerValue (1));
  
  

  Ptr<PositionAllocator> taPositionAlloc = pos.Create ()->GetObject<PositionAllocator> ();

    mobility.SetMobilityModel ("ns3::RandomWaypointMobilityModel",
                        
                        "Speed", StringValue ("ns3::UniformRandomVariable[Min=20|Max=70]"),
                         "Pause", StringValue ("ns3::ConstantRandomVariable[Constant=2.0]"),
                        "PositionAllocator", PointerValue (taPositionAlloc));  

     return mobility;
 */    
}

 
  
 
  



 



