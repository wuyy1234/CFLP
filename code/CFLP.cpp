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

	//��ȡ������Ϣ
	for (int i = 0; i < facilityNum; i++) {
		float capTemp, costTemp;
		iFile >> capTemp;
		iFile >> costTemp;
		facilityVec.push_back(pair<float,float>(capTemp,costTemp));
	}
	
	//��ȡ�û�����
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
		//���
		if (vec.size() != customerNum) {
			cout << "vec.size() != customerNum" << endl;
		}

		serviceCost.push_back(vec);
	}

	
	
	
	
	
	//���
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
	//��¼�����������

	int customerNum = customerDemand.size();
	vector<pair<bool, int>> assignTo(customerNum, pair<bool, int>(false, 0));
	//��¼ÿ�������߱�������������boolΪfalseʱ��ʾû�з���

	//ÿʮ������ӵ�startNo����ʼ������񹤳�
	for (int i = 0; i < customerNum / 10; i++) {
		for (int j = startNo; j < 10+startNo; j++) {
			int customerPosTemp = j % 10+10*i;

			//����ѡ������Ŀ��Ź���,�������Ĺ���û�п��ţ���һ�����ʿ����ù���
			int nearestUnOpenFacility = findNearestUnOpenFacility(customerPosTemp,isFacilityOpened, serviceCost);
			int nearestAvailbleOpenFacility = findNearestAvailableOpenFacility(customerPosTemp, 
				isFacilityOpened, serviceCost, facilityVec, assignTo, customerDemand);

			assignTo[customerPosTemp].first = true;
			//û�й������Խ���
			if (nearestAvailbleOpenFacility == -1) {
				assignTo[customerPosTemp].second = nearestUnOpenFacility;
				isFacilityOpened[nearestUnOpenFacility] = true;
			}
			else if (serviceCost[nearestUnOpenFacility][customerPosTemp]  
				<serviceCost[ nearestAvailbleOpenFacility][customerPosTemp]) {
				assignTo[customerPosTemp].second = nearestAvailbleOpenFacility;
			}
			else {
				//2/3������ӵ����й�����1/3�����¿�����
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
	//��ʼ���������
	/*������������������������������������ѡ��һλ�ѷ���Ĺ˿ͳ���*/
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
	//��¼�����������

	int customerNum = customerDemand.size();
	vector<pair<bool,int>> assignTo(customerNum,pair<bool, int>(false,0));
	//��¼ÿ�������߱�������������boolΪfalseʱ��ʾû�з���

	//����������
	float totalDemand = 0;
	for (float demand : customerDemand) {
		totalDemand += demand;
	}

	//�����n������ʹ�ù��������ܺʹ��ڿͻ�����
	int totalCap = 0;
	int openFacilityNum = 0;
	clock_t start, end;
	start = time(NULL);
	while (totalCap < totalDemand) {
		int random = rand() % (facilityNum-openFacilityNum);
		//��δ�����Ĺ������濪����random������
		for (int i = 0; i < isFacilityOpened.size();i++) {
			if (isFacilityOpened[i]==false&&random != 0) {
				random--;
			}
			else if (isFacilityOpened[i] == false && random == 0) {
				//��������
				isFacilityOpened[i] = true;
				//��������
				totalCap += facilityVec[i].first;
				openFacilityNum++;
				break;
			}
		}
		end = time(NULL);
		double dur = end - start;
		//��ೢ��2��
		if (dur > 2) {
			cout << "init solution failed" << endl;
			return false;
		}
	}



	//������һ�����еĳ�ʼ��
	for (int i = 0; i < customerNum; i++) {
		//��δ����Ŀͻ����뻹�ܽ��ܸÿͻ��Ĺ���
		if (assignTo[i].first == false) {
			//iΪ�˿�ID
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
				//��ೢ��2��
				if (dur>2) {
					cout << "init solution failed" << endl;
					return false;
				}
			}
		}
	}
	
	cout << "initial solution created,total cost:" << 
		calculateTotalCost(assignTo, isFacilityOpened, serviceCost, facilityVec) <<endl;

	/*������������������������������������ѡ��һλ�ѷ���Ĺ˿ͳ���*/
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






/*�鿴��No�Ź���ʣ��ռ�*/
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
		//�������ΪassignTo[i].second,�˿ͱ��Ϊi
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

/*���Ҷ���customerNo��˵����Ŀ��Ź������*/
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

/*���Ѱ��һ�����˵Ĺ���*/
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


/*���������˿ͣ����ʧ���򷵻�false*/
bool CFLP::swapTwoCustomer(vector<pair<bool, int>>& assignTo, vector<bool> isFacilityOpened, 
	vector<vector<float>> serviceCost, vector<pair<float, float>> facilityVec,vector<float> customerDemand)
{	
	
	int facilityNum = facilityVec.size();
	vector<vector<int>> facilityAssignment(facilityNum);
	//��¼ÿ�������õ��ķ���˿�
	
	vector<pair<bool, int>> assignToCopy(assignTo);

	int customerNum = assignToCopy.size();
	for (int i = 0; i < customerNum; i++) {
		if (assignToCopy[i].first == false) {
			cout << "swapTwoCustomer error:assignToCopy[i].first == false" << endl;
			return false;
		}
		facilityAssignment[assignToCopy[i].second].push_back(i);
	}
	
	/*��֤��һ����������*/
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
		//��֤������������ͬ
		if (randomSeletedFacilityFirst != randomSeletedFacilitySecond) {
			break;
		}
	}
	 

	/*�ӵ�һ������ѡ��һ���˿�*/


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


	/*���ڶ������ѡ���Ĺ�����,��������������*/
	if (unusedSpaceInFacilityNo(randomSeletedFacilitySecond, facilityVec, assignToCopy, customerDemand) 
		< customerDemand[randomCustomerFirst]) 
	{	
		int tryTimes = 100;

		while(true){
			//���ѡ��һ���˿ͣ��ճ��Ŀռ����ú�������
			randomCustomerSecond = facilityAssignment[randomSeletedFacilitySecond]
				[rand() % facilityAssignment[randomSeletedFacilitySecond].size()];
			if (unusedSpaceInFacilityNo(randomSeletedFacilitySecond, 
				facilityVec, assignToCopy, customerDemand)+customerDemand[randomCustomerSecond]
				>customerDemand[randomCustomerFirst]) {
				//��ԭ���Ĺ���ȥ��
				assignToCopy[randomCustomerSecond].first = false;
				assignToCopy[randomCustomerSecond].second = 0;
				/*���Ҫ��Ҫ����ν�����治�����õ�facilityAssignment
				for (auto iter = facilityAssignment[randomSeletedFacilitySecond].begin();
					iter != facilityAssignment[randomSeletedFacilitySecond].end(); iter++) {
					if (*iter == randomCustomerSecond) {
						facilityAssignment[randomSeletedFacilitySecond].erase(iter);
					}
				}*/	
				break;
			}

			/*����ѡ��*/
			randomCustomerSecond = facilityAssignment[randomSeletedFacilitySecond]
				[rand() % facilityAssignment[randomSeletedFacilitySecond].size()];
			if ((tryTimes--) == 0) {
				//cout << "swapTwoCustomer find place for customer fail" << endl;
				return false;
			}
		}

		//��ѡ�����Ĺ˿��������һ���пռ�Ĺ���
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

		//�Ӵ������ӽ�����Ĺ���
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

	//�����һ���˿�
	assignToCopy[randomCustomerFirst].first = true;
	assignToCopy[randomCustomerFirst].second = randomSeletedFacilitySecond;

	int previousCost = calculateTotalCost(assignTo, isFacilityOpened, serviceCost, facilityVec);
	int currentCost = calculateTotalCost(assignToCopy, isFacilityOpened, serviceCost, facilityVec);
	
	/*��ԭ���*/
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
