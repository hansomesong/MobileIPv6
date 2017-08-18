/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Jaume Nin <jaume.nin@cttc.cat>
 */

#include "ns3/lte-helper.h"
#include "ns3/epc-helper.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/ipv6-static-routing.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/config-store.h"

using namespace ns3;

/**
 * Sample simulation script for LTE+EPC. It instantiates several eNodeB,
 * attaches one UE per eNodeB starts a flow for each UE to  and from a remote host.
 */

NS_LOG_COMPONENT_DEFINE ("EpcFirstExampleForIpv6");

int
main (int argc, char *argv[])
{

  uint16_t numberOfNodes = 2;
  double simTime = 100;
  double distance = 60.0;


  // Command line arguments
  CommandLine cmd;
  cmd.AddValue ("numberOfNodes", "Number of eNodeBs + UE pairs", numberOfNodes);
  cmd.AddValue ("simTime", "Total duration of the simulation [s])", simTime);
  cmd.AddValue ("distance", "Distance between eNBs [m]", distance);
  cmd.Parse (argc, argv);

  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
  Ptr<PointToPointEpcHelper>  epcHelper = CreateObject<PointToPointEpcHelper> ();
  lteHelper->SetEpcHelper (epcHelper);

  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults ();

  // parse again so you can override default values from the command line
  cmd.Parse (argc, argv);

  Ptr<Node> pgw = epcHelper->GetPgwNode ();

   // Create a single RemoteHost
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);
  InternetStackHelper internet;
  internet.Install (remoteHostContainer);

  // Create the Internet
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);

  NodeContainer ueNodes;
  NodeContainer enbNodes;
  enbNodes.Create (numberOfNodes);
  ueNodes.Create (numberOfNodes);

  // Install Mobility Model
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  for (uint16_t i = 0; i < numberOfNodes; i++)
    {
      positionAlloc->Add (Vector (distance * i, 0, 0));
    }
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator (positionAlloc);
  mobility.Install (enbNodes);
  mobility.Install (ueNodes);

  // Install the IP stack on the UEs
  internet.Install (ueNodes);

  // Install LTE Devices to the nodes
  NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice (enbNodes);
  NetDeviceContainer ueLteDevs1 = lteHelper->InstallUeDevice (NodeContainer(ueNodes.Get (0)));
  NetDeviceContainer ueLteDevs2 = lteHelper->InstallUeDevice (NodeContainer(ueNodes.Get (1)));

  Ipv6InterfaceContainer ueIpIface;

  for (NetDeviceContainer::Iterator it = ueLteDevs1.Begin (); it != ueLteDevs1.End (); ++it)
    (*it)->SetAddress (Mac48Address::Allocate ());

  for (NetDeviceContainer::Iterator it = ueLteDevs2.Begin (); it != ueLteDevs2.End (); ++it)
    (*it)->SetAddress (Mac48Address::Allocate ());


  Ipv6AddressHelper ipv6h;
  ipv6h.SetBase (Ipv6Address ("6001:db80::"), Ipv6Prefix (64));
  Ipv6InterfaceContainer internetIpIfaces = ipv6h.Assign (internetDevices);

  internetIpIfaces.SetForwarding (0, true);
  internetIpIfaces.SetDefaultRouteInAllNodes (0);


  //Set prefix value for the first UE
  epcHelper->SetAttribute ("BaseIpv6Prefix", Ipv6AddressValue (Ipv6Address ("2a2a:2001:ef00::")));
  // Assign IP address to the first UE
  ueIpIface = epcHelper->AssignUeIpv6Address (NetDeviceContainer (ueLteDevs1));

  //set new route for the first UE
  Ipv6AddressValue ipval;
  epcHelper->GetAttribute ("BaseIpv6Prefix", ipval);
  Ipv6Address prefix = ipval.Get ();

  Ipv6StaticRoutingHelper ipv6RoutingHelper; 
  Ptr<Ipv6StaticRouting> remoteHostStaticRouting = ipv6RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv6> ());
  remoteHostStaticRouting->AddNetworkRouteTo (prefix, Ipv6Prefix (48), internetIpIfaces.GetAddress (0, 1), 1, 0);


  //Set prefix value for the second UE
  epcHelper->SetAttribute ("BaseIpv6Prefix", Ipv6AddressValue (Ipv6Address (("3a3a:2001:ef00::"))));
  // Assign IP address to the second UE
  ueIpIface.Add (epcHelper->AssignUeIpv6Address (NetDeviceContainer (ueLteDevs2)));

  //set new route for the second UE
  epcHelper->GetAttribute ("BaseIpv6Prefix", ipval);
  prefix = ipval.Get ();
 
  remoteHostStaticRouting->AddNetworkRouteTo (prefix, Ipv6Prefix (48), internetIpIfaces.GetAddress (0, 1), 1, 0);


  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
      Ptr<Node> ueNode = ueNodes.Get (u);
      // Set the default gateway for the UE
      Ptr<Ipv6StaticRouting> ueStaticRouting = ipv6RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv6> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress6 (), 1);
    }


  // Attach one UE per eNodeB
  lteHelper->Attach (ueLteDevs1.Get (0), enbLteDevs.Get (0));
  lteHelper->Attach (ueLteDevs2.Get (0), enbLteDevs.Get (1));






  // interface 0 is localhost, 1 is the p2p device
  Ipv6Address remoteHostAddr = internetIpIfaces.GetAddress (1, 1);


  // Install and start applications on UEs and remote host

  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (remoteHost);


  serverApps.Start (Seconds (4.0));
  serverApps.Stop (Seconds (50.0));


  UdpEchoClientHelper echoClient1 (remoteHostAddr, 9);
  UdpEchoClientHelper echoClient2 (remoteHostAddr, 9);

  echoClient1.SetAttribute ("MaxPackets", UintegerValue (1000));
  echoClient1.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient1.SetAttribute ("PacketSize", UintegerValue (1024));

  echoClient2.SetAttribute ("MaxPackets", UintegerValue (1000));
  echoClient2.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient2.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps1 = echoClient1.Install (ueNodes.Get(0));
  ApplicationContainer clientApps2 = echoClient2.Install (ueNodes.Get(1));


  clientApps1.Start (Seconds (4.0));
  clientApps1.Stop (Seconds (50.0));  

  clientApps2.Start (Seconds (4.5));
  clientApps2.Stop (Seconds (50.0));

  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_ALL);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_ALL);


  internet.EnablePcapIpv6 ("lena1", ueNodes.Get (0));
  internet.EnablePcapIpv6 ("lena2", ueNodes.Get (1));
  internet.EnablePcapIpv6 ("lena3", remoteHostContainer.Get (0));
  internet.EnablePcapIpv6 ("lena4", pgw);

  Ipv6StaticRoutingHelper routingHelper;
  Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> (&std::cout);
  routingHelper.PrintRoutingTableAt (Seconds (4.3), pgw, routingStream);


  Simulator::Stop (Seconds (simTime));
  Simulator::Run ();

  Simulator::Destroy ();
  return 0;

}

