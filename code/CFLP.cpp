#include "CFLP.h"



CFLP::CFLP()
{
}

void CFLP::initCFLPdata()
{
	CFLPbestCost = 100000;
	CFLPassignTo.clear();
	CFLPisFacilityOpened.clear();
}

void CFLP::printCFLPdata( float time,string path, int No)
{
	cout << "printCFLPdata:" << No << " CFLPbestCost:"<<CFLPbestCost <<" time:"<<time<<endl;
	ofstream oFile(path,ios::app);
	oFile << "p" << No <<":"<< endl;
	oFile <<"cost:"<< CFLPbestCost << endl;
	oFile << "time: " << time << endl;

	for (int i = 0; i < CFLPisFacilityOpened.size(); i++) {
		if (CFLPisFacilityOpened[i] == true) {
			oFile << 1 << " ";
		}
		else {
			oFile << 0 << " ";
		}
	}
	oFile << endl;
	for (int i = 0; i < CFLPassignTo.size(); i++) {
		oFile << CFLPassignTo[i].second << " ";
	}
	oFile << endl << endl;;
	oFile.close();
}

void CFLP::readData(string path, vector<float>& customerDemand, vector<pair<float, float>>& 
	facilityVec, vector<vector<float>>& serviceCost)
{
	
	float num;
	ifstream iFile;
	iFile.open(path);

	if (!iFile) {
		cout << "open " << path << " error" << endl;
	}

	int facilityNum = 0, customerNum = 0;
	iFile >> facilityNum;
	iFile >> customerNum;

	//读取工厂信息
	for (int i = 0; i < facilityNum; i++) {
		float capTemp, costTemp;
		iFile >> capTemp;
		iFile >> costTemp;
		facilityVec.push_back(pair<float,float>(capTemp,costTemp));
	}
	
	//读取用户需求
	for (int i = 0; i < customerNum; i++) {
		float demandTemp;
		iFile >> demandTemp;
		customerDemand.push_back(demandTemp);
	}

	
	for (int i = 0; i < facilityNum; i++) {
		vector<float> vec;
		for (int i = 0; i < customerNum; i++) {
			float costTemp;
			iFile >> costTemp;
			vec.push_back(costTemp);
		}
		//检查
		if (vec.size() != customerNum) {
			cout << "vec.size() != customerNum" << endl;
		}

		serviceCost.push_back(vec);
	}

	
	
	
	
	
	//检查
	if(facilityVec.size()!=facilityNum){
		cout << "facilityVec.size()!=facilityNum" << endl;
	}
	if (customerDemand.size() != customerNum) {
		cout<<"customerDemand.size() != customerNum" << endl;
	}
	if (serviceCost.size() != facilityNum) {
		cout << "serviceCost.size() != facilityNum" << endl;
	}

	iFile.close();
	cout << path << " read data success" << endl;
}

bool CFLP::improvedGreedy(const vector<float> customerDemand, const 
	vector<pair<float, float>> facilityVec, const vector<vector<float>> serviceCost, int startNo)
{
	int facilityNum = facilityVec.size();
	vector<bool> isFacilityOpened(facilityNum, false);
	//记录工厂开启情况

	int customerNum = customerDemand.size();
	vector<pair<bool, int>> assignTo(customerNum, pair<bool, int>(false, 0));
	//记录每个消费者被分配的情况，当bool为false时表示没有分配

	//每十个里面从第startNo个开始分配服务工厂
	for (int i = 0; i < customerNum / 10; i++) {
		for (int j = startNo; j < 10+startNo; j++) {
			int customerPosTemp = j % 10+10*i;

			//优先选择最近的开放工厂,如果最近的工厂没有开放，则一定概率开启该工厂
			int nearestUnOpenFacility = findNearestUnOpenFacility(customerPosTemp,isFacilityOpened, serviceCost);
			int nearestAvailbleOpenFacility = findNearestAvailableOpenFacility(customerPosTemp, 
				isFacilityOpened, serviceCost, facilityVec, assignTo, customerDemand);

			assignTo[customerPosTemp].first = true;
			//没有工厂可以接纳
			if (nearestAvailbleOpenFacility == -1) {
				assignTo[customerPosTemp].second = nearestUnOpenFacility;
				isFacilityOpened[nearestUnOpenFacility] = true;
			}
			else if (serviceCost[nearestUnOpenFacility][customerPosTemp]  
				<serviceCost[ nearestAvailbleOpenFacility][customerPosTemp]) {
				assignTo[customerPosTemp].second = nearestAvailbleOpenFacility;
			}
			else {
				//2/3概率添加到现有工厂，1/3概率新开工厂
				int ran = rand() % 3;
				if (ran == 0) {
					assignTo[customerPosTemp].second = nearestUnOpenFacility;
					isFacilityOpened[nearestUnOpenFacility] = true;
				}
				else {
					assignTo[customerPosTemp].second = nearestAvailbleOpenFacility;
				}
			}

		}
	}
	//初始解生成完毕
	/*下面进行随机交换，如果超过工厂容量则随机选出一位已分配的顾客出来*/
	int swapFailTimes = 0;
	while (true) {
		//cout << "swap times:" << swapTimes << endl;
		if (swapTwoCustomer(assignTo, isFacilityOpened, serviceCost, facilityVec, customerDemand)) {
			swapFailTimes = 0;
		}
		else {
			swapFailTimes++;
		}
		if (swapFailTimes > 500) {
			break;
		}
	}
	
	return true;
}

bool CFLP::greedy(const vector<float> customerDemand, const
	vector<pair<float, float>> facilityVec, const vector<vector<float>> serviceCost)
{	
	int facilityNum = facilityVec.size();
	vector<bool> isFacilityOpened(facilityNum,false);
	//记录工厂开启情况

	int customerNum = customerDemand.size();
	vector<pair<bool,int>> assignTo(customerNum,pair<bool, int>(false,0));
	//记录每个消费者被分配的情况，当bool为false时表示没有分配

	//计算总需求
	float totalDemand = 0;
	for (float demand : customerDemand) {
		totalDemand += demand;
	}

	//随机找n个工厂使得工厂容量总和大于客户需求
	int totalCap = 0;
	int openFacilityNum = 0;
	clock_t start, end;
	start = time(NULL);
	while (totalCap < totalDemand) {
		int random = rand() % (facilityNum-openFacilityNum);
		//在未开启的工厂里面开启第random个工厂
		for (int i = 0; i < isFacilityOpened.size();i++) {
			if (isFacilityOpened[i]==false&&random != 0) {
				random--;
			}
			else if (isFacilityOpened[i] == false && random == 0) {
				//开启工厂
				isFacilityOpened[i] = true;
				//增加容量
				totalCap += facilityVec[i].first;
				openFacilityNum++;
				break;
			}
		}
		end = time(NULL);
		double dur = end - start;
		//最多尝试2秒
		if (dur > 2) {
			cout << "init solution failed" << endl;
			return false;
		}
	}



	//先生成一个可行的初始解
	for (int i = 0; i < customerNum; i++) {
		//将未分配的客户插入还能接受该客户的工厂
		if (assignTo[i].first == false) {
			//i为顾客ID
			clock_t start, end;
			start = time(NULL);
			int randomSeletedFacility = getRandomNumOpenFacility(isFacilityOpened);
			while (true ){
				if (unusedSpaceInFacilityNo(randomSeletedFacility, facilityVec, assignTo, customerDemand) >= customerDemand[i]) {
					assignTo[i].first = true;
					assignTo[i].second = randomSeletedFacility;
					break;
				}
				randomSeletedFacility = getRandomNumOpenFacility(isFacilityOpened);

				end = time(NULL);
				double dur = end - start;
				//最多尝试2秒
				if (dur>2) {
					cout << "init solution failed" << endl;
					return false;
				}
			}
		}
	}
	
	cout << "initial solution created,total cost:" << 
		calculateTotalCost(assignTo, isFacilityOpened, serviceCost, facilityVec) <<endl;

	/*下面进行随机交换，如果超过工厂容量则随机选出一位已分配的顾客出来*/
	int swapFailTimes=0;
	while (true) {
		//cout << "swap times:" << swapTimes << endl;
		if (swapTwoCustomer(assignTo, isFacilityOpened, serviceCost, facilityVec, customerDemand)) {
			swapFailTimes = 0;
		}
		else {
			swapFailTimes++;
		}
		if (swapFailTimes > 500) {
			break;
		}
		
	}
	cout << "greedy algorithm finished" << endl;
	return true;
}






/*查看第No号工厂剩余空间*/
int  CFLP::unusedSpaceInFacilityNo(int facilityNo,vector<pair<float, float>> facilityVec,
	vector<pair<bool, int>> assignTo, vector<float> customerDemand)
{
	int faCap = facilityVec[facilityNo].first;
	float demandTemp = 0;
	for (int i = 0; i < assignTo.size(); i++) {
		if (assignTo[i].first == true && assignTo[i].second == facilityNo) {
			demandTemp += customerDemand[i];
		}
	}
	if (demandTemp > faCap) {
		return -1;
	}
	return faCap-demandTemp;
}

float CFLP::calculateTotalCost(vector<pair<bool, int>> assignTo, vector<bool> isFacilityOpened, 
	vector<vector<float>> serviceCost, vector<pair<float, float>> facilityVec)
{
	float result=0;
	for (int i = 0; i < facilityVec.size(); i++) {
		if (isFacilityOpened[i] == true) {
			result += facilityVec[i].second;
		}
	}

	for (int i = 0; i < assignTo.size(); i++) {
		if (assignTo[i].first == false) {
			cout << "assignTo[i].first == false in calculateTotalCost" << endl;
			return 0;
		}
		//工厂编号为assignTo[i].second,顾客编号为i
		result += serviceCost[assignTo[i].second][i];
	}

	return result;
}

int CFLP::findNearestUnOpenFacility(int customerNo, vector<bool> isFacilityOpened,
	vector<vector<float>> serviceCost)
{	
	int result = 0;
	int costTemp = 100000;
	int facilityNum = serviceCost.size();
	for (int i = 0; i < facilityNum; i++) {
		if (isFacilityOpened[i]==false&& serviceCost[i][customerNo] < costTemp) {
			result = i;
		}
	}

	return result;
}

/*查找对于customerNo来说最近的开放工厂编号*/
int CFLP::findNearestAvailableOpenFacility(int customerNo, vector<bool> isFacilityOpened,
	vector<vector<float>> serviceCost, vector<pair<float, float>> facilityVec,
	vector<pair<bool, int>> assignTo, vector<float> customerDemand)
{
	int result = -1;
	int facilityNum = serviceCost.size();
	int costTemp = 100000;


	for (int i = 0; i < facilityNum; i++) {
		if (isFacilityOpened[i] == true && serviceCost[i][customerNo] < costTemp&&
			unusedSpaceInFacilityNo(i,facilityVec,assignTo,customerDemand)>=customerDemand[customerNo]) {
			result = i;
		}
	}

	if (result == -1) {
		cout << "findNearestAvailableOpenFacility ERROR" << endl;
	}
	return result;
}

/*随机寻找一个开了的工厂*/
int CFLP::getRandomNumOpenFacility(vector<bool> isFacilityOpened)
{	
	int openNum = 0;
	for (int i = 0; i < isFacilityOpened.size(); i++) {
		if (isFacilityOpened[i] == true) {
			openNum++;
		}
	}

	int r = rand()%openNum;
	int result=-1;

	for (int i = 0; i < isFacilityOpened.size(); i++) {
		if (isFacilityOpened[i] == true) {
			if (r != 0) {
				r--;
			}
			else {
				result = i;
				break;
			}
		}
	}

	if (result == -1) {
		cout << "getRandomNumOpenFacility ERROR" << endl;
	}

	return result;
}


/*交换两个顾客，如果失败则返回false*/
bool CFLP::swapTwoCustomer(vector<pair<bool, int>>& assignTo, vector<bool> isFacilityOpened, 
	vector<vector<float>> serviceCost, vector<pair<float, float>> facilityVec,vector<float> customerDemand)
{	
	
	int facilityNum = facilityVec.size();
	vector<vector<int>> facilityAssignment(facilityNum);
	//记录每个工厂得到的分配顾客
	
	vector<pair<bool, int>> assignToCopy(assignTo);

	int customerNum = assignToCopy.size();
	for (int i = 0; i < customerNum; i++) {
		if (assignToCopy[i].first == false) {
			cout << "swapTwoCustomer error:assignToCopy[i].first == false" << endl;
			return false;
		}
		facilityAssignment[assignToCopy[i].second].push_back(i);
	}
	
	/*保证第一个工厂有人*/
	int randomSeletedFacilityFirst = getRandomNumOpenFacility(isFacilityOpened);
	while (true) {
		if (facilityAssignment[randomSeletedFacilityFirst].size() != 0) {
			break;
		}
		randomSeletedFacilityFirst= getRandomNumOpenFacility(isFacilityOpened);
	}
	
	


	int randomSeletedFacilitySecond = 0;
	while (true) {
		randomSeletedFacilitySecond = getRandomNumOpenFacility(isFacilityOpened);
		//保证两个工厂不相同
		if (randomSeletedFacilityFirst != randomSeletedFacilitySecond) {
			break;
		}
	}
	 

	/*从第一个工厂选出一个顾客*/


	int randomCustomerFirst = facilityAssignment[randomSeletedFacilityFirst]
		[rand() % facilityAssignment[randomSeletedFacilityFirst].size()];
	int randomCustomerSecond;
	
	assignToCopy[randomCustomerFirst].second = 0;
	assignToCopy[randomCustomerFirst].first = false;
	for (auto iter = facilityAssignment[randomSeletedFacilityFirst].begin(); 
		iter != facilityAssignment[randomSeletedFacilityFirst].end();iter++) {
		if (*iter == randomCustomerFirst) {
			facilityAssignment[randomSeletedFacilityFirst].erase(iter);
			break;
		}
	}


	/*往第二个随机选出的工厂塞,如果超过最大容量*/
	if (unusedSpaceInFacilityNo(randomSeletedFacilitySecond, facilityVec, assignToCopy, customerDemand) 
		< customerDemand[randomCustomerFirst]) 
	{	
		int tryTimes = 100;

		while(true){
			//随机选出一个顾客，空出的空间能让后者塞入
			randomCustomerSecond = facilityAssignment[randomSeletedFacilitySecond]
				[rand() % facilityAssignment[randomSeletedFacilitySecond].size()];
			if (unusedSpaceInFacilityNo(randomSeletedFacilitySecond, 
				facilityVec, assignToCopy, customerDemand)+customerDemand[randomCustomerSecond]
				>customerDemand[randomCustomerFirst]) {
				//从原来的工厂去除
				assignToCopy[randomCustomerSecond].first = false;
				assignToCopy[randomCustomerSecond].second = 0;
				/*这个要不要无所谓，下面不会在用到facilityAssignment
				for (auto iter = facilityAssignment[randomSeletedFacilitySecond].begin();
					iter != facilityAssignment[randomSeletedFacilitySecond].end(); iter++) {
					if (*iter == randomCustomerSecond) {
						facilityAssignment[randomSeletedFacilitySecond].erase(iter);
					}
				}*/	
				break;
			}

			/*重新选人*/
			randomCustomerSecond = facilityAssignment[randomSeletedFacilitySecond]
				[rand() % facilityAssignment[randomSeletedFacilitySecond].size()];
			if ((tryTimes--) == 0) {
				//cout << "swapTwoCustomer find place for customer fail" << endl;
				return false;
			}
		}

		//把选出来的顾客随机塞入一个有空间的工厂
		vector<int> availableFacility;
		for (int i = 0; i < isFacilityOpened.size(); i++) {
			if (isFacilityOpened[i] == true && unusedSpaceInFacilityNo(i, facilityVec, assignToCopy, customerDemand)
				> customerDemand[randomCustomerSecond]) {
				availableFacility.push_back(i);
			}
		}
		if (availableFacility.size() == 0) {
			//cout << "no availableFacility in swapTwoCustomer" << endl;
			return false;
		}

		//加大概率添加进最近的工厂
		int ran = rand() % 2;
		int  randomAvailableFacility = 0;
		if (ran == 0) {
			int nearestAvailableFacility = 0;
			int serviceCostTemp = 100000;
			for (int i = 0; i < availableFacility.size(); i++) {
				if (serviceCost[availableFacility[i]][randomCustomerSecond] < serviceCostTemp) {
					serviceCostTemp = serviceCost[availableFacility[i]][randomCustomerSecond];
					nearestAvailableFacility = availableFacility[i];
				}
			}
			assignToCopy[randomCustomerSecond].first = true;
			assignToCopy[randomCustomerSecond].second = nearestAvailableFacility;
		}
		else {
			randomAvailableFacility = availableFacility[rand() % availableFacility.size()];
			assignToCopy[randomCustomerSecond].first = true;
			assignToCopy[randomCustomerSecond].second = randomAvailableFacility;
		}
		

	}

	//塞入第一个顾客
	assignToCopy[randomCustomerFirst].first = true;
	assignToCopy[randomCustomerFirst].second = randomSeletedFacilitySecond;

	int previousCost = calculateTotalCost(assignTo, isFacilityOpened, serviceCost, facilityVec);
	int currentCost = calculateTotalCost(assignToCopy, isFacilityOpened, serviceCost, facilityVec);
	
	/*比原解好*/
	if ( previousCost>currentCost) {
		
		assignTo = assignToCopy;

		if (currentCost < CFLPbestCost) {
			CFLPbestCost = currentCost;
			CFLPassignTo.assign(assignTo.begin(), assignTo.end());
			CFLPisFacilityOpened.assign(isFacilityOpened.begin(), isFacilityOpened.end());
			cout << "swapTwoCustomer get a better result: " << currentCost << endl;
		}
		return true;
		
	}
	else {
		//cout << "swapTwoCustomer don't get a better result" << previousCost << endl;
	}

	return false;
}




CFLP::~CFLP()
{
}
