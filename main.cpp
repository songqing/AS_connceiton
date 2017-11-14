#include <iostream>
#include <set>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>

#include <time.h>
#include <stdlib.h>

#include "AS.h"

using namespace std;

//as connection, two side, if have 5 -> 6, then should have 6 -> 5
set<AsConn> sasconn;
//store as' info(as's num and ips) 
vector<AsInfo> vAsInfo; 
//still have not used ips, used for json's neighbor and update ip
set<AsIps> sotherips;
//have used ip for neighbor and update ip in json
set<string> susedips;
//used as's num in json
set<int> susedasnum;

int main() 
{
	srand((int)time(0));  //4th num for neighbor and update ip 

	//stroe as connections
	store_asconn("./as_rel_500.txt", sasconn);
	//read all files in the as directory
	init_asinfo("./as500/", vAsInfo);

	//handle other ips for json's networks parameter
	init_other_ips("./as00000.txt", sotherips);
	
	//output to json
	output_json("jsonoutput", sasconn, vAsInfo, sotherips,
			susedips, susedasnum);

	return 0;
}
