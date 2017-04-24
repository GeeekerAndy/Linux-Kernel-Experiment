#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>

#define LB_SIZE 80
enum TYPE {
    STANDARD, SHORT, LONG
};
FILE *thisProcFile;     //Proc 打开文件指针
struct timeval now;     //系统时间日期
enum TYPE reportType;   //观察报告类型
char repTypeName[16];
char *lineBuf;          //proc 文件读出行缓冲
int interval;           //系统负荷监测时间间隔
int duration;           //系统负荷监测时段
int iteration;
char c1, c2;             //字符处理单元

int temp;

void sampleLoadAvg() {   //观察系统负荷
    int i = 0;
//打开负荷文件

//读出、处理读出行,如去除前导空格和无用空格

//将读出行分出不同字段,按照字段的不同含义处理为可阅读格式

//打印处理好的信息内容

    fclose(thisProcFile);
}

void sampleTime() {//观察系统启动时间
    long uptime, idletime;
    int day, hour, minute, second;
    int i, j;
    char tempCharArr[LB_SIZE];
    i = j = 0;
    //打开计时文件
    thisProcFile = fopen("/proc/uptime", "r");
    if(thisProcFile == NULL) {
        perror("Error opening file.");
        exit(-1);
    }
    while(fgets(lineBuf, LB_SIZE, thisProcFile)) {
        for(iteration = 0; iteration < LB_SIZE; ++iteration) {
            tempCharArr[iteration] = lineBuf[iteration];
            lineBuf[iteration] = 0;
        }
    }
    sscanf(tempCharArr, "%d", &i);
    day = i/(60*60*24);
//    i = i - day*24*60*60;
    i = i%(24*60*60);
    hour = (i)/(60*60);
//    i = i - hour*60*60;
    i = i%(60*60);
    minute = (i)/60;
//    i = i - minute*60;
    i = i%60;
    second = i;
    printf("Time since last booting: %dd %dh %dm %ds\n", day, hour, minute, second);
    fclose(thisProcFile);

}

int main(int argc, char *argv[]) {
    lineBuf = (char *) malloc(LB_SIZE + 1);
    reportType = STANDARD;
    strcpy(repTypeName, "Standard");
    if (argc > 1) {
        sscanf(argv[1], "%c%c", &c1, &c2);//取命令行选择符
        if (c1 != '-') {
            //提示本程序命令参数的用法
            printf("User manual:\n"
                           "\n********\n"
                           "-a (默认参数，如果没有匹配则执行-a参数)\n"
                           "1. 提取CPU类型\n"
                           "2. 提取内核版本\n"
                           "\n********\n"
                           "-b\n"
                           "1. 启动以来经历的时间,以 dd:hh:mm:ss 报告\n"
                           "\n********\n"
                           "-c\n"
                           "1. cpu 执行用户态、系统态、空闲态所用时间\n"
                           "2. 多少次磁盘请求\n"
                           "3. 多少次上下文切换\n"
                           "4. 启动了多少次进程\n"
                           "\n********\n"
                           "-d\n"
                           "1. 内存总量\n"
                           "2. 可用内存\n"
                           "3. 系统平均负荷\n");
            exit(1);
        }
        if (c2 == 'b') {        //观察部分 B
            printf("******PART B **********\n");
//            //打开内存信息文件
//            thisProcFile = fopen("/proc/meminfo", "r");
//            if (thisProcFile == NULL) {
//                perror("Error opening file.");
//                return -1;
//            }
//            //读出文件全部的内容
//            while(fgets(lineBuf, LB_SIZE, thisProcFile)) {
//                for(iteration = 0; iteration < LB_SIZE; ++iteration) {
//                    printf("%c", lineBuf[iteration]);
//                    lineBuf[iteration] = 0;
//                }
//            }
//            //处理并用方便阅读的格式显示
//            fclose(thisProcFile);
//            //观察系统启动时间
            sampleTime();
            return 0;
        } else if (c2 == 'c') {       //观察部分 C
            printf("******PART C**********\n");
            char cpuHead[3];
            int userTime, niceTime, systemTime, idleTime;
            //cpu执行时间
            temp = 0;
            thisProcFile = fopen("/proc/stat", "r");
            fgets(lineBuf, LB_SIZE, thisProcFile);
            sscanf(lineBuf, "%s%d%d%d%d", cpuHead, &userTime, &niceTime, &systemTime, &idleTime);
            for(iteration = 0; iteration < LB_SIZE; ++iteration) {
                lineBuf[iteration] = 0;
            }
            printf("***************\n");
            printf("CPU execute time: \nUser stat:%d\nSystem stat:%d\nidle stat:%d\n\n", userTime, systemTime, idleTime);
            fclose(thisProcFile);
            //IO request
            thisProcFile = fopen("/proc/diskstats", "r");
            if(thisProcFile == NULL) {
                perror("Error opening file.");
                return -1;
            }
            int readNum, writeNum, useless;
            char devName[10];
            /*
             * 		 1 - major number
		 2 - minor mumber
		 3 - device name
		 4 - reads completed successfully
		 5 - reads merged
		 6 - sectors read
		 7 - time spent reading (ms)
		 8 - writes completed
		 9 - writes merged
		10 - sectors written
		11 - time spent writing (ms)
		12 - I/Os currently in progress
		13 - time spent doing I/Os (ms)
		14 - weighted time spent doing I/Os (ms)
             */
            printf("***************\n");
            while(fgets(lineBuf, LB_SIZE, thisProcFile)) {
                sscanf(lineBuf, "%d%d%s%d%d%d%d%d", &useless, &useless, devName, &readNum, &useless, &useless, &useless, &writeNum);
                for(iteration = 0; iteration < LB_SIZE; ++iteration) {
                    lineBuf[iteration] = 0;
                }
                if(devName[0] == 's' && devName[1] == 'd') {
                    printf("Device name：%s\nI/O request：\nreads completed:%d\nwhites completed:%d\n\n", devName, readNum, writeNum);
                }
                for (iteration = 0; iteration < LB_SIZE; ++iteration) {
                    lineBuf[iteration] = 0;
                }
            }
            fclose(thisProcFile);

            //上下文切换
            int countNum;
            char str[LB_SIZE];
            thisProcFile = fopen("/proc/stat", "r");
            while(fgets(lineBuf, LB_SIZE, thisProcFile)) {
                sscanf(lineBuf, "%s%d", str, &countNum);
                if(strcmp(str, "ctxt") == 0) {
                    printf("***************\n");
                    printf("Context switch number：\n%d\n", countNum);
                }
                if(strcmp(str, "processes") == 0) {
                    printf("***************\n");
                    printf("Started processes number：\n%d\n", countNum);
                }

            }
            fclose(thisProcFile);
            return 0;
        } else if (c2 == 'd') {       //观察部分 D
            printf("******PART D **********\n");
//            if (argc < 4) {
//                printf("usage:observer [-b] [-c][-d int dur]\n");
//                exit(1);
//            }
            reportType = LONG;
            strcpy(repTypeName, "Long");
            thisProcFile = fopen("/proc/meminfo", "r");
            temp = 0;
            while(fgets(lineBuf, LB_SIZE, thisProcFile)) {
                if(temp == 0 || temp == 2) {
                    for(iteration = 0; iteration < LB_SIZE; ++iteration) {
                        printf("%c", lineBuf[iteration]);
                    }
                    printf("***************\n");                }
                ++temp;
            }
            fclose(thisProcFile);

            //提取负载
            float load1, load5, load15;
            thisProcFile = fopen("/proc/loadavg", "r");
            fgets(lineBuf, LB_SIZE, thisProcFile);
            sscanf(lineBuf, "%f%f%f", &load1, &load5, &load15);
            printf("System average load:\n%.2f\n%.2f\n%.2f\n", load1, load5, load15);
            fclose(thisProcFile);
            return 0;
        } else {      //观察部分 A
            printf("******PART A **********\n");
            reportType = SHORT;
            strcpy(repTypeName, "Short");

            //取出并显示系统当前时间
            printf("System current time:\n");
            thisProcFile = fopen("/proc/driver/rtc", "r");
            if (thisProcFile == NULL) {
                perror("Error opening file");
                return -1;
            }
            temp = 0;
            while (fgets(lineBuf, LB_SIZE, thisProcFile)) {
                temp++;
                for (iteration = 0; iteration < LB_SIZE; ++iteration) {
                    printf("%c", lineBuf[iteration]);
                    lineBuf[iteration] = 0;
                }
                if (temp == 2) {
                    fclose(thisProcFile);
                    break;
                }
            }


            //读出并显示机器名
            printf("***************\n");
            printf("Host name:\n");
            thisProcFile = fopen("/proc/sys/kernel/hostname", "r");
            fgets(lineBuf, LB_SIZE, thisProcFile);
            for (iteration = 0; iteration < LB_SIZE; ++iteration) {
                printf("%c", lineBuf[iteration]);
            }
            fclose(thisProcFile);

            //读出并显示全部 CPU 信息
            printf("***************\n");
            printf("CPU information:\n");
            thisProcFile = fopen("/proc/cpuinfo", "r");
            temp = 0;
            while (fgets(lineBuf, LB_SIZE, thisProcFile)) {
                ++temp;
                for (iteration = 0; iteration < LB_SIZE; iteration++) {
                    if(temp == 3 || temp == 5 || temp == 8 || temp == 9) {
                        printf("%c", lineBuf[iteration]);
                    }
                    lineBuf[iteration] = 0;
                }
            }
            fclose(thisProcFile);

            //读出并显示系统版本信息
            printf("***************\n");
            printf("Read the show the information of system version:\n");
            thisProcFile = fopen("/proc/version_signature", "r");
            while (fgets(lineBuf, LB_SIZE, thisProcFile)) {
                for (iteration = 0; iteration < LB_SIZE; iteration++) {
                    printf("%c", lineBuf[iteration]);
                    lineBuf[iteration] = 0;
                }
            }
            fclose(thisProcFile);
            return 0;
        }
    }
    printf("\"./a.out h\" to get the user manual.\n");
    return 0;
}
