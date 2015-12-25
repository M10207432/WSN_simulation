extern double Vcc;			//BLE 驅動電壓
extern double I_sleep;		//Sleep 電流 
extern double I_notify;		//Notify 電流 
extern double Time_sleep;		//Sleep 電流 10ms
extern double Time_notify;	//Notify 時間 2.775ms
extern double I_Tran;	//Transmission 電流 14.2744mA
extern double Time_Tran;	//Transmission 時間 0.49ms
extern double BatteryCapacity;

extern double unit;			//時間單位為10ms
extern double parma;
extern double parmb;

void NodeEnergy();
void NodeState();
void Node_EnergyState(Node *);