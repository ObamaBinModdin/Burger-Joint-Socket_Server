#include "client.h"

#define c_BUFFER_SIZE 1

Client::Client(std::string strIPAddress, unsigned int wPortNumber, unsigned int wBurgerConsumption)
{
	int serverFd;
	
	struct sockaddr_in serverAddress;
	
	// Creates buffer.
	char buffer[c_BUFFER_SIZE];
	
	// Fills buffer with zeros.
	bzero(buffer, c_BUFFER_SIZE);
	
	for (int attempt = 0; attempt < 5; ++attempt)
     	{
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
     	
     	bzero((char *) &serverAddress, sizeof(serverAddress));
     	
     	// Sets address family to IPv4.
     	serverAddress.sin_family = AF_INET;
     	// Accepts connections from any IP address.
     	serverAddress.sin_addr.s_addr = inet_addr(strIPAddress.c_str());
     	// Converts port number to network byte order.
     	serverAddress.sin_port = htons(wPortNumber);
     	
     	// Attempts to connect to server.
     	for (int attempt = 0; attempt < 5; ++attempt)
     	{
     		if (connect(serverFd, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0)
     		{
			if (attempt < 4) printf("Error when connecting to server. Attempt %d. Retrying...\n", attempt + 1);
			else 
			{
				printf("Error when connecting to server. Attempt %d. Exiting...\n", attempt + 1);
				return;
			}
			sleep(12);
			continue;
		}
		else break;
     	}
     	
     	printf("Successfully connected to Ground Beef.\n");
     	
     	printf("I am hungry for %d burgers today.\n", wBurgerConsumption);
     	
     	printf("I ask the attendant for a burger.\n");
     	
     	// Dine.
     	while (true)
     	{
     		struct timeval timeout;
   		timeout.tv_sec = 10;
    		timeout.tv_usec = 0;
    		
    		if (setsockopt(serverFd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) 
    		{
        		printf("Error: Could not set socket receive timeout\n");
        		break;
    		}
     	
     		bzero(buffer, c_BUFFER_SIZE);
     		
     		
		int messageStatus = recv(serverFd, buffer, c_BUFFER_SIZE, 0);
     		if (messageStatus < 0)
		{
			printf("Error in receiving a message from the server.\n");
			break;
		}
		else if (messageStatus == 0)
		{
			printf("Server connection timeout.\n");
			break;
		}
		
		// Recieved burger.
		if (buffer[0] == '1')
		{
			printf("I recieved a burger.\n");
		
			--wBurgerConsumption;
			
			std::mt19937 rng(std::random_device{}());
					
			std::uniform_int_distribution<int> dist(1, 3);
					
			int randomInt = dist(rng);
					
			if (randomInt == 1) 
			{
				sleep(1);
				printf("I ate a burger in 1 second.\n");
			}
			else if (randomInt == 2)
			{ 
				sleep(3);
				printf("I ate a burger in 3 seconds.\n");
			}
			else 
			{
				sleep(5);
				printf("I ate a burger in 5 seconds.\n");
			}
			
			if (wBurgerConsumption > 0)
			{
				if (send(serverFd, "1", 1, 0) < 0)
				{
					printf("Error in sending a message to the server.\n");
					break;
				}
				
				printf("I asked for another burger.\n");
			}
			else
			{
				if (send(serverFd, "0", 1, 0) < 0)
				{
					printf("Error in sending a message to the server.\n");
				}
				
				printf("I am full.\n");
				
				break;
			}
		}
		// Told to wait.
		else if (buffer[0] == '0')
		{
			printf("I am told it will take a little until I get my burger.\n");
			
			sleep(1);
		}
		// No burgers left.
		else
		{	
			printf("No more burgers are available.\n");
			break;
		}
     	}
     	
     	close(serverFd);
}
