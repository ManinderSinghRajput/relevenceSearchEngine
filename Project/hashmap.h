#ifndef HASHMAP_H
#define HASHMAP_H

/*struct rankdir {
        char* document_id;
        double rankScore;
};*/

struct dirll {
	char* document_id;
        int tf;			//term frequency
        double tf_idf;		//tf-idf weighting tf * idf
	struct dirll* next;
};

struct llnode {
        char* word;
        struct dirll* dirlist;
        struct llnode* next;
	int df;		//document frequency
        double idf;	//inverse document frequency
};

struct hashmap {
        struct llnode** map;
        int num_buckets;
        int num_files;
};

struct hashmap* hm_create(int num_buckets);
int hm_get(struct hashmap* hm, char* word, char* document_id);
void hm_put(struct hashmap* hm, char* word, char* document_id);
struct llnode* stop_word(struct hashmap* hm, int key, struct llnode* current, struct llnode **prev);
void hm_destroy(struct hashmap* hm);
int hash(struct hashmap* hm, char* word);

#endif
