#include <cassert>
#include <cstdio>

int main()
{
    const char* source = "#include <cassert>%c#include <cstdio>%c%cint main()%c{%c    const char* source = %c%s%c;%c    assert(source != nullptr);%c    std::printf(source, 10, 10, 10, 10, 10, 34, source, 34, 10, 10, 10, 10, 10);%c    return 0;%c}%c";
    assert(source != nullptr);
    std::printf(source, 10, 10, 10, 10, 10, 34, source, 34, 10, 10, 10, 10, 10);
    return 0;
}
