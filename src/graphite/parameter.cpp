#include <variti/graphite/parameter.hpp>

namespace variti { namespace graphite {

parameter::parameter(const std::string& path)
  : path(path)
{}

parameter::parameter(parameter&& other)
{
  path = std::move(other.path);
}

parameter& parameter::operator=(parameter&& other)
{
  path = std::move(other.path);
  return *this;
}

}}

