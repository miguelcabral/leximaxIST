#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

double read_cpu_time()
{
    struct rusage ru;
    // parent process
    getrusage(RUSAGE_SELF, &ru);
    // user time
    double user_time ((double)ru.ru_utime.tv_sec + (double)ru.ru_utime.tv_usec / 1000000);
    // system time - negligible ?
    double sys_time ((double)ru.ru_stime.tv_sec + (double)ru.ru_stime.tv_usec / 1000000);
    // children that have been waited for
    getrusage(RUSAGE_CHILDREN, &ru);
    // user time
    user_time += (double)ru.ru_utime.tv_sec + (double)ru.ru_utime.tv_usec / 1000000;
    //system time - negligible ?
    sys_time += (double)ru.ru_stime.tv_sec + (double)ru.ru_stime.tv_usec / 1000000;
    double total_time (user_time + sys_time);
    return total_time;
}
