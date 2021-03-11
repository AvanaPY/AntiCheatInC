#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <ftw.h>
#include <openssl/md5.h>
#include <omp.h>
#include <pthread.h>
#include <stdatomic.h>


#define MAX_THREAD_COUNT 1
#define MAX_OPEN_FILE_HANDLES 20
#define MD5_READ_BYTES_SIZE 1024 * 1024

#define WANT_HASHING_DATA


const char* EXT_IGNORE[] = { ".cfg" };
const size_t EXT_IGNORE_COUNT = sizeof(EXT_IGNORE) / sizeof(char*);

int num_files;
char* files;
atomic_uchar* md5_hash_list;
int max_number_files = 16;
int max_path_length = 260;

pthread_mutex_t lock;

pthread_t threads[MAX_THREAD_COUNT];

int fn(const char* fpath, const struct stat *sb, int typeflag);

void hash_it(const char* fpath, int file_id);

void hash_it_all(atomic_uchar* list_of_hashes, unsigned char c[]);

void* worker();

int main(int argc, char **argv)
{
    // Check if input is correct
    if (argc < 3)
    {
        printf("Invalid input. Example of correct input: %s path-to-tree <correct_hash>", argv[0]);
        return EXIT_FAILURE;
    }

    // Get path and md5 length from input
    const char* path = argv[1];
    const char* correct_hash = argv[2];

    num_files = 0;
    files = malloc(max_path_length * max_number_files);

    // Set start time from omp
    double t0 = omp_get_wtime();

    // Get the files we want
    ftw(path, fn, MAX_OPEN_FILE_HANDLES);
    
    printf("Number of files: %i\n", num_files);

    // Create the MD5 container
    md5_hash_list = malloc(num_files * MD5_DIGEST_LENGTH + 1); // Container for md5 hashes

    //pthread_mutex_init(&lock, NULL);
    // Use threads to calculate the hashes
    int counting_files = 0;
    while (counting_files < num_files)
    {   
        for (size_t i = 0; i < MAX_THREAD_COUNT; i++)
        {
            // printf("Start of file %i\n", counting_files + 1);
            int file_ptr = counting_files;
            pthread_create(&threads[i], NULL, worker, (void*)&file_ptr);
            counting_files++;
            
        }
        for (size_t j = 0; j < MAX_THREAD_COUNT; j++)
        {
            pthread_join(threads[j], NULL);
            //printf("Hash %i done! Hash: %d\n", counting_files, &md5_hash_list[counting_files * MD5_DIGEST_LENGTH]);
        }  
    }
    printf("\n");
    //pthread_mutex_destroy(&lock);
    // Create the final hash
    unsigned char c[MD5_DIGEST_LENGTH];
    hash_it_all(md5_hash_list, c);

    // Calculate the lapsed time
    double time_lapsed = omp_get_wtime() - t0;

    // Turn the hash into a string
    char md5_hash[32];
    
    for (size_t i = 0; i < 16; i++)
    {
        sprintf(&md5_hash[i * 2], "%02x", c[i]);
    }
    
    for(int i = 0; i < num_files; i++){
        for(int j = 0; j < 16; j++)
            printf("%02x", md5_hash_list[i * MD5_DIGEST_LENGTH +  j]);
        printf("\n");
    }
    
    // Print results
    printf("True Hash: %s\n", correct_hash);
    printf("Tree Hash: %s\n", md5_hash);

    if (strcmp(correct_hash, md5_hash) == 0)
    {
        printf("Game files OK!\n");
    }
    
    else
    {
        printf("Game files differ!\n");
    }
        
    // If we want the time it took to hash and compare, print the time
    #ifdef WANT_HASHING_DATA
    printf("Time for hashing: %f seconds\n", time_lapsed);
    printf("Number of files: %i\n", num_files);
    #endif

    // Free data
    free(md5_hash_list);
    free(files);

    return EXIT_SUCCESS;
}

// Function that the ftw() function uses; Maps the tree.
int fn(const char* path, const struct stat *sb, int typeflag)
{
    // Check if found file is a regular file
    if (S_ISREG(sb->st_mode))
    {
        // Find the extention of the file
        char* ext = strrchr(path, '.');
        if (ext != NULL)
        {
            //check if the file extension is one of the extensions that should be ignored, if yes, return 0.
            for (size_t i = 0; i < EXT_IGNORE_COUNT; i++)
            {
                if(strcmp(EXT_IGNORE[i], ext) == 0)
                {
                    return 0;
                }
            }
            
        }
        
        if (num_files >= max_number_files)
        {
            max_number_files *= 2;
            files = realloc(files, max_number_files * max_path_length + 1);
        }
        
        //printf("%s\n", path);
        int index = num_files * max_path_length;
        strcpy(&files[index], path);
        //printf("%s\n", &files[index]);
        num_files += 1; // Increase the amount of files we have.
        //printf("Mid %i\n", num_files);
    }
    return 0;
}

// Function for creating he md5 of a file
void hash_it(const char* path, int file_id)
{
    unsigned char c[MD5_DIGEST_LENGTH];
    // printf("Start of hashing\n");
    // printf("  path: %s\n", path);
    FILE* file = fopen(path, "rb"); // rb: read binary. Is apparently better for non-text files
    MD5_CTX md_context;
    unsigned char data[MD5_READ_BYTES_SIZE]; // the data of the file
    
    MD5_Init(&md_context); // Starts the hashing

    int bytes;
    while ((bytes = fread(data, 1, MD5_READ_BYTES_SIZE, file)) != 0) // Runns while the file still has something to read 
    {
        MD5_Update(&md_context, data, bytes); // Hash the file in MD5_READ_BYTES_SIZE chunks.
    }
    
    MD5_Final(c, &md_context); // Finishes the hashing and puts it into the list c

    fclose(file); // Close thew file after hashing
    
    memcpy(&md5_hash_list[file_id * MD5_DIGEST_LENGTH], c, MD5_DIGEST_LENGTH); // memcpy(void * destination, const void * source, size_t num): copies source to destination with num number of bytes. I.e. sends the hash c to info's md5_buffer, with the size of MD5_DIGEST_LENGTH.
    printf("  hash: ");
    for(int i = 0; i < 16; i++)
        printf("%02x", md5_hash_list[file_id * MD5_DIGEST_LENGTH + i]);
    printf("\n");
    // printf("%d\n", md5_hash_list[file_id * MD5_DIGEST_LENGTH]);
    // printf("  End of hashing.\n");
}

// Calculate the final hash of the tree
void hash_it_all(atomic_uchar* list_of_hashes, unsigned char c[])
{
    printf("Fixing the final hash\n");
    
    MD5_CTX md_context;
    
    MD5_Init(&md_context); // Initialize the hashing
    
    for (size_t i = 0; i < num_files; i++)
    {
        MD5_Update(&md_context, &list_of_hashes[i * MD5_DIGEST_LENGTH], MD5_DIGEST_LENGTH); // Hash each file in the list of hashes
    }
    
    MD5_Final(c, &md_context); // Finalize the hashing.
    printf("Fixed the final hash: ");
    for(int i = 0; i < 16; i++)
        printf("%02x", c[i]);
    printf("\n");
}

// Function for thread
void* worker(void* id)
{
    //pthread_mutex_lock(&lock);
    int file_id = *(int*)id;

    //printf("start of thread %i\n", file_id);

    //printf("File ID: %i\n", file_id);

    char* file = &files[file_id * max_path_length]; // Get the file from the file list
    
    hash_it(file, file_id); // Get the hash of the file

    
    //printf("End of thread\n");
    //pthread_mutex_unlock(&lock);
    return NULL;
}