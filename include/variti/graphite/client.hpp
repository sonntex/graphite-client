#pragma once

#include <variti/graphite/parameter.hpp>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <mutex>
#include <set>
#include <string>
#include <vector>

namespace variti { namespace graphite {

class client
{
public:

  client(boost::asio::io_context& io, const boost::asio::ip::udp::endpoint& ep, const std::string& prefix);

  void cancel();

  std::size_t add_parameter(parameter&& param);
  std::size_t add_parameter(const std::string& name);
  std::size_t add_parameter(const std::string& ns, const std::string& name);

  void add(std::size_t id, unsigned value);
  void inc(std::size_t id) { add(id, 1); }
  void sub(std::size_t id, unsigned value);
  void dec(std::size_t id) { sub(id, 1); }
  void max(std::size_t id, unsigned value);

  std::string prefix() const;

private:

  void send(const char* buf, std::size_t size);
  void send();
  void loop();

  boost::asio::ip::udp::endpoint ep_;
  boost::asio::deadline_timer timer_;
  boost::asio::ip::udp::socket socket_;
  std::string prefix_;
  int start_;
  int interval_;
  std::mutex mutex_;
  std::vector<parameter> params_;
  std::set<std::string> existed_params_;
};

}}
