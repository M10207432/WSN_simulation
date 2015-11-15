#ifndef CONNINTERVAL_H
#define CONNINTERVAL_H

extern double Connectioninterval;
extern double DIFMinperiod;
extern short int Rateproposal;
extern double timeslot;

class EventInterval{
	public:
		EventInterval();
		void Event();
		void MEI(Node *);
		void DIF();
		void Rate_TO_Interval(int );

		void Algorithm(int);
		void Interval_TDMA_Algorithm(int);

		void EIMA();
		void EIMA_2();
		void IntervalDivide();

		void ConnectionPriority();
		void IntervalReassign();
};

#endif