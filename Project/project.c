#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <math.h>


#include "hashmap.h"

#define MAX_STRING 50

struct rankdir {
        char* document_id;
        double rankScore;
};

void hm_print(struct hashmap* hm);
struct hashmap* training(int size_of_hm);
void read_query(struct hashmap* hm);
void rank(struct hashmap* hm, char* allstrings);
int compare (const void * a, const void * b);


struct hashmap* training(int size_of_hm)
{
  FILE *fptr = NULL;
  char line[1024] = {0};
  char *word;
  char gDirectory[512]= "";
  DIR *d;
  struct dirent *dir;
  int i;

  //Create hashmap
  struct hashmap* hm = hm_create(size_of_hm);

  /*printf("Enter the space seprated names of files:");
  scanf("%c",searchString);
  scanf("%[^\n]%*c", searchString);*/
   
  if (!getcwd(gDirectory, 256)) {
    fprintf(stderr, "error in getting pwd\n");
    exit(-1);
  } 
  
  d = opendir(gDirectory);
  if(d)
  {
    while (((dir = readdir(d)) != NULL) && (dir->d_type == DT_REG ))
    { 
       fptr = fopen(dir->d_name, "r");
       if(fptr!=NULL)
       {
         while(fgets(line,1024,fptr) != NULL)
         {
           char *pos;
           if ((pos=strchr(line, '\n')) != NULL)  //Removing newline character from input
              *pos = '\0';
           word = strtok(line, " ");
           while (word != NULL)
           {
             char *tempWord = strdup(word);
             char *tempFile = strdup(dir->d_name);
             hm_put(hm, tempWord, tempFile);
             word = strtok(NULL, " ");
           }
         }
         hm->num_files++;
       }
       fclose(fptr);
     }
     closedir(d);
  }

  for(i=0; i<hm->num_buckets; i++)
  {
    //Iterate through list to find data pair
    struct llnode* ptr = (hm->map)[i];
    struct llnode* prev_ptr = NULL;
    while(ptr != NULL)
    {
      double numer = (double)(hm->num_files);
      double denom = (double)((ptr->df != 0) ? ptr->df : 1);
      ptr->idf = log(numer/denom);
      if(ptr->idf == 0)
      {
        ptr = stop_word(hm, i, ptr, &prev_ptr);
        continue;
      }

      struct dirll* dirptr = ptr->dirlist;
      while(dirptr != NULL)
      {   
        dirptr->tf_idf = dirptr->tf * ptr->idf;
        dirptr = dirptr->next;
      }

      prev_ptr = ptr;
      ptr = ptr->next;
    }
  }

  return hm;
}

//Function to print entire hashmap, used for testing only
void hm_print(struct hashmap* hm)
{
  int i;
  if(hm == NULL)
  {
    printf("Hashmap not initialized\n");
    return;
  }

  for(i=0; i<hm->num_buckets; i++)
  {
    //Iterate through list to find data pair
    struct llnode* ptr = (hm->map)[i];
    while(ptr != NULL)
    {
      printf("%s:\n",ptr->word);
      struct dirll* dirptr = ptr->dirlist;
      while(dirptr != NULL)
      {
        printf("  %s:%d\n",dirptr->document_id, dirptr->tf);
        dirptr = dirptr->next;
      }
      ptr = ptr->next;
    }
  }
}

void read_query(struct hashmap* hm)
{
  char allstring[2*1024] = {0};
  
  if(hm == NULL){
    printf("Hashmap not initialized\n");
    return;
  } 
  printf("Enter the space seprated string to be searched: ");
  scanf("%c", allstring);
  scanf("%[^\n]%*c", allstring);

  rank(hm, allstring);
}

int compare (const void * a, const void * b)
{

  struct rankdir *rankA = (struct rankdir *)a;
  struct rankdir *rankB = (struct rankdir *)b;

  return ((rankB->rankScore - rankA->rankScore) > 0);
}

void rank(struct hashmap* hm, char* allstrings)
{
  int i, j;
  char *search_ptr;
  char *search_token_ptr = NULL;
  char *search_strings[50] = {0};
  int search_totalFlds = 0;

  search_ptr = allstrings;

  while((search_token_ptr = strtok(search_ptr, " ")) != NULL)
  { 
    search_ptr = NULL;
    if(search_totalFlds >= MAX_STRING)
    { 
      fprintf(stderr, "Total fields are more than max fields (%d), remaining fields are ignored\n", MAX_STRING);
      break;  /* break from while */
    }
    search_strings[search_totalFlds] = search_token_ptr;
    search_totalFlds++;
  }

  if(hm == NULL)
  {
    printf("Hashmap not initialized\n");
    return;
  }

  int key;
  struct llnode* ptr;

  struct rankdir* rankd = (struct rankdir*)malloc(sizeof(struct rankdir) * (hm->num_files));
  memset(rankd, 0, sizeof(struct rankdir) * (hm->num_files));
  int rankd_count = 0;

  for(i=0; i<search_totalFlds; i++)
  {
    key = hash(hm, search_strings[i]);  //Get key for data pair
    ptr = (hm->map)[key];
    while(ptr != NULL)
    {
      if(strcmp(ptr->word, search_strings[i]) == 0)
      {
        //printf("%s:\n", search_strings[i]);
        struct dirll* dirptr = ptr->dirlist;
        while(dirptr != NULL)
        {
          //printf("  %s:%d\n",dirptr->document_id, dirptr->tf);
          for(j=0;j<hm->num_files;j++)
          {
            if(rankd[j].document_id == NULL)
            {
              rankd[j].document_id = dirptr->document_id;
              rankd_count++;
            }
            if(strcmp(dirptr->document_id, rankd[j].document_id) == 0)
            {
              rankd[j].rankScore += dirptr->tf_idf;
              break;
            }
          }
          dirptr = dirptr->next;
        }
      }
      ptr = ptr->next;
    }
  }
  qsort (rankd, hm->num_files, sizeof(struct rankdir), compare);
  struct rankdir* rankd_ptr = rankd;
  while(rankd_count--)
  {
    printf("  %s\n", rankd_ptr->document_id);
    rankd_ptr++;
  }
}

int main(void){

  int size_of_hm = 0;
  char choice;

  printf("Enter number of buckets:");
  scanf("%d", &size_of_hm);

  struct hashmap* hm = training(size_of_hm);

  printf("Enter 'S' for search or 'X' to Exit: ");
  scanf("%c",&choice);
  scanf("%c",&choice);

  while (1)
  {
    if(choice == 'S' || choice == 's')
      read_query(hm);
    else if(choice == 'X' || choice == 'x')
      break;
    printf("Enter 'S' for search or 'X' to Exit: ");
    choice = getchar();
    if(choice == '\n')
      scanf("%c",&choice);
  }

  //Print all string in all Files
  //hm_print(hm);

  //Test hm_destroy
  hm_destroy(hm);

  return 0;
}
