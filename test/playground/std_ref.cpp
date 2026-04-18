#include <functional>
#include <memory>
#include <string>

class AAA {
  public:
    AAA(const std::string &name) { name_ = name; }

    const std::string &Name() { return name_; }

    std::string name_;
};

// void DoAAA(const std::unique_ptr<AAA> &aaa) {
void DoAAA(const std::unique_ptr<AAA> aaa) { printf("%s\n", aaa->Name().c_str()); }

int main(const char *argv[], int argc) {
    std::string name = "HEEELO";
    auto aaa = std::make_unique<AAA>(name);
    // DoAAA(std::ref(aaa));
    DoAAA(std::move(aaa));
    printf("%s\n", aaa->Name().c_str());
}
