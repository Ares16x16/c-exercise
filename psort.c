// COMP3230 Programming Assignment Two
// The sequential version of the sorting using qsort

/*
# Filename: psort.c
# Development platform: wsl
# Remark:
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>

# define INFINITY 9999999999  //define a infinity for pos
# define NEGATIVE_INFINITY -1 //define a infinity for pos
# define MAX 32               //a max value for array

int checking(unsigned int *, long);
int compare(const void *, const void *);


// global variables
long size;                // size of the array
unsigned int * intarr;    // array of random integers
long thread_no;           // number of thread
sem_t mutex;	            // used for mutual exclusion
pthread_t pid[MAX];       // array of thread id


struct threadDat{    
      int id ;      
      long upper;
      int lower;
      int count;
      long upperVal;  
      int lowerVal;    
      int *unsortedDat; 
      int unsortLength; 
      int *sorted; 
   };


void* sampleSort(void* arg)
  { 
      struct threadDat data = *(struct threadDat*) arg;
      
      int *localArr = (unsigned int*) malloc(sizeof(unsigned int)*data.count);
      
      int index=0;
      for(int i=0;i<data.unsortLength;i++){
          if(data.unsortedDat[i] <= data.upperVal && data.unsortedDat[i] > data.lowerVal){
              localArr[index] = data.unsortedDat[i];
              index++;
          }
      }
    
    //sort i
    qsort(localArr,data.count,sizeof(unsigned int),compare);

    //  write back to the sorted_array 
    sem_wait(&mutex);
    index = 0;
    for(int i = data.lower; i < data.upper; i++){
        data.sorted[i] = localArr[index];
        index++;
    }
    sem_post(&mutex);
  }



void findSplit (int* arr,int * splitTemp,int * finalSplit, int array_length,int k){

      int bucket_gap = array_length/(k*k);   
      //sort in every bucket
      int temp_bucket_length;
      for(int i=0;i<k*k;i++){
          if(i%k == 0){
              if (i+bucket_gap*k > array_length){
                temp_bucket_length = array_length-i;
              }else{
                temp_bucket_length = bucket_gap*k;
              }
              //printf("temp_bucket_length = %d\n",temp_bucket_length);
              qsort(arr+i,temp_bucket_length,sizeof(int),compare);
          }
      }
      
      int index = 0; 
      for(int i=0;i<k*k;i++){
          if(i%k != 0){
              // write to the array splitTemp
              splitTemp[index] = arr[i*bucket_gap];
              index++;
          }
          
      }

       // sort the splitTemp
      qsort(splitTemp,k*(k-1),sizeof(int),compare);
    
      // find the last splitter
      for(int i=1;i<k;i++){
          finalSplit[i-1] = splitTemp[i*k-1];
      }
      
      /*
      printf("final splitter:");
      for(int i=0;i<(k-1);i++){
          printf("%d,", finalSplit[i]);
      }
      printf("\n");
      */
}


//count the number of elements in every bucket
void countInBucket(unsigned int *arr, unsigned int* finalSplit,
                unsigned int *bucketLen, int array_length,int k){
    
    //initialize
    for(int i=0;i<k+1;i++){
        bucketLen[i] = 0;
    }
    
    int flag=0;
    for(int i=0;i<array_length;i++){
        flag = 0;
        for(int j=0;j<k-1;j++){
            if(finalSplit[j]>= arr[i]){
                flag = 1;
                bucketLen[j]++;
                break;
            }
        }
        
        if(flag == 0){
            bucketLen[k-1]++;
        }
    }
}

//find the bucket bound in the sorted array
void findBound(unsigned int * bucketLen, unsigned int* bound,int k){     
      //initialize
      bound[0]=0;

      for(int i=1;i<k+1;i++){ 
          bound[i] = bound[i-1] +bucketLen[i-1];
      }
}


int main (int argc, char **argv)
{
  long i, j;
  struct timeval start, end;

  sem_init( &mutex, 0, 1);

  if ((argc != 2 && argc != 3))
  {
    printf("Usage: psort <number>[<no_of_workers>]\n");
    exit(0);
  }

  //check whether 2nd arg exists and set the number of threads
  if (argv[2] == NULL){
    thread_no = 4;
  }else{
    thread_no = atol(argv[2]);
  }

  //debug
  /*
  printf("argv1 : %s\n", argv[0]);  
  printf("argv2 : %s\n", argv[1]);
  printf("argv3 : %s\n", argv[2]);
  printf("Number of threads : %ld\n", thread_no);
  */

  //generate array size
  size = atol(argv[1]);
  intarr = (unsigned int *)malloc(size*sizeof(unsigned int));
  if (intarr == NULL) {perror("malloc"); exit(0); }
  
  // set the random seed for generating a fixed random
  // sequence across different runs
  char * env = getenv("RANNUM");  //get the env variable
  if (!env)                       //if not exists
    srandom(3230);
  else
    srandom(atol(env));
  
  for (i=0; i<size; i++) {
    intarr[i] = random();
  }

  //initialize
  int *arrayLen;
  arrayLen = (unsigned int*) malloc(sizeof(unsigned int));
  arrayLen[0] = atol(argv[1]);

  unsigned int *splitTemp;
  unsigned int *finalSplit;

  
  splitTemp = (unsigned int*) malloc(sizeof(unsigned int)*thread_no*(thread_no-1));
  finalSplit = (unsigned int*) malloc(sizeof(unsigned int)*(thread_no-1));

  int *bucketLen = (unsigned int*) malloc(sizeof(unsigned int) *thread_no);

  //the final array
  int *sorted_array = (unsigned int*) malloc(sizeof(unsigned int)*arrayLen[0]); 
  int *bound = (int*) malloc(sizeof(int)*(thread_no+1));

  // measure the start time
  gettimeofday(&start, NULL);
  
  // just call the qsort library
  // replace qsort by your parallel sorting algorithm using pthread
  //=============================================================
  //qsort(intarr, size, sizeof(unsigned int), compare);
  //=============================================================



  //debug  
  //printf("intarr[0]: %d\n",intarr[0]); 
  //printf("arrayLen[0]: %d\n",arrayLen[0]); 
  //printf("%d\n",intarr[1]); 

  //find splitter
  findSplit(intarr, splitTemp,finalSplit,arrayLen[0],thread_no);
  countInBucket(intarr,finalSplit, bucketLen,arrayLen[0], thread_no);
  findBound(bucketLen,bound,thread_no);

  //debug
  /*
  for(int i=0;i<thread_no;i++){     
          if(i == 0 && i == thread_no-1){
              printf("bucket %d dat =(%10d, %10ld]\n",i+1,NEGATIVE_INFINITY,INFINITY);
          }
          if(i == 0){
              printf("bucket %d dat =(%10d, %10d]\n",i+1,NEGATIVE_INFINITY,finalSplit[i]);
          }else if(i == thread_no-1){
              printf("bucket %d dat =(%10d, %10ld]\n",i+1,finalSplit[i-1],INFINITY);
          }else{            
              printf("bucket %d dat =(%10d, %10d]\n",i+1,finalSplit[i-1],finalSplit[i]);
          }
      }
  */

  struct threadDat *datList = (struct threadDat*) malloc(sizeof(struct threadDat)*thread_no);
  for (int i = 0; i < thread_no; i++) {
    // initialize
    struct threadDat data ;
        
    data.id = i;     
    data.upper = bound[i+1];
    data.lower = bound[i];
    data.count = bucketLen[i];
    data.unsortedDat = intarr;
    data.unsortLength = arrayLen[0];
    data.sorted = sorted_array;
        
    if(i == 0){
        data.upperVal = finalSplit[i];
        data.lowerVal = NEGATIVE_INFINITY;          
    }else if(i == thread_no - 1){
            
        data.upperVal = INFINITY;
        data.lowerVal = finalSplit[i-1];          
    }else{
        data.upperVal = finalSplit[i];
        data.lowerVal = finalSplit[i-1];           
    }
        
    if(i == 0 && i == thread_no - 1){
        data.upperVal = INFINITY;
        data.lowerVal = NEGATIVE_INFINITY;         
    }
    datList[i] = data;
  }
    
  //create threads
  for(int i = 0; i < thread_no; i++ )
  {
      pthread_create( &pid[i], NULL, sampleSort, (void*)(datList + i));
      //printf("%ld\n",pid[i]);
  }
    
  //Waiting for the created thread to terminate
  for(int i = 0; i < thread_no; i++ ){
      pthread_join( pid[i], NULL );
  }

  //put back the array into intarr
  /*
  for (long i=0; i < size; i++){
    intarr[i] = sorted_array[i];
  }
  */

  // measure the end time
  gettimeofday(&end, NULL);
  
  if (!checking(sorted_array, size)) {
    printf("The array is not in sorted order!!\n");
  }
  
  printf("Total elapsed time: %.4f s\n", (end.tv_sec - start.tv_sec)*1.0 + (end.tv_usec - start.tv_usec)/1000000.0);
    
  free(intarr);
  return 0;
}


int compare(const void * a, const void * b) {
  return (*(unsigned int *)a>*(unsigned int *)b) ? 1 : ((*(unsigned int *)a==*(unsigned int *)b) ? 0 : -1);
}

int checking(unsigned int * list, long size) {
  long i;
  printf("First : %d\n", list[0]);
  printf("At 25%%: %d\n", list[size/4]);
  printf("At 50%%: %d\n", list[size/2]);
  printf("At 75%%: %d\n", list[3*size/4]);
  printf("Last  : %d\n", list[size-1]);
  for (i=0; i<size-1; i++) {
    if (list[i] > list[i+1]) {
      return 0;
    }
  }
  return 1;
}


