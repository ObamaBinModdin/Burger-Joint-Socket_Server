#include "server.h"

#define c_BUFFER_SIZE 1
#define c_PORT_NUMBER 54321
#define c_MAX_CLIENTS 10

pthread_mutex_t Server::mutex = PTHREAD_MUTEX_INITIALIZER;
std::atomic<int> Server::g_dwBurgerCount(0);
std::queue<int> Server::threadQueue;
std::atomic<int> Server::g_availableBurgers(0);

// Structure to pass data to client handler threads.
struct ThreadData 
{
    int clientFd;
    int threadIndex;
};

// Creates a server for a burger joint.
Server::Server(int dwBurgerCount, int ubyChefCount)
{
	// Sets the value for how many burgers can be made this session.
	g_dwBurgerCount.store(dwBurgerCount);
	
	// Declare server and client address structures.
	struct sockaddr_in serverAddress;
	
	// Handles timeouts.
	fd_set readfds;
	
	// Server ID
	int serverFd;
	
	// Tries to create a socket for the server.
	for(int attempt = 0; attempt < 5; ++attempt)
	{
		// Creates a TCP socket.
		serverFd = socket(AF_INET, SOCK_STREAM, 0);
		if (serverFd < 0)
		{
			if (attempt < 4) printf("Error when creating a TCP socket. Attempt %d. Retrying...\n", attempt + 1);
			else 
			{
				printf("Error when creating a TCP socket. Attempt %d. Exiting...\n", attempt + 1);
				return;
			}
			
			sleep(3);
			continue;
		}
		else break;
	}
	
	// Initializes server address to all 0's.
	bzero((char*) &serverAddress, sizeof(serverAddress));
	
	// Allows address to be reused.
	int optval = 1;
	setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	
	// Sets address family to IPv4.
	serverAddress.sin_family = AF_INET;
	// Accepts connections from any IP address.
	serverAddress.sin_addr.s_addr = INADDR_ANY;
	// Converts port number to network byte order.
	serverAddress.sin_port = htons(c_PORT_NUMBER);
	
	// Attempts to bind socket to server address.
	for (int attempt = 0; attempt < 5; ++attempt)
	{
		// Binds socket to the server address structure.
		if (bind(serverFd, (struct sockaddr*) &serverAddress, sizeof(serverAddress)) < 0)
		{
			if (attempt < 4) printf("Error when binding socket to server address. Attempt %d. Retrying...\n", attempt + 1);
			else 
			{
				printf("Error when binding socket to server address. Attempt %d. Exiting...\n", attempt + 1);
				return;
			}
			
			sleep(3);
			continue;
		}
		else break;
	}
	
	// Tries to set passive variable for listening.
	for (int attempt = 0; attempt < 5; ++attempt)
	{
		// Listens for client connections with a max queue length of 10.
		if (listen(serverFd, c_MAX_CLIENTS) != 0)
		{
			if (attempt < 4) printf("Error when setting server to listen. Attempt %d. Retrying...\n", attempt + 1);
			else 
			{
				printf("Error when setting server to listen. Attempt %d. Exiting...\n", attempt + 1);
				return;
			}
			
			sleep(3);
			continue;
		}
		else break;
	}
	
	// Declares how many threads for client handlers.
	pthread_t ptid[c_MAX_CLIENTS];
	
	// Empty the thread queue.
	threadQueue = std::queue<int>();
	// Pushes all thread locations to queue.
	for (int iterator = 0; iterator < c_MAX_CLIENTS; ++iterator)
	{
		threadQueue.push(iterator);
	}
	
	// Declares how many threads will be chef threads.
	pthread_t chefThreads[ubyChefCount];
	
	// Creates the chef threads and runs them.
	for (int iterator = 0; iterator < ubyChefCount; ++iterator)
	{
		pthread_create(&chefThreads[iterator], NULL, Server::cook, NULL);
	}
	
	// Accepts clients until no burgers are left to make and none left to serve.
	while (g_dwBurgerCount.load() > 0 || g_availableBurgers.load() > 0)
	{
		// Set timeout for accept operation
    		struct timeval timeout{};
    		timeout.tv_sec = 10; // 10 seconds
    		timeout.tv_usec = 0;
	
		struct sockaddr_in clientAddress;
	
		// Sets length of client address structure.
		socklen_t clientLength = sizeof(clientAddress);
		
		FD_ZERO(&readfds);
		FD_SET(serverFd, &readfds);
		
		// Checks for timeout.
		int activity = select(c_MAX_CLIENTS + 1, &readfds, nullptr, nullptr, &timeout);
		
		// Error
		if (activity < 0) 
		{
            		printf("Error waiting for incoming connections.\n");
            		continue;
        	}
		// Timeout happened
        	if (activity == 0) 
        	{
        		printf("Checking supplies and cooked burgers while waiting for customers...\n");
        		
        		pthread_mutex_lock(&mutex);
        		
        		if (g_dwBurgerCount.load() < 1) printf("No supplies left and ");
        		else if (g_dwBurgerCount.load() == 1) printf("Enough supplies to make %d burger and ", g_dwBurgerCount.load());
        		else printf("Enough supplies to make %d burgers and ", g_dwBurgerCount.load());
        		
        		if (g_availableBurgers.load() < 1) printf("no burgers are ready to be served.\n");
        		else if (g_availableBurgers.load() == 1) printf("%d burger is ready to be served.\n", g_availableBurgers.load());
        		else printf("%d burgers are ready to be served.\n", g_availableBurgers.load());
        		
        		pthread_mutex_unlock(&mutex);
        		
            		continue;
        	}
        		
		// Accepts client connections.
		int clientFd = accept(serverFd, (struct sockaddr*) &clientAddress, &clientLength);
		if (clientFd < 0)
		{
			printf("Error connecting to client. Retrying...\n");
			continue;
		}
		
		printf("Client #%d connected.\n", clientFd);
	
		int threadIndex = threadQueue.front();
		threadQueue.pop();
		
		// Stores data to be passed to the cliend handler thread.
		ThreadData data = {clientFd, threadIndex};
		
		// Creates the client handler thread.
		pthread_create(&ptid[threadIndex], NULL, Server::handleClient, (void*)&data);
		
		// Pauses main thread until the thread queue has an available thread.
		while (threadQueue.empty()) sleep(1);
	}
	
	// Burger joint is closing.
	
	printf("Closing...\n");
	
	for (int timer = 5; timer > 0; --timer)
	{
		printf("%d\n", timer);
		sleep(1);
	}
	
	close(serverFd);
}

// Thread function to handle the clients. Only one thread per client.
void* Server::handleClient(void* args)
{
	// Cast the passed in parameters.
	ThreadData* data = (ThreadData*)args;
	int clientFd = data->clientFd;
	
	printf("Client #%d is now being handled by attendant #%ld.\n", clientFd, pthread_self());
	
	// Buffer stores incoming messages.
	char buffer[c_BUFFER_SIZE];
	
	int servedCount = 0;
	
	// Serves customers.
	while (true)
	{
		struct timeval timeout;
   		timeout.tv_sec = 10;
    		timeout.tv_usec = 0;
    		
    		// Sets timeout for recv.
    		if (setsockopt(clientFd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) 
    		{
        		printf("Error: Could not set socket receive timeout\n");
        		break;
    		}
	
		// Only enters if burgers are ready to serve.
		if (g_availableBurgers.load() > 0)
		{
			if (send(clientFd, "1", 1, 0) < 0)
			{
				printf("Error in sending a message to the client.\n");
				break;
			}
			
			if (servedCount == 1) printf("Client #%d has been served a burger by attendant #%ld. They have been served %d burger this sitting.\n", clientFd, pthread_self(), ++servedCount);
			else printf("Client #%d has been served a burger by attendant #%ld. They have been served %d burgers this sitting.\n", clientFd, pthread_self(), ++servedCount);
			
			g_availableBurgers--;
			
			bzero(buffer, c_BUFFER_SIZE);
			
			int messageStatus = recv(clientFd, buffer, c_BUFFER_SIZE, 0);
     			if (messageStatus < 0)
			{
				printf("Error in receiving a message from the client.\n");
				break;
			}
			else if (messageStatus == 0)
			{
				printf("Connection timeout on client #%d.\n", clientFd);
				break;
			}
			
			if (buffer[0] == '0')
			{
				if (servedCount == 1) printf("Client #%d is full. Attendant #%ld was the server. They were served %d burger this sitting.\n", clientFd, pthread_self(), servedCount);
				else printf("Client #%d is full. Attendant #%ld was the server. They were served %d burgers this sitting.\n", clientFd, pthread_self(), servedCount);
				break;
			}
		}
		// Enters only if there are burger supplies but no ready burgers.
		else if (g_dwBurgerCount.load() > 0 && g_availableBurgers.load() == 0)
		{
			if (send(clientFd, "0", 1, 0) < 0)
			{
				printf("Error in sending a message to the client.\n");
				break;
			}
			
			printf("Client #%d was informed by attendant #%ld that no burgers are ready to serve at the moment.\n", clientFd, pthread_self());
			
			sleep(1);
		}
		// Sold out.
		else
		{
			if (send(clientFd, "3", 1, 0) < 0)
			{
				printf("Error in sending a message to the client.\n");
			}
			
			break;
			
			printf("Client #%d was informed by attendant #%ld that no more burgers will be served due to a lack of supplies.\n", clientFd, pthread_self());
		}
	}
	
	bzero(buffer, c_BUFFER_SIZE);
	
	close(clientFd);
	threadQueue.push(data->threadIndex);
	
	pthread_exit(NULL);
	
	return args;
} 

// Thread method to cook the burgers. Each thread is one chef.
void* Server::cook(void* args)
{
	std::mt19937 rng(std::random_device{}());
	
	printf("Chef #%ld is starting up their grill for a productive shift.\n", pthread_self());
	
	int chefBurgerCount = 0;
	
	while (g_dwBurgerCount.load() > 0)
	{
		std::uniform_int_distribution<int> dist(1, 100);
		
		if (dist(rng) > 50)
		{
			printf("Chef #%ld is making a burger and estimates to be finished in 2 seconds.\n", pthread_self());
			sleep(2);
		}
		else 
		{
			printf("Chef #%ld is making a burger and estimates to be finished in 4 seconds.\n", pthread_self());
			sleep(4);
		}
	
		pthread_mutex_lock(&mutex);
		
		if (g_dwBurgerCount.load() <= 0)
		{
			pthread_mutex_unlock(&mutex);
			break;
		}
		
		printf("Chef #%ld is finished preparing their burger.\n", pthread_self());
		
		chefBurgerCount++;
		
		if (chefBurgerCount == 1)
		{
			printf("Chef #%ld has made %d burger this shift.\n", pthread_self(), chefBurgerCount);
		}
		else
		{
			printf("Chef #%ld has made %d burgers this shift.\n", pthread_self(), chefBurgerCount);
		}
		
		--g_dwBurgerCount;
		++g_availableBurgers;
		
		if (g_availableBurgers.load() == 1)
		{
			printf("There is %d burger ready to serve.\n", g_availableBurgers.load());
		}
		else
		{
			printf("There are %d burgers ready to serve.\n", g_availableBurgers.load());
		}
		
		if (g_dwBurgerCount.load() > 1) printf("Enough supplies left for %d more burgers.\n", g_dwBurgerCount.load());
		else if (g_dwBurgerCount.load() == 1) printf("Enough supplies left for %d more burger.\n", g_dwBurgerCount.load());
		else printf("Not enough supplies to make anymore burgers. Sending the chefs home.\n");
		
		pthread_mutex_unlock(&mutex);
	}
	
	if (chefBurgerCount == 1)
	{
		printf("Chef #%ld has clocked out after preparing %d burger.\n", pthread_self(), chefBurgerCount);
	}
	else
	{
		printf("Chef #%ld has clocked out after preparing %d burgers.\n", pthread_self(), chefBurgerCount);
	}
	
	pthread_exit(NULL);
	
	return args;
}
