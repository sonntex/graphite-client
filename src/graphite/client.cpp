#include <variti/graphite/client.hpp>
#include <variti/tc.hpp>

#include <boost/asio/ip/host_name.hpp>
#include <boost/log/trivial.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <stdexcept>

namespace variti { namespace graphite {

namespace {

std::string format(std::string s)
{
  std::replace(s.begin(), s.end(), '.', '_');
  return s;
}

}

client::client(boost::asio::io_context& io, const boost::asio::ip::udp::endpoint& ep, const std::string& prefix)
  : ep_(ep)
  , timer_(io)
  , socket_(io)
  , prefix_(format(prefix) + "." + boost::asio::ip::host_name())
{
  update_now();
  boost::random::mt19937 gen(get_now());
  boost::random::uniform_int_distribution<> dist(5000, 15000);
  start_ = dist(gen);
  interval_ = 60000;
  params_.reserve(1024);
  io.post([this]() { loop(); });
}

void client::cancel()
{
  timer_.cancel();
}

std::size_t client::add_parameter(parameter&& param)
{
  std::unique_lock<std::mutex> lock(mutex_);
  std::size_t id = params_.size();
  if (id >= params_.capacity())
    throw std::out_of_range("params max size exceeded");
  params_.push_back(std::move(param));
  return id;
}

std::size_t client::add_parameter(const std::string& name)
{
  return add_parameter(parameter(prefix_ + "." + format(name)));
}

std::size_t client::add_parameter(const std::string& ns, const std::string& name)
{
  return add_parameter(parameter(prefix_ + "." + format(ns) + "." + format(name)));
}

void client::add(std::size_t id, unsigned value)
{
  params_[id].val.fetch_add(value, std::memory_order_relaxed);
}

void client::sub(std::size_t id, unsigned value)
{
  params_[id].val.fetch_sub(value, std::memory_order_relaxed);
}

namespace {

void max_impl(std::atomic_int_fast64_t& p, std::int64_t rv)
{
  do {
    std::int64_t cv = p.load();
    if (rv > cv) {
      if (!p.compare_exchange_strong(cv, rv) || cv >= rv)
        break;
    } else
      break;
  } while (true);
}

}

void client::max(std::size_t id, unsigned value)
{
  max_impl(params_[id].val, value);
}

std::string client::prefix() const
{
  return prefix_;
}

void client::send(const char* data, std::size_t size)
{
  if (!socket_.is_open())
    socket_.open(boost::asio::ip::udp::v4());
  // BOOST_LOG_TRIVIAL(info) << std::string(data, size - 1);
  boost::system::error_code ec;
  std::size_t transferred = socket_.send_to(boost::asio::buffer(data, size), ep_, 0, ec);
  if (ec)
    BOOST_LOG_TRIVIAL(error) << "graphite error: " << ec;
  else if (transferred != size)
    BOOST_LOG_TRIVIAL(error) << "graphite error: size mismatch";
}

void client::send()
{
  auto now = get_now() / 1000;
  std::unique_lock<decltype(mutex_)> lock(mutex_);
  std::size_t size = params_.size();
  lock.unlock();
  if (!size)
    return;
  char data[1024];
  for (std::size_t i = 0; i < size; ++i) {
    auto& param = params_[i];
    param.tmp = param.val.exchange(0);
  }
  for (std::size_t i = 0; i < size; ++i) {
    auto& param = params_[i];
    if (!param.tmp)
      continue;
    std::size_t size = std::snprintf(data, sizeof(data), "%s %ld %ld\n", param.path.c_str(), param.tmp, now);
    if (size > sizeof(data)) {
      BOOST_LOG_TRIVIAL(error) << "graphite error: metric " << param.path << " is too long";
      continue;
    }
    send(data, size);
  }
}

void client::loop()
{
  int timeout = interval_ - get_now() % interval_ + start_;
  timer_.expires_from_now(boost::posix_time::milliseconds(timeout));
  timer_.async_wait(
    [this](boost::system::error_code ec) {
      update_now();
      if (ec) {
        if (ec != boost::asio::error::operation_aborted)
          loop();
        return;
      }
      send();
      loop();
    });
}

}}
