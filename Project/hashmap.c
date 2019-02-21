#include "hashmap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Create a hashmap with specified number of buckets
struct hashmap* hm_create(int num_buckets){
  struct hashmap* hm;
  hm = malloc(sizeof(struct hashmap));  //Allocate memory for Hashmap
  hm->map = (struct llnode**)malloc(sizeof(struct llnode*)*num_buckets);  //Allocate space for each bucket in map
  memset(hm->map, 0, sizeof(struct llnode*)*num_buckets);
  hm->num_buckets = num_buckets;
  hm->num_files = 0;
  return hm;
}

//Return number of occurences of word in document
//If pair not found, return -1
int hm_get(struct hashmap* hm, char* word, char* document_id)
{
  //If list has not been initialized, return -1
  if(hm == NULL)
  {
    printf("Hashmap not initialized\n");
    return -1;
  }
  int key = hash(hm, word);  //Get key for data pair
  if((hm->map)[key] == NULL)
  {   //If list head is null, return 0
    return 0;
  }

  //Iterate through list to find data pair
  struct llnode* ptr = (hm->map)[key];
  while(ptr != NULL)
  {
    if(strcmp(ptr->word, word) == 0)
    {
      struct dirll* dirptr = ptr->dirlist; 
      while(dirptr != NULL)
      {
        if(strcmp(dirptr->document_id, document_id) == 0)
        {
          return dirptr->tf;
        }
        dirptr = dirptr->next;
      }
    }
    ptr = ptr->next;
  }

  return 0;  //if not in list, return 0
}


//Put a key value pair into the hashmap
//If pair already exists, overwrite num_occurence value
void hm_put(struct hashmap* hm, char* word, char* document_id)
{
  //If list has not been initialized, return
  if(hm == NULL)
  {
    printf("Hashmap not initialized\n");
    return;
  }
  int key = hash(hm, word);  //Get key for data pair
  if(key < 0)
    return;
  if((hm->map)[key] == NULL)
  { //If list has not been initilized, create first node
    (hm->map)[key] = (struct llnode*)malloc(sizeof(struct llnode));
    memset((hm->map)[key], 0, sizeof(struct llnode));
    (hm->map)[key]->dirlist = (struct dirll*)malloc(sizeof(struct dirll));
    memset((hm->map)[key]->dirlist, 0, sizeof(struct dirll));
    (hm->map)[key]->word = word;
    (hm->map)[key]->dirlist->document_id = document_id;
    (hm->map)[key]->dirlist->tf++;
    (hm->map)[key]->dirlist->next = NULL;
    (hm->map)[key]->df++;
    (hm->map)[key]->next = NULL;
    return;
  }
  
  struct llnode* ptr = (hm->map)[key];
  struct llnode* last_ptr = ptr;
  while(ptr != NULL)
  {
    //If data matches current node, set num_occurences return
    if(strcmp(ptr->word, word) == 0)
    {
      struct dirll* dirptr = ptr->dirlist;
      struct dirll* last_dirptr = dirptr;
      while(dirptr != NULL)
      {
        if(strcmp(dirptr->document_id, document_id) == 0)
        {
          dirptr->tf++;
          return;
        }
        last_dirptr = dirptr;
        dirptr = dirptr->next;
      }
      //If directory is not present in the list
      last_dirptr->next = malloc(sizeof(struct dirll));
      memset(last_dirptr->next, 0, sizeof(struct dirll));
      last_dirptr->next->document_id = document_id;
      last_dirptr->next->tf++;
      last_dirptr->next->next = NULL;
      ptr->df++;
      return;
    }
    last_ptr = ptr;
    ptr = ptr->next;
  }


  //If data pair not in list, make new node at end of list
  last_ptr->next = malloc(sizeof(struct llnode));
  memset(last_ptr->next, 0, sizeof(struct llnode));
  last_ptr->next->word = word;
  last_ptr->next->dirlist = malloc(sizeof(struct dirll));
  memset(last_ptr->next->dirlist, 0, sizeof(struct dirll));
  last_ptr->next->dirlist->document_id = document_id;
  last_ptr->next->dirlist->tf++;
  last_ptr->next->dirlist->next = NULL;
  last_ptr->next->df++;
  last_ptr->next->next = NULL;

  return;
}

struct llnode* stop_word(struct hashmap* hm, int key, struct llnode* current, struct llnode **prev)
{
  if(*prev==NULL)
  {
    hm->map[key] = current->next;
    free(current);
    return hm->map[key];
  }
  else
  {
    (*prev)->next = current->next;
    free(current);
    return (*prev)->next;
  }
}

void hm_destroy(struct hashmap* hm)
{
  if(hm == NULL)
  {
    printf("Hashmap not initialized\n");
    return;
  }
  int i;
  //Free every list in map
  for(i=0; i<hm->num_buckets; i++)
  {
    struct llnode* ptr = (hm->map)[i];
    while(ptr != NULL)
    {
      struct llnode* temp = ptr->next;
      struct dirll* dirptr = ptr->dirlist;
      while(dirptr != NULL)
      {
        struct dirll* dirtemp = dirptr->next;
        free(dirptr);
        dirptr = dirtemp;
      }
      free(ptr);
      ptr = temp;
    }
  }
  //Free map
  free(hm->map);
  //Free hashmap
  free(hm);
}

//take given word and id and map them to correct bucket in hashmap
//return key
int hash(struct hashmap* hm, char* word){
  int sum = 0;
  int key;
  char* ptr = word;
  //Add ASCII values of word to sum
  while(*ptr != '\0'){
    sum += *ptr++;
  }
  //Key is sum mod size of hashmap
  key = sum%(hm->num_buckets);
  return key;
}
