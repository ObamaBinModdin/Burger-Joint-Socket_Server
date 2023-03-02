#include "client.h"

int main(int argc, char** argv)
{
	if (argc == 1)
	{
		Client();
	}
	else if (argc == 4)
	{
		std::string ipAddress = argv[1];
		
		int portNumber;
	
		try
		{
			portNumber = std::stoi(argv[2]);
			
			if (portNumber < 0) throw std::invalid_argument("Port number must be positive.");
		}
		catch(...)
		{
			printf("There was an issue with the port number. Make sure it is an integer.\n");
			
			return 1;
		}
		
		int burgerCount;
		
		try
		{
			burgerCount = std::stoi(argv[3]);
			
			if (std::stoi(argv[3]) < 1) throw std::invalid_argument("Burger count must be greater than 0.");
		}
		catch(...)
		{
			printf("There was an issue with the number of burgers argument (third parameter). Make sure it is positive.\n");
			
			return 1;
		}
		
		Client(ipAddress, portNumber, burgerCount);
	}
	else
	{
		printf("Only THREE parameters are accepted. [IP Address], [Port number], [Burgers can be eaten]");
	}
	
	return 0;
}
