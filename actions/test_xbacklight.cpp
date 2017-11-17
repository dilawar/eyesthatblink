#include "xbacklight.h"
#include <vector>

int main(int argc, const char *argv[])
{
    int narg = 3;
    std::vector<const char*> args  = { "xbacklight", "-set", "50" };
    xbacklight_main( narg, args );

    args[2]  = "100";
    xbacklight_main( narg, args );
    
    return 0;
}

