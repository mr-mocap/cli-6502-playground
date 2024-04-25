#include <cstdlib>
#include <iostream>
#include "test_srecord.hpp"
#include "test_simplehex.hpp"


int main(void)
{
    std::cout << "Running Tests..." << std::endl;

    SRecordTests::Run();
    SimpleHexTests::Run();

    std::cout << "Done" << std::endl;

    return EXIT_SUCCESS;
}
