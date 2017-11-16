#pragma once
#include <dirent.h>
#include <time.h>
#include <stdlib.h>

#include <iostream>
#include <set>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>

#include "json/json.h"

using std::string;
using std::vector;
using std::set;
using std::multiset;
using std::map;
//store as connection, example:
//as1 as2
//5   6
//45  456
struct AsConn
{
	AsConn(int as1, int asneighbor1):
	   	as(as1), asneighbor(asneighbor1) {}
	int as;
	int asneighbor;
	bool operator<(const AsConn &a) const
	{
		if(as != a.as) return as < a.as;
		else return asneighbor < a.asneighbor;
	}
};
//struct for ip: *.*.*.*
struct AsIps
{
	AsIps() {}
	AsIps(int i11, int i21, int i31, int i41):
		i1(i11), i2(i21), i3(i31), i4(i41) {}

	int i1;
	int i2;
	int i3;
	int i4;

	bool operator==(const AsIps &a) const
	{
		return a.i1 == i1 && a.i2 == i2 && a.i3 == i3;
	}
	bool operator<(const AsIps &a) const
	{
		if(i1!=a.i1) return i1<a.i1;
		else if(i2!=a.i2) return i2 < a.i2;
		else if(i3!=a.i3) return i3 < a.i3;
		else return i4  < a.i4;
	}
};

//store a as' info, include as's num and as' ips
struct AsInfo
{
	AsInfo(int asnom1, const vector<AsIps> &vasIps1): asnom(asnom1)
	{
		vasIps.assign(vasIps1.begin(), vasIps1.end());
	}

	int asnom;
	vector<AsIps> vasIps;
};

class AS
{
	public:

		//make sure only less than 2 same ip
		void less_than_2_same_ip();

		//test set and vector, whether have same ip
		void handle_repeated_ip(); 
	public:
		//init as connections
		void init_asconn(const string &f);
		//read all files in the as directory,init as's info
		void init_asinfo(const string &f);
		//init other as's ips
		void init_other_ips(const string &f);

		//get a as's ips
		void get_as_ips(const int asnum, vector<AsIps> &vo); 
		//update a as's ip (the 4th num)
		void update_as_ip(const int asnum, const AsIps & ai, const int v);
		//generate neighbor ip
		void generate_neighbor_ip(string &s, const AsIps &a, int &i4);
		//insert a as ip when they are connected but have no matching ips
		void insert_as_ip(const int asnum, const AsIps & a, const int v);
		//set basic json info
		void set_basic_json(Json::Value &item, const int asnum);
		//from the two vectors of ip to set json neighbor
		void set_json_neighbor_from_two_as(const vector<AsIps> &updata_source,
			   const vector<AsIps> &vneighbor_ip, Json::Value &neighbor, 
			   const AsConn &a, int &if_realconn);
		//add a new connection from other ips
		void add_extra_conn(Json::Value &neighbor, const int asnum, const int asneighbor);
		//set_json_neighbor_changed_ip
		void set_json_neighbor_ip(const AsConn &a, Json::Value &neighbor);
		//output json
		void output_json(const string &filename);
	private:
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
};

std::string tostring(int i);
//change int and struct to string
std::string tostringip(int i1, int i2, int i3, int i4);
std::string tostringip(const AsIps& a);

//set json neighbor
void set_json_neighbor(Json::Value &neighbor, const std::string & ni,
		const int ebgp, const int ras, const std::string &us);
