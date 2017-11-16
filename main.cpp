#include <iostream>
#include <string>

#include <time.h>
#include <stdlib.h>

#include "AS.h"

using namespace std;

int main() 
{
	srand((int)time(0));  //4th num for neighbor and update ip 
	//create a object
	AS as;

	//stroe as connections
	as.init_asconn("./as_rel_500.txt");
	//read all files in the as directory
	as.init_asinfo("./as500/");

	//handle other ips for json's networks parameter
	as.init_other_ips("./as00000.txt");

	//make sure only less than 2 same ip
	as.less_than_2_same_ip();
	//test set and vector, whether have same ip
	as.handle_repeated_ip();

	//output to json
	as.output_json("jsonoutput_c");

	return 0;
}
