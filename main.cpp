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
multiset<string> susedips;
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

	//test set and vector, whether have same ip
	set<AsIps> smulti_ip;
	for(const auto& a: vAsInfo)
	{
		for(const auto& ip:a.vasIps)
		{
			if(sotherips.find(ip) != sotherips.end())
			{
				smulti_ip.insert(ip);
				//cout<<tostringip(ip)<<endl;
			}
		}
	}
	for(const auto& s: smulti_ip)
	{
		sotherips.erase(s);
	}
	
	//output to json
	output_json("jsonoutput", sasconn, vAsInfo, sotherips,
			susedips, susedasnum);

	return 0;
}
