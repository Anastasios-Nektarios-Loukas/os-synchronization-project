#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "pizza.h"
#include <time.h>
#include <unistd.h>



// Condition,mutex for the customer when he is on call
pthread_cond_t OrderCond; 
pthread_mutex_t OrderMutex;

// Condition,mutex for the order when we wait for a cook to become available
pthread_cond_t CookCond; 
pthread_mutex_t CookMutex;

// Condition,mutex for the pizzas when they are ready and waiting for ovens to be available
pthread_cond_t OvenCond; 
pthread_mutex_t OvenMutex;

// Condition,mutex for the pizzas when they are in the oven waiting to be packed
pthread_cond_t DeliveryCond; 
pthread_mutex_t DeliveryMutex;

//output mutex
pthread_mutex_t OutputMutex;

typedef struct cust {
    int id;
    int randomn;
} cust_info;

void* Order(void* number) {
    struct timespec start, finish;
    double time_ready = 0; // variable to hold the time
    cust_info *info = (cust_info*)number;
    float cold=0;
    int rc;

    // Waiting for someone to answer the call
    rc=pthread_mutex_lock(&OrderMutex);
    if(rc!=0){
    	printf("ERROR: return code from pthread_mutex_lock() is %d.\n",rc);
    	pthread_exit(NULL);
    }
    clock_gettime(CLOCK_REALTIME, &start);
    while (Ntel == 0) {
    	rc=pthread_mutex_lock(&OutputMutex);
        printf("The order with number %d is waiting in line. \n", info->id);
        rc=pthread_mutex_unlock(&OutputMutex);
        
        rc=pthread_cond_wait(&OrderCond, &OrderMutex);
        if(rc!=0){
    	printf("ERROR: return code from pthread_cond_wait() is %d.\n",rc);
    	pthread_exit(NULL);
    	}
    }
    Ntel--;
    rc=pthread_mutex_lock(&OutputMutex);
    printf("The order with number %d is on call. \n", info->id);
    rc=pthread_mutex_unlock(&OutputMutex);
    // Generate a random number of pizzas
    int pizzas = rand_r(&(info->randomn)) % Norderhigh + Norderlow;
    int Nm = 0;
    int Np = 0;
    int Ns = 0;

    // Check the chances for each pizza
    for (int i = 0; i < pizzas; i++) {
        if (rand_r(&(info->randomn)) % 100 <= Pm * 100) {
            Nm++;
        } else if (rand_r(&(info->randomn)) % 100 <= Pp * 100) {
            Np++;
        } else {
            Ns++;
        }
    }

    // Keep the total number
    int total = Nm + Np + Ns;
    rc=pthread_mutex_unlock(&OrderMutex);
    if(rc!=0){
    	printf("ERROR: return code from pthread_mutex_unlock() is %d.\n",rc);
    	pthread_exit(NULL);
    }

    // Sleep while the customer pays
    sleep(rand_r(&(info->randomn)) % Tpaymenthigh + Tpaymentlow);

    rc=pthread_mutex_lock(&OrderMutex);
    if(rc!=0){
    	printf("ERROR: return code from pthread_mutex_lock() is %d.\n",rc);
    	pthread_exit(NULL);
    }

    // Calculate the chance for the order to fail
    double chance_payment_fail = rand_r(&(info->randomn)) % 100;

    if (chance_payment_fail <= Pfail * 100) {
    	rc=pthread_mutex_lock(&OutputMutex);
        printf("The order with number %d Failed \n", info->id);
        rc=pthread_mutex_unlock(&OutputMutex);
        Failed_Orders++;
        Ntel++;
        rc=pthread_cond_signal(&OrderCond);
    	if(rc!=0){
    	printf("ERROR: return code from pthread_cond_signal() is %d.\n",rc);
    	pthread_exit(NULL);
    	}
        rc=pthread_mutex_unlock(&OrderMutex);
        if(rc!=0){
    	printf("ERROR: return code from pthread_mutex_unlock() is %d.\n",rc);
    	pthread_exit(NULL);
    	}
        clock_gettime(CLOCK_REALTIME, &finish);
        pthread_exit(NULL);
    } else {
    	rc=pthread_mutex_lock(&OutputMutex);
        printf("The order with number %d Passed \n", info->id);
        rc=pthread_mutex_unlock(&OutputMutex);

   // If it passed add the price to the total profit and increase the pizza numbers
        Total_Selling_Profit += Nm * Cm + Np * Cp + Ns * Cs;
        Number_Margarita += Nm;
        Number_Peperoni += Np;
        Number_Special += Ns;
        Passed_Orders++;
    }
    Ntel++;
    
    rc=pthread_cond_signal(&OrderCond);
    if(rc!=0){
    	printf("ERROR: return code from pthread_cond_signal() is %d.\n",rc);
    	pthread_exit(NULL);
    }
    
    clock_gettime(CLOCK_REALTIME, &finish);
    time_ready+=(finish.tv_sec - start.tv_sec)/60;
    rc=pthread_mutex_unlock(&OrderMutex);
    if(rc!=0){
    	printf("ERROR: return code from pthread_mutex_unlock() is %d.\n",rc);
    	pthread_exit(NULL);
    }
    
    // Wait for a cook to be available
    rc=pthread_mutex_lock(&CookMutex);
    if(rc!=0){
    	printf("ERROR: return code from pthread_mutex_lock() is %d.\n",rc);
    	pthread_exit(NULL);
    }
    clock_gettime(CLOCK_REALTIME, &start);
  
    
    while (Ncook == 0) {
    	rc=pthread_mutex_lock(&OutputMutex);
        printf("The order with number %d is waiting for a cook. \n", info->id);
        rc=pthread_mutex_unlock(&OutputMutex);
        
        rc=pthread_cond_wait(&CookCond, &CookMutex);
        if(rc!=0){
    	printf("ERROR: return code from pthread_cond_wait() is %d.\n",rc);
    	pthread_exit(NULL);
    	}
    }
    //cook starts preparing the order so Ncook decreases
    Ncook--;
    rc=pthread_mutex_lock(&OutputMutex);
    printf("The order with number %d being prepared. \n", info->id);
    rc=pthread_mutex_unlock(&OutputMutex);
    clock_gettime(CLOCK_REALTIME, &finish);
    //add the time that the order was waiting for a cook and the prep time
    time_ready += (finish.tv_sec - start.tv_sec)/60 + total * Tprep;
    
    rc=pthread_mutex_unlock(&CookMutex);
    if(rc!=0){
    	printf("ERROR: return code from pthread_mutex_unlock() is %d.\n",rc);
    	pthread_exit(NULL);
    }
    
    //sleep while the cook prepares the order
    sleep(total * Tprep);
    
    //wait for ovens to become available
    
    rc=pthread_mutex_lock(&OvenMutex);
    if(rc!=0){
    	printf("ERROR: return code from pthread_mutex_lock() is %d.\n",rc);
    	pthread_exit(NULL);
    }
    
    clock_gettime(CLOCK_REALTIME, &start);
    while (Noven < total) {
    	rc=pthread_mutex_lock(&OutputMutex);
        printf("The order with number %d is waiting for ovens. \n", info->id);
        rc=pthread_mutex_unlock(&OutputMutex);
        rc=pthread_cond_wait(&OvenCond, &OvenMutex);
        if(rc!=0){
    	printf("ERROR: return code from pthread_cond_wait() is %d.\n",rc);
    	pthread_exit(NULL);
    	}
    }
    //decrease the amount of ovens available because order is inside of them
    Noven -= total;
    rc=pthread_mutex_lock(&OutputMutex);
    printf("The order with number %d is baking. \n", info->id);
    rc=pthread_mutex_unlock(&OutputMutex);
    
    clock_gettime(CLOCK_REALTIME, &finish);
    time_ready += (finish.tv_sec - start.tv_sec)/60;
    
    rc=pthread_mutex_unlock(&OvenMutex);
    if(rc!=0){
    	printf("ERROR: return code from pthread_mutex_unlock() is %d.\n",rc);
    	pthread_exit(NULL);
    }
    
    //sleep while order is inside of the ovens in order to bake
    sleep(Tbake);
    
    rc=pthread_mutex_lock(&OvenMutex);
    if(rc!=0){
    	printf("ERROR: return code from pthread_mutex_lock() is %d.\n",rc);
    	pthread_exit(NULL);
    }

    rc=pthread_mutex_lock(&CookMutex);
    if(rc!=0){
    	printf("ERROR: return code from pthread_mutex_lock() is %d.\n",rc);
    	pthread_exit(NULL);
    }
    
    //add the time that the order was baking
    time_ready += Tbake ;
    //the order is ready so cook becomes available again
    Ncook++;
    
    //unlock cook mutex and signal the condition to inform about the available cook
    
    rc=pthread_cond_signal(&CookCond);
    if(rc!=0){
    	printf("ERROR: return code from pthread_cond_signal() is %d.\n",rc);
    	pthread_exit(NULL);
    }
    
    rc=pthread_mutex_unlock(&CookMutex);
    if(rc!=0){
    	printf("ERROR: return code from pthread_mutex_unlock() is %d.\n",rc);
    	pthread_exit(NULL);
    }

    rc=pthread_mutex_unlock(&OvenMutex);
    if(rc!=0){
    	printf("ERROR: return code from pthread_mutex_unlock() is %d.\n",rc);
    	pthread_exit(NULL);
    }

	//wait for a deliverer to become available
	rc=pthread_mutex_lock(&DeliveryMutex);
	if(rc!=0){
    	printf("ERROR: return code from pthread_mutex_lock() is %d.\n",rc);
    	pthread_exit(NULL);
    }
    
    clock_gettime(CLOCK_REALTIME, &start);
    while (Ndeliverer == 0) {
    	rc=pthread_mutex_lock(&OutputMutex);
        printf("The order with number %d is waiting for a deliverer. \n", info->id);
        rc=pthread_mutex_unlock(&OutputMutex);
        rc=pthread_cond_wait(&DeliveryCond, &DeliveryMutex);
        if(rc!=0){
    	printf("ERROR: return code from pthread_cond_wait() is %d.\n",rc);
    	pthread_exit(NULL);
    	}
    }
    clock_gettime(CLOCK_REALTIME, &finish);
    Ndeliverer--;
    rc=pthread_mutex_lock(&OutputMutex);
    printf("The order with number %d is being packed and coming to you soon. \n", info->id);
    rc=pthread_mutex_unlock(&OutputMutex);
    //add the time of waiting and pack time
    time_ready += (Tpack * total ) + (finish.tv_sec - start.tv_sec)/60 ;
    //the above time that we added is also the time the pizzas were cold
    cold+=(Tpack * total ) + (finish.tv_sec - start.tv_sec)/60 ;
    
    rc=pthread_mutex_unlock(&DeliveryMutex);
    if(rc!=0){
    	printf("ERROR: return code from pthread_mutex_unlock() is %d.\n",rc);
    	pthread_exit(NULL);
    }
    
    //sleep while orders are being packed
    sleep(Tpack * total);
    
    
    rc=pthread_mutex_lock(&DeliveryMutex);
    if(rc!=0){
    	printf("ERROR: return code from pthread_mutex_lock() is %d.\n",rc);
    	pthread_exit(NULL);
    }
    rc=pthread_mutex_lock(&OvenMutex);
    if(rc!=0){
    	printf("ERROR: return code from pthread_mutex_lock() is %d.\n",rc);
    	pthread_exit(NULL);
    }
    
    rc=pthread_mutex_lock(&OutputMutex);
    printf("The order with number %d was ready in %f minutes. \n", info->id, time_ready);
    rc=pthread_mutex_unlock(&OutputMutex);
    
    //we are done with the ovens so increase them and signal the condition
    Noven += total;
    
    rc=pthread_cond_signal(&OvenCond);
    if(rc!=0){
    	printf("ERROR: return code from pthread_cond_signal() is %d.\n",rc);
    	pthread_exit(NULL);
    }
    
    //unlock oven mutex and signal the oven condition
    rc=pthread_mutex_unlock(&OvenMutex);
    if(rc!=0){
    	printf("ERROR: return code from pthread_mutex_unlock() is %d.\n",rc);
    	pthread_exit(NULL);
    }
    
 
    int transport = rand_r(&(info->randomn)) %Tdelhigh + Tdellow;
    
    rc=pthread_mutex_unlock(&DeliveryMutex);
    if(rc!=0){
    	printf("ERROR: return code from pthread_mutex_unlock() is %d.\n",rc);
    	pthread_exit(NULL);
    }
    
    //sleep while transport
    sleep(transport);
    
    rc=pthread_mutex_lock(&DeliveryMutex);
    if(rc!=0){
    	printf("ERROR: return code from pthread_mutex_lock() is %d.\n",rc);
    	pthread_exit(NULL);
    }
    
    
    //add the transport to the general time of the order but also the cold time
    time_ready += transport ;
    cold+=transport;
    
    //increase the total time by the time the order was ready and transported
    avg_srv+=time_ready;
    if(time_ready>max_srv){
        max_srv=time_ready;
    }
    if(cold>max_cold){
        max_cold=cold;
    }
    avg_cold+=cold;
    
    rc=pthread_mutex_lock(&OutputMutex);
    printf("The order with number %d was delivered in %f minutes. \n", info->id, time_ready);
    rc=pthread_mutex_unlock(&OutputMutex);
    
    rc=pthread_mutex_unlock(&DeliveryMutex);
    if(rc!=0){
    	printf("ERROR: return code from pthread_mutex_unlock() is %d.\n",rc);
    	pthread_exit(NULL);
    }
    
    //sleep while deliverer returns
    sleep(transport);
    
    rc=pthread_mutex_lock(&DeliveryMutex);
    if(rc!=0){
    	printf("ERROR: return code from pthread_mutex_lock() is %d.\n",rc);
    	pthread_exit(NULL);
    }
    
    //increase the deliverers and signal the condition
    Ndeliverer++;
    rc=pthread_cond_signal(&DeliveryCond);
    if(rc!=0){
    	printf("ERROR: return code from pthread_cond_signal() is %d.\n",rc);
    	pthread_exit(NULL);
    }
    
    rc=pthread_mutex_unlock(&DeliveryMutex);
    if(rc!=0){
    	printf("ERROR: return code from pthread_mutex_unlock() is %d.\n",rc);
    	pthread_exit(NULL);
    }
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("ERROR: the program should only take 2 arguments: the number of customers and a random seed\n");
        return -1;
    }

	//store the arguments in variables
	
    int maxnumofthreads = atoi(argv[1]);
    int seed = atoi(argv[2]);
    
    //check if both numbers are wrong
    
    if(seed<0 && maxnumofthreads<0){
    	printf("ERROR: the number of customers and seed should be a positive numbers. Current numbers given: %d %d.\n", maxnumofthreads , seed);
        return -1;
    }
    
    //check if one of the numbers is wrong
    
    if (maxnumofthreads < 0) {
        printf("ERROR: the number of customers should be a positive number. Current number given: %d.\n", maxnumofthreads);
        return -1;
    }
    
    if(seed<0){
    	printf("ERROR: seed should be a positive number. Current number given: %d.\n", seed);
        return -1;
    }

	//store the number of threads in Ncust after we checked it was right
	
    int Ncust = maxnumofthreads;

    pthread_t *Customer;
    Customer = malloc(Ncust * sizeof(pthread_t));
    
    //check for memory
    
    if (Customer == NULL) {
        printf("NOT ENOUGH MEMORY\n");
        return -1;
    }

	//create an array of cust_info size Ncust to pass to the thread
	
    cust_info Customers_Info[Ncust];
    int rc, i;

	//initialize the mutexes and conditions we use
	
    rc=pthread_mutex_init(&OrderMutex, NULL);
    if(rc!=0){
    	printf("ERROR: return code from pthread_mutex_init() is %d.\n",rc);
    	exit(-1);
    }
    rc=pthread_cond_init(&OrderCond, NULL);
    if(rc!=0){
    	printf("ERROR: return code from pthread_cond_init() is %d.\n",rc);
    	exit(-1);
    }
    
    
    
    rc=pthread_cond_init(&CookCond, NULL);
    if(rc!=0){
    	printf("ERROR: return code from pthread_cond_init() is %d.\n",rc);
    	exit(-1);
    }
    rc=pthread_mutex_init(&CookMutex, NULL);
    if(rc!=0){
    	printf("ERROR: return code from pthread_mutex_init() is %d.\n",rc);
    	exit(-1);
    }
    
    
    rc=pthread_cond_init(&OvenCond, NULL);
    if(rc!=0){
    	printf("ERROR: return code from pthread_cond_init() is %d.\n",rc);
    	exit(-1);
    }
    rc=pthread_mutex_init(&OvenMutex, NULL);
    if(rc!=0){
    	printf("ERROR: return code from pthread_mutex_init() is %d.\n",rc);
    	exit(-1);
    }
    
    
    rc=pthread_cond_init(&DeliveryCond, NULL);
    if(rc!=0){
    	printf("ERROR: return code from pthread_cond_init() is %d.\n",rc);
    	exit(-1);
    }
    rc=pthread_mutex_init(&DeliveryMutex, NULL);
    if(rc!=0){
    	printf("ERROR: return code from pthread_mutex_init() is %d.\n",rc);
    	exit(-1);
    }
    
    
    rc=pthread_mutex_init(&OutputMutex, NULL);
    if(rc!=0){
    	printf("ERROR: return code from pthread_mutex_init() is %d.\n",rc);
    	exit(-1);
    }

	//Create the threads
	srand(seed);
    for (i = 0; i < Ncust; i++) {
        sleep(rand() % Torderhigh + Torderlow);
        printf("Taking Order %d\n", i + 1);
        Customers_Info[i].id = i + 1;
        Customers_Info[i].randomn = seed + i + 1;
        rc = pthread_create(&Customer[i], NULL, &Order, &Customers_Info[i]);
        if (rc != 0) {
            printf("Failed to create thread %d\n", i + 1);
            exit(-1);
        }
    }
    for (i = 0; i < Ncust; i++) {
        rc = pthread_join(Customer[i], NULL);
        if (rc != 0) {
            printf("Failed to join thread\n");
            exit(-1);
        }
    }
    
    
    //Printing final results
    
    printf("Number of orders that passed: %d.\n", Passed_Orders);
    printf("Number of orders that failed: %d.\n", Failed_Orders);
    printf("Total profit: %d.\n", Total_Selling_Profit);
    printf("Number of Margaritas sold: %d.\n", Number_Margarita);
    printf("Number of Peperonis sold: %d.\n", Number_Peperoni);
    printf("Number of Specials sold: %d.\n", Number_Special);
    printf("Max length of service: %f minutes.\n", max_srv);
    printf("Average length of service: %f minutes.\n", avg_srv/(float)Passed_Orders);
    printf("Max Average time of a pizza getting cold: %f minutes.\n", max_cold);
    printf("Average time of a pizza getting cold: %f minutes.\n", avg_cold/(float)Passed_Orders);
    
    //Destroy mutexes and conditions and free memory
    
    rc=pthread_cond_destroy(&OrderCond);
    if(rc!=0){
    	printf("ERROR: return code from pthread_cond_destroy() is %d.\n",rc);
    	exit(-1);
    }
    rc=pthread_mutex_destroy(&OrderMutex);
    if(rc!=0){
    	printf("ERROR: return code from pthread_mutex_destroy() is %d.\n",rc);
    	exit(-1);
    }
    
    
    
    rc=pthread_cond_destroy(&CookCond);
    if(rc!=0){
    	printf("ERROR: return code from pthread_cond_destroy() is %d.\n",rc);
    	exit(-1);
    }
    rc=pthread_mutex_destroy(&CookMutex);
    if(rc!=0){
    	printf("ERROR: return code from pthread_mutex_destroy() is %d.\n",rc);
    	exit(-1);
    }
    
    
    
    rc=pthread_cond_destroy(&OvenCond);
    if(rc!=0){
    	printf("ERROR: return code from pthread_cond_destroy() is %d.\n",rc);
    	exit(-1);
    }
    rc=pthread_mutex_destroy(&OvenMutex);
    if(rc!=0){
    	printf("ERROR: return code from pthread_mutex_destroy() is %d.\n",rc);
    	exit(-1);
    }
    
    
    
    rc=pthread_cond_destroy(&DeliveryCond);
    if(rc!=0){
    	printf("ERROR: return code from pthread_cond_destroy() is %d.\n",rc);
    	exit(-1);
    }
    rc=pthread_mutex_destroy(&DeliveryMutex);
    if(rc!=0){
    	printf("ERROR: return code from pthread_mutex_destroy() is %d.\n",rc);
    	exit(-1);
    }
    
    
    
    rc=pthread_mutex_destroy(&OutputMutex);
    if(rc!=0){
    	printf("ERROR: return code from pthread_mutex_destroy() is %d.\n",rc);
    	exit(-1);
    }
    
    free(Customer);
    return 0;
}
