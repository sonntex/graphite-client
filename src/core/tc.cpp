#include <variti/tc.hpp>

#include <chrono>

namespace variti {

thread_local std::uint64_t now_ = 0;

std::uint64_t get_system_now()
{
  using namespace std::chrono;
  return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

std::uint64_t get_now()
{
  return now_;
}

void update_now()
{
  now_ = get_system_now();
}

}
