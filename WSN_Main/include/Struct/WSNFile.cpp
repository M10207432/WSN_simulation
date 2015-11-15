#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>

#include "WSNFile.h"
#include "WSNStruct.h"
#include "WSNEnergy.h"
#include "../Schedule/FlowSchedule.h"

#undef  _ShowLog

using namespace std;
static double miss_ration=0;
static double total_latency=0;
/*===========================
		GEN & Output 
		File Path
===========================*/
string filename;
fstream GENfile;
fstream Schdulefile;
fstream Finalfile;
fstream Resultfile;

string GENPath="..\\GENresult\\input_varied\\";
string SchedulePath="..\\WSNresult\\Debug\\";
string FinalPath="..\\WSNresult\\Debug\\";
string ResultPath="..\\WSNresult\\Debug\\";

/*===========================
		將GEN的資料取入 且
		建立輸出資料
===========================*/
void CreateFile(float U,int Set){
	//放入GEN的檔名
	filename="Rate";filename.append(to_string(U));filename.append("_Set");filename.append(to_string(Set));filename.append(".txt");

	//放入GEN的 路徑+檔名
	string GENBuffer=GENPath;
	GENBuffer.append(filename);

	//=========================開啟GENFile
	GENfile.open(GENBuffer, ios::in);	//開啟檔案.寫入狀態
	if(!GENfile){//如果開啟檔案失敗，fp為0；成功，fp為非0
		cout<<"Fail to open file: "<<GENBuffer<<endl;
		
		system("PAUSE");
	}

	//========================開啟ScheduleFile
	string ScheduleFileBuffer=SchedulePath;
	ScheduleFileBuffer.append("Schedule_");
	ScheduleFileBuffer.append(filename);

	Schdulefile.open(ScheduleFileBuffer, ios::out);	//開啟檔案.寫入狀態
	if(!Schdulefile){//如果開啟檔案失敗，fp為0；成功，fp為非0
		cout<<"Fail to open Schedule file: "<<ScheduleFileBuffer<<endl;		
		system("PAUSE");
	}

	//========================開啟Resultfile
	string ResultBuffer=ResultPath;
	ResultBuffer.append("Result_");
	ResultBuffer.append(filename);

	Resultfile.open(ResultBuffer, ios::out);	//開啟檔案.寫入狀態
	if(!Resultfile){//如果開啟檔案失敗，fp為0；成功，fp為非0
		cout<<"Fail to open Result file: "<<ResultBuffer<<endl;
		system("PAUSE");
	}

	//========================開啟Finalfile
	string FinalBuffer=FinalPath;
	FinalBuffer.append("FinalResult.txt");
	if(Finalfile.is_open()==false){
		Finalfile.open(FinalBuffer, ios::out);	//開啟檔案.寫入狀態
		if(!Finalfile){//如果開啟檔案失敗，fp為0；成功，fp為非0
			cout<<"Fail to open Final file: "<<FinalBuffer<<endl;
			system("PAUSE");
		}
	}
}

/*===========================
	單一taskset儲存
===========================*/
void SaveFile(short int setnum){
	double totalenergy=0;
	node=Head->nextnd;
	while(node!=NULL){
		node->energy=node->energy*Vcc;
		node->lifetime=BatteryCapacity/((node->energy/Vcc)/(Hyperperiod*0.01));

		#ifdef _ShowLog
				cout<<"Node"<<node->id<<" E:"<<node->energy<<" Lifetime:"<<node->lifetime<<endl;
		#endif

		totalenergy=totalenergy+node->energy;
		node=node->nextnd;
	}
			
	Resultfile<<"TotalEnergy:"<<totalenergy<<endl;
	#ifdef _ShowLog
		cout<<"TotalEnergy:"<<totalenergy<<endl;
	#endif
	
	//---------------------------------------------------TDMA table
	TDMATable *FlowTable=TDMA_Tbl;
	while(FlowTable!=NULL){
		#ifdef _ShowLog
				cout<<"S"<<FlowTable->slot<<" n"<<FlowTable->n1->id<<endl;
		#endif
		Resultfile<<"S"<<FlowTable->slot<<" n"<<FlowTable->n1->id<<endl;

		FlowTable=FlowTable->next_tbl;
	}

	//---------------------------------------------------存取各個node資訊
	Resultfile<<"	Lifetime Energy Tc NotifyEvt TranEvt, Color Conflict_node, coor_x coor_y Send_node"<<endl;

	node=Head->nextnd;
	while(node!=NULL){
		double AvgRate=0;
		double Pktnum=0;
		packet=node->pkt;
		while(packet!=NULL){
			AvgRate=AvgRate+packet->rate;
			Pktnum++;
			packet=packet->nodenextpkt;
		}
		AvgRate=AvgRate/Pktnum;

		//Node id:energy connectioninterval,color conflictnode...,coor_x coor_y SendNode 
		Resultfile<<"Node"<<node->id<<":";
		Resultfile<<node->lifetime<<" "<<node->energy<<" "<<node->eventinterval<<" "<<node->Notify_evtcount<<" "<<node->Tran_evtcount<<",";
		
		Resultfile<<node->color<<" ";
		Edge *printedge=ConflictEdge;
		while(printedge!=NULL){
			if(printedge->n1->id==node->id)
				Resultfile<<printedge->n2->id<<" ";
			printedge=printedge->next_edge;
		}
		Resultfile<<",";

		Resultfile<<node->coor_x<<" "<<node->coor_y<<" ";
		Resultfile<<node->SendNode->id<<endl;

		//Resultfile<<"Total Event:"<<totalevent<<endl;

		node=node->nextnd;
	}
	#ifdef _ShowLog
	cout<<"	FrameSize, FramePeriod"<<endl;
	#endif
	for(FrameTable* Ftbl=FrameTbl; Ftbl!=NULL; Ftbl=Ftbl->next_tbl){
	#ifdef _ShowLog
			cout<<"Frame"	<<Ftbl->id<<":"
							<<Ftbl->Size<<", "
							<<Ftbl->Period<<endl;
	#endif
		Resultfile<<"Frame"	<<Ftbl->id<<":"
							<<Ftbl->Size<<", "
							<<Ftbl->Period<<endl;
	}
	//--------------------------------------------計算miss ratio
	double Recv_count=0;
	double Miss_count=0;
	double latency=0;
	for(Packet* pkt=Head->nextnd->pkt; pkt!=NULL; pkt=pkt->nextpkt){
		Recv_count=Recv_count+Hyperperiod/pkt->period;
		Miss_count=Miss_count+pkt->Miss_count;
		latency=latency+pkt->latency;
	}
	Resultfile<<"Meet ratio:"<<Miss_count/Recv_count<<endl;
	Resultfile<<"Latency:"<<latency/Miss_count<<endl;
	//cout<<"Meet ratio:"<<Miss_count/Recv_count<<endl;

	//--------------------------------------------是否meet
	//Cal Energy
	node=Head->nextnd;
	SetNode=SetHead->nextnd;
	while(node!=NULL){
		SetNode->avgenergy=SetNode->avgenergy+node->energy;

		SetNode=SetNode->nextnd;
		node=node->nextnd;
	}

	//Cal lifetime(放於SetHead中)
	double minlifetime=-1;
	SetNode=SetHead->nextnd;
	for(Node* n=Head->nextnd; n!=NULL; n=n->nextnd){
		if(minlifetime==-1 || n->lifetime<minlifetime){
			minlifetime=n->lifetime;
		}
	}
	SetHead->lifetime+=minlifetime;
	AverageE=AverageE+totalenergy;

	if(Meetflag==true){
		Resultfile<<"Meet Deadline:MEET"<<endl;
		#ifdef _ShowLog
				cout<<"Meet Deadsline:MEET"<<endl;
		#endif
		/*
		//Cal Energy
		node=Head->nextnd;
		SetNode=SetHead->nextnd;
		while(node!=NULL){
			SetNode->avgenergy=SetNode->avgenergy+node->energy;

			SetNode=SetNode->nextnd;
			node=node->nextnd;
		}

		//Cal lifetime(放於SetHead中)
		double minlifetime=-1;
		SetNode=SetHead->nextnd;
		for(Node* n=Head->nextnd; n!=NULL; n=n->nextnd){
			if(minlifetime==-1 || n->lifetime<minlifetime){
				minlifetime=n->lifetime;
			}
		}
		SetHead->lifetime+=minlifetime;

		AverageE=AverageE+totalenergy;
		*/
		Meetcount++;
	}
	else{
		miss_ration=miss_ration+(Miss_count/Recv_count);
		total_latency=total_latency+(latency);

		Resultfile<<"Meet Deadline:MISS"<<endl;
		#ifdef _ShowLog
		cout<<"Meet Deadline:MISS"<<endl;
		#endif
	}
	
	Resultfile<<"==============================================="<<setnum<<endl;
	Schdulefile<<"=============================================="<<setnum<<endl;
	#ifdef _ShowLog
	cout<<"=============================================="<<setnum<<endl;
	#endif
}

/*===========================
	此利用率Set儲存
===========================*/
void SaveSet(int Set){
	cout<<"FinalResult"<<endl;
	cout<<"Meet="<<Meetcount<<endl;
	cout<<"Miss="<<Set-Meetcount<<endl;
	cout<<"MeetRatio="<<Meetcount/Set<<endl;
	SetNode=SetHead->nextnd;
	while(SetNode!=NULL){
		cout<<"Node"<<SetNode->id<<"="<<SetNode->avgenergy/Set<<endl;
		SetNode=SetNode->nextnd;
	}
	cout<<"Miss ratio="<<miss_ration/(Set-Meetcount)<<endl;
	cout<<"Latency="<<total_latency/(Set-Meetcount)<<endl;
	cout<<"Lifetime="<<(SetHead->lifetime)/(Set)<<endl;
	cout<<"AverageEnergy="<<AverageE/Set<<endl;
	cout<<"=============================================="<<endl;

	Finalfile<<"FinalResult"<<endl;
	Finalfile<<"Meet="<<Meetcount<<endl;
	Finalfile<<"Miss="<<Set-Meetcount<<endl;
	Finalfile<<"SetAmount_MeetRatio="<<Meetcount/Set<<endl;
	SetNode=SetHead->nextnd;
	while(SetNode!=NULL){
		Finalfile<<"Node"<<SetNode->id<<"="<<SetNode->avgenergy/Set<<endl;
		SetNode=SetNode->nextnd;
	}
	Finalfile<<"Miss ratio="<<miss_ration/(Set-Meetcount)<<endl;
	Finalfile<<"Latency="<<total_latency/(Set-Meetcount)<<endl;
	Finalfile<<"Lifetime="<<(SetHead->lifetime)/(Set)<<endl;
	Finalfile<<"AverageEnergy="<<AverageE/Set<<endl;
	Finalfile<<"=============================================="<<endl;
	
	total_latency=0;
	miss_ration=0;
	GENfile.close();
	Schdulefile.close();
	Resultfile.close();
}

void CloseFinal(){
	Finalfile.close();
}