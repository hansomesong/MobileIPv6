/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 Jadavpur University, India
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
 * Author: Manoj Kumar Rana <manoj24.rana@gmail.com>
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
#include <algorithm>


/* *
   * Scenario:  3 UEs, 2 ENBs, 1 Remote Host, UE0<-->ENB0, UE1<-->ENB0, UE2<-->ENB1
                Servers: UE1, UE2, Remote Host
                Client: UE0 (3 clients)                
                UDP Echo Packets transmitted between client and server

   * Pass criteria: 1) Every UDP Echo Request and Reply messages sent and received respectively
                       at UE0 must be matched by their UID, source address, destination address,
                       source port and destination port
                    2) Every request reply must follow proper route (e.g. In case of UE0->UE1,
                       packet must travel this route: UE0->ENB0->PGW->ENB1->UE1->ENB1->PGW->ENB0->UE0)
                    3) The above check also ensures no redundancy of the followed route for a packet
* */



using namespace ns3;

class LenaIpv6RoutingTestCase : public TestCase
{
public:
  LenaIpv6RoutingTestCase ();
  virtual ~LenaIpv6RoutingTestCase ();

  /**
   * \brief Initialize testing parameters.
   */
  void Checker ();

  /**
   * \brief sent Packets from client's IPv6 interface.
   * \param: p packet
   * \param: ipv6 Ipv6 object
   * \param: interface Ipv6interface from which the packet is transmitted
   */
  void SentAtClient (Ptr<const Packet> p, Ptr<Ipv6> ipv6, uint32_t interface);

  /**
   * \brief Received Packets at client's IPv6 interface.
   * \param: p packet
   * \param: ipv6 Ipv6 object
   * \param: interface Ipv6interface at which the packet is received
   */
  void ReceivedAtClient (Ptr<const Packet> p, Ptr<Ipv6> ipv6, uint32_t interface);

  /**
   * \brief Received Packet at pgw from enb.
   * \param: p packet
   */
  void EnbToPgw (Ptr<Packet> p);

  /**
   * \brief Received Packet at pgw from enb.
   * \param: p packet
   */
  void TunToPgw (Ptr<Packet> p);

  /**
   * \brief search uid in a list and delete if found.
   * \param: l list
   * \param: item a uid
   */
  bool SearchAndDelete (std::list<uint64_t> *l, uint64_t item);


private:
  virtual void DoRun (void);
  Ipv6InterfaceContainer ueIpIface; //IPv6 interface container for ue
  Ipv6Address remoteHostAddr; //remote host address
  std::list<uint64_t> * EnbPgw; //list of uids of packets received at pgw from enb 
  std::list<uint64_t> * TunPgw; //list of uids of packets received at pgw from tunnel net device

  std::list<Ptr<Packet> > ClientSent; //list of sent packets from client
  std::list<Ptr<Packet> > ClientReceived; //list of received packets at client
};


LenaIpv6RoutingTestCase::LenaIpv6RoutingTestCase ()
  : TestCase ("Test IPv6 Routing at LTE")
{
  EnbPgw = new std::list<uint64_t>;
  TunPgw = new std::list<uint64_t>;
}

LenaIpv6RoutingTestCase::~LenaIpv6RoutingTestCase ()
{
}

void LenaIpv6RoutingTestCase::SentAtClient (Ptr<const Packet> p, Ptr<Ipv6> ipv6, uint32_t interface)
{
  Ipv6Header ipv6Header;
  p->PeekHeader (ipv6Header);
  if (ipv6Header.GetNextHeader () == UdpL4Protocol::PROT_NUMBER)
    {
      ClientSent.push_back (p->Copy ());
    }
}

void LenaIpv6RoutingTestCase::ReceivedAtClient (Ptr<const Packet> p, Ptr<Ipv6> ipv6, uint32_t interface)
{
  Ipv6Header ipv6Header;
  p->PeekHeader (ipv6Header);
  if (ipv6Header.GetNextHeader () == UdpL4Protocol::PROT_NUMBER)
    {
      ClientReceived.push_back (p->Copy ());
    }
}

void LenaIpv6RoutingTestCase::EnbToPgw (Ptr<Packet> p)
{  
  Ipv6Header ipv6Header;
  p->PeekHeader (ipv6Header);
  if (ipv6Header.GetNextHeader () == UdpL4Protocol::PROT_NUMBER)
    {
      EnbPgw->push_back (p->GetUid ());
    }
}

void LenaIpv6RoutingTestCase::TunToPgw (Ptr<Packet> p)
{  
  Ipv6Header ipv6Header;
  p->PeekHeader (ipv6Header);
  if (ipv6Header.GetNextHeader () == UdpL4Protocol::PROT_NUMBER)
    {
      TunPgw->push_back (p->GetUid ());
    }
}

bool LenaIpv6RoutingTestCase::SearchAndDelete (std::list<uint64_t> *l, uint64_t item)
{
  bool b = false;
  std::list<uint64_t>::iterator it;
  it = std::find (l->begin (), l->end (), item);
  if (it != l->end ())
    {
      b = true;
      l->erase(it);
    }
  return b;
}

void LenaIpv6RoutingTestCase::Checker ()
{
  bool b = false;
  bool check = true;
  //Extract each received reply packet of the client
  for (std::list<Ptr<Packet> >::iterator it1 = ClientReceived.begin (); it1 != ClientReceived.end (); it1++)
    {
      Ipv6Header ipv6header1;
      UdpHeader udpHeader1;
      Ptr<Packet> p1 = (*it1)->Copy ();
      p1->RemoveHeader (ipv6header1);
      uint64_t uid = p1->GetUid ();
      p1->RemoveHeader (udpHeader1);
      //Search each packet in list of sent request packet of the client
      for (std::list<Ptr<Packet> >::iterator it2 = ClientSent.begin (); it2 != ClientSent.end (); it2++)
        {
          Ptr<Packet> p2 = (*it2)->Copy ();
          Ipv6Header ipv6header2;
          p2->RemoveHeader (ipv6header2);
          Ipv6Address sorceAddress = ipv6header2.GetSourceAddress ();
          Ipv6Address destinationAddress = ipv6header2.GetDestinationAddress ();
          UdpHeader udpHeader2;
          p2->RemoveHeader (udpHeader2);
          uint16_t sourcePort;
          uint16_t destinationPort;
          sourcePort = udpHeader2.GetSourcePort ();
          destinationPort = udpHeader2.GetDestinationPort ();
          //Check whether the uids, addresses and ports match
          if ((p2->GetUid () == p1->GetUid ()) && sorceAddress.IsEqual (ipv6header1.GetDestinationAddress ()) && destinationAddress.IsEqual (ipv6header1.GetSourceAddress ()) && sourcePort == udpHeader1.GetDestinationPort () && destinationPort == udpHeader1.GetSourcePort ())
            { 
               b=true;
               break;
            }
        }  
      check &= b;
      if (ipv6header1.GetSourceAddress ().IsEqual (remoteHostAddr))
        {
          check &= (SearchAndDelete (EnbPgw, uid) && SearchAndDelete (TunPgw, uid));
        }
      else
        {
          check &= (SearchAndDelete (EnbPgw, uid) && SearchAndDelete (TunPgw, uid) && SearchAndDelete (EnbPgw, uid) && SearchAndDelete (TunPgw, uid));
        }

     b = false;
  }

  NS_TEST_ASSERT_MSG_EQ (check, true, "Failure Happens IPv6 routing of LENA");
  NS_TEST_ASSERT_MSG_EQ (ClientSent.size (), ClientReceived.size (), "No. of Request and Reply messages mismatch");
  NS_TEST_ASSERT_MSG_EQ (EnbPgw->size (), 0, "Route is not Redundant in Lena IPv6 test");
  NS_TEST_ASSERT_MSG_EQ (TunPgw->size (), 0, "Route is not Redundant in Lena IPv6 test");

}

void
LenaIpv6RoutingTestCase::DoRun (void)
{
  double distance = 60.0;

  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
  Ptr<PointToPointEpcHelper>  epcHelper = CreateObject<PointToPointEpcHelper> ();
  lteHelper->SetEpcHelper (epcHelper);

  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults ();

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
  enbNodes.Create (2);
  ueNodes.Create (3);

  // Install Mobility Model
  Ptr<ListPositionAllocator> positionAlloc1 = CreateObject<ListPositionAllocator> ();
  Ptr<ListPositionAllocator> positionAlloc2 = CreateObject<ListPositionAllocator> ();

  positionAlloc1->Add (Vector (distance * 0, 0, 0));
  positionAlloc1->Add (Vector (distance * 0 + 5, 0, 0));
  positionAlloc1->Add (Vector (distance * 1, 0, 0));

  positionAlloc2->Add (Vector (distance * 0, 0, 0));
  positionAlloc2->Add (Vector (distance * 1, 0, 0));

  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator (positionAlloc1);
  mobility.Install (ueNodes);

  mobility.SetPositionAllocator (positionAlloc2);
  mobility.Install (enbNodes);

  // Install the IP stack on the UEs
  internet.Install (ueNodes);

  // Install LTE Devices to the nodes
  NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice (enbNodes);
  NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice (ueNodes);



  for (NetDeviceContainer::Iterator it = ueLteDevs.Begin (); it != ueLteDevs.End (); ++it)
    (*it)->SetAddress (Mac48Address::Allocate ());

  // Assign IP address to UEs, and install applications
  ueIpIface = epcHelper->AssignUeIpv6Address (NetDeviceContainer (ueLteDevs));


  Ipv6StaticRoutingHelper ipv6RoutingHelper;

  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
      Ptr<Node> ueNode = ueNodes.Get (u);
      // Set the default gateway for the UE
      Ptr<Ipv6StaticRouting> ueStaticRouting = ipv6RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv6> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress6 (), 1);
    }


  // Attach two UEs at first eNodeB and one UE at second eNodeB
  lteHelper->Attach (ueLteDevs.Get (0), enbLteDevs.Get (0));
  lteHelper->Attach (ueLteDevs.Get (1), enbLteDevs.Get (0));
  lteHelper->Attach (ueLteDevs.Get (2), enbLteDevs.Get (1));


  Ipv6AddressHelper ipv6h;
  ipv6h.SetBase (Ipv6Address ("6001:db80::"), Ipv6Prefix (64));
  Ipv6InterfaceContainer internetIpIfaces = ipv6h.Assign (internetDevices);

  internetIpIfaces.SetForwarding (0, true);
  internetIpIfaces.SetDefaultRouteInAllNodes (0);

 
  Ptr<Ipv6StaticRouting> remoteHostStaticRouting = ipv6RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv6> ());
  remoteHostStaticRouting->AddNetworkRouteTo (epcHelper->GetEpcIpv6NetworkAddress (), Ipv6Prefix (48), internetIpIfaces.GetAddress (0, 1), 1, 0);


  // interface 0 is localhost, 1 is the p2p device
  remoteHostAddr = internetIpIfaces.GetAddress (1, 1);


  // Install and start applications on UEs and remote host
  UdpEchoServerHelper echoServer1 (10);
  UdpEchoServerHelper echoServer2 (11);
  UdpEchoServerHelper echoServer3 (12);

  ApplicationContainer serverApps = echoServer1.Install (remoteHost);
  serverApps.Add (echoServer2.Install (ueNodes.Get (1)));
  serverApps.Add (echoServer3.Install (ueNodes.Get (2)));


  serverApps.Start (Seconds (4.0));
  serverApps.Stop (Seconds (12.0));


  UdpEchoClientHelper echoClient1 (remoteHostAddr, 10);
  UdpEchoClientHelper echoClient2 (ueIpIface.GetAddress (1,1), 11);
  UdpEchoClientHelper echoClient3 (ueIpIface.GetAddress (2,1), 12);

  echoClient1.SetAttribute ("MaxPackets", UintegerValue (1000));
  echoClient1.SetAttribute ("Interval", TimeValue (Seconds (0.2)));
  echoClient1.SetAttribute ("PacketSize", UintegerValue (1024));

  echoClient2.SetAttribute ("MaxPackets", UintegerValue (1000));
  echoClient2.SetAttribute ("Interval", TimeValue (Seconds (0.2)));
  echoClient2.SetAttribute ("PacketSize", UintegerValue (1024));

  echoClient3.SetAttribute ("MaxPackets", UintegerValue (1000));
  echoClient3.SetAttribute ("Interval", TimeValue (Seconds (0.2)));
  echoClient3.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps1 = echoClient1.Install (ueNodes.Get(0));
  ApplicationContainer clientApps2 = echoClient2.Install (ueNodes.Get(0));
  ApplicationContainer clientApps3 = echoClient3.Install (ueNodes.Get(0));

  clientApps1.Start (Seconds (4.0));
  clientApps1.Stop (Seconds (6.0));  

  clientApps2.Start (Seconds (6.1));
  clientApps2.Stop (Seconds (8.0));

  clientApps3.Start (Seconds (8.1));
  clientApps3.Stop (Seconds (10.0));

  EnbPgw = new std::list<uint64_t>;
  TunPgw = new std::list<uint64_t>;

  //Set Cllback for Client Sent and Received packets
  Ptr<Ipv6L3Protocol> ipl3 = (ueNodes.Get(0))->GetObject<Ipv6L3Protocol> ();
  ipl3->TraceConnectWithoutContext ("Tx", MakeCallback(&LenaIpv6RoutingTestCase::SentAtClient, this));
  ipl3->TraceConnectWithoutContext ("Rx", MakeCallback(&LenaIpv6RoutingTestCase::ReceivedAtClient, this));

  //Set Callback at SgwPgWApplication of epc to get the packets from enb and from tunnel net device
  Ptr<Application> apppgw = pgw->GetApplication (0);
  apppgw->TraceConnectWithoutContext ("RxfromS1u", MakeCallback(&LenaIpv6RoutingTestCase::EnbToPgw, this));
  apppgw->TraceConnectWithoutContext ("RxfromTun", MakeCallback(&LenaIpv6RoutingTestCase::TunToPgw, this));

  Simulator::Schedule (Time (Seconds (12.0)), &LenaIpv6RoutingTestCase::Checker, this);

  Simulator::Stop (Seconds (14));
  Simulator::Run ();

  Simulator::Destroy ();
}


/**
 * \brief test suite 1
 */
class LenaIpv6RoutingTestSuite : public TestSuite
{
  public:
    LenaIpv6RoutingTestSuite ();
};

LenaIpv6RoutingTestSuite::LenaIpv6RoutingTestSuite ()
  : TestSuite ("lena-ipv6-routing-test", UNIT)
{
  AddTestCase (new LenaIpv6RoutingTestCase, TestCase::QUICK);
}

static LenaIpv6RoutingTestSuite lenaipv6testsuite;
