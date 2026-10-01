#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

/**
 * Executes the command "cat scores | grep <arg> | sort".
 * P1 (parent)         : cat scores
 * P2 (child)          : grep <arg>
 * P3 (child's child)  : sort
 * As in the example, the parent doesn't wait, so the prompt may
 * reappear before the children finish.
 */

int main(int argc, char **argv)
{
  int pipe1[2];   // cat  -> grep
  int pipe2[2];   // grep -> sort
  int pid1, pid2;

  if (argc != 2)
    {
      fprintf(stderr, "Usage: %s <grep argument>\n", argv[0]);
      return 1;
    }

  char *cat_args[] = {"cat", "scores", NULL};
  char *grep_args[] = {"grep", argv[1], NULL};
  char *sort_args[] = {"sort", NULL};

  // pipe 1 connects P1 (cat) to P2 (grep)
  if (pipe(pipe1) == -1)
    {
      perror("pipe");
      return 1;
    }

  pid1 = fork();

  if (pid1 < 0)
    {
      perror("fork");
      return 1;
    }
  else if (pid1 == 0)
    {
      // P2 (child): handles "grep <arg>"

      // pipe 2 connects P2 (grep) to P3 (sort)
      if (pipe(pipe2) == -1)
        {
          perror("pipe");
          exit(1);
        }

      pid2 = fork();

      if (pid2 < 0)
        {
          perror("fork");
          exit(1);
        }
      else if (pid2 == 0)
        {
          // P3 (child's child): handles "sort"

          // stdin comes from pipe 2
          dup2(pipe2[0], 0);

          // close every pipe end this process isn't using
          close(pipe1[0]);
          close(pipe1[1]);
          close(pipe2[0]);
          close(pipe2[1]);

          execvp("sort", sort_args);
          perror("execvp sort");
          exit(1);
        }
      else
        {
          // P2 continues: grep reads from pipe 1, writes to pipe 2
          dup2(pipe1[0], 0);
          dup2(pipe2[1], 1);

          // close every pipe end this process isn't using
          close(pipe1[0]);
          close(pipe1[1]);
          close(pipe2[0]);
          close(pipe2[1]);

          execvp("grep", grep_args);
          perror("execvp grep");
          exit(1);
        }
    }
  else
    {
      // P1 (parent): handles "cat scores"

      // stdout goes into pipe 1
      dup2(pipe1[1], 1);

      // close both ends of pipe 1 now that stdout is redirected
      close(pipe1[0]);
      close(pipe1[1]);

      execvp("cat", cat_args);
      perror("execvp cat");
      exit(1);
    }

  return 0;
}
