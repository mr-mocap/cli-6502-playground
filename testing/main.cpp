#include <cstdlib>
#include <iostream>
#include "test_srecord.hpp"


int main(void)
{
    std::cout << "Running Tests..." << std::endl;

    SRecordTests::Run();

    std::cout << "Done" << std::endl;

    return EXIT_SUCCESS;
}
