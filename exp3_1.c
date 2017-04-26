#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>

static char tmbuf[64]; //存储当前时间的字符串变量

static void currentsig_real(int signum); //全局计时器中断处理

static void psig_real(int signum); //父进程的三个定时中断处理函数
static void psig_virtual(int signum);
static void psig_prof(int signum);

static void c1sig_real(int signum); //子进程1的三个定时中断处理函数
static void c1sig_virtual(int signum);
static void c1sig_prof(int signum);

static void c2sig_real(int signum); //子进程2的三个定时中断处理函数
static void c2sig_virtual(int signum);
static void c2sig_prof(int signum);

//记录3种定时的秒数的变量
static time_t p_real_secs = 0, c1_real_secs = 0, c2_real_secs = 0;
static time_t p_virtual_secs = 0, c1_virtual_secs = 0, c2_virtual_secs = 0;
static time_t p_prof_secs = 0, c1_prof_secs = 0, c2_prof_secs = 0;
static time_t p_real_usecs = 0; //微秒

//记录全局定时的秒数/微秒的变量
static time_t current_real_secs = 0;
static time_t current_real_usecs = 0;
//记录全局定时秒数/微秒的结构变量
static struct itimerval current_realt;

//记录3种定时的毫秒秒数的结构变量
static struct itimerval p_realt, c1_realt, c2_realt;
static struct itimerval p_virtt, c1_virtt, c2_virtt;
static struct itimerval p_proft, c1_proft, c2_proft;

static void currentsig_real(int signum) {
    current_real_secs += 1;
    getitimer(ITIMER_REAL, &current_realt);
    strftime(tmbuf, sizeof tmbuf, "%Y-%m-%d %H:%M:%S", localtime(&current_real_secs));
    printf("%s\n", tmbuf);
}

static void psig_real(int signum) {
    p_real_secs += 10;
}
static void psig_virtual(int signum) {
    p_virtual_secs += 10;
}
static void psig_prof(int signum) {
    p_prof_secs += 10;
}

//c1的3个中断处理函数
static void c1sig_real(int signum) {
    c1_real_secs += 10;
}
static void c1sig_virtual(int signum) {
    c1_virtual_secs += 10;
}
static void c1sig_prof(int signum) {
    c1_prof_secs += 10;
}

//c2的3个中断处理函数
static void c2sig_real(int signum) {
    c2_real_secs += 10;
}
static void c2sig_virtual(int signum) {
    c2_virtual_secs += 10;
}
static void c2sig_prof(int signum) {
    c2_prof_secs += 10;
}

long unsigned int fibonacci(unsigned int n) {
    if(n == 0) {
        return 0;
    }
    if(n == 1) {
        return 0;
    }
    if(n == 2) {
        return 1;
    }    
    return fibonacci(n-1) + fibonacci(n-2);
}

struct timeval mygettimeofday(struct timeval tv, struct timezone *tz) {
    //设置信号
    signal(SIGALRM, currentsig_real);
    current_realt.it_interval.tv_sec = 0;
    current_realt.it_interval.tv_usec = 999999;
    current_realt.it_value.tv_sec = 0;
    current_realt.it_value.tv_usec = 999999;
    setitimer(ITIMER_REAL, &current_realt, NULL);
    getitimer(ITIMER_REAL, &current_realt);
    tv.tv_sec = current_real_secs + 9 - current_realt.it_value.tv_sec;
    tv.tv_usec = 999999 - current_realt.it_value.tv_usec;
    return tv;
}


int main(int argc, char **argv) {
    gettimeofday(&current_realt.it_value, NULL);
    current_real_secs = current_realt.it_value.tv_sec;
    if(argv[1][0] == '-' && argv[1][1] == 'a') {
        p_realt.it_value = mygettimeofday(p_realt.it_value, NULL);
        while(1);        
    } else if(argv[1][0] == '-' && argv[1][1] == 'b') {
        p_realt.it_value = mygettimeofday(p_realt.it_value, NULL);        
        while(1) {
            getitimer(ITIMER_REAL, &current_realt);
            p_real_secs = current_real_secs - current_realt.it_value.tv_sec;
            p_real_usecs = 999999 - current_realt.it_value.tv_usec;
            strftime(tmbuf, sizeof tmbuf, "%Y-%m-%d %H:%M:%S", localtime(&p_real_secs));
            printf("%s.%06ld\n", tmbuf, p_real_usecs);
            int i;
            for(i = 1000000000; i >0; i--) {

            }
        }
    } else if(argv[1][0] == '-' && argv[1][1] == 'c') {
        int pid1, pid2;
        // unsigned int arg1 = argv[1][2];
        // unsigned int arg2 = argv[1][3];
        // unsigned int arg3 = argv[1][4];
        long unsigned int fib1 = 0;
        long unsigned int fib2 = 0;
        long unsigned int fib3 = 0;
        int status;

        signal(SIGALRM, psig_real);
        signal(SIGVTALRM, psig_virtual);
        signal(SIGPROF, psig_prof);
        p_realt.it_interval.tv_sec = 9;
        p_realt.it_interval.tv_usec = 999999;
        p_realt.it_value.tv_sec = 9;
        p_realt.it_value.tv_usec = 999999;
        setitimer(ITIMER_REAL, &p_realt, NULL);
        p_virtt.it_interval.tv_sec = 9;
        p_virtt.it_interval.tv_usec = 999999;
        p_virtt.it_value.tv_sec = 9;
        p_virtt.it_value.tv_usec = 999999;
        setitimer(ITIMER_VIRTUAL, &p_virtt, NULL);
        p_proft.it_interval.tv_sec = 9;
        p_proft.it_interval.tv_usec = 999999;
        p_proft.it_value.tv_sec = 9;
        p_proft.it_value.tv_usec = 999999;        
        setitimer(ITIMER_PROF, &p_proft, NULL);

        pid1 = fork();
        if(pid1 == 0) {
            signal(SIGALRM, c1sig_real);
            signal(SIGVTALRM, c1sig_virtual);
            signal(SIGPROF, c1sig_prof);
            c1_realt.it_interval.tv_sec = 9;
            c1_realt.it_interval.tv_usec = 999999;
            c1_realt.it_value.tv_sec = 9;
            c1_realt.it_value.tv_usec = 999999;
            setitimer(ITIMER_REAL, &c1_realt, NULL);
            c1_virtt.it_interval.tv_sec = 9;
            c1_virtt.it_interval.tv_usec = 999999;
            c1_virtt.it_value.tv_sec = 9;
            c1_virtt.it_value.tv_usec = 999999;
            setitimer(ITIMER_VIRTUAL, &c1_virtt, NULL);
            c1_proft.it_interval.tv_sec = 9;
            c1_proft.it_interval.tv_usec = 999999;
            c1_proft.it_value.tv_sec = 9;
            c1_proft.it_value.tv_usec = 999999;         
            setitimer(ITIMER_PROF, &c1_proft, NULL);

            fib2 = fibonacci(atoi(argv[3]));

            getitimer(ITIMER_REAL, &c1_realt);
            getitimer(ITIMER_VIRTUAL, &c1_virtt);
            getitimer(ITIMER_PROF, &c1_proft);
            printf("##########Child 1############\n"
                    "Child1 fibonacci is:%ld\n"
                    "Child1 real time:%ld.%lds\n"
                    "Child1 virt time:%ld.%lds\n"
                    "Child1 prof time:%ld.%lds\n",
                    fib2,
                    c1_real_secs + 9 - c1_realt.it_value.tv_sec,
                    (999999 - c1_realt.it_value.tv_usec),
                    c1_virtual_secs + 9 - c1_virtt.it_value.tv_sec,
                    (999999 - c1_virtt.it_value.tv_usec)/1000,
                    c1_prof_secs + 9 - c1_proft.it_value.tv_sec,
                    (999999 - c1_proft.it_value.tv_usec)/1000);
        } else if((pid2 = fork()) == 0) {
            signal(SIGALRM, c2sig_real);
            signal(SIGVTALRM, c2sig_virtual);
            signal(SIGPROF, c2sig_prof);
            c2_realt.it_interval.tv_sec = 9;
            c2_realt.it_interval.tv_usec = 999999;
            c2_realt.it_value.tv_sec = 9;
            c2_realt.it_value.tv_usec = 999999;
            setitimer(ITIMER_REAL, &c2_realt, NULL);            
            c2_virtt.it_interval.tv_sec = 9;
            c2_virtt.it_interval.tv_usec = 999999;
            c2_virtt.it_value.tv_sec = 9;
            c2_virtt.it_value.tv_usec = 999999;
            setitimer(ITIMER_VIRTUAL, &c2_virtt, NULL);            
            c2_proft.it_interval.tv_sec = 9;
            c2_proft.it_interval.tv_usec = 999999;
            c2_proft.it_value.tv_sec = 9;
            c2_proft.it_value.tv_usec = 999999;
            setitimer(ITIMER_PROF, &c2_proft, NULL);

            fib3 = fibonacci(atoi(argv[4]));

            getitimer(ITIMER_REAL, &c2_realt);
            getitimer(ITIMER_VIRTUAL, &c2_virtt);
            getitimer(ITIMER_PROF, &c2_proft);
            printf("##########Child 2############\n"
                    "Child2 fibonacci is:%ld\n"
                    "Child2 real time:%ld.%lds\n"
                    "Child2 virt time:%ld.%lds\n"
                    "Child2 prof time:%ld.%lds\n",
                    fib3,
                    c2_real_secs + 9 - c2_realt.it_value.tv_sec,
                    (999999 - c2_realt.it_value.tv_usec),
                    c2_virtual_secs + 9 - c2_virtt.it_value.tv_sec,
                    (999999 - c2_virtt.it_value.tv_usec)/1000,
                    c2_prof_secs + 9 - c2_proft.it_value.tv_sec,
                    (999999 - c2_proft.it_value.tv_usec)/1000);           
        } else {

            fib1 = fibonacci(atoi(argv[2]));

            getitimer(ITIMER_REAL, &p_realt);
            getitimer(ITIMER_VIRTUAL, &p_virtt);


            
            getitimer(ITIMER_PROF, &p_proft);
            printf("##########Father############\n"
                    "Parent fibonacci is:%ld\n"
                    "Parent real time:%ld.%lds\n"
                    "Parent virt time:%ld.%lds\n"
                    "Parent prof time:%ld.%lds\n",
                    fib1,
                    p_real_secs + 9 - p_realt.it_value.tv_sec,
                    (999999 - p_realt.it_value.tv_usec),
                    p_virtual_secs + 9 - p_virtt.it_value.tv_sec,
                    (999999 - p_virtt.it_value.tv_usec)/1000,
                    p_prof_secs + 9 - p_proft.it_value.tv_sec,
                    (999999 - p_proft.it_value.tv_usec)/1000);                  
            waitpid(pid1, &status, 0);
            waitpid(pid2, &status, 0);         
        }  
    } 

}