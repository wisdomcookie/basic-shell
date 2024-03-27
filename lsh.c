#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

char *get_next_token(char **line_ptr, char* delimiters) {
    char *token = NULL;
    while (*line_ptr != NULL && (token == NULL || token[0] == '\0')) {
        token = strsep(line_ptr, delimiters);
    }
    return token;
}


int main(int argc, char *argv[]) {

//    printf("%d\n", access("/home/nathan/cas-test.txt", F_OK));

    /* add code here */
    // printf("Hello World\n");
//    for (int i = 0; argv[i] != NULL; i++) {
//        printf("%s,", argv[i]);
//    }

    enum ExecutionMode {Interactive = 1, Batch = 2};

    enum ExecutionMode curr_mode = argc;

    FILE *input_stream;
    if (curr_mode == Interactive) {
        input_stream = stdin;
    } else if (curr_mode == Batch){
        input_stream = fopen(argv[1], "r");
    }

    char *PATH = malloc(10 * sizeof(char));
    *PATH = '\0';
    strcat(PATH, "/bin:");

    char *line = NULL;
    char *token = NULL;
    size_t len = 0;

    char *delimiters = "\n ";

    int running = 1;
    while (running)
    {
        if (curr_mode == Interactive) {
            printf("lsh> ");
        }
        line = NULL;
        token = NULL;
        len = 0;

        long characters_read = getline(&line, &len, input_stream);
        char *original_line = line;

        if(characters_read == -1) {// EOF marker hit or error in reading
            free(original_line);
            exit(0);
        }
        token = get_next_token(&line, delimiters);

        if (strcmp(token, "exit") == 0) { // manual exit
            char *invalid_arg = get_next_token(&line, delimiters);
            if (*invalid_arg != '\0') {
                fprintf(stderr, "An error has occurred\n"); // Exit doesn't take any args
            };
            running = 0;
        }
        else if (strcmp(token, "cd") == 0) {
            char *directory = get_next_token(&line, delimiters);
            char *invalid_arg = get_next_token(&line, delimiters);

            if (*directory == '\0' || *invalid_arg != '\0') {
                fprintf(stderr, "An error has occurred\n"); // Invalid argument count
            } else {
//                printf("cd %s\n", directory);
                chdir(directory);
            }
        }
        else if (strcmp(token, "path") == 0) {
            PATH = realloc(PATH, characters_read + 10);
            *PATH = '\0';
            char *directory = get_next_token(&line, delimiters);

            while (directory != NULL && *directory != '\0') {
                strcat(PATH, directory);
                strcat(PATH, ":");
                directory = get_next_token(&line, delimiters);
            }

//            printf("PATH %s\n", PATH);
        }
        else if (strcmp(token, "") == 0) { // Empty case

        }
        else { // Non built-in commands

            char *executable = token;

            char *temp_PATH = malloc(strlen(PATH) + 10);
            char *original_temp_PATH = temp_PATH;
            temp_PATH = strcpy(temp_PATH, PATH);
            char *path_delimiters = ":";

            char *directory = get_next_token(&temp_PATH, path_delimiters);
            int access_code = -1;
            char *full_path = malloc(strlen(executable) + strlen(directory) + 10);
            full_path = strcpy(full_path, directory);
            full_path = strcat(full_path, "/");
            full_path = strcat(full_path, executable);

            while (directory != NULL && *directory != '\0') {
                access_code = access(full_path, X_OK);
                if (access_code == 0) {
                    break;
                }
                directory = get_next_token(&temp_PATH, path_delimiters);
                full_path = realloc(full_path, strlen(executable) + strlen(directory) + 10);
                full_path = strcpy(full_path, directory);
                full_path = strcat(full_path, "/");
                full_path = strcat(full_path, executable);

            }
            if (access_code == -1) {
                fprintf(stderr, "An error has occurred\n"); // Executable could not be found in path
            } else {
//                printf("executing %s\n", full_path);
                char *prog_args[characters_read];

                int index = 0;
                while (token != NULL && *token != '\0') {
                    prog_args[index] = token;
                    token = get_next_token(&line, delimiters);
                    index++;
                }
                prog_args[index] = NULL;

                int rc = fork();
                int child_status = 0;
                if (rc < 0) {
                    fprintf(stderr, "An error has occurred\n");
                } else if (rc == 0) {
                    execv(full_path, prog_args);
                } else {
                    waitpid(rc, &child_status, 0);
//                    printf("%d", child_status);
                }

//                for (int i = 0; i < index; i++) {
//                    printf("%s, ", prog_args[i]);
//                }

            }

            free(original_temp_PATH);
            free(full_path);
        }

        free(original_line);
    }

    if (argc == 2) {
        fclose(input_stream);
    }

    exit(0);

    return 0;
}
