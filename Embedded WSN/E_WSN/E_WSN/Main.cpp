#include <iostream>
#include "stdio.h"
#include "IRQ.h"
#include <time.h>

#include <tchar.h>
#include <windows.h>
#include <assert.h>

using namespace std;

void ConstChange();
void SUM_sign_unsign();



int main(){
	DCB dcb={0};
	TCHAR *PortName = TEXT("\\\\.\\COM31");//?
	HANDLE hComm;
	BOOL Success_f;
	
	char RXBuffer;
	DWORD dwNoByte;

	BuildCommDCB("9600,n,8,1",&dcb);
	hComm=CreateFile(PortName,
					GENERIC_READ | GENERIC_WRITE,
					0,
					0,
					OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL,
					0);


	if(hComm == INVALID_HANDLE_VALUE){
		printf("Error Open Port\n");
		system("PAUSE");
		return 0;
	}else{
		printf("Open Port!!!\n");
	}

	SetCommState(hComm, &dcb);
	SetupComm(hComm,2048,1024);

	while(1){
		ReadFile(hComm, &RXBuffer, 1 ,&dwNoByte, NULL);
		printf("V:%c\n",RXBuffer);
	}

	/*
	if(WaitCommEvent(hComm, &dwRead, &osReader)){
		printf("Value:%d\n",dwRead);
	}else{
		printf("Nothng\n");
	}
	*/
	/*
	// Init DCB struct
	SecureZeroMemory(&dcb, sizeof(DCB));
	dcb.DCBlength=sizeof(DCB);
	Success_f=GetCommState(hComm, &dcb);

	if(!Success_f){
		printf("DCB error\n");
		system("PAUSE");
		return 0;
	}

	_tprintf("BaudRate = %d, ByteSize = %d, Parity = %d, StopBits = %d",
		dcb.BaudRate, dcb.ByteSize, dcb.Parity, dcb.StopBits);
	*/


	clock_t t;
	int f=clock();
	
	//IRQ irq1("22");
	//irq1.GetPortName();
	
	printf("Hello\n");

	printf("Time: %d\n",clock()-f);
	/*================
		Serial port
	================*/
	//CSerial BTserial;
	//BTserial.Open(_T("COM9"));

	system("PAUSE");
	return 0;	
}

/*=============================
		變Const變數
=============================*/
void ConstChange(){
	int v1=10;
	static const int *Con_value=&v1;

	printf("Constant value=%d\n",*Con_value);

	int * Chg_v=(int *)Con_value;
	*Chg_v=1;

	printf("Change value=%d\n",*Chg_v);
}

/*=============================
			SWAP
	(without any tmp variable)
=============================*/
int swap(bool sw,int v1, int v2){
	v1=v1+v2;
	v2=v1-2*v2;
	v1=(v1-v2)/2;
	v2=v2+v1;

	if (sw==1)
		return v1; 
	if (sw==2)
		return v2;
}

/*=============================
		Unsigned & sign

=============================*/
void SUM_sign_unsign(){
	unsigned int a =6;
	int b=-20;

	cout<<a+b<<endl;	//b會變成無號數
	if(a+b >0)
		printf("Bigger\n");
	else
		printf("Small\n");
}