/*
 * Copyright (c) 2021, Dana Burkart <dana.burkart@gmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <dirent.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define SUBSYSTEM_COUNT 5
#define USAGE "Usage %s:\n    --rebase  Rebaseline failing tests\n"
#define HRULE "================================================================================\n"

// We want to run our tests in a bottom up order for each subsystem, because if
// one system fails, we'll want to abort higher-level tests (since they will
// most likely fail as well)
const char *subsystems[SUBSYSTEM_COUNT] = {
    "lex",
    "parse",
    "compile",
    "vm",
    "interpret"
};

typedef struct
{
    size_t size;
    size_t capacity;
    char **tests;
} test_list_t;

test_list_t *test_list_create(void)
{
    test_list_t *list;

    list = (test_list_t *)malloc(sizeof(test_list_t));
    list->size = 0;
    list->capacity = 8;
    list->tests = (char **)malloc(sizeof(char *) * list->capacity);

    return list;
}

void test_list_destroy(test_list_t *list)
{
    for (int i = 0; i < list->size; i++)
    {
        free(list->tests[i]);
    }
    free(list->tests);
    free(list);
}

void test_list_add(test_list_t *list, char **path)
{
    if (list->size >= list->capacity)
    {
        list->capacity *= 2;
        list->tests = realloc(list->tests, sizeof(char *) * list->capacity);
    }

    list->tests[list->size++] = *path;
}

void discover_tests(test_list_t *list, char *base)
{
    DIR *directory = opendir(base);
    struct dirent *entry = readdir(directory);

    do
    {
        char *entry_path;

        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        // Calculate the name for this path
        asprintf(&entry_path, "%s/%s", base, entry->d_name);

        switch (entry->d_type)
        {
            case DT_REG:
                test_list_add(list, &entry_path);
                break;
            case DT_DIR:
                discover_tests(list, entry_path);
                free(entry_path);
                break;
        }
    } while ((entry = readdir(directory)) != NULL);

    closedir(directory);
}

char *expectation_for_test(const char *subsystem, const char *test_path)
{
    char *expectation_path;
    size_t len = strlen(subsystem) + 1;

    char *common_subpath = strchr(test_path + len, '/');
    asprintf(&expectation_path, "%s/expectations%sxt", subsystem, common_subpath);

    // HACK: we need to change the extension and I'm lazy.
    char *n = strrchr(expectation_path, 'n');
    *n = 't';

    return expectation_path;
}

char *diff_files(const char *first, const char *second)
{
    char *diff = NULL;

    int pipefd[2];
    pipe(pipefd);

    if (fork() == 0)
    {
        // Child
        close(pipefd[0]);

        dup2(pipefd[1], 1);    // Send stdout
        dup2(pipefd[1], 2);    // Send stderr

        close(pipefd[1]);

        execl("/usr/bin/diff", "diff", "-rubBd", first, second, NULL);
    }
    else
    {
        // Parent
        char buffer[1024];
        int len, bytes_written = 0, total_size = 0;

        close(pipefd[1]);

        while ((len = read(pipefd[0], buffer, 1024)) != 0)
        {
            total_size += len + 1;

            diff = realloc(diff, total_size);
            memcpy(diff + bytes_written, buffer, len);
            bytes_written += len;
            diff[bytes_written] = 0;
        }
    }

    return diff;
}

void usage(char *name)
{
    printf(USAGE, name);
    exit(1);
}

int main(int argc, char *argv[])
{
    bool rebase = false;
    int all_passes, all_fails;
    unsigned long start_time;

    if (argc == 2)
    {
        if (strcmp(argv[1], "--rebase") == 0)
        {
            rebase = true;
        }
        else
        {
            usage(argv[0]);
        }
    }

    if (argc > 2)
        usage(argv[0]);

    start_time = time(NULL);
    for (int i = 0; i < SUBSYSTEM_COUNT; i++)
    {
        const char *subsystem = subsystems[i];
        char *input_directory, *expectations_directory, *run_cmd;
        test_list_t *list = test_list_create();
        int passes = 0, fails = 0;

        asprintf(&input_directory, "%s/input", subsystem);
        asprintf(&expectations_directory, "%s/expectations", subsystem);
        asprintf(&run_cmd, "%s/run", subsystem);

        discover_tests(list, input_directory);

        printf("%sTesting %s subsystem\n%sFound %zu tests.\n", HRULE, subsystem, HRULE, list->size);

        for (int j = 0; j < list->size; j++)
        {
            printf("%s...", list->tests[j]);
            int pipefd[2];
            pipe(pipefd);

            if (fork() == 0)
            {
                // Child
                close(pipefd[0]);

                dup2(pipefd[1], 1);    // Send stdout
                dup2(pipefd[1], 2);    // Send stderr

                close(pipefd[1]);

                execl(run_cmd, "run", list->tests[j], NULL);
            }
            else
            {
                // Parent
                char buffer[1024];
                char temp_name[32];
                int len;
                char *expectation_path = expectation_for_test(subsystem, list->tests[j]);
                int fd;

                close(pipefd[1]);

                // If we're rebasing, we'll write the results out to our expectation path
                if (rebase)
                {
                    fd = open(expectation_path, O_WRONLY | O_CREAT | O_TRUNC);
                }
                else
                {
                    if (access(expectation_path, F_OK))
                        close(open(expectation_path, O_WRONLY | O_CREAT));
                    memset(temp_name, 0, sizeof(temp_name));
                    strncpy(temp_name, "/tmp/testrunner-XXXXXX", 22);
                    fd = mkstemp(temp_name);
                }

                while ((len = read(pipefd[0], buffer, 1024)) != 0)
                {
                    write(fd, buffer, len);
                }

                close(fd);

                if (!rebase)
                {
                    char *diff = diff_files(expectation_path, temp_name);

                    if (diff != NULL)
                    {
                        printf(" FAILED:\n");
                        printf("%s\n", diff);
                        free(diff);
                        fails += 1;
                    }
                    else
                    {
                        printf(" PASSED\n");
                        passes += 1;
                    }

                    unlink(temp_name);
                }
                else
                {
                    printf(" REBASED\n");
                    passes += 1;
                }

                close(pipefd[0]);
                free(expectation_path);
            }
        }

        free(input_directory);
        free(expectations_directory);
        free(run_cmd);
        test_list_destroy(list);

        int pass_rate = (int)(passes / (float) (passes + fails) * 100);
        printf("\nSummary: %d%% pass rate (%d/%d) \n\n", pass_rate, passes, passes + fails);
        all_passes += passes;
        all_fails += fails;
    }

    printf(HRULE);
    int pass_rate = (int)(all_passes / (float) (all_passes + all_fails) * 100);
    printf("Ran %d tests in %lu seconds, with a pass rate of %d%%\n",
            all_passes + all_fails, time(NULL) - start_time, pass_rate
          );
    return all_fails;
}
