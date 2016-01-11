#ifndef CONNINTERVAL_H
#define CONNINTERVAL_H

extern double Connectioninterval;
extern short int Rateproposal;
extern double Minumum_interval;
extern bool EIMADemand_flag;
extern double IOS_ServiceInterval;
extern double IOS_ConnectionInterval;

class EventInterval{
	public:
		EventInterval();
		
		void ServiceInterval_Algorithm(int);		//選Single node方法
		void ConnectionInterval_Algorithm(int);		//選Muliple node方法

		/*------------------------------
			Service interval計算
			(Single node)
		------------------------------*/
		void Event();						//每一node connection interval 都為 10ms
		void MEI(Node *);					//用Demand bound計算 service interval
		void DIF();							//用各個pkt區間計算與load計算rate給pkt，會再轉換成service interval
		void Greedy();						//用最短minimum period當作service interval
		void Single_IOS(double);

		/*------------------------------
			Connection interval計算
			(Muliple nodes network)
			(一定要給FrameTbl)
		------------------------------*/
		void EIMA();						//用avg current作為weight，分配connection interval
		void LDC();							//各個service interval除上node1level作為weight，分配connection interval
		void IntervalDivide();				//用minimum service interval除上node1level作為weight，分配connection interval
		void Multiple_IOS(double);

		/*------------------------------
				Others
		------------------------------*/
		void Rate_TO_Interval(int );
		void IntervalReassign();
};

#endif