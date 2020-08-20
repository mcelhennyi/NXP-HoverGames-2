// HoverGamesApp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include "utils/pipeline/pair.hpp"
//#include "utils/modules/tester/Tester.hpp"
#include <modules/tester/Tester.hpp>


int main()
{
    std::cout << "Hello World!\n";


    auto tester1 = new Scarecrow::Modules::Tester("tester 1");
    auto tester2 = new Scarecrow::Modules::Tester("tester 2");

    auto paired = new Scarecrow::Utils::Pair<std::string, std::string, std::string>(tester1, tester2);

    std::cout << "Pair output: " << paired->execute("TRIGGER INPUT") << std::endl;

}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
