#ifndef NETWORKING_TESTS_TEST_HELPER_HPP
#define NETWORKING_TESTS_TEST_HELPER_HPP

namespace example::tests {
inline std::string generateRandomString(size_t size) {
  std::random_device rd;
  std::mt19937 eng{rd()};
  std::uniform_int_distribution<char> dist;
  std::string str(size, {});
  std::generate(str.begin(), str.end(), std::bind(dist, eng));
  return str;
}
}  // namespace example::tests

#endif
