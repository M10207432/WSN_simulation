extern long int Timeslot;
extern double totalevent;			//Event數量
extern bool Meetflag;				//看是否meet deadline
extern int ReadyQ_overflag; 
extern double Maxrate;				//最高速度為20bytes/slot
extern double payload;				//payload 為 20bytes
extern int Maxbuffersize;			//Maxbuffersize 為 6個packets
extern int Pktsize;					//計算IntervalPower的pkt num
extern int TDMASlot;
extern int EXECBclock;			//做DIF與Lazy 計時器
extern int Callbackclock;			//做DIF與Lazy 計時器
extern TDMATable *NotifyTable;
extern int overheadcount;
extern FrameTable *Cycle;
extern short int pollingcount;

/*=====================
	Node Packet setting
	Main Sche
=====================*/
void PacketQueue();
void NodeBufferSet(Node *);
void BLE_EDF(Node *);

/*=====================
	Write-Request方法
=====================*/
void Schedule(int,int);
void NPEDF();
void RoundRobin();
void EIF();
void Polling();

void SingleNodeSchedule(int);
void LazyOnWrite();
void LazyIntervalCB();
void DIFCB();

void SingleStatic();
void CheckPkt();
void Finalcheck();

void Schedulability();
