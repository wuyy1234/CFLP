#include "CFLP.h"



float bestCost = 0;

int main() {
	CFLP cflp;
	
	
	
	


	for (int i = 1; i <= 71; i++) {
		
		
		stringstream ss;
		string str;
		ss << i;
		ss >> str;
		string path="./Instances/p"+str;

		vector<float> customerDemand;
		vector<pair<float, float>> facilityVec;
		vector<vector<float>> serviceCost;
		cflp.readData(path,customerDemand,facilityVec,serviceCost);


		float totalTime=0;
		int successtimes = 0;
		cflp.initCFLPdata();
		for (int j = 0; j < 10; j++) {
			cout << "p:"<<i<< " greedy No." << j << " starts:" << endl;
			clock_t t1, t2;
			t1 = clock();
			if (cflp.greedy(customerDemand, facilityVec, serviceCost)) {
				t2 = clock();
				totalTime += (t2 - t1);
				successtimes++;
			}
		}
		cflp.printCFLPdata((float)(totalTime/1000 )/ successtimes, "result1", i);

		totalTime = 0;
		cflp.initCFLPdata();
		for (int j = 0; j < 10; j++) {
			cout << "p:" << i << " improvedGreedy No." << j << " starts:" << endl;
			clock_t t1, t2;
			t1 = clock();
			cflp.improvedGreedy(customerDemand, facilityVec, serviceCost, j);
			t2 = clock();
			totalTime += (t2 - t1);
		}
		cflp.printCFLPdata((float)(totalTime / 1000) / 10, "result2", i);
	}

	

	cout << "all finish" << endl;
	
	int t;
	cin >> t;
}



