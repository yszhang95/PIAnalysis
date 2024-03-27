#ifndef __PI_EventData__
#define __PI_EventData__

#include <any>
#include <map>
#include <string>

namespace PIAna
{
/**
 * A data holder for all types of data;
 * The data type must satisfy several requirements for the usage of std::any.
 * For example, the data must be copyable.
 * I also do not expect derived classes can work very well.
 * They are under test.
 */
  class PIEventData
  {
  public:
    using RegistryType = std::map<const std::string, std::any>;
    using RegistryKeyType = std::string;
    PIEventData(){};

    /**
     * Register data to registry.
     * Data will be copied to registry.
     */
    template <typename T>
    std::pair<RegistryType::iterator, bool>
    Register(RegistryKeyType n, const T& d)
    { return data_.insert({n, d}); }
    /**
     * A way like static cast to get data out.
     * Virtual methods cannot work.
     */
    template <typename T>
    const T &Get(const std::string &n) const
    {
      return std::any_cast<const T &>(data_.at(n));
    };

  private:
    RegistryType data_;
  };
};

#endif
