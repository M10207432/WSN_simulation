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
	int id;
	short int hop;		//range 1~3
	short int color;	//顏色
	bool ContinueNotify;	//Connection interval arrival則為True, Buffer=Empty為False

	Node* SendNode;//傳送節點
	double coor_x,coor_y;//座標
	double radius;
	double distanceto_BS;//到Base station 距離
	double energy;
	double avgenergy;
	double eventinterval;
	short int ExTimeslot;
	short int LatestTimeslot;
	short int edge;		//連接數目
	bool order_flag;	//在coloring時，是否找過
	short int arrival_flag;//0->Interval並未到、1->已arrival、-1->剛剛傳輸完畢

	string State;		//Wakeup、Sleep、Transmission & Idle
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
struct TDMATable{
	int slot;
	Node *n1;
	
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
extern TDMATable *TDMA_Tbl;
extern PacketBuffer* Buffer;
extern Node* SetHead;
extern Node* Head;
extern Packet* Headpacket;
extern Node *SetNode;
extern Node* node;	
extern Node*NotifyNode;	//正在傳輸的node,最多可連傳6個packets
extern Packet* packet;
extern Packet *ReadyQ;
extern Flow *Headflow;
extern stringstream stream;
extern int nodenum;
extern int pktnum;
extern long int Hyperperiod;
extern string str_coor_x,str_coor_y,str_radius;
extern string strload,strperiod,strutilization,strhop;

/*===========================
	建立Node & Packet
	的 Link List
===========================*/
void StructGEN();