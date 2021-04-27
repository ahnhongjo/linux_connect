#include <iostream>
#include <time.h>

int main() {
    clock_t start, end;

    start=clock();

    std::cout << "Hello, World!" << std::endl;

    for(int i=0;i<10000;i++){
        std::cout<<i<<std::endl;
    }

    end=clock();

    double duration=(double)(end-start)/CLOCKS_PER_SEC;

    std::cout<<duration<<std::endl;

    return 0;
}
