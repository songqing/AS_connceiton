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
		return a.i1 == this->i1 && a.i2 == this->i2 &&
		   	a.i3 == this->i3;
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
	AsInfo(int asnom1, const std::vector<AsIps> &vasIps1): asnom(asnom1)
	{
		vasIps.assign(vasIps1.begin(), vasIps1.end());
	}

	int asnom;
	std::vector<AsIps> vasIps;
};


//change int and struct to string
std::string tostringip(int i1, int i2, int i3, int i4);
std::string tostringip(const AsIps& a);
//stroe as connections
void store_asconn(const std::string& filename, 
		std::set<AsConn> &sasconn);
//read all files in the as directory
//init as's info
void init_asinfo(const std::string &filename,
		std::vector<AsInfo> &vAsInfo);
//init other as's ips
void init_other_ips(const std::string &filename,
		std::set<AsIps> &sotherips);

//get a as's ips
void get_as_ips(int asnum, std::vector<AsIps> & vo, 
		const std::vector<AsInfo> & vi);
//generate neighbor ip
void generate_neighbor_ip(std::string &s, const std::set<std::string> &sa,
	   const AsIps &a, int &i4);
//output json
void output_json(const std::string &filename,
		const std::set<AsConn> &sasconn,
		std::vector<AsInfo> &vAsInfo,
		std::set<AsIps> &sotherips,
		std::set<std::string> &susedips,
		std::set<int> & susedasnum);
