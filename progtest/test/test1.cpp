#include <iostream>

int foo ( int a )
{
    return 10;
}
const char * foo ( int b )
{
    return "Hello";
}

int main() {
    int x = foo ( 10 );

    std::cout << x << std::endl;
}
