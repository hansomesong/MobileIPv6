#include <stdio.h>
#include <sstream>

#include "ns3/log.h"
#include "ns3/assert.h"
#include "ns3/packet.h"
#include "ns3/node.h"
#include "ns3/boolean.h"
#include "ns3/ipv6-routing-protocol.h"
#include "ns3/ipv6-route.h"
#include "ns3/ipv6-l3-protocol.h"
#include "ns3/ipv6-interface.h"
#include "ns3/ipv6-extension-header.h"

#include "ns3/wifi-net-device.h"
#include "ns3/wifi-mac.h"
#include "ns3/regular-wifi-mac.h"
#include "ns3/wimax-net-device.h"

#include "ns3/ipv6-static-routing-helper.h"
#include "ns3/ipv6-static-routing.h"

#include "mipv6-header.h"
#include "mipv6-mobility.h"
#include "mipv6-demux.h"
#include "mipv6-l4-protocol.h"

#include "mipv6-cn.h"

using namespace std;

NS_LOG_COMPONENT_DEFINE ("mipv6CN");

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED (mipv6CN);

mipv6CN::mipv6CN ()
: m_bCache (0)
{
}

mipv6CN::~mipv6CN ()
{
m_bCache = 0;
}

void mipv6CN::NotifyNewAggregate ()
{
  if(GetNode () == 0)
    {
      Ptr<Node> node = this->GetObject<Node> ();
      m_bCache = CreateObject<BCache> ();	  
	  
      
      SetNode (node);
      m_bCache->SetNode (node);



    }
    
  mipv6Agent::NotifyNewAggregate ();
}


Ptr<Packet> mipv6CN::BuildBA(Ipv6MobilityBindingUpdateHeader bu, Ipv6Address hoa, uint8_t status) 
{
  NS_LOG_FUNCTION (this << status << "BUILD BACK");
  
   Ptr<Packet> p = Create<Packet> ();
  
  Ipv6MobilityBindingAckHeader ba;
  Ipv6ExtensionType2RoutingHeader type2extn;
  type2extn.SetReserved(0);
  type2extn.SetHomeAddress(hoa);
  
  ba.SetSequence (bu.GetSequence ());
  ba.SetFlagK (true);
  ba.SetStatus (status);
  ba.SetLifetime ((uint16_t)MIPv6L4Protocol::MAX_BINDING_LIFETIME);
  p->AddHeader(type2extn);
  p->AddHeader (ba);
  
  return p;
}

Ptr<Packet> mipv6CN::BuildHoT (Ipv6HoTIHeader hoti, Ipv6Address hoa)
{

  Ptr<Packet> p = Create<Packet> ();
  BCache::Entry *bce;
  bce=m_bCache->Lookup(hoa);

  Ipv6HoTHeader hot;

  hot.SetReserved (0);
  hot.SetHomeInitCookie(hoti.GetHomeInitCookie());
  hot.SetHomeNonceIndex(bce->GetHomeNonceIndex());
  hot.SetHomeKeygenToken(bce->GetHomeKeygenToken());
  Ipv6ExtensionType2RoutingHeader type2extn;
  type2extn.SetReserved(0);
  type2extn.SetHomeAddress(hoa);
  
 

  p->AddHeader(type2extn); 
  p->AddHeader(hot);

  return p;
}

Ptr<Packet> mipv6CN::BuildCoT (Ipv6CoTIHeader coti, Ipv6Address hoa)
{

  Ptr<Packet> p = Create<Packet> ();
  BCache::Entry *bce;
  bce=m_bCache->Lookup(hoa);

  Ipv6CoTHeader cot;

  
 
  cot.SetReserved (0);
  cot.SetCareOfInitCookie(coti.GetCareOfInitCookie());
  cot.SetCareOfNonceIndex(bce->GetCareOfNonceIndex());
  cot.SetCareOfKeygenToken(bce->GetCareOfKeygenToken());
  Ipv6ExtensionType2RoutingHeader type2extn;
  type2extn.SetReserved(0);
  type2extn.SetHomeAddress(hoa);
  
 

  p->AddHeader(type2extn); 

 
  p->AddHeader(cot);

  return p;
}


uint8_t mipv6CN::HandleBU(Ptr<Packet> packet, const Ipv6Address &src, const Ipv6Address &dst, Ptr<Ipv6Interface> interface)
{
  NS_LOG_FUNCTION (this << packet << src << dst << interface);
  
  Ptr<Packet> p = packet->Copy ();
  
  Ipv6MobilityBindingUpdateHeader bu;
  p->RemoveHeader (bu);
  
  Ipv6ExtensionDestinationHeader dest;
  p->RemoveHeader (dest);

  Buffer buf;
  
  Buffer::Iterator start;
  buf=dest.GetOptionBuffer();
  start=buf.Begin();
  Ipv6HomeAddressOptionHeader homopt;
  homopt.Deserialize(start);
  Ipv6Address homeaddr;
  homeaddr=homopt.GetHomeAddress();
  
  Ptr<MIPv6Demux> ipv6MobilityDemux = GetNode ()->GetObject<MIPv6Demux> ();
  NS_ASSERT (ipv6MobilityDemux);
  
  Ptr<MIPv6Mobility> ipv6Mobility = ipv6MobilityDemux->GetMobility (bu.GetMhType ());
  NS_ASSERT (ipv6Mobility);
  
  
  uint8_t errStatus = 0;
  BCache::Entry *bce = 0;
  BCache::Entry *bce2 = 0;
  
  //bool delayedRegister = false;
  

  bce = m_bCache->Lookup (homeaddr);
  if(bce)
  {
  bce->SetCoa(src);
  bce->SetHoa(homeaddr);
  bce->SetHA(dst);
  bce->SetLastBindingUpdateSequence(bu.GetSequence());
  bce->SetLastBindingUpdateTime(Time(bu.GetLifetime()));
  errStatus = MIPv6Header::BA_STATUS_BINDING_UPDATE_ACCEPTED;
  bce->MarkReachable();
  }

  else
  {
  bce2->SetCoa(src);
  bce2->SetHoa(homeaddr);
  bce2->SetHA(dst);
  bce2->SetLastBindingUpdateSequence(bu.GetSequence());
  bce2->SetLastBindingUpdateTime(Time(bu.GetLifetime()));
  errStatus = MIPv6Header::BA_STATUS_BINDING_UPDATE_ACCEPTED;
  bce2->MarkReachable();

  m_bCache->Add(bce2);
  }

  if(bu.GetFlagA())
  {
    Ptr<Packet> ba;
    ba = BuildBA (bu, homeaddr, errStatus);
      
    SendMessage (ba, src, 64);
  }

  return 0;
}
uint8_t mipv6CN::HandleHoTI (Ptr<Packet> packet, const Ipv6Address &src, const Ipv6Address &dst, Ptr<Ipv6Interface> interface)
{
  NS_LOG_FUNCTION (this << packet << src << dst << interface);
  
  Ptr<Packet> p = packet->Copy ();
  
  Ipv6HoTIHeader hoti;
  p->RemoveHeader (hoti);
  
  Ipv6ExtensionDestinationHeader dest;
  p->RemoveHeader (dest);

  Buffer buf;
  
  Buffer::Iterator start;
  buf=dest.GetOptionBuffer();
  start=buf.Begin();
  Ipv6HomeAddressOptionHeader homopt;
  homopt.Deserialize(start);
  Ipv6Address homeaddr;
  homeaddr=homopt.GetHomeAddress();
  
  Ptr<MIPv6Demux> ipv6MobilityDemux = GetNode ()->GetObject<MIPv6Demux> ();
  NS_ASSERT (ipv6MobilityDemux);
  
  Ptr<MIPv6Mobility> ipv6Mobility = ipv6MobilityDemux->GetMobility (hoti.GetMhType ());
  NS_ASSERT (ipv6Mobility);
  
  

  BCache::Entry *bce = 0;

  
  //bool delayedRegister = false;
  

  bce = m_bCache->Lookup (homeaddr);
  

  if (bce)
  {
  bce->SetHomeInitCookie(hoti.GetHomeInitCookie());
  }
  else
  {
  BCache::Entry *bce2 = 0;
  bce2->SetHoa(homeaddr);
  bce2->SetHomeInitCookie(hoti.GetHomeInitCookie());
  m_bCache->Add(bce2);
  }


    Ptr<Packet> hot;
    hot = BuildHoT (hoti, homeaddr);
      
    SendMessage (hot, src, 64);

    return 0;
}

uint8_t mipv6CN::HandleCoTI (Ptr<Packet> packet, const Ipv6Address &src, const Ipv6Address &dst, Ptr<Ipv6Interface> interface)
{
  NS_LOG_FUNCTION (this << packet << src << dst << interface);
  
  Ptr<Packet> p = packet->Copy ();
  
  Ipv6CoTIHeader coti;
  p->RemoveHeader (coti);
  
  Ipv6ExtensionDestinationHeader dest;
  p->RemoveHeader (dest);

  Buffer buf;
  
  Buffer::Iterator start;
  buf=dest.GetOptionBuffer();
  start=buf.Begin();
  Ipv6HomeAddressOptionHeader homopt;
  homopt.Deserialize(start);
  Ipv6Address homeaddr;
  homeaddr=homopt.GetHomeAddress();
  
  Ptr<MIPv6Demux> ipv6MobilityDemux = GetNode ()->GetObject<MIPv6Demux> ();
  NS_ASSERT (ipv6MobilityDemux);
  
  Ptr<MIPv6Mobility> ipv6Mobility = ipv6MobilityDemux->GetMobility (coti.GetMhType ());
  NS_ASSERT (ipv6Mobility);
  
  

  BCache::Entry *bce = 0;

  
  //bool delayedRegister = false;
  

  bce = m_bCache->Lookup (homeaddr);
  

  if (bce)
  {
  bce->SetCareOfInitCookie(coti.GetCareOfInitCookie());
  }
  else
  {
  BCache::Entry *bce2 = 0;
  bce2->SetHoa(homeaddr);
  bce2->SetCareOfInitCookie(coti.GetCareOfInitCookie());
  m_bCache->Add(bce2);
  }


    Ptr<Packet> cot;
    cot = BuildCoT (coti, homeaddr);
      
    SendMessage (cot, src, 64);

    return 0;
}


} /* namespace ns3 */

