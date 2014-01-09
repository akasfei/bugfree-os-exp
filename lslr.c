#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int opt_r = 0;
int opt_l = 0;
int opt_a = 0;

struct dirlist {
  char path[255];
  struct dirlist *next;
};

void getAccess(char *access, int mode)
{
  int i;
  if (S_ISDIR(mode))
    access[0] = 'd';
  else
    access[0] = '-';
  for (i=1; i<10; i++)
  {
    if (mode & (1<<(9-i)))
      access[i] = 'r';
    else
      access[i] = '-';
    i++;
    if (mode & (1<<(9-i)))
      access[i] = 'w';
    else  
      access[i] = '-';
    i++;
    if (mode & (1<<(9-i)))
      access[i] = 'x';
    else
      access[i] = '-';
  }
  access[i] = '\0';
}

void printStat(char *path, char *name)
{
  struct stat thisStat;
  char filepath[512], timestr[80], access[11], nameColor[8];
  sprintf(filepath, "%s/%s", path, name);
  stat(filepath, &thisStat);
  strftime(timestr, 80, "%c", localtime(&thisStat.st_ctime));
  getAccess(access, thisStat.st_mode);
  if (S_ISDIR(thisStat.st_mode))
    strcpy(nameColor, "\033[34m");
  else if (thisStat.st_mode & 1<<6)
    strcpy(nameColor, "\033[32m");
  else
    strcpy(nameColor, "");
  if (opt_l)
    printf("%s %6d %s %4d %4d %s%s\033[37m\n", access, thisStat.st_size, timestr, thisStat.st_uid, thisStat.st_gid,nameColor, name);
  else
    printf("%s %s%s\033[37m\n", timestr, nameColor, name);
}

void iterateDir(char *path)
{
  DIR *thisDIR;
  struct dirent *thisDirent;
  struct dirlist *dirHead=NULL, *dirThis=NULL;
  char subDir[63][255];
  int subDirc = 0;
  thisDIR = opendir(path);
  if (thisDIR == NULL)
  {
    printf("Error. Invalid directory %s\n", path);
    exit(2);
  }
  printf("Listing files under %s/\n", path);
  while((thisDirent = readdir(thisDIR)) != NULL)
  {
    if (strcmp(thisDirent->d_name, ".") == 0 || strcmp(thisDirent->d_name, "..") == 0)
    {  
      if (opt_a)
        printStat(path, thisDirent->d_name);
      continue;
    }
    if (opt_a == 0 && thisDirent->d_name[0] == '.')
      continue;
    if (thisDirent->d_type == DT_DIR && opt_r)
    {
      if (dirThis == NULL)
      {
        dirHead = (struct dirlist *)malloc(sizeof(struct dirlist));
        dirThis = dirHead;
      } else {
        dirThis->next = (struct dirlist *)malloc(sizeof(struct dirlist));
        dirThis = dirThis->next;
      }
      sprintf(dirThis->path, "%s/%s", path, thisDirent->d_name);
      subDirc++;
    }
    printStat(path, thisDirent->d_name);
  }
  closedir(thisDIR);
  printf("\n");
  for (dirThis = dirHead; dirThis != NULL; dirThis = dirThis->next)
  {
    iterateDir(dirThis->path);
  }
}

int main(int argc, char * argv[])
{
  char path[512];
  int i, j, hasPath = 0;
  for (i=1; i<argc; i++)
  {
    if (argv[i][0] == '-')
    {
      for (j = strlen(argv[i]) - 1; j>0; j--)
      {
        if ('r' == argv[i][j])
          opt_r = 1;
        else if ('l' == argv[i][j])
          opt_l = 1;
        else if ('a' == argv[i][j])
          opt_a = 1;
        else
        {
          printf("Invalid option: %s \n", argv[i]);
          exit(1);
        }
      }
    } else {
      hasPath = 1;
      strcpy(path, argv[i]);
    }
  }
  if (hasPath == 0)
    strcpy(path, ".");
  iterateDir(path);
  return 0;
}

