#include <iostream>
#include "Scan.cpp"


#include <filesystem>
int main()
{
    std::wstring path;
    std::wcin >> path;


    Scan(path);


    std::cin.ignore();
}

