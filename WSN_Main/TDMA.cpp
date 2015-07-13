#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "WSNFile.h"
#include "WSNStruct.h"
#include "ConnInterval.h"
#include "TDMA.h"

using namespace std;

TDMA::TDMA(){
	printf("TDMA Obj\n");
}
/*===========================
		WSN拓鋪
===========================*/
void TDMA::Topology(){
	Node *TNode=Head->nextnd;
	double distance=100;

	/*---------------------
		找最短距離上的
		相連接
	---------------------*/
	while(TNode!=NULL){
		//--------------------------Level 1
		if(TNode->hop==1){
			TNode->SendNode=Head;
		}

		//--------------------------Level 2~
		if(TNode->hop!=1){
			Node *tmp_TNode=Head->nextnd;
			distance=100;
			while(tmp_TNode!=NULL){
				if(tmp_TNode->hop==TNode->hop-1){
					if(distance>sqrt(pow(TNode->coor_x-tmp_TNode->coor_x,2)+pow(TNode->coor_y-tmp_TNode->coor_y,2))){
						distance=sqrt(pow(TNode->coor_x-tmp_TNode->coor_x,2)+pow(TNode->coor_y-tmp_TNode->coor_y,2));
						TNode->SendNode=tmp_TNode;
					}
				}
				tmp_TNode=tmp_TNode->nextnd;
			}
		}

		TNode=TNode->nextnd;
	}

	/*---------------------
		Graph Conflict
	---------------------*/
	//=====================建立Edge
	Edge *tmpedge;
	HeadEdge=MainEdge;
	node=Head->nextnd;
	while(node!=NULL){
		tmpedge=new Edge;
		MainEdge->n1=node;
		MainEdge->n2=node->SendNode;
		
		//建立Child node
		if(node->SendNode->ChildNode==NULL){
			node->SendNode->ChildNode=node;
			node->SendNode->ChildNode->next_child=NULL;
		}else{
			Node* tmp_child=node->SendNode->ChildNode;
			while(tmp_child->next_child!=NULL){
				tmp_child=tmp_child->next_child;
			}
			tmp_child->next_child=node;
			tmp_child=tmp_child->next_child;
			tmp_child->next_child=NULL;
		}

		MainEdge->next_edge=tmpedge;
		tmpedge->pre_edge=MainEdge;
		MainEdge=tmpedge;

		node=node->nextnd;
	}
	MainEdge->next_edge=NULL;

	//=====================建立Conflict Edge
	//ConflictEdge
	Edge *MainConflictEdge=ConflictEdge;
	Node *Childchild_node;
	node=Head->nextnd;
	while(node!=NULL){

		//---------------------------------------------------SendNode
		ConflictEdge->n1=node;
		ConflictEdge->n2=node->SendNode;
		tmpedge=new Edge;
		ConflictEdge->next_edge=tmpedge;
		tmpedge->pre_edge=ConflictEdge;
		ConflictEdge=tmpedge;
		//------SendNode's edge1(Send)
		if(node->SendNode!=Head){
			ConflictEdge->n1=node;
			ConflictEdge->n2=node->SendNode->SendNode;
			tmpedge=new Edge;
			ConflictEdge->next_edge=tmpedge;
			tmpedge->pre_edge=ConflictEdge;
			ConflictEdge=tmpedge;
		}
		//------SendNode's edge2~(Child)
		Childchild_node=node->SendNode->ChildNode;
		if(Childchild_node==node)
				Childchild_node=Childchild_node->next_child;
		while(Childchild_node!=NULL){
			ConflictEdge->n1=node;
			ConflictEdge->n2=Childchild_node;
			tmpedge=new Edge;
			ConflictEdge->next_edge=tmpedge;
			tmpedge->pre_edge=ConflictEdge;
			ConflictEdge=tmpedge;

			Childchild_node=Childchild_node->next_child;
			if(Childchild_node==node)
				Childchild_node=Childchild_node->next_child;
		}

		//---------------------------------------------------ChildNode
		Node* Children=node->ChildNode;
		while(Children!=NULL){
			ConflictEdge->n1=node;
			ConflictEdge->n2=Children;
			tmpedge=new Edge;
			ConflictEdge->next_edge=tmpedge;
			tmpedge->pre_edge=ConflictEdge;
			ConflictEdge=tmpedge;
			//------ChildNode's edge2(Child)
			if(Children!=NULL){
				Childchild_node=Children->ChildNode;
				while(Childchild_node!=NULL){
					ConflictEdge->n1=node;
					ConflictEdge->n2=Childchild_node;
					tmpedge=new Edge;
					ConflictEdge->next_edge=tmpedge;
					tmpedge->pre_edge=ConflictEdge;
					ConflictEdge=tmpedge;

					Childchild_node=Childchild_node->next_child;
				}
			}

			Children=Children->next_child;
		}

		node=node->nextnd;
	}
	ConflictEdge->pre_edge->next_edge=NULL;
	ConflictEdge=MainConflictEdge;
}

/*========================
	各自節點的顏色
========================*/
void TDMA::NodeColoring(){
	Edge *N_CEdge=ConflictEdge;
	Edge *AssignEdge;
	Node *AssignNode;
	bool Assign_flag=false;
	int colorid=1;

	/*--------------------
		edge計算 & Init
	--------------------*/
	node=N_CEdge->n1;
	while(N_CEdge!=NULL){
		
		if(node==N_CEdge->n1){
			node->edge=node->edge+1;
		}else{
			node=N_CEdge->n1;
			node->edge=node->edge+1;
		}
			
		N_CEdge=N_CEdge->next_edge;
	}
	
	node=Head->nextnd;
	while(node!=NULL){
		node->order_flag=false;
		node->color=0;
		node=node->nextnd;
	}

	/*--------------------
		先將color assign
		給最大的edge
	--------------------*/
	while(!Assign_flag){
		//--------------------------------找出最大edge的node，為AssignNode 
		AssignNode=Head->nextnd;
		node=Head->nextnd;
		//先找出還未assign的node
		while(node!=NULL){
			if(node->order_flag==false){
				AssignNode=node;
				break;
			}
			node=node->nextnd;
		}

		node=Head->nextnd;
		while(node!=NULL){
			if(node->edge > AssignNode->edge && node->order_flag==false){
				AssignNode=node;
			}
			node=node->nextnd;
		}

		//--------------------------------給AssignNode color
		N_CEdge=ConflictEdge;
		while(N_CEdge->n1 !=AssignNode)
			N_CEdge=N_CEdge->next_edge;
		AssignEdge=N_CEdge;				//AssignNode在ConflictEdge中的第一個node
	
		colorid=1;
		AssignNode->color=colorid;
		node=Head->nextnd;
		bool research_flag=false;
		while(node!=NULL){

			if(node->color == colorid){	//相同顏色的---(1)
				N_CEdge=AssignEdge;				
				while(N_CEdge!=NULL){				
					if(N_CEdge->n1==AssignNode && N_CEdge->n2==node ){//碰撞的---(2)
						colorid++;
						research_flag=true;	//若有相同顏色且碰撞的node，則需要重新全部node比較一次
					}
					N_CEdge=N_CEdge->next_edge;
				}
			}
			if(research_flag){
				node=Head;
				research_flag=false;
			}
			node=node->nextnd;
		}
		AssignNode->order_flag=true;
		AssignNode->color=colorid;

		//--------------------------------判斷是否全部以搜尋完畢
		Assign_flag=true;
		node=Head->nextnd;
		while(node!=NULL){
			if(node->color==0)
				Assign_flag=false;
			node=node->nextnd;
		}
	}
}

/*========================
	TDMA Schedule
	0->自己的方法，只取一個superslot
	1->已傳到AP為主，所以有接收到的節點需再加入superslot 
========================*/
void TDMA::TDMA_Assignment(int method){
	delete TDMA_Tbl;TDMA_Tbl=NULL;
	TDMA_Tbl=new TDMATable;
	node=Head->nextnd;
	int colorid=1;		//等同於slot time
	TDMATable *tmp_tbl;
	TDMATable *MainTable=TDMA_Tbl;
	Edge *N_CEdge=ConflictEdge;
	int Maxcolor=0;

	//----------------------------------------先找出最大color id
	node=Head->nextnd;
	while(node!=NULL){
		if(node->color > Maxcolor)
			Maxcolor=node->color;
		node=node->nextnd;
	}

	//-----------------------------------先分配相同顏色的node 在同一slot上 <只有單一superslot>
	while(Maxcolor>=colorid){

		node=Head->nextnd;
		
		while(node!=NULL){
			if(node->color == colorid){
				//看是否傳給其他node
				if(node->SendNode==Head){
					node->TDMArecv_flag=0;
				}else{
					node->SendNode->TDMArecv_flag=1;
				}

				//assign pair{slot,node}
				TDMA_Tbl->slot=colorid;
				TDMA_Tbl->n1=node;

				tmp_tbl=new TDMATable;
				TDMA_Tbl->next_tbl=tmp_tbl;
				tmp_tbl->pre_tbl=TDMA_Tbl;
				TDMA_Tbl=tmp_tbl;
			}

			node=node->nextnd;
		}
		
		colorid++;
		node=Head->nextnd;
	}

	/*===========================================
				看是否要接續第二個superslot
	===========================================*/
	int Mincolor;
	bool Doneflag=false;

	switch(method){

	case 1:
		//-----------------------------------加上接續的superslot，讓每一封包適時的傳到BS (針對2~ hop的節點)	
		do{
			//----------------------先找最小color
			Mincolor=colorid+1;
			node=Head->nextnd;
			while(node!=NULL){
				if(node->TDMArecv_flag==1 && node->color<Mincolor){
					Mincolor=node->color;
				}
				node=node->nextnd;
			}
	
			//----------------------Assign TDMA {slot,node}
			node=Head->nextnd;
			while(node!=NULL){
				if(node->TDMArecv_flag==1 && node->color==Mincolor){
			
					//assign pair{slot,node}
					TDMA_Tbl->slot=colorid;
					TDMA_Tbl->n1=node;

					tmp_tbl=new TDMATable;
					TDMA_Tbl->next_tbl=tmp_tbl;
					tmp_tbl->pre_tbl=TDMA_Tbl;
					TDMA_Tbl=tmp_tbl;

					node->TDMArecv_flag=0;
				}
				node=node->nextnd;
			}
			colorid++;

			//----------------------Check all node flag is 0
			Doneflag=false;
			node=Head->nextnd;
			while(node!=NULL){
				if(node->TDMArecv_flag==1){
					Doneflag=true;
				}
				node=node->nextnd;
			}

		}while(Doneflag);
		break;

	default:
		break;
	}

	TDMA_Tbl=TDMA_Tbl->pre_tbl;
	TDMA_Tbl->next_tbl=NULL;

	TDMA_Tbl=MainTable;
}