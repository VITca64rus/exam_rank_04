#include <unistd.h>
#include <stdlib.h>
#include <string.h>

typedef struct s_list
{
    char **argv;
    int fd_pipe[2];
    int type;
    int prev_type;
    int prev_pipe;
}   t_list;

int ft_strlen(char *s) {
	int i = 0;
	while (s && s[i])
		i++;
	return i;
}

void fatal()
{
    write(2, "error: fatal\n", ft_strlen("error: fatal\n"));
    exit(1);
}

int get_list(t_list *list, char **argv, int i)
{
    int len, end = i;
    while (argv[end] && strcmp(argv[end], "|") != 0 && strcmp(argv[end], ";") != 0)
        end++;
    len = end - i;
    list->argv = &argv[i];
    list->prev_type = list->type;
    list->prev_pipe = list->fd_pipe[0];
    if (!argv[end])
        list->type = 0;
    else if (strcmp(argv[end], "|") == 0)
        list->type = 1;
    else
        list->type = 2;
    argv[end] = NULL;
    return (end);
}

void shell(t_list *list, char **env)
{
    pid_t pid;
    if ((list->prev_type == 1 || list->type == 1) && pipe(list->fd_pipe))
        fatal();
    if ((pid = fork()) < 0)
        fatal();
    if (pid == 0)
    {
        if (list->prev_type == 1 && dup2(list->prev_pipe, 0) < 0)
            fatal();
        if (list->type == 1 && dup2(list->fd_pipe[1], 1) < 0)
            fatal();
        if (execve(list->argv[0], list->argv, env) < 0)
        {
            write(2, "error: cannot execute ", ft_strlen("error: cannot execute "));
            write(2, list->argv[0], ft_strlen(list->argv[0]));
            write(2, "\n", 1);
            exit(1);
        }
    }
    else
    {
        waitpid(pid, NULL, 0);
        if (list->prev_type == 1 || list->type == 1)
        {
            close(list->fd_pipe[1]);
            if (list->type != 1)
                close(list->fd_pipe[0]);
        }
        if (list->prev_type == 1)
            close(list->prev_pipe);
    }
}

int main(int argc, char **argv, char **env)
{
    t_list  list;
    int start, i = 0;
    while (i < argc && argv[++i])
    {
        start = i;
        i = get_list(&list, argv, i);
        if (strcmp(argv[start], "cd") == 0)
        {
            if (i - start != 2)
                write(2, "error: cd: bad arguments\n", ft_strlen("error: cd: bad arguments\n"));
            else if (chdir(list.argv[1]) < 0)
            {
                write(2, "error: cd: cannot change directory to ", ft_strlen("error: cd: cannot change directory to "));
                write(2, list.argv[1], ft_strlen(list.argv[1]));
                write(2, "\n", 1);
            }
        }
        else if (i > start)
            shell(&list, env);
    }
}