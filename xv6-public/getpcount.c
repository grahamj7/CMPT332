#include "types.h"
#include "stat.h"
#include "user.h"

int
main(void){
        int running_count;
        running_count = getpcount();
        printf(1, "Processes currently running: %d\n", running_count);
        exit();
}
