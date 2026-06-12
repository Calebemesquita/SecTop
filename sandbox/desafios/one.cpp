#include <iostream>

#include <cstring>

#include <iomanip>


struct twoDoubles{ // cada double tem 8 bytes
    double a;
    double b;   
};



int main (){

    unsigned char buffer[16]; // 16bytes ou 

    memcpy(buffer, "AABBCCDDEEFFGGHH", 16);


    for(int i =0; i < 16; i++){
        std::cout << "0x" << std::hex << (int)buffer[i] << std::endl;
        std::cout << buffer[i] << std::endl;
    }

    int *ptr = (int*)buffer;




    std::cout << " Valor: " << std::dec << *ptr << std::endl;
    std::cout << "Valor de a: " << std::hex << *ptr << std::endl;


    // struct twoDoubles* ptr = (struct twoDoubles*)buffer;

    // std::cout << " Valor: " << std::hex << ptr << std::endl;
    // std::cout << "Valor de a: " << std::dec << ptr->a << std::endl;

}