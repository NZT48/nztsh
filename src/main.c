#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define SH_TOK_BUFSIZE 65
#define SH_TOK_DELIM " \t\r\n\a"

// Function declarations for builtin shell commands
int shell_cd(char **args);
int shell_help(char **args);
int shell_exit(char **args);

char *builtin_str[] = {
        "cd",
        "help",
        "exit"
};

int (*builtin_func[])(char **) = {
        &shell_cd,
        &shell_help,
        &shell_exit
};

int shell_num_bultins() {
    return sizeof(builtin_str) / sizeof(char *);
}

int shell_cd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "nztsh: expected argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("nztsh");
        }
    }
    return 1;
}

int shell_help(char **args) {
    int i;
    printf("NZT's shell\n");
    printf("Type program names and arguments, and hit enter.\n");
    printf("The following are built in:\n");

    for (int i = 0; i < shell_num_bultins(); i++) {
        printf("  %s\n", builtin_str[i]);
    }

    printf("Use the man command for information on other programs. \n");
    return 1;
}

int shell_exit(char **args) {
    return 0;
}

int shell_launch(char **args) {
    pid_t pid, wpid;
    int status;

    pid = fork();


    if (pid == 0) {
        // Child process
        signal(SIGINT, SIG_DFL);
        if (execvp(args[0], args) == -1) {
            perror("nztsh");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // Error forking
        perror("nztsh");
    } else {
        // Parent process
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}

int shell_execute(char **args) {
    int i;

    if (args[0] == NULL) {
        // An empty command was entered
        return 1;
    }

    for (i = 0; i < shell_num_bultins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }

    return shell_launch(args);
}

char **shell_split_line(char *line) {

    int bufsize = SH_TOK_BUFSIZE, position = 0;
    char **tokens = malloc(bufsize * sizeof(char));
    char *token;

    if (!tokens) {
        fprintf(stderr, "nztsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, SH_TOK_DELIM);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += SH_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char *));
            if (!tokens) {
                fprintf(stderr, "nztsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, SH_TOK_DELIM);
    }

    tokens[position] = NULL;
    return tokens;
}


char *shell_read_line() {

    char *line = NULL;
    ssize_t bufsize = 0;
    // have getline allocate a buffer for us
    getline(&line, &bufsize, stdin);
    return line;
}

void shell_loop() {

    char *line;
    char **args;
    char hostname[HOST_NAME_MAX];
    char username[LOGIN_NAME_MAX];
    int status;

    status = gethostname(hostname, HOST_NAME_MAX);
    if (status){
        perror("gethostname");
        exit(EXIT_FAILURE);
    }

    status = getlogin_r(username, LOGIN_NAME_MAX);
    if (status){
        perror("getlogin_r");
        exit(EXIT_FAILURE);
    }


    do {
        printf("%s@%s > ", username, hostname);
        line = shell_read_line();
        args = shell_split_line(line);
        status = shell_execute(args);

        free(line);
        free(args);
    } while (status);

}


int main(int argc, char **argv) {

    // Disable SIGINT
    signal(SIGINT, SIG_IGN);

    // Run command loop
    shell_loop();

    return EXIT_SUCCESS;
}
