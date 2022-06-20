#include "../carl.h"
#include "../source/ReflectedVariable.h"

#include <iostream>

class Foo {
public:
    CARL_DECLARE_REFLECTED_CLASS(Foo);

    Foo() = default;
//private:
    int x;
    float y;
};

CARL_REFLECT_CLASS(Foo) {
    CARL_REFLECT_MEMBER(x);
    CARL_REFLECT_MEMBER(y);
}

int main() {
    Foo f;
    f.x = 10;
    f.y = 13;

    carl::ReflectionDataManager &manager = carl::ReflectionDataManager::instance();
    const carl::ReflectionData *data = manager.reflectionData("Foo");
    for (auto &member : data->members()) {
        std::cout << "Name: " << member->name() << " Size: " << member->size() << std::endl;
    }
    
    carl::ReflectedVariable v(f);
    v.serialize(std::cout);

    Foo *f2 = static_cast<Foo*>(data->allocateInstance());
    f2->x = 3;
    f2->y = 7;
    carl::ReflectedVariable v2(*f2);
    v2.serialize(std::cout);

    return 0;
}