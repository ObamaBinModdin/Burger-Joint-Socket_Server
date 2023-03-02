#pragma once

#include <string.h>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <random>
#include <time.h> 

class Client
{
	public:
		// Client constructor with default parameters.
		Client(std::string strIPAddress = "127.0.0.1", unsigned int wPortNumber = 54321, unsigned int wBurgerConsumption = 10);
	
	private:
};
