#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MSH_LB 80
#define MSH_TOK_BUF_SIZE 64

//To store a command
struct command
{
    const char **argv;
};

//Produce a process to read or write from/to pipe
int spawn_proc(int in, int out, struct command *cmd)
{
    pid_t pid;

    if((pid = fork()) == 0)
    {
        if(in != 0)
        {
            dup2(in, 0);
            close(in);
        }
        if(out != 1)
        {
            dup2(out, 1);
            close(out);
        }
        return execvp(cmd->argv[0], (char * const *)cmd->argv);
    }

    return pid;
}

int fork_pipes (int n, struct command *cmd)
{
    int i;
    pid_t pid;
    int in, fd[2];

    in = 0;

    for(i = 0; i < n - 1; ++i)
    {
        pipe(fd);
        spawn_proc(in, fd[1], cmd + i);
        close(fd [1]);
        in = fd [0];
    }

    if(in != 0)
    {
        dup2(in, 0);
    }

    return execvp(cmd[i].argv[0], (char * const *)cmd[i].argv);
}

//Split a string with given args.
char **msh_split_line(char *split_char, char *line) {
    int buf_size = MSH_TOK_BUF_SIZE;
    char **tokens = malloc(MSH_TOK_BUF_SIZE * sizeof(char *));
    char *token;
    int position = 0;

    if (!tokens) {
        fprintf(stderr, "msh: allocation error\n");
        exit(EXIT_FAILURE);
    }    

    char *cpy_line = malloc(strlen(line) + 1);
    strcpy(cpy_line, line);
    token = strtok(cpy_line, split_char);
    while (token != NULL) {
        tokens[position] = token;
        ++position;
        if (position > MSH_TOK_BUF_SIZE) {
            buf_size += MSH_TOK_BUF_SIZE;
            tokens = realloc(tokens, buf_size * sizeof(char *));
            if (!tokens) {
                fprintf(stderr, "msh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, split_char);
    }
    tokens[position] = 0;

    return tokens;
}

//Judge then execute specific commands.
int msh_execute(char *line) 
{
    if(strcmp(line, "\n") == 0)
    {
        system("\n");
        return 1;
    }
    char **args;
    int i = 0; //index of **args
    args = msh_split_line(" |&<>", line);
    if (strcmp(args[0],"exit") == 0) 
    {
        return 0;
    }
    else if (strcmp(args[0], "fg") == 0)
    {
        // system("fg");
        return 1;
    } 
    else  if(strcmp(args[0], "\n") == 0)
    {
        system("\n");
        return 1;
    }
    free(args);

    while (line[i] != '\0')
    {
        //To process background command
        if (line[i] == '&')
        {
            pid_t pid;
            pid = fork();
            args = msh_split_line(" &", line);
            if (pid == 0)
            {
                //child process
                char *cmd = args[0];
                char *argv[2];
                argv[0] = cmd;
                argv[1] = NULL;
                execvp(cmd, argv);
                puts("execvp() failed!");
                free(args);
            }
            else if (pid > 0)
            {
                //parent process
                printf("PID: %d\n", pid);
                return 1;
            }
            else
            {
                //fork failed
                perror("fork() failed.");
                return 1;
            }
        }
        //To process pipe
        else if (line[i] == '|') 
        {
            if(line[i+1] != '\n') 
            {
                if(line[i+1] == '|')
                {
                    printf("Do not support \"||\".\n");
                    return 1;
                }
            }
            int cmd_num = 0;
            int position = 0;
            args = msh_split_line("|", line);
            while(args[position] != NULL)
            {
                ++cmd_num;
                ++position;
            }
            struct command cmd[cmd_num];
            for(position = 0; position < cmd_num; ++position)
            {
                cmd[position].argv = (const char**)msh_split_line(" ", args[position]);
            }
                        
            pid_t pid;
            pid = fork();
            if(pid == 0)
            {
                fork_pipes(cmd_num, cmd);
                return 1;
            }
            else if(pid > 0)
            {
                wait(0);
                return 1;
            }
            else
            {
                perror("fork() failed!");
                return 1;
            }
        }
        //To process out redirection
        else if (line[i] == '>') {
            pid_t pid;
            pid = fork();
            if(pid == 0)
            {
                args = msh_split_line(">;", line);
                char **des_file = msh_split_line(" ;", args[1]);
                int fd = creat(des_file[0], 0644);
                dup2(fd, STDOUT_FILENO);
                close(fd);
                char ** cmd = msh_split_line(" ", args[0]);
                execvp(cmd[0], cmd);
                return 0;
            }
            else if(pid > 0)
            {
                int status;
                wait(&status);
                return 1;
            }
            else
            {
                perror("fork() failed!\n");
            }
            
        }
        //To process in redirection
        else if (line[i] == '<') {
            pid_t pid;
            pid = fork();
            if(pid == 0)
            {
                char **args = msh_split_line("<", line);
                char **cmd = msh_split_line(" ", args[0]);
                char **source_file = msh_split_line(" ;", args[1]);
                int fd = open(source_file[0], S_IRUSR);
                dup2(fd, STDIN_FILENO);
                close(fd);
                execvp(cmd[0], cmd);
                return 0;
            }
            else if(pid > 0)
            {
                wait(0);
                return 1;
            }
            else if(pid < 0)
            {
                perror("fork() failed.");
                return 1;
            }
        }
        ++i;
    }
    printf("command not found\n");

    return 1;
}

//Read a line from stdin.
char *msh_read_line() {
    char *line = NULL;
    size_t buf_size = 0;
    getline(&line, &buf_size, stdin);
    char *cpy_line = malloc(strlen(line) + 2);
    strcpy(cpy_line, line);
    cpy_line[strlen(line) - 1] = ';';
    return cpy_line;
}

//loop of shell
void msh_loop() {
    char *line;
    char **args;
    int status = 1;
    int position = 0;

    do 
    {
        printf("mShell $ ");
        line = msh_read_line();
        args = msh_split_line(";", line);
        while (args[position] != NULL) 
        {
            //Make sure one command at a time
            status = msh_execute(args[position]);
            if (status == 0)
                break;
            ++position;
        }
        position = 0;
        free(line);
        free(args);
    } while (status);
}

int main() 
{
    printf("\nmShell手册:\n"
                   "1.允许一行输入多条命令，多条命令之间使用\";\"分隔\n"
                   "2.一条指令最多包含5个子命令，比如最多使用4个管道\n"
                   "3.允许使用的元字符：\n"
                   "管道：\"|\"\n"
                   "后台：\"&\"\n"
                   "重定向：\">\"或\"<\"\n"
                   "4.\"exit\"退出\n");
    //This command tests all these features;
    //gedit&;firefox&;ls -l | grep a | grep u; ps -aux > test; wc < test;
    msh_loop();
    return EXIT_SUCCESS;
}
