# real-time-ridesharing
Real-time messaging for ridesharing in C

-	A toy ridesharing service provider connects riders and drivers through a real-time messaging system that employs a producer – consumer communication scheme.

-	The program starts by creating two producers and two consumers of rider requests as pthreads, which coordinate using POSIX semaphores.

-	Producer threads will produce and publish rider requests to the broker until reaching the limit of the production, then exit.

-	Consumer threads will consume all requests from the broker before exiting.

-	Main thread then waits for the consumer threads to complete consuming the last request before exiting.
