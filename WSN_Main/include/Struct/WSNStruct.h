/*=================================
		Structure
==================================*/
struct Flow{
	struct Packet* pkt;
	struct Node* sourcenode;
	struct Node* destinationnode;
};
struct PacketBuffer{
	double pktsize;	//1~6 packets
	int load;		//0~120bytes
	Packet* pkt;
};
struct Node{
	int id;					//Node ID
	short int hop;			//range 1~3
	short int color;		//顏色
	bool EvtArrival;
	string CMD;				//通知動作 "IDLE" "Notify" "SCAN" (只會是ConnSet)
	int FrameSize;			//TDMA對應的FrameSize
	char16_t Notify_evtcount;	//Notify evt數量 (typedef uint16_t char16_t)
	char16_t Tran_evtcount;		//Tran evt數量 (typedef uint16_t char16_t)

	Node* SendNode;			//傳送節點
	double coor_x,coor_y;	//座標
	double radius;
	double distanceto_BS;	//到Base station 距離
	double energy;
	double avgenergy;
	double EIMA_avgcurrent;
	double lifetime;
	double eventinterval;	//connection interval or advertisement interval
	short int ExTimeslot;
	short int LatestTimeslot;
	short int edge;			//連接數目
	bool order_flag;		//在coloring時，是否找過
	short int arrival_flag;	//0->Interval並未到、1->已arrival、-1->剛剛傳輸完畢
	double miss_ratio;

	string State;			//Wakeup, Sleep, Transmission, Idle & Scan
	Packet* pkt;
	Packet* pktQueue;
	Node* nextnd;
	Node* prend;
	Node* ChildNode;//子節點
	Node* next_child;

	Node* RecvNode;//接收節點
	Node* next_recvnode;
	Packet* RecvPkt;
	Packet* next_recvpkt;
	int TDMArecv_flag;

	PacketBuffer* NodeBuffer;

	int ScanWin;
	int ScanInter;
	int AdvInter;

	bool ScanFlag;
	double ScanDuration;
	double EXEScanDuration;
	double SCAN_Compute(int ScanWin, int ScanInter, int AdvInter, int device){
		double AdvDelay=0.5*AdvInter+(10*(ScanWin/ScanInter)+AdvInter*((ScanInter-ScanWin)/ScanInter));
		double SCANDelay=AdvDelay*exp((2*device)/(3*AdvDelay));

		return SCANDelay;
	};
};

struct Packet{
	int id;
	int nodeid;
	int load;
	int exeload;
	double arrival;
	double period;
	double deadline;
	double utilization;
	int	 pirority;
	int	 doneflag;
	int	 readyflag;
	int	 searchdone;
	double eventinterval;
	string State;		//Transmission 、 Idle

	short int hop;		//range 1~3
	short int exehop;		//range 1~3
	int destination;	//Nest node
	double rate;
	double SCAN_Duration;	//

	double CMP_D;		//判斷是否在此deadline前arrival，有=>往上疊加 否=>Miss_count++ 並往上疊加 (放於FlowSchedule下CheckPkt method)
	double Miss_count;	//Miss counting (放於FlowSchedule下CheckPkt method)
	double latency;			//計算miss時的latency
	double meetlatency;		//計算duration of sensor sensing arrival to tranmission
	double meetlatency_cnt;

	Node* node;

	Packet* nextpkt;		//Global packet link
	Packet* prepkt;			//Global packet link
	Packet* nodenextpkt;	//Local node packet link
	Packet* nodeprepkt;		//Local node packet link
	
	Packet* readynextpkt;	//Global ready packet link
	Packet* readyprepkt;	//Global ready packet link
	Packet* nodereadynextpkt;	//local node ready packet link
	Packet* nodereadyprepkt;	//local node ready packet link
	
	Packet* next_recvpkt;

	Packet* buffernextpkt;
};
struct Edge{
	Node *n1;
	Node *n2;

	Edge *next_edge;
	Edge *pre_edge;
};

struct FrameTable{
	bool Currentflag;
	
	short int id;
	double arrival;
	double Period;
	double Size;
	double Utilization;
	double Deadline;
	
	Node *ConnNode;
	Node *AdvNode;
	
	FrameTable* next_tbl;
	FrameTable* pre_tbl;

	FrameTable* polling_next;
};

struct TDMATable{
	short int slot;
	Node *n1;

	bool currslot;	//是否為現在執行的slot
	double count;	//此slot所需等待時間
	
	TDMATable * next_tbl;
	TDMATable * pre_tbl;
};
struct DIFtable{
	double load;
	double length;
	double density;
	double arrival;
	double deadline;
};

/*===========================
	Extern Global Variable
===========================*/

extern Edge *HeadEdge;
extern Edge *MainEdge;
extern Edge *ConflictEdge;
extern TDMATable *TDMA_Tbl;		//主要傳輸的TDMA列表
extern FrameTable* FrameTbl;	//放於Frame上的schedule用
extern PacketBuffer* Buffer;
extern Node* SetHead;
extern Node* Head;
extern Packet* Headpacket;
extern Node *SetNode;
extern Node* node;	
extern Packet* packet;
extern Packet *ReadyQ;
extern Flow *Headflow;
extern stringstream stream;
extern int nodenum;
extern int nodelevel1;
extern int nodelevel2;
extern int pktnum;
extern long int Hyperperiod;
extern string str_coor_x,str_coor_y,str_radius;
extern string strload,strperiod,strutilization,strhop;

/*===========================
	建立Node & Packet
	的 Link List
===========================*/
void StructGEN();