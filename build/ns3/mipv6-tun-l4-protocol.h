#ifndef IPV6_TUNNEL_L4_PROTOCOL_H
#define IPV6_TUNNEL_L4_PROTOCOL_H

#include "ns3/ipv6-address.h"
#include "ns3/ip-l4-protocol.h"
#include "ns3/tunnel-net-device.h"
#include <iostream>
#include <fstream>
namespace ns3
{

class Node;
class Packet;

/**
 * \class Ipv6TunnelL4Protocol
 * \brief An implementation of the Ipv6 Tunnel protocol.
 */
class Ipv6TunnelL4Protocol : public IpL4Protocol
{
public:
  /**
   * \brief Interface ID
   */
  static TypeId GetTypeId ();

  /**
   * \brief IPv6 Mobility protocol number (135).
   */
  static const uint8_t PROT_NUMBER;
  
  /**
   * \brief Get MIPv6 protocol number.
   * \return protocol number
   */
  static uint16_t GetStaticProtocolNumber ();

  /**
   * \brief Constructor.
   */
  Ipv6TunnelL4Protocol ();

  /**
   * \brief Destructor.
   */
  virtual ~Ipv6TunnelL4Protocol ();

  /**
   * \brief Set the node.
   * \param node the node to set
   */
  void SetNode (Ptr<Node> node);
  
  /**
   * \brief Get the node.
   * \return node
   */
  Ptr<Node> GetNode (void);

    /**
   * \brief This method is called by AddAgregate and completes the aggregation
   * by setting the node in the Ipv6 Mobility stack.
   */
  virtual void NotifyNewAggregate ();

  /**
   * \brief Get the protocol number.
   * \return protocol number
   */
  virtual int GetProtocolNumber () const;

  /**
   * \brief Send a packet via IPv6 Mobility, note that packet already contains IPv6 Mobility header.
   * \param packet the packet to send which contains IPv6 Mobility header
   * \param src source address
   * \param dst destination address
   * \param ttl next hop limit
   */
  void SendMessage (Ptr<Packet> packet, Ipv6Address src, Ipv6Address dst, uint8_t ttl);
  
  /**
   * \brief Receive method.
   * \param p the packet
   * \param src source address
   * \param dst destination address
   * \param interface the interface from which the packet is coming
   */
  virtual enum IpL4Protocol::RxStatus Receive (Ptr<Packet> p,Ipv6Header const &header,Ptr<Ipv6Interface> incomingInterface); 
  virtual void SetDownTarget (IpL4Protocol::DownTargetCallback cb);
  virtual void SetDownTarget6 (IpL4Protocol::DownTargetCallback6 cb);
  virtual IpL4Protocol::DownTargetCallback GetDownTarget (void) const;
  virtual IpL4Protocol::DownTargetCallback6 GetDownTarget6 (void) const;
  virtual enum IpL4Protocol::RxStatus Receive (Ptr<Packet> p,
                                 Ipv4Header const &header,
                                 Ptr<Ipv4Interface> incomingInterface);

  uint16_t AddTunnel(Ipv6Address remote, Ipv6Address local=Ipv6Address::GetZero());
  void RemoveTunnel(Ipv6Address remote);
  uint16_t ModifyTunnel(Ipv6Address remote, Ipv6Address newRemote, Ipv6Address local=Ipv6Address::GetZero());
  Ptr<TunnelNetDevice> GetTunnelDevice(Ipv6Address remote);
  
  
  
  // myfile.open ("example.txt");
  //myfile.close();
  //void filewriter(uint64_t u, Time t);
  void SetHomeAddress(Ipv6Address hoa);
  Ipv6Address GetHomeAddress();
  
  
protected:
 
  /**
   * \brief Dispose this object.
   */
  virtual void DoDispose ();
  
private:
  typedef std::map<Ipv6Address, Ptr<TunnelNetDevice> > TunnelMap;
  typedef std::map<Ipv6Address, Ptr<TunnelNetDevice> >::iterator TunnelMapI;
  
  /**
   * \brief The node.
   */
  Ptr<Node> m_node;
  uint64_t counter;
  Time sctime;
  //std::ofstream myfile;
  TunnelMap m_tunnelMap;
  Ipv6Address m_hoa;
};

} /* namespace ns3 */

#endif /* IPV6_TUNNEL_L4_PROTOCOL_H */
