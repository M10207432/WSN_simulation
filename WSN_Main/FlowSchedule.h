extern long int Timeslot;
extern double totalevent;			//Event數量
extern bool Meetflag;				//看是否meet deadline
extern int ReadyQ_overflag; 
extern double Maxrate;				//最高速度為20bytes/slot
extern double payload;				//payload 為 20bytes
extern int Maxbuffersize;			//Maxbuffersize 為 6個packets
extern int Pktsize;					//計算IntervalPower的pkt num
extern int TDMASlot;

void FlowEDF();
void PacketQueue();
void BufferSet();
void MainSchedule(int ,bool );
void Schedulability();

void NodeBufferSet(Node *);
void BLESchedule(int, bool);
void BLE_EDF();