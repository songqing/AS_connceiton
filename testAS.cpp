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
	ifstream fin("jsonoutput_c", ios::binary);
	if(reader.parse(fin, root))
	{
		server = root["server"];
		int server_size = server.size();
		//store ip and it's count as a map
		for(int i=0;i<server_size;++i)
		{
			Json::Value a_neighbor = server[i]["neighbors"];
			for(int j =0;j<a_neighbor.size();++j)
			{
				string neighbor_ip = a_neighbor[j]["neighbor_ip"].asString();
				string update_source = a_neighbor[j]["update_source"].asString();

				++(ip_count[neighbor_ip]);
				++(ip_count[update_source]);
			}
		}
		//test whether the ip's count equals 2
		for(const auto& m: ip_count)
		{
			if(m.second != 2)
			{
				cout << m.first << ": " <<m.second <<endl;
			}
		}
	}
	else
	{
		cout<<"json parse error!!!"<<endl;
	}
	return 0;
}
