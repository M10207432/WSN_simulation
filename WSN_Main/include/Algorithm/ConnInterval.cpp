#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <math.h>
#include <map> 
#include <memory>

#include "../Struct/WSNFile.h"
#include "../Struct/WSNStruct.h"
#include "ConnInterval.h"
#include "../Schedule/FlowSchedule.h"

using namespace std;

EventInterval::EventInterval(){
	printf("Connection Interval Object\n");
}
/*==============================================
		��ܻݭn���@�� 
		Connection interval �p��覡
==============================================*/
void EventInterval::Algorithm(int Rateproposal){
	switch (Rateproposal)
	{
	case 0:
		Event();
		break;
	case 1:
		MEI();
		break;
	case 2:
		DIF();
		break;
	default:
		break;
	}

	packet=Head->nextnd->pkt;
	while(packet!=NULL){
		packet->exeload=packet->load;
		packet->exehop=packet->hop;

		packet=packet->nextpkt;
	}
}

/*==============================================
		��ܻݭn���@�� 
		Connection interval ���XTDMA�ץ��覡
==============================================*/
void EventInterval::Interval_TDMA_Algorithm(int proposal){
	switch (proposal)
	{
	case 0:
		EIMA();
		break;
	case 1:
		IntervalDivide();
		break;
	default:
		break;
	}

	//�T�{�C�@node interval���p��1
	for(Node* node=Head->nextnd; node!=NULL; node=node->nextnd){
		if(node->eventinterval<1){
			node->eventinterval=1;
		}
	}
}

/*==============================================
		�C�@node connection interval ���� 1
==============================================*/
void EventInterval::Event(){
	node=Head->nextnd;
	while(node!=NULL){
		node->eventinterval=1;
		node=node->nextnd;
	}
}
/*==============================================
(�p��⭿Minperiod��pkt size)	->Minsize
	(�̤jload��pkt size)			->Maxsize

	�Y(2*Minsize+Maxsize)�j��⭿Buffersize
		Connection interval��Minperiod/2
	�_
		�̷Ӷ��jbuffersize���p��
==============================================*/
void EventInterval::MEI(){
	PacketQueue();		//����Ready Queue
	Packet *TSBpktQ=ReadyQ;
	Packet *TSBpkt=Head->nextnd->pkt;
	double Tc=0;
	
	if(false){//(2*Minsize+Maxsize) > 2*Maxbuffersize (���ѵ�non-preemption ��)
		//Tc=floor(Minperiod/2);
	}else{
		PacketQueue();		//����Ready Queue

		Node *TSBnode=Head->nextnd;
		while(TSBnode!=NULL){
			Packet *TSBpkt=TSBnode->pktQueue;
			int nodehop=TSBnode->hop;
			double Totalsize=0;
			double PacketSize=0;
			double totalevent=0;
			double Tslot=0;
			bool doneflag=false;

			//======================��Minperiod, �]�w��Tc init
			Tc=TSBpkt->period;

			//======================���R�C�@period�U, �O�_��meet deadline
			Tslot=TSBpkt->period;
	
			while(doneflag!=true){
				Totalsize=0;

				//��X�һ�buffer�q (Packet �ƶq --> Totalsize)
				TSBpkt=TSBnode->pktQueue;
				while(TSBpkt->period <= Tslot){
					Totalsize=Totalsize+(ceil(TSBpkt->load/payload)*ceil(Tslot/TSBpkt->period));	
					TSBpkt=TSBpkt->nodereadynextpkt;
					if(TSBpkt==NULL)
						break;
				}

				//�p��ݭn��event�ƶq�A�ϱ��һ�buffer�q (�ѨMHop���s��W�����D)
				if(nodehop>1){
					totalevent=ceil(Totalsize/Maxbuffersize);
					Totalsize=(totalevent*Maxbuffersize)*nodehop;
				}

				//�p��Connection interval
				PacketSize=floor(Tslot/Tc)*double(Maxbuffersize);
				while(Totalsize > PacketSize){
					Tc--;
					PacketSize=floor(Tslot/Tc)*double(Maxbuffersize);
				}

				//��sTime slot
				if(TSBpkt!=NULL){
					Tslot=TSBpkt->period;
				}else{
					doneflag=true;
				}
			}	

			TSBnode->eventinterval=Tc-Maxbuffersize;
			TSBnode=TSBnode->nextnd;
		}
	}
	
}

/*===========================
		�����
	��U�Ӱ϶�(interval)
	<arrival -> period> 
step1:��U�϶� ���㪺packet
step2:�U�϶���(packet->load�[�`) ���H (interval)
step3:�p��U�϶� rate 
step4:��X�̤jrate , ��b�϶���packet assign ��rate
(��϶��ɭn�N��rate���϶��ɶ�����)
===========================*/
void EventInterval::DIF(){
	PacketQueue();
	DIFMinperiod=ReadyQ->readynextpkt->period;
	Packet * DIFpacket;
	map<double,map<double,DIFtable>> Table;	//�G��map ���e�榡��DIFtable
	double maxarrvial,maxdeadline;			//��̤jDensity�� ���϶�
	double Maxdesity=0;						//��϶��� �̤jDensity
	bool Doneflag=false;					//����assign��rate
	
	while(Doneflag!=true){
		/*============================
			DIF init
			��U�϶�
		============================*/
		DIFpacket=Head->nextnd->pkt;
		while(DIFpacket!=NULL){
			Table[DIFpacket->arrival][DIFpacket->deadline];
		
			for (map<double,map<double,DIFtable>>::iterator a=Table.begin(); a!=Table.end(); a++){
				for(map<double,DIFtable>::iterator p=Table[a->first].begin(); p!=Table[a->first].end(); p++){
					Table[a->first][p->first];
					Table[a->first][p->first].load=0;
					Table[a->first][p->first].length=0;
				}
			}
		
			DIFpacket=DIFpacket->nextpkt;
		}
		/*============================
			�U�϶���(packet->load�[�`)
				   (�϶�����)
				   (�p��U�϶� rate)
		============================*/
		for (map<double,map<double,DIFtable>>::iterator a=Table.begin(); a!=Table.end(); a++){
			for(map<double,DIFtable>::iterator p=Table[a->first].begin(); p!=Table[a->first].end(); p++){
				DIFpacket=Head->nextnd->pkt;

				while(DIFpacket!=NULL){
					//�T�warrival �� deadline�p
					if(a->first < p->first){
						if(a->first <= DIFpacket->arrival && DIFpacket->deadline <= p->first && DIFpacket->rate==0){
							Packet* tmpDIFpacket;
							double start=a->first;
							double end=p->first;
							
							//��J�϶��B�϶���load�`�� �H�� ���϶�Density
							Table[a->first][p->first].length=p->first - a->first;
							while(start!=end){
								
								tmpDIFpacket=Head->nextnd->pkt;
								while(tmpDIFpacket!=NULL){
									if(tmpDIFpacket->rate!=0){
										if(start>=tmpDIFpacket->arrival && (start+1)<=tmpDIFpacket->deadline){
											Table[a->first][p->first].length--;
											tmpDIFpacket=NULL;
										}
									}
									if(tmpDIFpacket!=NULL)
										tmpDIFpacket=tmpDIFpacket->nextpkt;
								}
								start++;
							}

							Table[a->first][p->first].load=Table[a->first][p->first].load+DIFpacket->load;
							Table[a->first][p->first].density=Table[a->first][p->first].load/Table[a->first][p->first].length;
							
							//��X�̤jrate,�ì����϶�
							if(Table[a->first][p->first].density>=Maxdesity){
								maxarrvial=a->first;
								maxdeadline=p->first;
								Maxdesity=Table[a->first][p->first].density;
							}
						}
					}

					DIFpacket=DIFpacket->nextpkt;
				}
			}	
		}
		/*============================
			�b�϶���X�϶�����packet
		��assign rate �� packet->rate�W
		============================*/
		DIFpacket=Head->nextnd->pkt;
		while(DIFpacket!=NULL){
			if(maxarrvial <= DIFpacket->arrival && DIFpacket->deadline <= maxdeadline ){
				//�|��assign rate
				if(DIFpacket->rate==0){
					DIFpacket->rate=Table[maxarrvial][maxdeadline].density;
					
					DIFpacket->rate=(DIFpacket->rate);
				}
			}

			DIFpacket=DIFpacket->nextpkt;
		}
		Maxdesity=0;

		/*==================
			�T�{�Ҧ�packet
			�w�gassign�� rate
		====================*/
		Doneflag=true;
		DIFpacket=Head->nextnd->pkt;
		while(DIFpacket!=NULL){
			if(DIFpacket->rate==0)
				Doneflag=false;
			DIFpacket=DIFpacket->nextpkt;
		}
	}
	
	delete []DIFpacket;
}

void EventInterval::Rate_TO_Interval(int defaultMinperiod){
	//�P�_�O�_�ݭn����Tc
	double exconnectioninterval=Connectioninterval;
	Connectioninterval=0;
	Packet *tmppkt=Buffer->pkt;
	double Allload=Buffer->load;
	double Maxrate=0;
	double Minperiod;

	while(tmppkt!=NULL){
		if(tmppkt->rate > Maxrate){
			if(tmppkt->exeload <= payload){
				Connectioninterval=Connectioninterval+(tmppkt->exeload/tmppkt->rate);
				Allload=Allload-tmppkt->exeload;
			}else{
				Connectioninterval=Connectioninterval+(payload/tmppkt->rate);
				Allload=Allload-payload;
			}
		}
		tmppkt=tmppkt->buffernextpkt;
	}
		
	if(Connectioninterval<1){
		Connectioninterval=exconnectioninterval;//�]�w�����e��interval
	}else{
		Connectioninterval=floor(Connectioninterval);
	}

	//�P�_Connectioninterval�O�_�j��Minperiod
	if(Connectioninterval>defaultMinperiod)
		Connectioninterval=defaultMinperiod;

}

/*==============================================
		���X Energy Efficiency Interval & TDMA
		���i�@�B�T�{�i�b��deadline�e����

		=>�|�ץ��Unode�W��interval
		=>Assign�n�U��SCAN duration
==============================================*/
void EventInterval::EIMA(){
	bool EIMAEDF_flag=true;	//����EIMA EDF �W���inteval�W���վ�

	double TDMASize=1;	//TDMASIZE
	double Mininterval=Hyperperiod;	//�̤p��interval
	double tmpinterval=Hyperperiod;
	int devicenum=0;	//device �ƶq
	int MaxAdvinter=0;	//�����s���s���̤j���s�����Z
	short int frameid=1;

	//-------------------------------Assign��FrameTbl,�u��Connection node��3�ӦA��
	FrameTbl=new FrameTable;
	FrameTable* Ftbl=FrameTbl;

	for(TDMATable* tbl=TDMA_Tbl; tbl!=NULL; tbl=tbl->next_tbl){
		if(tbl->slot==frameid && tbl->n1->SendNode==Head){
			Ftbl->id=frameid++;
			Ftbl->Currentflag=false;
			Ftbl->Period=tbl->n1->eventinterval;
			Ftbl->Deadline=tbl->n1->eventinterval;
			Ftbl->Size=Ftbl->Period/nodelevel1;
			Ftbl->Utilization=1/nodelevel1;

			Ftbl->ConnNode= tbl->n1;			//���V��Conn Node
			tbl->n1->eventinterval=Ftbl->Size;	//��snode�W��connection interval

			Ftbl->next_tbl=new FrameTable;
			Ftbl->next_tbl->pre_tbl=Ftbl;
			Ftbl=Ftbl->next_tbl;
		}
	}
	Ftbl->pre_tbl->next_tbl=NULL;

	if(--frameid>3){
		printf("The Frame size is larger than three, the FrameTable is error\n");
		system("PAUSE");
	}
	
	//-------------------------------Scan duration �p��
	for(Node* node=Head->nextnd; node!=NULL; node=node->nextnd){
		devicenum=0;
		MaxAdvinter=0;
		if(node->SendNode==Head){
			//����Device �ƶq & �����̤j�s�����Z
			Node *BelongNode=Head->nextnd;
			while(BelongNode!=NULL){
				if(node==BelongNode->SendNode){
					devicenum++;			
					if(BelongNode->eventinterval>MaxAdvinter){
						MaxAdvinter=BelongNode->eventinterval;
					}
				}
				BelongNode=BelongNode->nextnd;
			}

			//�p��Scan duration
			node->ScanDuration=node->SCAN_Compute(	node->ScanWin,
													node->ScanInter,
													MaxAdvinter,
													devicenum);
		}else{
			node->ScanDuration=0;
		}
	}
	
	//---------------------------------Print �X��T
	for(Node* node=Head->nextnd; node!=NULL; node=node->nextnd){
		cout<<	"Node"<<node->id<<"=> "<<
				"Interval="<<node->eventinterval<<", "<<
				"Slot="<<node->color<<", "<<
				"Scan Duaration="<<node->ScanDuration<<", "<<
				"SendNode="<<node->SendNode->id<<endl;
	}
	for(FrameTable* Ftbl=FrameTbl; Ftbl!=NULL; Ftbl=Ftbl->next_tbl){
		cout<<"Frame"<<Ftbl->id<<" =>"<<
			" size="<< Ftbl->Size<<","<<
			" period="<<Ftbl->Period<<endl;
	}
}
/*==================================
		Small Interval Divide
==================================*/
void EventInterval::IntervalDivide(){

	double TDMASize=1;	//TDMASIZE
	double Mininterval=Hyperperiod;	//�̤p��interval
	double tmpinterval=Hyperperiod;
	int devicenum=0;	//device �ƶq
	int MaxAdvinter=0;	//�����s���s���̤j���s�����Z
	short int frameid=1;
	
	//-------------------------------�ҶqTDMA�[�c�A��Node interval�W���u��
	//��TDMA size
	for(TDMATable* tbl=TDMA_Tbl; tbl!=NULL;tbl=tbl->next_tbl){
		if(tbl->slot>TDMASize)
			TDMASize=tbl->slot;
	}
	
	//��̤p��connection interval
	for(Node *node=Head->nextnd; node!=NULL; node=node->nextnd){
		if(node->SendNode==Head && node->eventinterval<Mininterval){
			Mininterval=node->eventinterval;
		}
	}

	//�ץ�connection interval
	for(Node *node=Head->nextnd; node!=NULL; node=node->nextnd){
		if(node->SendNode==Head){
			node->eventinterval=(Mininterval/TDMASize);
			//node->eventinterval=node->eventinterval/TDMASize;
		}
	}

	//FrameTbl�إ�
	FrameTbl=new FrameTable;
	FrameTable* Ftbl=FrameTbl;

	for(TDMATable* tbl=TDMA_Tbl; tbl!=NULL; tbl=tbl->next_tbl){
		if(tbl->slot==frameid && tbl->n1->SendNode==Head){
			Ftbl->id=frameid++;
			Ftbl->Currentflag=false;
			Ftbl->Period=tbl->n1->eventinterval;
			Ftbl->Deadline=tbl->n1->eventinterval;
			Ftbl->Size=tbl->n1->eventinterval;
			Ftbl->Utilization=0.3;

			Ftbl->ConnNode= tbl->n1;			//���V��Conn Node
			tbl->n1->eventinterval=Ftbl->Size;	//��snode�W��connection interval

			Ftbl->next_tbl=new FrameTable;
			Ftbl->next_tbl->pre_tbl=Ftbl;
			Ftbl=Ftbl->next_tbl;
		}
	}
	Ftbl->pre_tbl->next_tbl=NULL;

	if(--frameid>3){
		printf("The Frame size is larger than three, the FrameTable is error\n");
		system("PAUSE");
	}
	
	//-------------------------------Scan duration �p��
	for(Node* node=Head->nextnd; node!=NULL; node=node->nextnd){
		devicenum=0;
		MaxAdvinter=0;
		if(node->SendNode==Head){
			//����Device �ƶq & �����̤j�s�����Z
			Node *BelongNode=Head->nextnd;
			while(BelongNode!=NULL){
				if(node==BelongNode->SendNode){
					devicenum++;			
					if(BelongNode->eventinterval>MaxAdvinter){
						MaxAdvinter=BelongNode->eventinterval;
					}
				}
				BelongNode=BelongNode->nextnd;
			}

			//�p��Scan duration
			node->ScanDuration=node->SCAN_Compute(	node->ScanWin,
													node->ScanInter,
													MaxAdvinter,
													devicenum);
		}else{
			node->ScanDuration=0;
		}
	}

	//---------------------------------Print �X��T
	for(Node* node=Head->nextnd; node!=NULL; node=node->nextnd){
		cout<<	"Node"<<node->id<<"=> "<<
				"Interval="<<node->eventinterval<<", "<<
				"Slot="<<node->color<<", "<<
				"Scan Duaration="<<node->ScanDuration<<", "<<
				"SendNode="<<node->SendNode->id<<endl;
	}
	for(FrameTable* Ftbl=FrameTbl; Ftbl!=NULL; Ftbl=Ftbl->next_tbl){
		cout<<"Frame"<<Ftbl->id<<" =>"<<
			" size="<< Ftbl->Size<<","<<
			" period="<<Ftbl->Period<<endl;
	}
}
/*==================================
		�s������
==================================*/
void EventInterval::ConnectionPriority(){
	Node* assignnode=NULL;
	double Max_eventinterval=0;
	double connorder=0;

	while(Max_eventinterval!=2000000000){
		Max_eventinterval=2000000000;

		for(Node* node=Head->nextnd; node!=NULL; node=node->nextnd){
			if(node->SendNode==Head && node->EventTime==-1){
				if(node->eventinterval<Max_eventinterval){
					assignnode=node;	
					Max_eventinterval=node->eventinterval;
				}
			}
		}

		if(assignnode!=NULL){
			assignnode->EventTime=connorder++;
		}
		assignnode=NULL;
	}
}