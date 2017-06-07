
#ifndef MIPv6_OPTION_H
#define MIPV6_OPTION_H

#include <list>

#include "ns3/object.h"
#include "ns3/node.h"
#include "ns3/ptr.h"
#include "ns3/header.h"
#include "ns3/ipv6-address.h"
#include "ns3/nstime.h"
#include "ns3/identifier.h"

namespace ns3
{

class MIPv6OptionBundle;

/**
 * \class Ipv6MobilityOption
 * \brief Ipv6 Mobility Option base
 *
 * If you want to implement a new Ipv6 Mobility option, all you have to do is
 * implement a subclass of this class and add it to an Ipv6MobilityOptionDemux.
 */
class MIPv6Option : public Object
{
public:
  /**
   * \brief Get the type identificator.
   * \return type identificator
   */
  static TypeId GetTypeId (void);
  
  /**
   * \brief Destructor.
   */
  virtual ~MIPv6Option ();
  
  /**
   * \brief Set the node.
   * \param node the node to set
   */
  void SetNode (Ptr<Node> node);
  
  /**
   * \brief Get the option number.
   * \return option number
   */
  virtual uint8_t GetMobilityOptionNumber () const = 0;
  
  /**
   * \brief Process method
   *
   * Called from Ipv6MobilityL4Protocol::Receive.
   * \param packet the packet
   * \param bundle bundle of all option data
   * \return the processed size
   */
  virtual uint8_t Process (Ptr<Packet> packet, uint8_t offset, MIPv6OptionBundle& bundle) = 0;
  
private:
  /**
   * \brief The node.
   */
  Ptr<Node> m_node;
};

class MIPv6OptionBundle
{
public:
  MIPv6OptionBundle();
  
  Ipv6Address GetHomeAddress() const;
  void SetHomeAddress(Ipv6Address hoa);
  
  Ipv6Address GetCareofAddress() const;
  void SetCareofAddress(Ipv6Address coa);
  
  uint16_t GetHomeNonceIndex() const;
  void SetHomeNonceIndex(uint16_t hi);
  
  uint16_t GetCareofNonceIndex() const;
  void SetCareofNonceIndex(uint16_t coi);
  
  uint64_t GetAuthenticator() const;
  void SetAuthenticator(uint64_t auth);
  
  uint16_t GetRefreshInterval() const;
  void SetRefreshInterval(uint16_t intvl);
  
protected:
private:
  //for MIPv6
  Ipv6Address m_hoa;
  Ipv6Address m_coa;
  uint16_t m_hi;
  uint16_t m_coi;
  uint64_t m_auth;
  uint16_t m_interval;
};

/**
 * \class Ipv6MobilityOptionPad1
 * \brief Ipv6 Mobility Option 
 */
class Ipv6MobilityOptionPad1 : public MIPv6Option
{
public:
  static const uint8_t OPT_NUMBER = 0;
  
  /**
   * \brief Get the type identificator.
   * \return type identificator
   */
  static TypeId GetTypeId (void);
  
  /**
   * \brief Destructor.
   */
  virtual ~Ipv6MobilityOptionPad1 ();
  
  /**
   * \brief Get the option number.
   * \return option number
   */
  virtual uint8_t GetMobilityOptionNumber () const;
  
  /**
   * \brief Process method
   *
   * Called from Ipv6MobilityL4Protocol::Receive.
   * \param packet the packet
   * \param bundle bundle of all option data
   * \return the processed size
   */
  virtual uint8_t Process (Ptr<Packet> packet, uint8_t offset, MIPv6OptionBundle& bundle);
  
private:
};

/**
 * \class Ipv6MobilityOptionPadn
 * \brief Ipv6 Mobility Option 
 */
class Ipv6MobilityOptionPadn : public MIPv6Option
{
public:
  static const uint8_t OPT_NUMBER = 1;
  
  /**
   * \brief Get the type identificator.
   * \return type identificator
   */
  static TypeId GetTypeId (void);
  
  /**
   * \brief Destructor.
   */
  virtual ~Ipv6MobilityOptionPadn ();
  
  /**
   * \brief Get the option number.
   * \return option number
   */
  virtual uint8_t GetMobilityOptionNumber () const;
  
  /**
   * \brief Process method
   *
   * Called from Ipv6MobilityL4Protocol::Receive.
   * \param packet the packet
   * \param bundle bundle of all option data
   * \return the processed size
   */
  virtual uint8_t Process (Ptr<Packet> packet, uint8_t offset, MIPv6OptionBundle& bundle);
  
private:
};

/**
 * \class Ipv6MobilityOptionMobileNodeIdentifier
 * \brief Ipv6 Mobility Option 
 */
class Ipv6MobilityOptionBindingRefreshAdvice : public MIPv6Option
{
public:
  static const uint8_t OPT_NUMBER = 2;
  
  /**
   * \brief Get the type identificator.
   * \return type identificator
   */
  static TypeId GetTypeId (void);
  
  /**
   * \brief Destructor.
   */
  virtual ~Ipv6MobilityOptionBindingRefreshAdvice ();
  
  /**
   * \brief Get the option number.
   * \return option number
   */
  virtual uint8_t GetMobilityOptionNumber () const;
  
  /**
   * \brief Process method
   *
   * Called from Ipv6MobilityL4Protocol::Receive.
   * \param packet the packet
   * \param bundle bundle of all option data
   * \return the processed size
   */
  virtual uint8_t Process (Ptr<Packet> packet, uint8_t offset, MIPv6OptionBundle& bundle);
  
private:
};

/**
 * \class Ipv6MobilityOptionHomeNetworkPrefix
 * \brief Ipv6 Mobility Option 
 */
class Ipv6MobilityOptionAlternateCareofAddress : public MIPv6Option
{
public:
  static const uint8_t OPT_NUMBER = 3;
  
  /**
   * \brief Get the type identificator.
   * \return type identificator
   */
  static TypeId GetTypeId (void);
  
  /**
   * \brief Destructor.
   */
  virtual ~Ipv6MobilityOptionAlternateCareofAddress ();
  
  /**
   * \brief Get the option number.
   * \return option number
   */
  virtual uint8_t GetMobilityOptionNumber () const;
  
  /**
   * \brief Process method
   *
   * Called from Ipv6MobilityL4Protocol::Receive.
   * \param packet the packet
   * \param bundle bundle of all option data
   * \return the processed size
   */
  virtual uint8_t Process (Ptr<Packet> packet, uint8_t offset, MIPv6OptionBundle& bundle);
  
private:
};

/**
 * \class Ipv6MobilityOptionHandoffIndicator
 * \brief Ipv6 Mobility Option 
 */
class Ipv6MobilityOptionNonceIndices : public MIPv6Option
{
public:
  static const uint8_t OPT_NUMBER = 4;
  
  /**
   * \brief Get the type identificator.
   * \return type identificator
   */
  static TypeId GetTypeId (void);
  
  /**
   * \brief Destructor.
   */
  virtual ~Ipv6MobilityOptionNonceIndices ();
  
  /**
   * \brief Get the option number.
   * \return option number
   */
  virtual uint8_t GetMobilityOptionNumber () const;
  
  /**
   * \brief Process method
   *
   * Called from Ipv6MobilityL4Protocol::Receive.
   * \param packet the packet
   * \param bundle bundle of all option data
   * \return the processed size
   */
  virtual uint8_t Process (Ptr<Packet> packet, uint8_t offset, MIPv6OptionBundle& bundle);
  
private:
};

/**
 * \class Ipv6MobilityOptionAccessTechnologyType
 * \brief Ipv6 Mobility Option 
 */
class Ipv6MobilityOptionBindingAuthorizationData : public MIPv6Option
{
public:
  static const uint8_t OPT_NUMBER = 5;
  
  /**
   * \brief Get the type identificator.
   * \return type identificator
   */
  static TypeId GetTypeId (void);
  
  /**
   * \brief Destructor.
   */
  virtual ~Ipv6MobilityOptionBindingAuthorizationData ();
  
  /**
   * \brief Get the option number.
   * \return option number
   */
  virtual uint8_t GetMobilityOptionNumber () const;
  
  /**
   * \brief Process method
   *
   * Called from Ipv6MobilityL4Protocol::Receive.
   * \param packet the packet
   * \param bundle bundle of all option data
   * \return the processed size
   */
  virtual uint8_t Process (Ptr<Packet> packet, uint8_t offset, MIPv6OptionBundle& bundle);
  
private:
};



} /* namespace ns3 */

#endif /* IPV6_MOBILITY_OPTION_H */
