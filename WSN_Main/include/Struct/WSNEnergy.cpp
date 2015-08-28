#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "WSNFile.h"
#include "WSNStruct.h"
#include "../Schedule/FlowSchedule.h"
#include "WSNEnergy.h"

using namespace std;

/*==========================================
		分別計算各個node上的Power
		每一slot上做加總
		即為E=P*t
==========================================*/
void NodeEnergy(){
	/*----------------------------
		Wakeup Sleep	  未連接情況
		Transmission Idle 連接情況
	----------------------------*/
	Node *Enode;
	Packet *Epkt;
	
	Enode=Head->nextnd;
	while(Enode!=NULL){
		Enode->LatestTimeslot=Timeslot;
		Enode=Enode->nextnd;
	}

	Enode=Head->nextnd;
	while(Enode!=NULL){
		//==========================Sleep 狀態
		if(Enode->State=="Sleep"){
			double P=0;
			P=Vcc*I_sleep;
			Enode->energy=Enode->energy+P;
			Enode->State="Sleep";
		}
		//==========================Transmission 狀態
		if(Enode->State=="Transmission"){
			double P=0;

			//P=IntervalPower(Pktsize,Enode->LatestTimeslot-Enode->ExTimeslot);
			
			//cout<<"Interval:"<<(Enode->LatestTimeslot-Enode->ExTimeslot)<<" E:"<<Enode->energy<<endl;
			//Enode->ExTimeslot=Enode->LatestTimeslot;//紀錄此次發送時間
			P=Vcc*I_notify;
			Enode->energy=Enode->energy+P;
			Enode->State="Sleep";
			
		}
		//==========================Scan 狀態
		if(Enode->State=="Scan"){
			
		}

		Enode=Enode->nextnd;
	}

}
/*==========================================
		計算在此Connection interval下的
			Power consumption
==========================================*/
double IntervalPower(int Pktnum,int interval){
	if(interval!=0){
		double Ipeak,Vpeak;
		double Tc;
		double power;

		Tc=interval*unit;
		
		//Ipeak=(Pktnum*Ie*Te+Isleep*(Tc-Te*Pktnum))/Tc;
		Ipeak=parma*Pktnum*exp(-parmb*Tc)+I_sleep;
		
		Vpeak=Vcc-Ipeak;

		power=Vcc*Ipeak-(Ipeak*Ipeak)*K;
	
		//cout<<"T:"<<Timeslot<<" P:"<<power<<endl;

		return power;
	}else
		return 0;
}

/*==========================================
		計算Node State
==========================================*/

void NodeState(){
	Node   *Snode;
	Packet *Spacket;
	bool tx_flag=false;

	Snode=Head->nextnd;
	
	while(Snode!=NULL){
		Spacket=Snode->pkt;
		while(Spacket!=NULL){
			if(Spacket->State=="Transmission"){
				Snode->State="Transmission";
				tx_flag=true;
			}
			if(Spacket->State=="Idle" && tx_flag!=true){
				Snode->State="Idle";
			}

			Spacket=Spacket->nodenextpkt;
		}
		Snode=Snode->nextnd;
		tx_flag=false;
	}
	delete Snode;
	delete Spacket;
}

/*========================================
		對當前的Node做Energy上的計算
		且當buffer淨空，立即切換State
========================================*/

void Node_EnergyState(Node *node){

	if(node!=NULL){
		//==========================Sleep 狀態
		if(node->State=="Sleep"){
			double P=0;
			P=Vcc*I_sleep;
			node->energy=node->energy+P;
			node->State="Sleep";
		}

		//==========================Transmission 狀態
		if(node->State=="Transmission"){
			double P=0;

			//P=IntervalPower(Pktsize,Enode->LatestTimeslot-Enode->ExTimeslot);
			
			//cout<<"Interval:"<<(Enode->LatestTimeslot-Enode->ExTimeslot)<<" E:"<<Enode->energy<<endl;
			//Enode->ExTimeslot=Enode->LatestTimeslot;//紀錄此次發送時間
			P=Vcc*I_notify;
			node->energy=node->energy+P;

			if(node->NodeBuffer->load==0)
				node->State="Sleep";
		}

		//==========================Scan 狀態
		if(node->State=="Scan"){
			
		}

	}
}