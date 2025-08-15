#include <iostream>
#include <string>
#include <stdint.h>

class Solution {
public:
  int reverse(int x) {
    std::string xs = std::to_string(x);
    if (xs.size() == 1) {
      return x;
    }

    bool is_negative = xs.at(0) == '-';
    if (is_negative || xs.at(0) == '+') {
      xs = xs.substr(1, xs.size() - 1);
    }

    for (int i=0; i<xs.size()/2; i++) {
      char lc = xs.at(i);
      char rc = xs.at(xs.size() - i - 1);
      xs.at(i) = rc;
      xs.at(xs.size() - i - 1) = lc;
    }

    int xs2 = std::stoi(xs);

    if (is_negative) {
      xs2 = 0 - xs2;
    }

    std::cout << "x = " << x << ", xs = \""+xs+"\", xs2 = " << xs2 << std::endl;
    return xs2;
  }
};


int main(int argc, const char *argv[])
{
  Solution s;
  std::cout <<  " reversed " << s.reverse(123) << std::endl;
  std::cout << " reversed " << s.reverse(-4965) << std::endl;
  std::cout << " reversed " << s.reverse(0) << std::endl;
  std::cout <<  " reversed " << s.reverse(-1) << std::endl;
  std::cout << " reversed " << s.reverse(98) << std::endl;
  return 0;
}
