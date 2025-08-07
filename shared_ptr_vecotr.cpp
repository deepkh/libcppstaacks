#include <memory>
#include <iostream>
#include <vector>

class AAA {
public:
  int& Value() {
    return value_;
  }

  const int& Value() const {
    return value_;
  }

private:
  int value_;
};

class BBB {
public:
  std::shared_ptr<AAA>& SharedPtrFromVector(int i) {
    return aaa_vec.at(i);
  }

  // Helper to add AAA instances
  void AddAAA(const std::shared_ptr<AAA>& ptr) {
    aaa_vec.push_back(ptr);
  }

private:
  std::vector<std::shared_ptr<AAA>> aaa_vec;
};

int main() {
  BBB bbb;

  // Must add an AAA instance before accessing index 0
  bbb.AddAAA(std::make_shared<AAA>());

  bbb.SharedPtrFromVector(0)->Value() = 321;

  std::cout << bbb.SharedPtrFromVector(0)->Value() << std::endl; // Output: 321

  return 0;
}
