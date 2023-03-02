#include "server.h"
#include <iostream>

int main(int argc, char** argv)
{
	if (argc == 1)
	{
		printf("Welcome to Ground Beef. The only down to earth burgers.\n");
	
		Server();
	}
	else if (argc == 3)
	{
		int dwBurgerCount;
	
		try
		{
			dwBurgerCount = std::stoi(argv[1]);
			
			if (dwBurgerCount < 0) throw std::invalid_argument("Burger count must be greater than 0.");
		}
		catch(...)
		{
			printf("There was an issue with the total burgers argument (first parameter). Make sure it is a positive integer.\n");
			
			return 1;
		}
		
		int ubyChefCount;
		
		try
		{
			if (std::stoi(argv[2]) > USHRT_MAX) throw std::invalid_argument("Chef count is greater than " + USHRT_MAX + '.');
		
			ubyChefCount = std::stoi(argv[2]);
			
			if (std::stoi(argv[2]) < 1) throw std::invalid_argument("Chef count must be greater than 0.");
		}
		catch(...)
		{
			printf("There was an issue with the number of chefs argument (second parameter). Make sure it is positive and less than %d.\n", USHRT_MAX);
			
			return 1;
		}
		
		printf("Welcome to Ground Beef. The only down to earth burgers.\n");
		
		Server(dwBurgerCount, ubyChefCount);
	}
	else
	{
		printf("Only two arguments of positive integers are accepted.\n");
			
		return 1;
	}
	
	return 0;
}
