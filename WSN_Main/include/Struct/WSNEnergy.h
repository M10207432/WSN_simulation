extern double Vcc;			//BLE �X�ʹq��
extern double I_sleep;		//Sleep �q�y 
extern double I_notify;		//Notify �q�y 
extern double Time_sleep;		//Sleep �q�y 10ms
extern double Time_notify;	//Notify �ɶ� 2.775ms
extern double I_Tran;	//Transmission �q�y 14.2744mA
extern double Time_Tran;	//Transmission �ɶ� 0.49ms
extern double BatteryCapacity;

extern double Ie;			//�ǿ�p�� �q�y
extern double Te;			//�ǿ�ɶ�
extern double K;			//Rate power�`��
extern double unit;			//�ɶ���쬰10ms
extern double TotalEnergy;
extern double parma;
extern double parmb;

void NodeEnergy();
double IntervalPower(int ,int );
void NodeState();
void Node_EnergyState(Node *);