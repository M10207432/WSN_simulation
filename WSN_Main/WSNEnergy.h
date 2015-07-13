extern double Vcc;			//BLE 驅動電壓
extern double Isleep;		//Sleep 電流 
extern double Ie;			//傳輸峰值 電流
extern double Te;			//傳輸時間
extern double K;			//Rate power常數
extern double unit;			//時間單位為10ms
extern double TotalEnergy;
extern double parma;
extern double parmb;


void NodeEnergy();
double IntervalPower(int ,int );
void NodeState();
