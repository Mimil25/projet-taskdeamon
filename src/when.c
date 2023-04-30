#include <time.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    if(argc != 2) {
        fprintf(stderr, "Error : wrong number of argumetns\nUsage : %s EPOCH_TIME\n", argv[0]);
        return 1;
    }
    time_t t = atoi(argv[1]);
    printf("%s", ctime(&t));
    return 0;
}
