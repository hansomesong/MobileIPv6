#ifndef B_CACHE_H
#define B_CACHE_H

#include <stdint.h>

#include <list>

#include "ns3/packet.h"
#include "ns3/nstime.h"
#include "ns3/node.h"
#include "ns3/ipv6-address.h"
#include "ns3/ptr.h"
#include "ns3/timer.h"
#include "ns3/net-device.h"
#include "ns3/sgi-hashmap.h"
#include "ns3/ptr.h"

namespace ns3
{

class BCache : public Object
{
public:
  class Entry;
  
  static TypeId GetTypeId ();

  BCache();
  
  ~BCache();

  BCache::Entry *Lookup(Ipv6Address mnhoa);
  void Add(BCache::Entry *bce);
  void Remove(BCache::Entry *entry);

  bool LookupSHoa(Ipv6Address shoa);

  
  void Flush();
  
  Ptr<Node> GetNode() const;
  void SetNode(Ptr<Node> node);
  void SetHomePrefixes(std::list<Ipv6Address> halist);

  class Entry
  {
  public:

    enum State_e
    {
    TENTATIVE, /**< Address is tentative, no packet can be sent unless DAD finished */
    DEPRECATED, /**< Address is deprecated and should not be used */
    PREFERRED, /**< Preferred address */
    PERMANENT, /**< Permanent address */
    HOMEADDRESS, /**< Address is a HomeAddress */
    TENTATIVE_OPTIMISTIC, /**< Address is tentative but we are optimistic so we can send packet even if DAD is not yet finished */
    INVALID, /**< Invalid state (after a DAD failed) */
    };
    
    Entry(Ptr<BCache> bcache);
    
    Entry *Copy();
    
    bool IsUnreachable() const;
    bool IsReachable() const;
    

    void MarkReachable();
    
    bool Match(Ipv6Address mnhoa) const;
    
    Ipv6Address GetCoa() const;
    void SetCoa(Ipv6Address mnhoa);

    Ipv6Address GetHA() const;
    void SetHA(Ipv6Address haa);
    
    Ipv6Address GetSolicitedHoA() const;
    void SetSolicitedHoA(Ipv6Address shoa);

    Ipv6Address GetHoa() const;
    void SetHoa(Ipv6Address hoa);
    
    int16_t GetTunnelIfIndex() const;
    void SetTunnelIfIndex(int16_t tunnelif);
    
    Time GetLastBindingUpdateTime() const;
    void SetLastBindingUpdateTime(Time tm);
    
    uint16_t GetLastBindingUpdateSequence() const;
    void SetLastBindingUpdateSequence(uint16_t seq);
    
    Entry *GetNext() const;
    void SetNext(Entry *entry);
    
    Ipv6Address GetOldCoa() const;
   
	uint64_t GetHomeInitCookie() const;
	void SetHomeInitCookie(uint64_t hcookie);

	uint64_t GetCareOfInitCookie() const;
	void SetCareOfInitCookie(uint64_t ccookie);

	uint64_t GetHomeKeygenToken() const;
	void SetHomeKeygenToken(uint64_t htoken);

	uint64_t GetCareOfKeygenToken() const;
	void SetCareOfKeygenToken(uint64_t ctoken);

	uint16_t GetHomeNonceIndex() const;
	void SetHomeNonceIndex(uint16_t hnonce);

	uint16_t GetCareOfNonceIndex() const;
	void SetCareOfNonceIndex(uint16_t cnonce);

 void SetState (BCache::Entry::State_e state);
 BCache::Entry::State_e GetState ();


	

    
  private:
    Ptr<BCache> m_bCache;
    
    enum BCacheState_e {
      UNREACHABLE,
      DEREGISTERING,
      REGISTERING,
      REACHABLE
    };


    BCacheState_e m_state;
    
    Ipv6Address m_haa;

    Ipv6Address m_coa;

    Ipv6Address m_hoa;

    Ipv6Address m_shoa;
    
    int16_t m_tunnelIfIndex;

    Time m_lastBindingUpdateTime;
    uint16_t m_lastBindingUpdateSequence;
    
    Entry *m_next;
    
    //internal
    
    Ipv6Address m_oldCoa;
    	uint64_t m_homeinitcookie;
	uint64_t m_careofinitcookie;
	uint64_t m_homekeygentoken;
	uint64_t m_careofkeygentoken;
	uint16_t m_homenonceindex;
	uint16_t m_careofnonceindex;

  State_e m_addrstate;
    


  };


protected:

private:
  typedef sgi::hash_map<Ipv6Address, BCache::Entry *, Ipv6AddressHash> bCache;
  typedef sgi::hash_map<Ipv6Address, BCache::Entry *, Ipv6AddressHash>::iterator BCacheI;
  
  void DoDispose();
  
  bCache m_bCache;
  std::list<Ipv6Address> m_HaaList;
  std::list<Ipv6Address> m_HomePrefixList;
  
  Ptr<Node> m_node;
};

} 

#endif /* BINDING_CACHE_H */
