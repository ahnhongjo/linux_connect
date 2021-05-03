#include <iostream>
#include <string.h>


int main() {
    char s1[30] = "The/Little/Prince";  // 크기가 30인 char형 배열을 선언하고 문자열 할당

    char *ptr ="";      // " " 공백 문자를 기준으로 문자열을 자름, 포인터 반환
    char *next=strtok(s1, "/");

    while (next != NULL)               // 자른 문자열이 나오지 않을 때까지 반복
    {
        ptr=next;
        printf("%s\n", ptr);          // 자른 문자열 출력
        next = strtok(NULL, "/");      // 다음 문자열을 잘라서 포인터를 반환
    }

    printf("%s\n",ptr);

    return 0;
}
