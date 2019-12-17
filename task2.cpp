#include <iostream> 
#include <pthread.h> 
#include <time.h> 
#include <unistd.h>
#include <sched.h>

#define MAX 50000000 // number of elements in array 
#define MAX_NUM 1000000 //that is the max value of element in array
#define IS_SHOW false //whether or not sorted array printed
#define THREAD_MAX 8 // number of threads 
#define THREAD_WORKING 8 // number of threads we need for work

long long *a=(long long*)malloc(sizeof(long long)*MAX); 
pthread_mutex_t is_job_mut;
int is_job;

struct job //threads will need this to know, what to do
{
	long long max;
	int part;
	int pools;
	struct job * prev;
};

job * job_chain = NULL;

using namespace std; 

void merge(long long low, long long mid, long long high,long long *left_m,long long *right_m,long long*aa) // merge function for merging two parts 
{ 
	long long n1 = mid - low + 1, n2 = high - mid, i, j; 

	for (i = 0; i < n1; i++) 
		left_m[i + low] = aa[i + low]; 
	for (i = 0; i < n2; i++) 
		right_m[i + mid + 1] = aa[i + mid + 1]; 
	long long k = low; 
	i = low;
	j = mid + 1; 
	while (i < n1+low && j < n2+mid + 1) // merge left_m and right_m in ascending order 
	{ 
		if (left_m[i] <= right_m[j]) 
			aa[k++] = left_m[i++]; 
		else
			aa[k++] = right_m[j++]; 
	} 
	while (i < n1+low) // insert remaining values from left_m 
	{ 
		aa[k++] = left_m[i++]; 
	} 
	while (j < n2+mid + 1) // insert remaining values from right_m 
	{ 
		aa[k++] = right_m[j++]; 
	} 
} 

void merge_sort(long long low, long long high,long long *left_m,long long* right_m,long long* aa) // merge sort function 
{ 
	long long mid = low + (high - low) / 2; // calculating mid point of array 
	if (low < high) 
	{ 	
		merge_sort(low, mid,left_m,right_m,aa); // calling first half 
		merge_sort(mid + 1, high,left_m,right_m,aa); // calling second half 
		merge(low, mid, high,left_m,right_m,aa); // merging the two halves 
	} 
} 
job get_job()
{
	job result = *(job_chain);
	job_chain = job_chain->prev;
	if (job_chain == NULL)
	{
		is_job = 0;
	}
	return result;
} 

void* merge_sort(void* arg) // thread function for multi-threading 
{ 
	
	int is_working;
	job job_to_do;
	long long low;
	long long high;
	long long mid; 
	long long * aa;
	long long * left_m;
	long long * right_m;
	while (1)//cicle of life
	{
		is_working = 0;
		pthread_mutex_lock(&is_job_mut);
		if (is_job == 2)//if someone sad to die...
		{
			is_working = 2;
			pthread_mutex_unlock(&is_job_mut);
			break;
		}
        if  (is_job == 1)//if we have something to do
        {
			job_to_do = get_job();//so we getting the structure with all nessesery information
			is_working = 1;
			
		}
		pthread_mutex_unlock(&is_job_mut);
		if (is_working == 1)
		{
			low = job_to_do.part * (job_to_do.max / job_to_do.pools); 
			if (job_to_do.part == job_to_do.pools-1)
				high = job_to_do.max - 1;
			else
				high = (job_to_do.part + 1) * (job_to_do.max / job_to_do.pools) - 1; 
			mid = low + (high - low) / 2;
			aa=(long long*)malloc(sizeof(long long)*(high-low+1));
			for (long long i = 0; i < (high-low+1); i++) 
				aa[i] = a[i+low]; 
			left_m=(long long*)malloc(sizeof(long long)*(high-low+1));
			right_m=(long long*)malloc(sizeof(long long)*(high-low+1)); //there was some preparations of arrays and their borders for future calculations
			if (low < high) 
			{ 
				merge_sort(0, mid-low,left_m,right_m,aa); //we divide the array into 2 parts and then merge them
				merge_sort(mid + 1-low, high-low,left_m,right_m,aa); 
				merge(0, mid-low, high-low,left_m,right_m,aa);  
			} 
			for (long long i = 0; i < (high-low+1); i++) //putting all local calculations back to the initial array
					a[i+low] = aa[i];
		}
		if (is_working == 2) //don't forget to clean after you are done
		{
			free(left_m);
			free(right_m);
			free(aa);
			break;
		}
	}
	
} 

class Pool //pool itself
{
    public:
    int num_threads;
    pthread_t *threads;
		Pool(int num_threads)
        {
			is_job = 0;
			Pool::num_threads = num_threads;
			Pool::threads = (pthread_t*)malloc(sizeof(pthread_t)*num_threads);
			for (int i = 0; i < num_threads; i++) 
				pthread_create(&Pool::threads[i], NULL, merge_sort, 
										(void*)NULL); 
			
        }
        void work(int num_of_parts,long long max)
        {	
			job * tmp = NULL;
			job * tmp2;
			for (int i = 0; i < num_of_parts; i++) 
			{
				tmp2 = (job*)malloc(sizeof(job));
				tmp2->prev = tmp;
				tmp2->max = max;
				tmp2->part = i;
				tmp2->pools = num_of_parts;
				tmp = tmp2;
			}
			job_chain = tmp;
			pthread_mutex_lock(&is_job_mut);
			is_job = 1;
			pthread_mutex_unlock(&is_job_mut);

		}
        void join_pools()
        {	
			int should_break=0;
			while (1)
			{
				pthread_mutex_lock(&is_job_mut);
				if (is_job == 0)
				{
					is_job = 2;
					should_break = 1;
				}
					
				pthread_mutex_unlock(&is_job_mut);
				if (should_break)
					break;
			}
			for (int i = 0; i < Pool::num_threads; i++) 
					pthread_join(Pool::threads[i], NULL); 
			free(Pool::threads);
		}
        
};


int main() 
{ 
	
	for (long long i = 0; i < MAX; i++) // generating random values in array 
		a[i] = rand() % MAX_NUM; 
	clock_t t1, t2; // t1 and t2 for calculating processor time 
 
	Pool pool(THREAD_MAX);
	
	struct timespec start, finish; //and this for the actual time
	double elapsed;
	
	t1 = clock();
	clock_gettime(CLOCK_MONOTONIC, &start);//timers start
	
	pool.work(THREAD_WORKING, MAX);//creating the pool for main part of the job
    pool.join_pools();
    
    clock_gettime(CLOCK_MONOTONIC, &finish);//and timers stop
    t2 = clock();
    elapsed = (finish.tv_sec - start.tv_sec);
	elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
	
	
	int together = 1;//this is the part of merging of all parts, that is the linear tail, so it doesnt count for the experiment
	long long size = MAX / THREAD_WORKING;
	long long i = 0;
	long long *left_m; 
	long long *right_m; 
	left_m=(long long*)malloc(sizeof(long long)*MAX);
	right_m=(long long*)malloc(sizeof(long long)*MAX);
	while (together < THREAD_WORKING)
	{
		
		together = together * 2;
		
		for (i = 0; i < THREAD_WORKING/together; i++)
		{
			merge(i*size*together,(2*i+1)*size*together/2 -1, (i+1)*size*together-1,left_m,right_m,a); 
		}
		
		
		if (THREAD_WORKING/together==0)//with 2^n no need for last merge
		{
			merge(0,size*together/2-1 , MAX-1,left_m,right_m,a); 
		}
		
	}
 

	cout << "Processor time taken: " << (t2 - t1) / 
			(double)CLOCKS_PER_SEC << endl; 
	cout << "Real time taken: " << elapsed << endl; 
	if (IS_SHOW)
	{
		cout << "Sorted array: "; 
		for (long long i = 0; i < MAX; i++) 
			cout << a[i] << " "; 
	}
	return 0; 
}
