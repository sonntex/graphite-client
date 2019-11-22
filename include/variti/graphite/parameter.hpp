#pragma once

#include <atomic>
#include <string>

namespace variti { namespace graphite {

struct parameter
{
  parameter(const std::string& path);
  parameter(const parameter& other) = delete;
  parameter(parameter&& other);
  parameter& operator=(const parameter& other) = delete;
  parameter& operator=(parameter&& other);

  std::string path;
  std::atomic_int_fast64_t val{0};
  std::int64_t tmp{0};
};

}}
