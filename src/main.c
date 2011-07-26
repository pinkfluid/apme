#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#include "console.h"
#include "aion.h"
#include "cmd.h"
#include "chatlog.h"

int aptrack_main(int argc, char* argv[])
{
    (void)argc;
    (void)argv;

    con_init();

    if (!aion_init())
    {
        con_printf("Unable to initialize the Aion subsystem.\n");
        return 1;
    }

    if (!chatlog_init())
    {
        con_printf("Error initializing the Chatlog parser.\n");
        return 1;
    }

    for (;;)
    {
        cmd_poll();
        chatlog_poll();
        usleep(300);
    }

    return 0;
}


int main(int argc, char *argv[])
{
    int retval;

    retval = aptrack_main(argc, argv);

#ifdef SYS_WINDOWS
    /* On windows, pause before exiting */
    {
        char buf[16];
        printf("Press ENTER to exit.\n");
        fgets(buf, sizeof(buf), stdin);
    }
#endif
    

    return retval;
}

