#include "xbacklight.h"

int main(int argc, const char *argv[])
{
    int narg = 3;
    char* args[3]  = { "xbacklight", "-set", "50" };
    xbacklight_main( narg, args );

    args[2]  = "100";
    xbacklight_main( narg, args );
    
    return 0;
}

