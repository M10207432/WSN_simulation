#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "WSNFile.h"
#include "WSNStruct.h"

using namespace std;

int node_level1=0;
int node_level2=0;
/*===========================
		建立linklist
		並將訊息寫入
===========================*/
void StructGEN(){
	
	delete Head;
	Head=NULL;
	delete Headpacket;delete node;delete packet;delete Headflow;
	Headpacket=NULL;node=NULL;packet=NULL;Headflow=NULL;
	delete HeadEdge;delete MainEdge;delete ConflictEdge;
	HeadEdge=NULL;MainEdge=NULL;ConflictEdge=NULL;
	delete TDMA_Tbl;TDMA_Tbl=NULL;
	delete FrameTbl;FrameTbl=NULL;

	Head=new Node;
	Headpacket=new Packet;
	node=new Node;
	packet=new Packet;
	ReadyQ=new Packet;
	Headflow=new Flow;
	HeadEdge=new Edge;
	MainEdge=new Edge;
	ConflictEdge=new Edge;
	TDMA_Tbl=new TDMATable;
	/*==========================
			建立Link list
		    Node & Packet
	===========================*/
	/*-------------------------
		Gen node(Linklist)
	-------------------------*/
	string str;

	GENfile>>str;stream.clear();stream<<str;stream>>node_level1; 
	GENfile>>str;stream.clear();stream<<str; stream>>node_level2;
	GENfile>>str;stream.clear();stream<<str; stream>>pktnum;
	GENfile>>str;stream.clear();stream<<str; stream>>Hyperperiod;

	nodenum=node_level1+node_level2;
	nodelevel1=node_level1;
	nodelevel2=node_level2;

	Head->nextnd=node;
	Head->id=0;
	Head->ChildNode=NULL;
	for(int n=0;n<nodenum;n++){
		if(node_level1>0)
			node_level1--;
		else
			pktnum=1;
		/*-------------------------
			packet(Linklist)
		-------------------------*/

		node->pkt=packet;
		packet->nodeprepkt=NULL;
		for (int p=0;p<pktnum;p++){

			Packet* nextpacket=new Packet;
			Packet* prepacket=packet;

			packet->nextpkt=nextpacket;
			packet->nodenextpkt=nextpacket;
			packet->readynextpkt=nextpacket;
			packet=nextpacket;
			packet->prepkt=prepacket;
			packet->nodeprepkt=prepacket;
			packet->readyprepkt=prepacket;
		}
		packet->nodeprepkt->nodenextpkt=NULL;

		//--------------------------Packet Done

		Node* nextnode=new Node;
		Node* prenode=node;
		node->nextnd=nextnode;
		node=nextnode;
		node->prend=prenode;
	}
	packet=packet->prepkt;
	packet->nextpkt=NULL;
	packet->readynextpkt=NULL;
	node=node->prend;
	node->nextnd=NULL;

	Headpacket->nextpkt=Head->nextnd->pkt;
	Head->nextnd->pkt->prepkt=Headpacket;
	Headpacket->readynextpkt=Head->nextnd->pkt;
	Head->nextnd->pkt->readyprepkt=Headpacket;
	//-----------------------------Node Done
	
	/*==========================
		寫入GEN的資訊
	==========================*/
	node=Head;
	int pktid=1;
	int ndid=1;
	int pktnum_count=0;

	while(str!="=========="){
		
		GENfile>>str;
		if(str=="Node"){
			node=node->nextnd;

			GENfile>>str_coor_x>>str_coor_y>>str_radius;
			stream.clear();	stream<<str_coor_x;stream>>node->coor_x;		//coor_x
			stream.clear();	stream<<str_coor_y;stream>>node->coor_y;		//coor_y
			stream.clear();	stream<<str_radius;stream>>node->radius;		//radius
			
			node->energy=0;
			node->color=0;
			node->edge=0;
			node->pktQueue=NULL;
			node->NodeBuffer=new PacketBuffer;
			node->ChildNode=NULL;
			node->order_flag=false;
			node->arrival_flag=0;
			node->Notify_evtcount=0;
			node->Tran_evtcount=0;
			node->EIMA_avgcurrent=0;

			packet=node->pkt;

			node->id=ndid++;

		}
		if(str=="Pkt"){
			pktnum_count++;
			GENfile>>strload>>strperiod>>strutilization>>strhop;
			
			stream.clear();	stream<<strload;stream>>packet->load;					//Load
			stream.clear();	stream<<strperiod;stream>>packet->period;				//Period
			stream.clear();	stream<<strutilization;	stream>>packet->utilization;	//Utilization
			stream.clear();	stream<<strhop;	stream>>packet->hop;	//Hop
			
			node->hop=packet->hop;
			packet->nodeid=node->id;
			packet->period=packet->period;
			packet->node=node;														//所屬的感測器
			packet->exehop=packet->hop;
			packet->exeload=packet->load;
			packet->arrival=0;														//Arrival
			packet->deadline=packet->arrival+packet->period;						//Deadline
			packet->id=pktid++;														//id
			packet->readyflag=1;													//ready flag
			packet->searchdone=0;													//ready flag
			packet->rate=0;
			packet->State="Idle";
			packet->CMP_D=packet->deadline;
			packet->Miss_count=0;
			packet->latency=0;
			packet->meetlatency=0;
			packet->meetlatency_cnt=0;

			//下一個packet
			packet=packet->nextpkt;
		}
	}
	GENfile>>str;//useless

	if(SetNode==NULL){
		int Setid=1;
		Node *tmpSetNode;
		Node *MainSetNode;
		SetNode=new Node;
		MainSetNode=SetNode;

		node=Head->nextnd;
		while(node!=NULL){
			tmpSetNode=new Node;
			SetNode->nextnd=tmpSetNode;
			tmpSetNode->prend=SetNode;

			SetNode->id=Setid++;
			SetNode->avgenergy=0;

			SetNode=tmpSetNode;
			
			node=node->nextnd;
		}
		SetNode=SetNode->prend;
		SetNode->nextnd=NULL;

		SetNode=MainSetNode;
		SetHead->nextnd=SetNode;
	}
}