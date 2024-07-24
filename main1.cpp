



// int main(int argc, char *argv[]) {
//     // Create server thread

    
//     int val;
//     val = atoi(argv[1]);
//     g_Difficulty = 25;


//     pthread_attr_t attr; // Thread attributes
//     struct sched_param param; // Scheduling parameters
//     pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
//     param.sched_priority = 1; // Set priority to 10 (you can choose any value within the priority range)
//     pthread_attr_setschedparam(&attr,&param);
    

//     pthread_t server;
//     pthread_cond_init(&newBlockCreated, NULL);
//     pthread_mutex_init(&blockchain_mutex, NULL);
//     pthread_cond_init(&waitingForServer, NULL);

//     pthread_create(&server, &attr, serverLoop, NULL);


//     // Join server thread
//     pthread_join(server, NULL);

//     return 0;
// }
