#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

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
  char filepath[512], timestr[80], access[11];
  sprintf(filepath, "%s/%s", path, name);
  stat(filepath, &thisStat);
  strftime(timestr, 80, "%c", localtime(&thisStat.st_ctime));
  getAccess(access, thisStat.st_mode);

  printf("%s %6d %s %d %d %s\n", access, thisStat.st_size, timestr, thisStat.st_uid, thisStat.st_gid, name);
}

void iterateDir(char *path)
{
  DIR *thisDIR;
  struct dirent *thisDirent;
  char subDir[63][255];
  int subDirc = 0;
  thisDIR = opendir(path);
  printf("Listing files under %s:\n", path);
  while((thisDirent = readdir(thisDIR)) != NULL)
  {
    if (strcmp(thisDirent->d_name, ".") == 0 || strcmp(thisDirent->d_name, "..") == 0)
      continue;
    if (thisDirent->d_type == DT_DIR)
    {
      sprintf(subDir[subDirc], "%s/%s", path, thisDirent->d_name);
      subDirc++;
      // continue;
    }
    printStat(path, thisDirent->d_name);
  }
  closedir(thisDIR);
  printf("\n");
  int i;
  for (i=0; i<subDirc; i++)
  {
    iterateDir(subDir[i]);
  }
}

int main(int argc, char * argv[])
{
  char path[512];
  if (argc > 2)
  {
    printf("Usage: %s <pathname>\n", argv[0]);
  }
  if (argc == 1)
    strcpy(path, ".");
  else
    strcpy(path, argv[1]);
  iterateDir(path);
  return 0;
}

