#include<stdio.h>
#include<getopt.h>
#include<string.h>
#include<stdlib.h>
#include<dirent.h>
#include<unistd.h>

//take the global args that we used for string that we have to match and directory in which we have to search that is optional

char searchString[1024] = "" ;
char gDirectory[512]= "";
char *strings[50];  //It means sentence having 50 spaces will be allowed ..not more than this. 


int get_no_of_strings(char *read_buf, char *fields[], char *token, int max_flds)
{       
  int totalFlds = 0;
  char *ptr;
  char *token_ptr = NULL;
          
  ptr = read_buf;
  while((token_ptr = strtok(ptr, token)) != NULL)
  {  
    ptr = NULL;
    totalFlds++;
    if(totalFlds > max_flds)
    {
      fprintf(stderr, "Total fields are more than max fields (%d), remaining fields are ignored\n", max_flds);
      totalFlds = max_flds;
      break;  /* break from while */
    }
    fields[totalFlds - 1] = token_ptr;
  }
  return(totalFlds);
}

static void usage(char *err_msg)
{
  if(err_msg)
    fprintf(stderr, "%s\n", err_msg);
  fprintf(stderr, "Usage: \n");
  fprintf(stderr, "     : binaryfile -s <string that you want to search>  -d <directory>\n");
  exit (-1);
}



void traverse_and_search(char *filename, int numString)
{
  char file[512];
  FILE *fp;
  char *ptr;
  char buf[4*1024+1];

  sprintf(file, "%s/%s", gDirectory, filename);
  if((fp = fopen(file, "r")) == NULL)
  {
    fprintf(stderr, "Error in opening file - %s\n", filename);
    exit(0);
  }

  while(fgets(buf, 1024, fp)) 
  {
     buf[strlen(buf) - 1] = '\0'; //Removing new lines.
    //ignore emtpy lines.
    if((buf[0] == '\n')||(buf[0] == '#'))
      continue;


    int i;
    char *ptr;

    for(i = 0; i < numString; i ++)
    {
      ptr = strstr(buf, strings[i]);
      if(ptr)
      {
        //ther will be a case when one string already macthed with that file ,
        //so to avoid this, just keep track of previous one.
        printf("Pattern Found in fileName - %s\n", filename); 
        break;
      }
    }
  }

  fclose(fp);
}

void check_and_search_string_in_files(int numString)
{
  //first get the files name and iterate one by one .
  DIR *d;
  struct dirent *dir;
  d = opendir(gDirectory);
  if(d)
  {
     while ((dir = readdir(d)) != NULL)
     {
        //traverse each string one by one in file
        traverse_and_search(dir->d_name, numString);
     }
     closedir(d);
  }
}


#define MAX_STRING 50
//main
int main(int argc , char *argv[])
{
  char c;
  struct dirent **thread1_namelist;

  while ((c = getopt(argc, argv, "s:d:")) != -1) {
  switch(c) {
    case 's':
       strcpy(searchString, optarg);
       break;
    case 'd':
       strcpy(gDirectory, optarg);
       break;
    case '?':
     default:
       usage(0);
    }
 } 

  //validate the string
  if(searchString[0] == 0)
  {
     printf("Entered string is empty.. hence exiting");
     usage(0);
     exit(-1);
  }

  //now check if we something in directory or not , if not then take it as a current direcory.
  if(gDirectory[0] == 0)
  {
    if (!getcwd(gDirectory, 256)) {
      fprintf(stderr, "error in getting pwd\n");
      exit(-1);
    }
  }

  //tokenize your string because its is not mandatory that we have to compare whole string .
  //so tokenize using space

  int numString = get_no_of_strings(searchString, strings/*char array*/, " ", MAX_STRING); 

  //now get all the files in the directory.
  check_and_search_string_in_files(numString);

  return 0;
}
