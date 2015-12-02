#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "WSNFile.h"
#include "WSNStruct.h"
#include "WSNEnergy.h"
#include "../Schedule/FlowSchedule.h"

#undef  _ShowLog

using namespace std;
static double miss_ratio=0;
static double total_latency=0;
static double total_meetlatency=0;
static double total_meetlatency_cnt=0;
static string Single_invterval;
static string Star_invterval;
static string Scheduler;
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
		�NGEN����ƨ��J �B
		�إ߿�X���
===========================*/
void CreateFile(float U, int Set, char* output_path){

	if(readsetting==1){
		PathSetting(output_path);
	}
	//=========================�}��GENFile
	filename="Rate";filename.append(to_string(U));filename.append("_Set");filename.append(to_string(Set));filename.append(".txt");
	string GENBuffer=GENPath;
	GENBuffer.append(filename);
	GENfile.open(GENBuffer, ios::in);	//�}���ɮ�.�g�J���A
	if(!GENfile){//�p�G�}���ɮץ��ѡAfp��0�F���\�Afp���D0
		cout<<"Fail to open file: "<<GENBuffer<<endl;
		system("PAUSE");
	}

	//========================�}��ScheduleFile
	string ScheduleFileBuffer=SchedulePath;
	ScheduleFileBuffer.append("Schedule_");
	ScheduleFileBuffer.append(filename);

	Schdulefile.open(ScheduleFileBuffer, ios::out);	//�}���ɮ�.�g�J���A
	if(!Schdulefile){//�p�G�}���ɮץ��ѡAfp��0�F���\�Afp���D0
		cout<<"Fail to open Schedule file: "<<ScheduleFileBuffer<<endl;		
		system("PAUSE");
	}

	//========================�}��Resultfile
	string ResultBuffer=ResultPath;
	ResultBuffer.append("Result_");
	ResultBuffer.append(filename);

	Resultfile.open(ResultBuffer, ios::out);	//�}���ɮ�.�g�J���A
	if(!Resultfile){//�p�G�}���ɮץ��ѡAfp��0�F���\�Afp���D0
		cout<<"Fail to open Result file: "<<ResultBuffer<<endl;
		system("PAUSE");
	}

	//========================�}��Finalfile
	string FinalBuffer=FinalPath;
	FinalBuffer.append("FinalResult.txt");
	if(Finalfile.is_open()==false){
		Finalfile.open(FinalBuffer, ios::out);	//�}���ɮ�.�g�J���A
		if(!Finalfile){//�p�G�}���ɮץ��ѡAfp��0�F���\�Afp���D0
			cout<<"Fail to open Final file: "<<FinalBuffer<<endl;
			system("PAUSE");
		}
	}
}

/*===========================
	��@taskset�x�s
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

	//---------------------------------------------------�s���U��node��T
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
	//--------------------------------------------�p��miss ratio
	double Recv_count=0;
	double Miss_count=0;
	double latency=0;
	double meetlatency=0, meetlatency_cnt=0;
	for(Packet* pkt=Head->nextnd->pkt; pkt!=NULL; pkt=pkt->nextpkt){
		Recv_count=Recv_count+Hyperperiod/pkt->period;
		Miss_count=Miss_count+pkt->Miss_count;
		latency=latency+pkt->latency;
		meetlatency=meetlatency+pkt->meetlatency;
		meetlatency_cnt=meetlatency_cnt+pkt->meetlatency_cnt;
	}
	Resultfile<<"Meet_ratio:"<<(1-Miss_count/Recv_count)<<endl;
	Resultfile<<"MissLatency_perpkt:"<<latency/Miss_count<<endl;
	Resultfile<<"MissLatency_time:"<<latency<<endl;
	Resultfile<<"MeetLatency_perpkt:"<<meetlatency/meetlatency_cnt<<endl;
	Resultfile<<"MeetLatency_time:"<<meetlatency<<endl;

	total_meetlatency=total_meetlatency+meetlatency;
	total_meetlatency_cnt=total_meetlatency_cnt+meetlatency_cnt;
	//cout<<"Meet ratio:"<<Miss_count/Recv_count<<endl;

	//--------------------------------------------�O�_meet
	//Cal Energy
	node=Head->nextnd;
	SetNode=SetHead->nextnd;
	while(node!=NULL){
		SetNode->avgenergy=SetNode->avgenergy+node->energy;

		SetNode=SetNode->nextnd;
		node=node->nextnd;
	}

	//Cal lifetime(���SetHead��)
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

		//Cal lifetime(���SetHead��)
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
		miss_ratio=miss_ratio+(Miss_count/Recv_count);	//
		total_latency=total_latency+(latency/Miss_count);
		
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
	���Q�βvSet�x�s
===========================*/
void SaveSet(float rate, int Set){
	cout<<"FinalResult"<<endl;
	cout<<"Meet="<<Meetcount<<endl;
	cout<<"Miss="<<Set-Meetcount<<endl;
	cout<<"MeetRatio="<<Meetcount/Set<<endl;
	SetNode=SetHead->nextnd;
	while(SetNode!=NULL){
		cout<<"Node"<<SetNode->id<<"="<<SetNode->avgenergy/Set<<endl;
		SetNode=SetNode->nextnd;
	}
	cout<<"Miss ratio="<<miss_ratio/(Set-Meetcount)<<endl;
	cout<<"Latency="<<total_latency/(Set-Meetcount)<<endl;
	cout<<"Lifetime="<<(SetHead->lifetime)/(Set)<<endl;
	cout<<"AverageEnergy="<<AverageE/Set<<endl;
	cout<<"=============================================="<<endl;

	Finalfile<<"Rate"<<rate<<endl;
	Finalfile<<"Meet="<<Meetcount<<endl;
	Finalfile<<"Miss="<<Set-Meetcount<<endl;
	Finalfile<<"SetAmount_MeetRatio="<<Meetcount/Set<<endl;
	SetNode=SetHead->nextnd;
	while(SetNode!=NULL){
		Finalfile<<"Node"<<SetNode->id<<"="<<SetNode->avgenergy/Set<<endl;
		SetNode=SetNode->nextnd;
	}
	Finalfile<<"Missratio_perset="<<miss_ratio/(Set-Meetcount)<<endl;
	Finalfile<<"MissLatency_perpkt="<<total_latency/(Set-Meetcount)<<endl;
	Finalfile<<"MeetLatency_perpkt="<<total_meetlatency/total_meetlatency_cnt<<endl;
	Finalfile<<"Lifetime="<<(SetHead->lifetime)/(Set)<<endl;
	Finalfile<<"AverageEnergy="<<AverageE/Set<<endl;
	Finalfile<<"=============================================="<<endl;
	
	total_latency=0;
	miss_ratio=0;
	total_meetlatency=0;
	total_meetlatency_cnt=0;
	GENfile.close();
	Schdulefile.close();
	Resultfile.close();
}

void CloseFinal(){
	Finalfile.close();
}

/*========================================
		Setting Resource file
		
		Setting GENFILE
		Setting 
========================================*/
void PathSetting(string outputpath){
	
	//============================================Find last {\\}
	int i=0, len=outputpath.length();
	for(i=len; outputpath[i]!='\\' ; i--){}
	SchedulePath=outputpath.assign(outputpath,0,i);
	SchedulePath.append("\\");
	FinalPath=SchedulePath;
	ResultPath=SchedulePath;

	cout<<"Output file: "<<SchedulePath<<endl;

	//============================================get setting file
	fstream Settingfile;
	string SettingBuffer=SchedulePath;
	SettingBuffer.append("\\Setting.txt");
	Settingfile.open(SettingBuffer, ios::in);	//�}���ɮ�.�g�J���A
	if(!Settingfile){//�p�G�}���ɮץ��ѡAfp��0�F���\�Afp���D0
		cout<<"Fail to open file: "<<SettingBuffer<<endl;
		system("PAUSE");
	}

	string str;
	Settingfile>>str; stream.clear();
	cout<<"Resource file: "<<str<<endl;
	GENPath=str.append("\\");

	Settingfile>>str; stream.clear();
	cout<<"Single node proposal: "<<str<<endl;
	Single_invterval=str;

	Settingfile>>str; stream.clear();
	cout<<"Star proposal: "<<str<<endl;
	Star_invterval=str;

	Settingfile>>str; stream.clear();
	cout<<"Schedule : "<<str<<endl;
	Scheduler=str;

	Settingfile.close();
}

void ExperimentSetting(short int*S_inv, short int*Star_inv, short int*Sche){
	//==========================Single Node setting
	if(Single_invterval.compare("MEI")==0){
		*S_inv=1;
	}else if(Single_invterval.compare("DIF")==0){
		*S_inv=2;
	}else if(Single_invterval.compare("Lazy")==0){
		*S_inv=3;
	}else if(Single_invterval.compare("Static")==0){
		*S_inv=4;
	}else{
		printf("Error No Single interval proposal\n");
		system("PAUSE");
	}

	//==========================Star interval setting
	if(Star_invterval.compare("EIMA")==0){
		*Star_inv=2;
	}else if(Star_invterval.compare("LDC")==0){
		*Star_inv=0;
	}else if(Star_invterval.compare("Static")==0){
		*Star_inv=1;
	}else{
		printf("Error No Star efficiency proposal\n");
		system("PAUSE");
	}

	//==========================Scheduler setting
	if(Scheduler.compare("EIF")==0){
		*Sche=2;
	}else if(Scheduler.compare("NPEDF")==0){
		*Sche=0;
	}else if(Scheduler.compare("Polling")==0){
		*Sche=3;
	}else if (Scheduler.compare("Table")==0){
		*Sche=1;
	}else{
		printf("Error No write-request proposal\n");
		system("PAUSE");
	}
}

