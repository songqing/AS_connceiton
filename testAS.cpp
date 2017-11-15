#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>

#include "json/json.h"

using namespace std;

int main()
{
	map<string,int> ip_count;
	Json::Value root;
	Json::Reader reader;

	Json::Value server;
	ifstream fin("jsonoutput", ios::binary);
	if(reader.parse(fin, root))
	{
		server = root["server"];
		int server_size = server.size();
		//test whether have more than 2 same ip
		for(int i=0;i<server_size;++i)
		{
			Json::Value a_neighbor = server[i]["neighbors"];
			for(int j =0;j<a_neighbor.size();++j)
			{
				string neighbor_ip = a_neighbor[j]["neighbor_ip"].asString();
				string update_source = a_neighbor[j]["update_source"].asString();

				if(++(ip_count[neighbor_ip]) > 2)
				{
					cout << neighbor_ip <<endl;
					return 0;
				}
				if(++(ip_count[update_source]) > 2)
				{
					cout << update_source <<endl;
					return 0;
				}
			}
		}
	}
	else
	{
		cout<<"json parse error!!!"<<endl;
	}
	return 0;
}
