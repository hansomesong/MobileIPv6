#ifndef MIPV6_HELPER_H
#define MIPV6_HELPER_H

#include "ns3/node-container.h"
#include "ns3/net-device-container.h"
#include "ns3/packet.h"
#include "ns3/ptr.h"
#include "ns3/object-factory.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/ipv6-l3-protocol.h"
#include "ns3/trace-helper.h"


namespace ns3 {

class Node;

class mipv6HAHelper {
public:
  mipv6HAHelper();
  ~mipv6HAHelper();
  /**
   * 
   * \param node The node on which to install the stack.
   */
  void Install (Ptr<Node> node);
  
  std::list<Ipv6Address> GetHomeAgentAddressList();
protected:

private:
Ptr<Node> m_node;
  };
//CN
class mipv6CNHelper {
public:
  mipv6CNHelper();
  ~mipv6CNHelper();
  
  /**
   * 
   * \param node The node on which to install the stack.
   */
  void Install (Ptr<Node> node) const;
  
protected:

private:
  
};

class mipv6MNHelper {
public:
  mipv6MNHelper(std::list<Ipv6Address> haalist, bool rotopt);
  ~mipv6MNHelper();
  /**
   * 
   * \param node The node on which to install the stack.
   */
  void Install (Ptr<Node> node) const;
  
  
protected:

private:

  std::list<Ipv6Address> m_Haalist;
  bool m_rotopt;

  };

} // namespace ns3

#endif /* MIPv6_HELPER_H */
