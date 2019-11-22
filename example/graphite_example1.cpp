#include <variti/graphite.hpp>

#include <thread>

int main(int argc, char* argv[])
{
  using namespace variti;
  using boost::asio::io_context;
  using boost::asio::ip::udp;
  using boost::asio::ip::address;
  using std::chrono::milliseconds;
  assert(argc == 3);
  std::srand(std::time(nullptr));
  io_context io;
  udp::endpoint ep(address::from_string(argv[1]), std::stoi(argv[2]));
  struct {
    std::size_t metric0;
    std::size_t metric1;
    std::size_t metric2;
  } id;
  graphite::client client(io, ep, "examples");
  id.metric0 = client.add_parameter("metric0");
  id.metric1 = client.add_parameter("ns", "metric1");
  id.metric2 = client.add_parameter("ns", "metric2");
  std::thread work([&io]() { io.run(); });
  for (std::size_t i = 0; ; ++i) {
    client.inc(id.metric0);
    client.add(id.metric1, std::rand() % 10);
    client.add(id.metric2, std::rand() % 10);
    std::this_thread::sleep_for(milliseconds(10));
  }
  work.join();
  return 0;
}
