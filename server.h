#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h> 
#include <pthread.h>
#include <math.h>
#include <queue>
#include <climits>
#include <stdexcept>
#include <random>
#include <iostream>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <atomic> 

class Server
{
	public:
		// Server constructor. Passes in burger count and number of chefs. If both or one is missing then default values will be used instead.
		Server(int dwBurgerCount = 25, int ubyChefCount = 2);
	
	private:
		// Mutex to lock threads.
		static pthread_mutex_t mutex;
		// Atomic int to count how many burgers can be cooked.
		static std::atomic<int> g_dwBurgerCount;
		// Saves thread addresses so no thread gets replaced while running.
		static std::queue<int> threadQueue;
		// Atomic int of burgers ready to serve.
		static std::atomic<int> g_availableBurgers;
	
		// Mulithread function that handles a specific client connection.
		static void* handleClient(void* args);
		// Multithread function that cooks burgers.
		static void* cook(void* args);
};
