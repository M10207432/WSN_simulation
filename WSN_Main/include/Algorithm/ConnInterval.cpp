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
#include "../Struct/WSNEnergy.h"

#undef  _ShowLog

using namespace std;

EventInterval::EventInterval(){
	printf("Connection Interval Object\n");
}
/*==============================================
		��ܻݭn���@�� 
		Connection interval �p��覡
==============================================*/
void EventInterval::Algorithm(int Rateproposal){
	switch (Rateproposal){
	case 0:
		Event();
		break;
	case 1:
		MEI(NULL);
		break;
	case 2:
		DIF();
		break;
	case 4:
		Greedy();
		break;
	default:
		break;
	}

	for(Packet *pkt=Head->nextnd->pkt; pkt!=NULL; pkt=pkt->nextpkt){
		pkt->exeload=pkt->load;
		pkt->exehop=pkt->hop;
	}
	
}

/*==============================================
		��ܻݭn���@�� 
		Connection interval ���XTDMA�ץ��覡
==============================================*/
void EventInterval::Interval_TDMA_Algorithm(int proposal){
	switch (proposal){
	case 0:
		EIMA();
		break;
	case 1:
		IntervalDivide();
		break;
	case 2:
		EIMA_2();
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
void EventInterval::MEI(Node * MEINode){
	Packet *TSBpktQ=ReadyQ;
	Packet *TSBpkt=Head->nextnd->pkt;
	double Tc=0;
	
	/*------------------------------------------
		�����n�Ҧ�Conn/Adv Node
		�W��connection/advertisement interval
	------------------------------------------*/
	PacketQueue();		//����Ready Queue
	Node *TSBnode=Head->nextnd;
	while(TSBnode!=NULL){
		Packet *TSBpkt=TSBnode->pktQueue;

		if(MEINode!=NULL){
			TSBpkt=MEINode->pktQueue;
		}
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

		TSBnode->eventinterval=Tc;
		TSBnode=TSBnode->nextnd;

		if(MEINode!=NULL){
			break;
		}
	}
}

/*=====================================
	��ݭnScan ��Conn Node
	��Scan duration �p��A�H��Tc���s�p��
=====================================*/

void EventInterval::IntervalReassign(){
	double MaxAdvinter=0;	//�̪��s�����Z
	short int devicenum=0;	//Adv Device �ƶq

	for(Node *node=Head->nextnd; node!=NULL; node=node->nextnd){
		if(node->SendNode==Head){
			MaxAdvinter=0;
			devicenum=0;

			//����Device �ƶq & �����̤j�s�����Z
			for(Node *AdvNode=Head->nextnd; AdvNode!=NULL; AdvNode=AdvNode->nextnd){
				if(node==AdvNode->SendNode){
					AdvNode->pkt->node=node;	//�D�n�O�n�N����node�ର����Conn Node�A�ت����bPacketQueue�� �n��node�Wreadynextpkt�w�ơA�BMEI���s�p��
												//��������n�]�w�^�� (Node�W��nodenextpkt�å�����)
					devicenum++;			
					if(AdvNode->eventinterval>MaxAdvinter){
						MaxAdvinter=AdvNode->eventinterval;
					}
				}
			}

			if(devicenum>0){
				//�p��Scan duration (int ScanWin, int ScanInter, int AdvInter, int device)
				node->ScanDuration=node->SCAN_Compute(	node->ScanWin,
																											node->ScanInter,
																											MaxAdvinter,
																											devicenum);
				node->EXEScanDuration=node->ScanDuration;
				node->ScanFlag=false;

				MEI(node);

				for(Node *AdvNode=Head->nextnd; AdvNode!=NULL; AdvNode=AdvNode->nextnd){
					if(node==AdvNode->SendNode){
						AdvNode->pkt->node=AdvNode;
					}
				}
			}
		}
	}
}

/*===========================
		�����
	��U�Ӱ϶�(interval)
	<arrival -> period> 
step1:��U�϶� ���㪺packet
step2:�U�϶���(packet->load�[�`) ���H (interval) <interval�|��϶����̤jarrival�P�̤jdeadline�B�n�ư��Hassign�L��packet�϶�>
step3:�p��U�϶� rate 
step4:��X�̤jrate , ��b�϶���packet assign ��rate
(��϶��ɭn�N��rate���϶��ɶ�����)

���t�n�C�@packet��rate
===========================*/
void EventInterval::DIF(){
	PacketQueue();

	DIFMinperiod=ReadyQ->readynextpkt->period;
	Packet * DIFpacket;
	map<double,map<double,DIFtable>> Table;	//�G��map ���e�榡��DIFtable map[arrival][deadline]
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
						if(a->first <= DIFpacket->arrival && DIFpacket->deadline <= p->first && DIFpacket->rate==0){ //�|��assigh rate�B����϶���
							Packet* tmpDIFpacket;
							double start=a->first;
							double end=p->first;
							
							//��J�϶��B�϶���load�`�� �H�� ���϶�Density
							Table[a->first][p->first].length=p->first - a->first;	//length <deadline-arrival>
							while(start!=end){
								
								tmpDIFpacket=Head->nextnd->pkt;
								while(tmpDIFpacket!=NULL){
									//�ư��b���϶���assign�Lrate��packet
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

							Table[a->first][p->first].load=Table[a->first][p->first].load+DIFpacket->load*((p->first-a->first)/DIFpacket->period);
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

	//-------------------------------Assign��FrameTbl,�u��Connection node��3�ӥ�
	FrameTbl=new FrameTable;
	FrameTable* Ftbl=FrameTbl;

	for(TDMATable* tbl=TDMA_Tbl; tbl!=NULL; tbl=tbl->next_tbl){
		if(tbl->slot==frameid && tbl->n1->SendNode==Head){
			Ftbl->id=frameid++;
			Ftbl->Currentflag=false;
			Ftbl->arrival=0;
			Ftbl->Period=tbl->n1->eventinterval;
			Ftbl->Deadline=tbl->n1->eventinterval;
			
			//Ftbl->Size=Ftbl->Period/nodelevel1;
			Ftbl->Size=floor(Ftbl->Period/nodelevel1);

			Ftbl->Utilization=1/nodelevel1;
			
			Ftbl->ConnNode= tbl->n1;			//���V��Conn Node
			tbl->n1->eventinterval=Ftbl->Size;	//��snode�W��connection interval

			Ftbl->next_tbl=new FrameTable;
			Ftbl->next_tbl->pre_tbl=Ftbl;
			Ftbl=Ftbl->next_tbl;
		}
	}
	Ftbl->pre_tbl->next_tbl=NULL;
	//-------------------------------------Assign �� AdvNode�ϥ�
	/*
	for(FrameTable* Ftbl=FrameTbl; Ftbl!=NULL; Ftbl=Ftbl->next_tbl){
		for(Node* n=Head->nextnd; n!=NULL; n=n->nextnd)	{
			if(n->SendNode!=Head){
				
			}
		}
	}
	*/
	if(--frameid>nodelevel1){
		printf("The Frame size is larger than three, the FrameTable is error\n");
		system("PAUSE");
	}
	
	
	//---------------------------------Print �X��T
#ifdef _Showlog
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
#endif
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
			Ftbl->Utilization=1/nodelevel1;

			Ftbl->ConnNode= tbl->n1;			//���V��Conn Node
			tbl->n1->eventinterval=Ftbl->Size;	//��snode�W��connection interval

			Ftbl->next_tbl=new FrameTable;
			Ftbl->next_tbl->pre_tbl=Ftbl;
			Ftbl=Ftbl->next_tbl;
		}
	}
	Ftbl->pre_tbl->next_tbl=NULL;

	if(--frameid>nodelevel1){
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
	
	#ifdef _Showlog
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
	#endif
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

void EventInterval::EIMA_2(){
	bool EIMAEDF_flag=true;	//����EIMA EDF �W���inteval�W���վ�

	double TDMASize=1;	//TDMASIZE
	double Mininterval=Hyperperiod;	//�̤p��interval
	double tmpinterval=Hyperperiod;
	int devicenum=0;	//device �ƶq
	int MaxAdvinter=0;	//�����s���s���̤j���s�����Z
	short int frameid=1;
	double min_interval=-1;//�̤p��interval

	//�Y��interval�j��4�� (400 unit��10ms)�A�n���e�yscaling�ҥHunit�|��
	double res_total_u=0;
	for(Node* n=Head->nextnd; n!=NULL; n=n->nextnd){
		if(n->eventinterval>400){
			unit=0.001;		
		}
	}
	//����Node lifetime��interval�ե�
	for(Node* n=Head->nextnd; n!=NULL; n=n->nextnd){
		double avgcurrent=(((I_notify*Time_notify)+(I_sleep*(n->eventinterval*unit-Time_notify)))/(n->eventinterval*unit));
		
		//=======================
		
		double Qc=Hyperperiod*I_notify*Time_notify/(n->eventinterval);
		double totalcnt=0;
		double Qt=0;
		for(Packet* pkt=n->pkt; pkt!=NULL; pkt=pkt->nodenextpkt){
			double count=((Hyperperiod/pkt->period)*ceil(pkt->load/payload));
			totalcnt+=count;
		}
		Qt=(totalcnt-(Hyperperiod/n->eventinterval))*I_Tran*Time_Tran;
		
		//Qt=0;
		double Seqavgcurrent=((Qc+Qt)+I_sleep*(Hyperperiod*unit-(Qc/I_notify)-(Qt/I_Tran))) / (Hyperperiod*unit);
		//avgcurrent=Seqavgcurrent;
		//Seqavgcurrent=(((I_notify*Time_notify*totalcnt)+(I_sleep*(n->eventinterval*unit-Time_notify*totalcnt)))/(Hyperperiod*unit));
		//=======================
		
		n->avgcurrent=avgcurrent;
		double lifetime=(1/n->avgcurrent);
		double weight=1/lifetime;
		res_total_u=res_total_u+weight;

		//��̤pinterval
		if(min_interval==-1){
			min_interval=n->eventinterval;
		}else if(n->eventinterval<min_interval){
			min_interval=n->eventinterval;
		}
	}

	//-------------------------------Assign��FrameTbl,�u��Connection node��3�ӥ�
	FrameTbl=new FrameTable;
	FrameTable* Ftbl=FrameTbl;
	
	for(TDMATable* tbl=TDMA_Tbl; tbl!=NULL; tbl=tbl->next_tbl){
		if(tbl->slot==frameid && tbl->n1->SendNode==Head){
			Ftbl->id=frameid++;
			Ftbl->Currentflag=false;
			Ftbl->arrival=0;
			Ftbl->Period=tbl->n1->eventinterval;
			Ftbl->Deadline=tbl->n1->eventinterval;
			/*---------------------------------------
			---------------------------------------*/
			
			//Ftbl->Size=(((1/(BatteryCapacity/(((I_notify*Time_notify)+(I_sleep*(tbl->n1->eventinterval*unit-Time_notify)))/(tbl->n1->eventinterval*unit))))/res_total_u))
			//			* tbl->n1->eventinterval;
			//Ftbl->Size=(((1/(BatteryCapacity/ tbl->n1->avgcurrent))/res_total_u))
			//			* tbl->n1->eventinterval;
			Ftbl->Size=((tbl->n1->avgcurrent)/res_total_u)
						* tbl->n1->eventinterval;
			/*---------------------------------------
			---------------------------------------*/
			Ftbl->Utilization=1/nodelevel1;
			Ftbl->ConnNode= tbl->n1;			//���V��Conn Node
			tbl->n1->eventinterval=Ftbl->Size;	//��snode�W��connection interval

			Ftbl->next_tbl=new FrameTable;
			Ftbl->next_tbl->pre_tbl=Ftbl;
			Ftbl=Ftbl->next_tbl;
		}
	}
	Ftbl->pre_tbl->next_tbl=NULL;

	/*=======================================
			�[�JDemand bound
	=======================================*/
	/*
	double Minperiod_size=0;
	double Minperiod_period=-1;
	double _Maxsize=0;
	for(FrameTable* Ftbl=FrameTbl; Ftbl!=NULL; Ftbl=Ftbl->next_tbl){
		//Find size of minimum period
		if(Minperiod_period==-1){
			Minperiod_period=Ftbl->Period;
			Minperiod_size=Ftbl->Size;
		}else{
			if(Ftbl->Period<Minperiod_period){
				Minperiod_period=Ftbl->Period;
				Minperiod_size=Ftbl->Size;
			}
		}

		//Find max size
		if(Ftbl->Size>_Maxsize){
			_Maxsize=Ftbl->Size;
		}
	}

	if((_Maxsize+Minperiod_size)>Minperiod_period){
		for(FrameTable* Ftbl=FrameTbl; Ftbl!=NULL; Ftbl=Ftbl->next_tbl){
			Ftbl->Size=Minperiod_period/2;
			Ftbl->ConnNode->eventinterval=Ftbl->Size;	//��snode�W��connection interval
		}	
	}
	*/
	//-------------------------------------Assign �� AdvNode�ϥ�
	/*
	for(FrameTable* Ftbl=FrameTbl; Ftbl!=NULL; Ftbl=Ftbl->next_tbl){
		for(Node* n=Head->nextnd; n!=NULL; n=n->nextnd)	{
			if(n->SendNode!=Head){
				
			}
		}
	}
	*/
	unit=0.01;
	if(--frameid>nodelevel1){
		printf("The Frame size is larger than three, the FrameTable is error\n");
		system("PAUSE");
	}
	
	
	//---------------------------------Print �X��T
#ifdef _Showlog
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
#endif
}

void EventInterval::Greedy(){
	for(Node *n=Head->nextnd; n!=NULL; n=n->nextnd){
		//find the minimum period
		Packet* Minpkt=NULL;
		for(Packet* pkt=n->pkt; pkt!=NULL; pkt=pkt->nodenextpkt){
			if(Minpkt==NULL){
				Minpkt=pkt;
			}else{
				if(Minpkt->period > pkt->period){
					Minpkt=pkt;
				}
			}
		}

		n->eventinterval=Minpkt->period;
	}
}