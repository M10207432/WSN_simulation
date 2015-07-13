#ifndef CONNINTERVAL_H
#define CONNINTERVAL_H

extern double Connectioninterval;
extern double DIFMinperiod;
extern short int Rateproposal;

class ConnectionInterval{
	public:
		ConnectionInterval();
		void Event();
		void TSB();
		void DIF();
		void Rate_TO_Interval(int );
		void ConnAlgorithm(int);
};

#endif