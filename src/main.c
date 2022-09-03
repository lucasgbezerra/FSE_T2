#include <unistd.h>
#include <signal.h>

#include "controller.h"


int main()
{
    signal(SIGINT, sigintHandler);
    signal(SIGUSR1, exit_thread);
    init();
    main_controller();

    return 0;
}
