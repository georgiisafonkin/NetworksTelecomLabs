#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<unistd.h>

int main(int argc, char* argv[]) {
    clock_t t = clock();
    sleep(5);
    t = clock() - t;
    printf("%lf secs\n", ((double)t)/CLOCKS_PER_SEC);

    return 0;
}