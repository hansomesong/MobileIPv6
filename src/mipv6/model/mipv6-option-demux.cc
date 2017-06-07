
#include "ns3/assert.h"
#include "ns3/address-utils.h"
#include "ns3/log.h"
#include "ns3/object-vector.h"
#include "mipv6-option.h"
#include "mipv6-option-demux.h"

namespace ns3
{

NS_OBJECT_ENSURE_REGISTERED (MIPv6OptionDemux);

TypeId MIPv6OptionDemux::GetTypeId ()
{
  static TypeId tid = TypeId ("ns3::MIPv6OptionDemux")
    .SetParent<Object> ()
    .AddAttribute ("MobilityOptions", "The set of IPv6 Mobility options registered with this demux.",
                   ObjectVectorValue (),
                   MakeObjectVectorAccessor (&MIPv6OptionDemux::m_options),
                   MakeObjectVectorChecker<MIPv6Option> ())
    ;
  return tid;
}

MIPv6OptionDemux::MIPv6OptionDemux ()
{
}

MIPv6OptionDemux::~MIPv6OptionDemux ()
{
}

void MIPv6OptionDemux::DoDispose ()
{
  for (Ipv6MobilityOptionList_t::iterator it = m_options.begin (); it != m_options.end (); it++)
    {
      (*it)->Dispose ();
      *it = 0;
    }
  m_options.clear ();
  m_node = 0;
  Object::DoDispose ();
}

void MIPv6OptionDemux::SetNode (Ptr<Node> node)
{
  m_node = node;
}

void MIPv6OptionDemux::Insert (Ptr<MIPv6Option> option)
{
  m_options.push_back (option);
}

Ptr<MIPv6Option> MIPv6OptionDemux::GetOption (int optionNumber)
{
  for (Ipv6MobilityOptionList_t::iterator i = m_options.begin (); i != m_options.end (); ++i)
    {
      if ((*i)->GetMobilityOptionNumber () == optionNumber)
        {
          return *i;
        }
    }
  return 0;
}

void MIPv6OptionDemux::Remove (Ptr<MIPv6Option> option)
{
  m_options.remove (option);
}

} /* namespace ns3 */
