#include <shell.h>

void    check_open_fds(void)
{
    int max_fd = sysconf(_SC_OPEN_MAX);
    printf("Checking FDs from 0 to %d...\n", max_fd - 1);
    
    for (int fd = 0; fd < max_fd; fd++)
    {
        if (fcntl(fd, F_GETFD) != -1)
        {
            printf("FD %d is open\n", fd);
        }
    }
}
