extern long int Timeslot;
extern double totalevent;			//Event數量
extern bool Meetflag;				//看是否meet deadline
extern int ReadyQ_overflag; 
extern double Maxrate;				//最高速度為20bytes/slot
extern double payload;				//payload 為 20bytes
extern int Maxbuffersize;			//Maxbuffersize 為 4個packets
extern int Pktsize;					//計算IntervalPower的pkt num

extern int EXECBclock;				//做DIF與Lazy 計時器
extern int dec_cof;					//Lazy的decrease係數
extern int Callbackclock;			//做DIF與Lazy 計時器
extern int overheadcount;
extern FrameTable *Cycle;
extern short int pollingcount;

void Schedule(int,int);	//Write-Request method, Service Interval method
/*=====================
	Node Packet setting
	Main Sche
=====================*/
void PacketQueue();
void NodeBufferSet(Node *);
void BLE_EDF(Node *);
void Write_Request();

/*=====================
	Write-Request方法
=====================*/
void NPEDF();			//Non-premptive Earliest Deadline First
void RoundRobin();		//Sequence request data in one cycle
void EIF();				//Earliest Interval First
void Polling();			//Sequnce request data with different cycle

void SingleNodeSchedule(int);
void LazyOnWrite();
void LazyIntervalCB();
void DIFCB();

/*=====================
	確認	每一packet
	沒有miss 
=====================*/
void CheckPkt();
void Finalcheck();

void Schedulability();
