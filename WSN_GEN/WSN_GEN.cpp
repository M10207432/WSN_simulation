#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include <iostream>
#include<fstream>
#include <string.h>
#include <string>
using namespace std;

/*=================================
		Structure
==================================*/
struct Node{
	double coor_x,coor_y;//�y��
	double distanceto_BS;//��Base station �Z��
	double energy;
	double radus;
	short int hop;		//range 1~3

	struct Packet* pkt;
	struct Node* nextnd;
	struct Node* prend;
};
struct Packet{
	int id;
	int load;
	double period;
	double utilization;
	int	 pirority;

	short int hop;		//range 1~3
	int destination;	//Nest node
	double rate;

	struct Packet* nextpkt;
	struct Packet* prepkt;
};
void create();
/*=================================
		Global value
==================================*/
short int Max_X_Axis = 100;
short int Max_Y_Axis = 100;
int Level1_Nodenum = 2;
int Level2_Nodenum = 1;
int pktnum=10;
int nodenum=Level1_Nodenum+Level2_Nodenum;

const double leastperiod=100;
const double largestperiod=1000;
const double Hyperperiod=1000;
const float MIN_Uti=1.0;
const float MAX_Uti=5.0;
const short int Set=50;

string GENfile="..\\GENresult\\";//���e�@�ؿ��U��GENresult�ؿ��A����txt��
char Resultfile[]="..\\GENresult\\WSNGEN.txt";//���e�@�ؿ��U��GENresult�ؿ��A����txt��
float Total_Uti=1.0;
const double Max_pktuti=0.9;
double tmp_totaluti=0;
int Gobackcount=0;
Node* HEAD=new Node;			//�`�YHEAD
Node* node=new Node;
Packet* packet=new Packet;

int main(void){
	
	srand(time(NULL));			//�H���ؤl
	
	for(float U=MIN_Uti;U<=MAX_Uti;U++){
		Total_Uti=U;

		//��JGEN���ɦW
		string filename="U";
		filename.append(to_string(Total_Uti));
		filename.append("_Set");
		filename.append(to_string(Set));
		filename.append(".txt");
		cout<<filename<<endl;

		//��JGEN�� ���|+�ɦW
		string GENBuffer=GENfile;
		GENBuffer.append(filename);

		char *GENbuffer=(char*)GENBuffer.c_str();
	
		fstream fp;
		fp.open(GENbuffer, ios::out);	//�}���ɮ�.�g�J���A
		if(!fp){//�p�G�}���ɮץ��ѡAfp��0�F���\�Afp���D0
			cout<<"Fail to open file: "<<GENbuffer<<endl;
		}

		/*==================================================
						GEN U �� Set��
		==================================================*/
		for(int setcount=0;setcount<Set;setcount++){

			/*==================================================
							Create Node & Packet
			==================================================*/
			create();

			/*==================================================
							�g�J��T TXT
			==================================================*/
			double tmpu=0;
			Packet* tmppkt=new Packet;
			Node* tmpnode=new Node;
			tmpnode=HEAD->nextnd;
			int nodeid=1;
			int packetid=1;
			fp<<nodenum<<" "<<pktnum<<" "<<Hyperperiod<<endl;
			while(tmpnode!=NULL){
				tmppkt=tmpnode->pkt;

				fp<<"Node"<<" ";
				fp<<tmpnode->coor_x<<" "<<tmpnode->coor_y<<" "<<tmpnode->radus<<endl;
					
				while(tmppkt!=NULL){
					fp<<"Pkt"<<" ";
					fp<<tmppkt->load<<" ";
					fp<<tmppkt->period<<" ";
					fp<<tmppkt->utilization<<" ";
					fp<<tmpnode->hop<<endl;//fp<<tmppkt->hop<<endl;

					cout<<tmppkt->load<<endl;
					cout<<tmppkt->period<<endl;
					tmpu=tmpu+tmppkt->utilization;

					tmppkt=tmppkt->nextpkt;
				}
				packetid=1;
				if(tmpnode!=NULL){
					tmpnode=tmpnode->nextnd;
				}
			}
			cout<<"Totaluti:"<<tmp_totaluti<<endl;
			fp<<"Totaluti:"<<tmp_totaluti<<endl;
			cout<<"=========="<<endl;
			fp<<"=========="<<endl;
		}//============================================Set end

		fp.close();
		filename.clear();
		
	}
	system("PAUSE");
	return 0;
}

void create(){
	bool Done_flag=false;
	delete HEAD;delete node;delete packet;
	HEAD=new Node();
	node=new Node();
	packet=new Packet();

	/*==================================================
					�إ�Link list
					Node & Packet
	==================================================*/
	/*-------------------------
		Gen node(Linklist)
	-------------------------*/
	
	HEAD->nextnd=node;
	node->prend=HEAD;
	for(int n=0;n<nodenum;n++){
		/*-------------------------
			Gen packet(Linklist)
		-------------------------*/
		packet=new Packet;
		node->pkt=packet;
		packet->prepkt=NULL;
		for (int p=0;p<pktnum;p++){

			Packet* nextpacket=new Packet;
			Packet* prepacket=packet;
			packet->nextpkt=nextpacket;
			packet=nextpacket;
			packet->prepkt=prepacket;
			
		}

		packet=packet->prepkt;
		packet->nextpkt=NULL;

		//--------------------------Packet Done

		Node* nextnode=new Node;
		Node* prenode=node;
		node->nextnd=nextnode;
		node=nextnode;
		node->prend=prenode;
	}
	node=node->prend;
	node->nextnd=NULL;
	//-----------------------------Node Done

	while(Done_flag!=true){
		/*==================================================
						���t�U�ʥ]Period
						�n�ŦXHyperperiod
		==================================================*/
		tmp_totaluti=0;
		node=HEAD->nextnd;
		double tmp_packetperiod=0;//�nrand���ʥ]period

		while(node!=NULL){
			packet=node->pkt;
			while(packet!=NULL){
			
				int periodrange=(largestperiod-leastperiod);	//�nRand���d��
				tmp_packetperiod=rand()%periodrange+leastperiod;							//Rand�Xpacket��period��� largestperiod~leastperiod
				while((int(Hyperperiod)%int(tmp_packetperiod))!=0){
					tmp_packetperiod=rand()%periodrange+leastperiod;						//Rand�Xpacket��period��� largestperiod~leastperiod
				}
				packet->period=double(tmp_packetperiod);										//��J�Q�βv

				packet=packet->nextpkt;
			}
			node=node->nextnd;
		}
		/*==================================================
						���t�U�ʥ]�Q�βv
		==================================================*/
		node=HEAD->nextnd;
		double avg_packetuti=Total_Uti/(nodenum*pktnum);
		double tmp_pktuti=0.0;
		double totalpacket=nodenum*pktnum;
		int rangetoavg_pktuti=nodenum*pktnum*avg_packetuti;	//Gen���d�� 0~avg_packetuti
		int rangetomax_pktuti=nodenum*pktnum*Max_pktuti;	//Gen���d�� 0~Max_pktuti
		double tmpcount=1;
		bool averageuti_flag=false;
	
		while(node!=NULL){
			packet=node->pkt;
			while(packet!=NULL){
			
				/*-------------------
					Rand Uti
				-------------------*/
				if(tmpcount*avg_packetuti < tmp_totaluti){
					if(rangetoavg_pktuti!=0)
						tmp_pktuti=((rand()%rangetoavg_pktuti)+1)/totalpacket;//0~avg_packetuti
					else
						tmp_pktuti=((rand()%1)+1)/totalpacket;
					averageuti_flag=false;
				}
				else{
					averageuti_flag=true;
					tmp_pktuti=((rand()%rangetomax_pktuti)+1)/totalpacket;//0~Max_pktuti
				}

				//�}�ҥ����Q�βv�p��
				if(averageuti_flag==true){
					if(((nodenum*pktnum)-tmpcount)!=0){
						avg_packetuti=(Total_Uti-tmp_totaluti)/((nodenum*pktnum)-tmpcount);
						rangetoavg_pktuti=nodenum*pktnum*avg_packetuti;
					}
				}

				//��Jpacket uti��
				tmpcount++;
				tmp_totaluti=tmp_totaluti+tmp_pktuti;
				packet->utilization=tmp_pktuti;
				packet=packet->nextpkt;
			
			}
			node=node->nextnd;
		}
		/*==================================================
						���������`�Q�βv
		==================================================*/
		//-------------------------------------�Q�βv����
		node=HEAD->nextnd;
		packet=node->pkt;
		if(tmp_totaluti<Total_Uti){
			double remainuti=Total_Uti-tmp_totaluti;
		
			//�ܴ�pkt�Q�βv
			while(packet!=NULL){
				if(packet->utilization+remainuti>1)
					packet=packet->nextpkt;
				else{
					packet->utilization=packet->utilization+remainuti;
					tmp_totaluti=tmp_totaluti+remainuti;
					break;
				}
			}
		}
		//-------------------------------------�Q�βv�W�L
		node=HEAD->nextnd;
		packet=node->pkt;
		if(tmp_totaluti>Total_Uti){
			int adjustnum=0;
			double adjuti=0;
			double remainuti=tmp_totaluti-Total_Uti;
		
			//�p����ܴ���pkt�ƶq
			while(remainuti>=0){
				adjustnum++;
				remainuti=remainuti-packet->utilization;
				tmp_totaluti=tmp_totaluti-packet->utilization;
				packet->utilization=0;
				packet=packet->nextpkt;
				if(packet==NULL){
					node=node->nextnd;
					packet=node->pkt;
				}
			}
			//�p�⥭���Q�βv
			adjuti=(Total_Uti-tmp_totaluti)/(adjustnum);
			node=HEAD->nextnd;
			packet=node->pkt;
			while(packet->utilization==0){
				tmp_totaluti=tmp_totaluti+adjuti;
				packet->utilization=adjuti;
				packet=packet->nextpkt;
				if(packet==NULL){
					node=node->nextnd;
					packet=node->pkt;
				}
			}
		}
		/*==================================================
						�̷�Packet����
					Utilization & Period�h��Xload
				  (Done_flag�|�P�w�O�_��load��0�����p)
		==================================================*/
		Done_flag=true;
		node=HEAD->nextnd;
		while(node!=NULL){
			packet=node->pkt;
			while(packet!=NULL){
				packet->load=packet->utilization * packet->period;
				if(packet->load==0)
					Done_flag=false;
				if(tmp_totaluti!=Total_Uti)
					Done_flag=false;
				packet=packet->nextpkt;
			}
			node=node->nextnd;
		}
	}

	/*==================================================
					���t�U�`�I��m
	==================================================*/
	node=HEAD->nextnd;
	int tmplevel1=Level1_Nodenum;
	int tmplevel2=Level2_Nodenum;
	double ceter_x=Max_X_Axis/2;
	double ceter_y=Max_Y_Axis/2;
	double R=-1;

	while(node!=NULL){
		if(tmplevel1){
			R=-1;
			while(!(R<=Max_X_Axis/2 && R>0)){
				node->coor_x=(rand()%Max_X_Axis/2)+Max_X_Axis/4; 
				node->coor_y=(rand()%Max_Y_Axis/2)+Max_Y_Axis/4;
				
				R=sqrt(pow((ceter_x-node->coor_x),2)+pow((ceter_y-node->coor_y),2));
			}
			node->radus=R;
			node->hop=1;

			tmplevel1--;
		}else if(tmplevel2){
			R=-1;
			while(!(R>Max_X_Axis/2)){
				node->coor_x=(rand()%Max_X_Axis);
				node->coor_y=(rand()%Max_Y_Axis);

				R=sqrt(pow((ceter_x-node->coor_x),2)+pow((ceter_y-node->coor_y),2));
			}
			node->radus=R;
			node->hop=2;

			tmplevel2--;
		}

		cout<<"Node Address: "<<node->coor_x<<" "<<node->coor_y<<endl;
		node=node->nextnd;
	}
}