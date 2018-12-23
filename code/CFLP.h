#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include <stdlib.h>
#include <time.h>
#include <sstream>
#include <Windows.h>
using namespace std;

class CFLP
{	
	float CFLPbestCost;
	vector<pair<bool, int>> CFLPassignTo;
	vector<bool> CFLPisFacilityOpened;
	


public:
	CFLP();

	void initCFLPdata();

	void printCFLPdata(float time, string path,int No);

	void readData(string path, vector<float>& customerDemand, 
		vector<pair<float, float>>& facilityVec, vector<vector<float>>& serviceCost);

	bool improvedGreedy(const vector<float> customerDemand, const
		vector<pair<float, float>> facilityVec, const vector<vector<float>> serviceCost,int startNo);

	bool greedy(const vector<float> customerDemand, const 
		vector<pair<float, float>> facilityVec, const vector<vector<float>> serviceCost);

	int unusedSpaceInFacilityNo(int facilityNo, vector<pair<float, float>> facilityVec,
		vector<pair<bool, int>> assignTo, vector<float> customerDemand);

	float calculateTotalCost(vector<pair<bool, int>> assignTo, vector<bool> isFacilityOpened,
		vector<vector<float>> serviceCost,vector<pair<float, float>> facilityVec);

	int findNearestUnOpenFacility(int customerNo, vector<bool> isFacilityOpened,
		vector<vector<float>> serviceCost);

	int findNearestAvailableOpenFacility(int customerNo,vector<bool> isFacilityOpened,
		vector<vector<float>> serviceCost, vector<pair<float, float>> facilityVec,
		vector<pair<bool, int>> assignTo, vector<float> customerDemand);

	int getRandomNumOpenFacility(vector<bool> isFacilityOpened);

	bool swapTwoCustomer(vector<pair<bool, int>> &assignTo, vector<bool> isFacilityOpened,
		vector<vector<float>> serviceCost, vector<pair<float, float>> facilityVec,vector<float> customerDemand);



	~CFLP();
};

