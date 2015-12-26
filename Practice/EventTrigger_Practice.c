/**************************************************************************************************
  Filename:       simpleBLEPeripheral.c
  Revised:        $Date: 2010-08-06 08:56:11 -0700 (Fri, 06 Aug 2010) $
  Revision:       $Revision: 23333 $

  Description:    This file contains the Simple BLE Peripheral sample application
                  for use with the CC2540 Bluetooth Low Energy Protocol Stack.

  Copyright 2010 - 2013 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED “AS IS?WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
**************************************************************************************************/

/*********************************************************************
 * INCLUDES
 */

#include "bcomdef.h"
#include "OSAL.h"
#include "OSAL_PwrMgr.h"

#include "OnBoard.h"
#include "hal_adc.h"
#include "hal_led.h"
#include "hal_key.h"
#include "hal_lcd.h"

#include "gatt.h"

#include "hci.h"

#include "gapgattserver.h"
#include "gattservapp.h"
#include "devinfoservice.h"
#include "simpleGATTprofile.h"

#if defined( CC2540_MINIDK )
  #include "simplekeys.h"
#endif

#if defined ( PLUS_BROADCASTER )
  #include "peripheralBroadcaster.h"
#else
  #include "peripheral.h"
#endif

#include "gapbondmgr.h"

#include "simpleBLEPeripheral.h"

#if defined FEATURE_OAD
  #include "oad.h"
  #include "oad_target.h"
#endif

#include "central.h"
#include "ll.h"
/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

// How often to perform periodic event
#define SBP_PERIODIC_EVT_PERIOD                   1000

// What is the advertising interval when device is discoverable (units of 625us, 160=100ms)
#define DEFAULT_ADVERTISING_INTERVAL          160

// Limited discoverable mode advertises for 30.72s, and then stops
// General discoverable mode advertises indefinitely

#if defined ( CC2540_MINIDK )
#define DEFAULT_DISCOVERABLE_MODE             GAP_ADTYPE_FLAGS_LIMITED
#else
#define DEFAULT_DISCOVERABLE_MODE             GAP_ADTYPE_FLAGS_GENERAL
#endif  // defined ( CC2540_MINIDK )

// Minimum connection interval (units of 1.25ms, 80=100ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_MIN_CONN_INTERVAL     8

// Maximum connection interval (units of 1.25ms, 800=1000ms) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_MAX_CONN_INTERVAL     8

// Slave latency to use if automatic parameter update request is enabled
#define DEFAULT_DESIRED_SLAVE_LATENCY         0

// Supervision timeout value (units of 10ms, 1000=10s) if automatic parameter update request is enabled
#define DEFAULT_DESIRED_CONN_TIMEOUT          1000

// Whether to enable automatic parameter update request when a connection is formed
#define DEFAULT_ENABLE_UPDATE_REQUEST         TRUE

// Connection Pause Peripheral time value (in seconds)
#define DEFAULT_CONN_PAUSE_PERIPHERAL         6

// Company Identifier: Texas Instruments Inc. (13)
#define TI_COMPANY_ID                         0x000D

#define INVALID_CONNHANDLE                    0xFFFF

// Length of bd addr as a string
#define B_ADDR_STR_LEN                        15

#if defined ( PLUS_BROADCASTER )
  #define ADV_IN_CONN_WAIT                    500 // delay 500 ms
#endif

#define MAX_CONNECTIONS                       3
#define DEFAULT_SCAN_DURATION                 4000
#define ROLE_CENTRAL                          1
#define ROLE_PERIPHERAL                       2
// TRUE to use high scan duty cycle when creating link
#define DEFAULT_LINK_HIGH_DUTY_CYCLE          FALSE
// TRUE to use white list when creating link
#define DEFAULT_LINK_WHITE_LIST               FALSE
// Central Application states
enum
{
  BLE_STATE_IDLE,
  BLE_STATE_CONNECTING,
  BLE_STATE_CONNECTED,
  BLE_STATE_DISCONNECTING,
  BLE_STATE_ADVERTISING 
};
/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */
// Connection handle of current connection 
static uint16 simpleBLEConnHandle = GAP_CONNHANDLE_INIT;
static bool simpleBLEDoWrite = FALSE;
static bool setpowerflag=FALSE;

//mode0->Advertising(slave), mode1->Discovery(master), mode2->tx power setting
static int center_mode=0;
static uint8 deviceRole = ROLE_PERIPHERAL;
static uint8 masterSlave_State = BLE_STATE_ADVERTISING;
uint16 recevdata;

static bool simple_status = FALSE;
static uint16 gapConnHandle;

static void simpleBLEPeripheral_HandleKeys( uint8 shift, uint8 keys );
static void simpleBLEPeripheralProcessGattMsg( gattMsgEvent_t *pMsg );

attHandleValueNoti_t noti;

#define SBP_BURST_EVT 0x0008 
int SBP_BURST_EVT_PERIOD= 8;
uint8 RecvInterval=0x08;
uint8 testmode=0;
static void sendData( void );
int packetnum=1;

/*********************************************************************
 * Packet Information

 Preiod unit 1ms
 */
int TIMESLOT=0;			//uint SBP_PERIODIC_EVT_PERIOD
#define AMOUNT_OF_EVENT 3

typedef struct{
  short int SIZE;
  int PERIOD;
  int ARRIVAL_COUNT;
  
  short int EXESIZE;
  int DEADLINE;
  int ARRIVAL;
  short int findflag;
  short int  READYFLAG;
  short int  ID;
} Periodic_Event;

Periodic_Event PERIODIC_EVENT[AMOUNT_OF_EVENT];
static void PeriodicTask1(void);
static void PeriodicTask2(void);
static void PeriodicTask3(void);

static void BufferQueue(void);
short int Queue[AMOUNT_OF_EVENT];//Queue[priority]=EVENT_ID
static void SEND_PACKET(short int);
/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
static uint8 simpleBLEPeripheral_TaskID;   // Task ID for internal task/event processing

static gaprole_States_t gapProfileState = GAPROLE_INIT;

// GAP - SCAN RSP data (max size = 31 bytes)
static uint8 scanRspData[] =
{
  // complete name
  0x14,   // length of this data
  GAP_ADTYPE_LOCAL_NAME_COMPLETE,
  0x53,   // 'S'
  0x69,   // 'i'
  0x6d,   // 'm'
  0x70,   // 'p'
  0x6c,   // 'l'
  0x65,   // 'e'
  0x42,   // 'B'
  0x4c,   // 'L'
  0x45,   // 'E'
  0x50,   // 'P'
  0x65,   // 'e'
  0x72,   // 'r'
  0x69,   // 'i'
  0x70,   // 'p'
  0x68,   // 'h'
  0x65,   // 'e'
  0x72,   // 'r'
  0x61,   // 'a'
  0x6c,   // 'l'

  // connection interval range
  0x05,   // length of this data
  GAP_ADTYPE_SLAVE_CONN_INTERVAL_RANGE,
  LO_UINT16( DEFAULT_DESIRED_MIN_CONN_INTERVAL ),   // 100ms
  HI_UINT16( DEFAULT_DESIRED_MIN_CONN_INTERVAL ),
  LO_UINT16( DEFAULT_DESIRED_MAX_CONN_INTERVAL ),   // 1s
  HI_UINT16( DEFAULT_DESIRED_MAX_CONN_INTERVAL ),

  // Tx power level
  0x02,   // length of this data
  GAP_ADTYPE_POWER_LEVEL,
  0       // 0dBm
};

// GAP - Advertisement data (max size = 31 bytes, though this is
// best kept short to conserve power while advertisting)
static uint8 advertData[] =
{
  0x02,   // length of this data
  GAP_ADTYPE_FLAGS,
  DEFAULT_DISCOVERABLE_MODE | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,

  0x03,   // length of this data
  GAP_ADTYPE_16BIT_MORE,      // some of the UUID's, but not all
  LO_UINT16( SIMPLEPROFILE_SERV_UUID ),
  HI_UINT16( SIMPLEPROFILE_SERV_UUID ),

};

// GAP GATT Attributes
static uint8 attDeviceName[GAP_DEVICE_NAME_LEN] = "Simple BLE Peripheral";

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static void simpleBLEPeripheral_ProcessOSALMsg( osal_event_hdr_t *pMsg );
static void peripheralStateNotificationCB( gaprole_States_t newState );
static void simpleProfileChangeCB( uint8 paramID );

static void simpleBLECentralRssiCB( uint16 connHandle, int8  rssi );
static void simpleBLECentralEventCB( gapCentralRoleEvent_t *pEvent );
static void simpleBLECentralPasscodeCB( uint8 *deviceAddr, uint16 connectionHandle,uint8 uiInputs, uint8 uiOutputs );
static void simpleBLECentralPairStateCB( uint16 connHandle, uint8 state, uint8 status );
static const gapCentralRoleCB_t simpleBLERoleCB={
  simpleBLECentralRssiCB,       // RSSI callback
  simpleBLECentralEventCB       // Event callback
};
// Bond Manager Callbacks
static const gapBondCBs_t simpleBLEBondCB =
{
  simpleBLECentralPasscodeCB,
  simpleBLECentralPairStateCB
};

typedef struct
{
  uint8 state;
  uint8 connHandle;
  uint8 addr[B_ADDR_LEN]; 
} deviceList_t;
static deviceList_t deviceList[3];
static uint8 currentDevice=0;

static void performPeriodicTask( void );
static void periodicCentralTask( void );

#if (defined HAL_LCD) && (HAL_LCD == TRUE)
static char *bdAddr2Str ( uint8 *pAddr );
#endif // (defined HAL_LCD) && (HAL_LCD == TRUE)



/*********************************************************************
 * PROFILE CALLBACKS
 */

// GAP Role Callbacks
static gapRolesCBs_t simpleBLEPeripheral_PeripheralCBs =
{
  peripheralStateNotificationCB,  // Profile State Change Callbacks
  NULL                            // When a valid RSSI is read from controller (not used by application)
};

// GAP Bond Manager Callbacks
static gapBondCBs_t simpleBLEPeripheral_BondMgrCBs =
{
  NULL,                     // Passcode callback (not used by application)
  NULL                      // Pairing / Bonding state Callback (not used by application)
};

// Simple GATT Profile Callbacks
static simpleProfileCBs_t simpleBLEPeripheral_SimpleProfileCBs =
{
  simpleProfileChangeCB    // Charactersitic value change callback
};

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      SimpleBLEPeripheral_Init
 *
 * @brief   Initialization function for the Simple BLE Peripheral App Task.
 *          This is called during initialization and should contain
 *          any application specific initialization (ie. hardware
 *          initialization/setup, table initialization, power up
 *          notificaiton ... ).
 *
 * @param   task_id - the ID assigned by OSAL.  This ID should be
 *                    used to send messages and set timers.
 *
 * @return  none
 */
void SimpleBLEPeripheral_Init( uint8 task_id )
{
  simpleBLEPeripheral_TaskID = task_id;

  /*===========================================================
					PERIPHERAL APP STARTUP
  ===========================================================*/
  // Setup the GAP
  VOID GAP_SetParamValue( TGAP_CONN_PAUSE_PERIPHERAL, DEFAULT_CONN_PAUSE_PERIPHERAL );
  
  // Setup the GAP Peripheral Role Profile
  {
    #if defined( CC2540_MINIDK )
      // For the CC2540DK-MINI keyfob, device doesn't start advertising until button is pressed
      uint8 initial_advertising_enable = FALSE;
    #else
      // For other hardware platforms, device starts advertising upon initialization
      uint8 initial_advertising_enable = TRUE;
    #endif

    // By setting this to zero, the device will go into the waiting state after
    // being discoverable for 30.72 second, and will not being advertising again
    // until the enabler is set back to TRUE
    uint16 gapRole_AdvertOffTime = 0;

    uint8 enable_update_request = DEFAULT_ENABLE_UPDATE_REQUEST;
    uint16 desired_min_interval = DEFAULT_DESIRED_MIN_CONN_INTERVAL;
    uint16 desired_max_interval = DEFAULT_DESIRED_MAX_CONN_INTERVAL;
    uint16 desired_slave_latency = DEFAULT_DESIRED_SLAVE_LATENCY;
    uint16 desired_conn_timeout = DEFAULT_DESIRED_CONN_TIMEOUT;

    // Set the GAP Role Parameters
    GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &initial_advertising_enable );
    GAPRole_SetParameter( GAPROLE_ADVERT_OFF_TIME, sizeof( uint16 ), &gapRole_AdvertOffTime );

    GAPRole_SetParameter( GAPROLE_SCAN_RSP_DATA, sizeof ( scanRspData ), scanRspData );
    GAPRole_SetParameter( GAPROLE_ADVERT_DATA, sizeof( advertData ), advertData );

    GAPRole_SetParameter( GAPROLE_PARAM_UPDATE_ENABLE, sizeof( uint8 ), &enable_update_request );
    GAPRole_SetParameter( GAPROLE_MIN_CONN_INTERVAL, sizeof( uint16 ), &desired_min_interval );
    GAPRole_SetParameter( GAPROLE_MAX_CONN_INTERVAL, sizeof( uint16 ), &desired_max_interval );
    GAPRole_SetParameter( GAPROLE_SLAVE_LATENCY, sizeof( uint16 ), &desired_slave_latency );
    GAPRole_SetParameter( GAPROLE_TIMEOUT_MULTIPLIER, sizeof( uint16 ), &desired_conn_timeout );
  }

  // Set the GAP Characteristics
  GGS_SetParameter( GGS_DEVICE_NAME_ATT, GAP_DEVICE_NAME_LEN, attDeviceName );

  // Set advertising interval
  {
    uint16 advInt = DEFAULT_ADVERTISING_INTERVAL;

    GAP_SetParamValue( TGAP_LIM_DISC_ADV_INT_MIN, advInt );
    GAP_SetParamValue( TGAP_LIM_DISC_ADV_INT_MAX, advInt );
    GAP_SetParamValue( TGAP_GEN_DISC_ADV_INT_MIN, advInt );
    GAP_SetParamValue( TGAP_GEN_DISC_ADV_INT_MAX, advInt );
  }
  
  
  /*===========================================================
					GAP Bond Manager
  ===========================================================*/  

  {
    uint32 passkey = 0; // passkey "000000"
    uint8 pairMode = GAPBOND_PAIRING_MODE_WAIT_FOR_REQ;
    uint8 mitm = TRUE;
    uint8 ioCap = GAPBOND_IO_CAP_DISPLAY_ONLY;
    uint8 bonding = TRUE;
	
    GAPBondMgr_SetParameter( GAPBOND_DEFAULT_PASSCODE, sizeof ( uint32 ), &passkey );
    GAPBondMgr_SetParameter( GAPBOND_PAIRING_MODE, sizeof ( uint8 ), &pairMode );
    GAPBondMgr_SetParameter( GAPBOND_MITM_PROTECTION, sizeof ( uint8 ), &mitm );
    GAPBondMgr_SetParameter( GAPBOND_IO_CAPABILITIES, sizeof ( uint8 ), &ioCap );
    GAPBondMgr_SetParameter( GAPBOND_BONDING_ENABLED, sizeof ( uint8 ), &bonding );
  }

  // Initialize GATT attributes
  GGS_AddService( GATT_ALL_SERVICES );            // GAP
  GATTServApp_AddService( GATT_ALL_SERVICES );    // GATT attributes
  DevInfo_AddService();                           // Device Information Service
  SimpleProfile_AddService( GATT_ALL_SERVICES );  // Simple GATT Profile
  
#if defined FEATURE_OAD
  VOID OADTarget_AddService();                    // OAD Profile
#endif

  // Setup the SimpleProfile Characteristic Values
  {
    uint8 charValue1 = 1;
    uint8 charValue2 = 2;
    uint8 charValue3 = 3;
    uint8 charValue4 = 4;
    uint8 charValue5[SIMPLEPROFILE_CHAR5_LEN] = { 1, 2, 3, 4, 5 };
    SimpleProfile_SetParameter( SIMPLEPROFILE_CHAR1, sizeof ( uint8 ), &charValue1 );
    SimpleProfile_SetParameter( SIMPLEPROFILE_CHAR2, sizeof ( uint8 ), &charValue2 );
    SimpleProfile_SetParameter( SIMPLEPROFILE_CHAR3, sizeof ( uint8 ), &charValue3 );
    SimpleProfile_SetParameter( SIMPLEPROFILE_CHAR4, sizeof ( uint8 ), &charValue4 );
    SimpleProfile_SetParameter( SIMPLEPROFILE_CHAR5, SIMPLEPROFILE_CHAR5_LEN, charValue5 );
  }


#if defined( CC2540_MINIDK )

  SK_AddService( GATT_ALL_SERVICES ); // Simple Keys Profile

  // Register for all key events - This app will handle all key events
  RegisterForKeys( simpleBLEPeripheral_TaskID );

  // makes sure LEDs are off
  HalLedSet( (HAL_LED_1 | HAL_LED_2), HAL_LED_MODE_OFF );

  // For keyfob board set GPIO pins into a power-optimized state
  // Note that there is still some leakage current from the buzzer,
  // accelerometer, LEDs, and buttons on the PCB.

  P0SEL = 0; // Configure Port 0 as GPIO
  P1SEL = 0; // Configure Port 1 as GPIO
  P2SEL = 0; // Configure Port 2 as GPIO

  P0DIR = 0xFC; // Port 0 pins P0.0 and P0.1 as input (buttons),
                // all others (P0.2-P0.7) as output
  P1DIR = 0xFF; // All port 1 pins (P1.0-P1.7) as output
  P2DIR = 0x1F; // All port 1 pins (P2.0-P2.4) as output

  P0 = 0x03; // All pins on port 0 to low except for P0.0 and P0.1 (buttons)
  P1 = 0;   // All pins on port 1 to low
  P2 = 0;   // All pins on port 2 to low

#endif // #if defined( CC2540_MINIDK )

#if (defined HAL_LCD) && (HAL_LCD == TRUE)

#if defined FEATURE_OAD
  #if defined (HAL_IMAGE_A)
    HalLcdWriteStringValue( "BLE Peri-A", OAD_VER_NUM( _imgHdr.ver ), 16, HAL_LCD_LINE_1 );
  #else
    HalLcdWriteStringValue( "BLE Peri-B", OAD_VER_NUM( _imgHdr.ver ), 16, HAL_LCD_LINE_1 );
  #endif // HAL_IMAGE_A
#else
  HalLcdWriteString( "BLE Peripheral", HAL_LCD_LINE_1 );
#endif // FEATURE_OAD

#endif // (defined HAL_LCD) && (HAL_LCD == TRUE)

  // Register callback with SimpleGATTprofile
  VOID SimpleProfile_RegisterAppCBs( &simpleBLEPeripheral_SimpleProfileCBs );

  // Enable clock divide on halt
  // This reduces active current while radio is active and CC254x MCU
  // is halted
  HCI_EXT_ClkDivOnHaltCmd( HCI_EXT_ENABLE_CLK_DIVIDE_ON_HALT );

#if defined ( DC_DC_P0_7 )

  // Enable stack to toggle bypass control on TPS62730 (DC/DC converter)
  HCI_EXT_MapPmIoPortCmd( HCI_EXT_PM_IO_PORT_P0, HCI_EXT_PM_IO_PORT_PIN7 );

#endif // defined ( DC_DC_P0_7 )


  /*===========================================================
					CENTRAL APP STARTUP
  ===========================================================*/  
  // Set SCAN interval
  {
	GAP_SetParamValue( TGAP_GEN_DISC_SCAN, DEFAULT_SCAN_DURATION );
	GAP_SetParamValue( TGAP_LIM_DISC_SCAN, DEFAULT_SCAN_DURATION );
  }
  //devices the central will try and connect to
  uint8 addr1[6] ={0xFF,0xFF,0xFF,0xFF,0xFF,0x02};
  uint8 addr2[6] ={0xFF,0xFF,0xFF,0xFF,0xFF,0x03};
  uint8 addr3[6] ={0xFF,0xFF,0xFF,0xFF,0xFF,0x04};

  //uint8 addr3[6] ={0xFF,0xFF,0xFF,0xFF,0xFF,0x00}; //MASTER FOB
  
  VOID osal_memcpy( deviceList[0].addr, addr1, 6 );
  VOID osal_memcpy( deviceList[1].addr, addr2, 6 );
  VOID osal_memcpy( deviceList[2].addr, addr3, 6 );
  
  deviceList[0].state = BLE_STATE_IDLE;
  deviceList[1].state = BLE_STATE_IDLE;
  deviceList[2].state = BLE_STATE_IDLE;

  deviceList[0].connHandle = 1;         
  deviceList[1].connHandle = 2;
  deviceList[2].connHandle = 3;

  /*===========================================================
					Setting process event
  ===========================================================*/
  
  //Open Gatt Indications/Notifications
  VOID GATT_InitClient();
  
  //Signal SYS_EVENT_MSG
  GATT_RegisterForInd( simpleBLEPeripheral_TaskID);//Gatt register-->GATT_MSG_EVENT
  RegisterForKeys( simpleBLEPeripheral_TaskID );//Keys register-->KEY_CHANGE
  
  
  // Setup a delayed profile startup
  osal_set_event( simpleBLEPeripheral_TaskID, SBP_START_DEVICE_EVT );
  
	//--------------------------------------------------EVENT 1
	//SBP_PERIODIC_TASK1
	PERIODIC_EVENT[0].ID=SBP_PERIODIC_TASK1;
	PERIODIC_EVENT[0].ARRIVAL=0;
	PERIODIC_EVENT[0].SIZE=100;
	PERIODIC_EVENT[0].PERIOD=4000;
	PERIODIC_EVENT[0].ARRIVAL_COUNT=0;
	PERIODIC_EVENT[0].EXESIZE=PERIODIC_EVENT[0].SIZE;
	PERIODIC_EVENT[0].DEADLINE=PERIODIC_EVENT[0].PERIOD/1000;
	//--------------------------------------------------EVENT 2
	//SBP_PERIODIC_TASK2
	PERIODIC_EVENT[1].ID=SBP_PERIODIC_TASK2;
	PERIODIC_EVENT[1].ARRIVAL=0;
	PERIODIC_EVENT[1].SIZE=100;
	PERIODIC_EVENT[1].PERIOD=5000;
	PERIODIC_EVENT[1].ARRIVAL_COUNT=0;
	PERIODIC_EVENT[1].EXESIZE=PERIODIC_EVENT[1].SIZE;
	PERIODIC_EVENT[1].DEADLINE=PERIODIC_EVENT[1].PERIOD/1000;
	//--------------------------------------------------EVENT 3
	//SBP_PERIODIC_TASK3
	PERIODIC_EVENT[2].ID=SBP_PERIODIC_TASK3;
	PERIODIC_EVENT[2].ARRIVAL=0;
	PERIODIC_EVENT[2].SIZE=100;
	PERIODIC_EVENT[2].PERIOD=8000;
	PERIODIC_EVENT[2].ARRIVAL_COUNT=0;
	PERIODIC_EVENT[2].EXESIZE=PERIODIC_EVENT[2].SIZE;
	PERIODIC_EVENT[2].DEADLINE=PERIODIC_EVENT[2].PERIOD/1000;

        /*
  osal_set_event( simpleBLEPeripheral_TaskID, SBP_PERIODIC_EVT );   //TimeSlot count
  osal_set_event( simpleBLEPeripheral_TaskID, SBP_PERIODIC_TASK1 ); //Event1
  osal_set_event( simpleBLEPeripheral_TaskID, SBP_PERIODIC_TASK2 ); //Event2
  osal_set_event( simpleBLEPeripheral_TaskID, SBP_PERIODIC_TASK3 ); //Event3
  osal_set_event( simpleBLEPeripheral_TaskID, SBP_BURST_EVT );      //TX Event
        */
  //osal_start_timerEx( simpleBLEPeripheral_TaskID, SBP_BURST_EVT, SBP_BURST_EVT_PERIOD );
  
}

/*********************************************************************
 * @fn      SimpleBLEPeripheral_ProcessEvent
 *
 * @brief   Simple BLE Peripheral Application Task event processor.  This function
 *          is called to process all events for the task.  Events
 *          include timers, messages and any other user defined events.
 *
 * @param   task_id  - The OSAL assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  events not processed
 */
int priority=0;
int CountBufferSize=0;
uint16 SimpleBLEPeripheral_ProcessEvent( uint8 task_id, uint16 events )
{
  VOID task_id;
  
  /*==========================================
	SYS_EVENT_MSG(Handle Key & Receive Msg)
  ==========================================*/
  if ( events & SYS_EVENT_MSG )
  {
    uint8 *pMsg;

    if ( (pMsg = osal_msg_receive( simpleBLEPeripheral_TaskID )) != NULL ){		
      simpleBLEPeripheral_ProcessOSALMsg( (osal_event_hdr_t *)pMsg );
      VOID osal_msg_deallocate( pMsg );// Release the OSAL message
    }
    return (events ^ SYS_EVENT_MSG);
  }

  /*==========================================
		SBP_START_DEVICE_EVT (Master/Slave mode)
  ==========================================*/
  if ( events & SBP_START_DEVICE_EVT )
  {
    if(center_mode==0){
		VOID GAPRole_StartDevice( &simpleBLEPeripheral_PeripheralCBs );// Start the Slave mode
		masterSlave_State = BLE_STATE_ADVERTISING; 
		VOID GAPBondMgr_Register( &simpleBLEPeripheral_BondMgrCBs );// Start Bond Manager(Init)
		//masterSlave_State = BLE_STATE_DISCONNECTING; // CENTRAL --> PERIPHERAL
		GAPCentralRole_TerminateLink( 0XFFFE );//cancel current link request 
		uint8 turnOnAdv = TRUE;
		GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &turnOnAdv );
		
		HalLcdWriteString( "Start Slave EVT",  HAL_LCD_LINE_6 );
	}else if(center_mode==1){
		VOID GAPCentralRole_StartDevice( (gapCentralRoleCB_t *) &simpleBLERoleCB );// Start the Master mode
		GAPBondMgr_Register( (gapBondCBs_t *) &simpleBLEBondCB );
		masterSlave_State = BLE_STATE_CONNECTING;
		deviceRole = ROLE_CENTRAL;
		uint8 new_adv_enabled_status = FALSE;// PERIPHERAL --> CENTRAL
		GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &new_adv_enabled_status );
		
		HalLcdWriteString( "Start Master EVT",  HAL_LCD_LINE_6 );
	}
	
    // Set timer for first periodic event
    //osal_start_timerEx( simpleBLEPeripheral_TaskID, SBP_PERIODIC_EVT, SBP_PERIODIC_EVT_PERIOD );
	//osal_start_timerEx( simpleBLEPeripheral_TaskID, SBP_Packet3_EVT, P3_Period );	
	
    return ( events ^ SBP_START_DEVICE_EVT );
  }
  
  /*==========================================
    SBP_PERIODIC_EVT(Periodic event)
  ==========================================*/
  if ( events & SBP_PERIODIC_EVT )
  {
    // Restart timer
    if ( SBP_PERIODIC_EVT_PERIOD ){
      osal_start_timerEx( simpleBLEPeripheral_TaskID, SBP_PERIODIC_EVT, SBP_PERIODIC_EVT_PERIOD );	
    }
	TIMESLOT++;
	HalLcdWriteStringValue( "TIME SLOT:", (uint16)(TIMESLOT), 10,  HAL_LCD_LINE_1 );
	if(TIMESLOT==1000)
          TIMESLOT=0;
	//performPeriodicTask();// Perform periodic application task
	//periodicCentralTask();
    
	return (events ^ SBP_PERIODIC_EVT);
  }
  
  if ( events & SBP_PERIODIC_TASK1 )
  {
    
    // Restart timer
    if ( PERIODIC_EVENT[0].PERIOD ){
      osal_start_timerEx( simpleBLEPeripheral_TaskID, SBP_PERIODIC_TASK1, PERIODIC_EVENT[0].PERIOD );	
    }
	//HalLcdWriteStringValue( "Task1 arrival", (uint16)(PERIODIC_EVENT[0].ARRIVAL), 10 , HAL_LCD_LINE_4);
	//PeriodicTask1();    
	
	return (events ^ SBP_PERIODIC_TASK1);
  }
  
  if ( events & SBP_PERIODIC_TASK2 )
  {
    // Restart timer
    if ( PERIODIC_EVENT[1].PERIOD ){
      osal_start_timerEx( simpleBLEPeripheral_TaskID, SBP_PERIODIC_TASK2, PERIODIC_EVENT[1].PERIOD );	
    }
	
	//HalLcdWriteStringValue( "Task2 arrival", (uint16)(PERIODIC_EVENT[1].ARRIVAL), 10 , HAL_LCD_LINE_5);
	//PeriodicTask2();    
	
	return (events ^ SBP_PERIODIC_TASK2);
  }
    
  if ( events & SBP_PERIODIC_TASK3 )
  {
    // Restart timer
    if ( PERIODIC_EVENT[2].PERIOD ){
      osal_start_timerEx( simpleBLEPeripheral_TaskID, SBP_PERIODIC_TASK3, PERIODIC_EVENT[2].PERIOD );	
    }
	
	//HalLcdWriteStringValue( "Task3 arrival", (uint16)(PERIODIC_EVENT[2].ARRIVAL), 10 , HAL_LCD_LINE_6);
	//PeriodicTask3();  
	
	return (events ^ SBP_PERIODIC_TASK3);
  }
  /*==========================================
	SBP_BURST_EVT(Send msg)
  ==========================================*/
 if ( events & SBP_BURST_EVT )
  {
    // Restart timer
    if ( SBP_BURST_EVT_PERIOD ){
      osal_start_timerEx( simpleBLEPeripheral_TaskID, SBP_BURST_EVT, SBP_BURST_EVT_PERIOD );
    }
    
	if(simpleBLEDoWrite==true){
		HalLcdWriteStringValue( "Connect", (uint16)(1), 10,  HAL_LCD_LINE_2);
		
		//Setting up ReadyQueue -->Queue[]
		BufferQueue();
		
		//Call send function
		priority=0;
		CountBufferSize=0;
                
		while(priority<AMOUNT_OF_EVENT && CountBufferSize<6){
                
                
			if((PERIODIC_EVENT[Queue[priority]].READYFLAG==1) && (PERIODIC_EVENT[Queue[priority]].EXESIZE>0)){
				
				PERIODIC_EVENT[Queue[priority]].EXESIZE--;
				
				CountBufferSize++;
				
				SEND_PACKET(PERIODIC_EVENT[Queue[priority]].ID);
		                		
				if(PERIODIC_EVENT[Queue[priority]].EXESIZE==0){
					PERIODIC_EVENT[Queue[priority]].READYFLAG=0;
					
					if(PERIODIC_EVENT[Queue[priority]].ID==SBP_PERIODIC_TASK1){
                                          PERIODIC_EVENT[0].EXESIZE=  PERIODIC_EVENT[0].SIZE;//Size reload 
                                          PERIODIC_EVENT[0].ARRIVAL=PERIODIC_EVENT[0].DEADLINE;
                                          PERIODIC_EVENT[0].DEADLINE=PERIODIC_EVENT[0].DEADLINE+PERIODIC_EVENT[0].PERIOD/1000;
					}
					if(PERIODIC_EVENT[Queue[priority]].ID==SBP_PERIODIC_TASK2){
                                          PERIODIC_EVENT[1].EXESIZE=  PERIODIC_EVENT[1].SIZE;//Size reload 
                                          PERIODIC_EVENT[1].ARRIVAL=PERIODIC_EVENT[1].DEADLINE;
                                          PERIODIC_EVENT[1].DEADLINE=PERIODIC_EVENT[1].DEADLINE+PERIODIC_EVENT[0].PERIOD/1000;
					}
					if(PERIODIC_EVENT[Queue[priority]].ID==SBP_PERIODIC_TASK3){
                                          PERIODIC_EVENT[2].EXESIZE=  PERIODIC_EVENT[2].SIZE;//Size reload 
                                          PERIODIC_EVENT[2].ARRIVAL=PERIODIC_EVENT[2].DEADLINE;
                                          PERIODIC_EVENT[2].DEADLINE=PERIODIC_EVENT[2].DEADLINE+PERIODIC_EVENT[0].PERIOD/1000;
					}
				}     
			}else
				priority++;
		  }
		  
	}else{
		HalLcdWriteStringValue( "DisConnect", (uint16)(0), 10,  HAL_LCD_LINE_2);
	}
    
	/*
    for(int c=0;c<packetnum;c++){
      sendData();  
    }
	 */
    return (events ^ SBP_BURST_EVT);
  }  
  
#if defined ( PLUS_BROADCASTER )
  if ( events & SBP_ADV_IN_CONNECTION_EVT )
  {
    uint8 turnOnAdv = TRUE;
    // Turn on advertising while in a connection
    GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &turnOnAdv );

    return (events ^ SBP_ADV_IN_CONNECTION_EVT);
  }
#endif // PLUS_BROADCASTER

  // Discard unknown events
  return 0;
}

/*********************************************************************
 * @fn      simpleBLEPeripheral_ProcessOSALMsg
 *
 * @brief   Process an incoming task message.
 *
 * @param   pMsg - message to process
 *
 * @return  none
 */
static void simpleBLEPeripheral_ProcessOSALMsg( osal_event_hdr_t *pMsg )
{
		
  switch ( pMsg->event )
  {
  
    case KEY_CHANGE:
      simpleBLEPeripheral_HandleKeys( ((keyChange_t *)pMsg)->state, ((keyChange_t *)pMsg)->keys );
    break;
  
    case GATT_MSG_EVENT://HCI_LE_ExtEvent ATT_HandleValueNotification
      simpleBLEPeripheralProcessGattMsg((gattMsgEvent_t *) pMsg);
    break;
  
	default:
    // do nothing
    break;
  }
}
static void simpleBLEPeripheralProcessGattMsg(gattMsgEvent_t *pMsg)
{
 //Message Indication Confirmation
 if ( pMsg->method == ATT_HANDLE_VALUE_NOTI || pMsg->method == ATT_HANDLE_VALUE_IND )
 {
	static uint16 Notify_handle=0;
	static uint8 Notify_len=0;
	
	Notify_handle=pMsg->msg.handleValueNoti.handle;
	Notify_len=pMsg->msg.handleValueNoti.len;
	
	uint8* Notify_value;
	osal_memcpy(Notify_value,pMsg->msg.handleValueNoti.value,Notify_len);//array copy
	
	#if (defined HAL_LCD) && (HAL_LCD == TRUE)
		HalLcdWriteStringValue( "", (uint16)(Notify_handle), 10,  HAL_LCD_LINE_5 );
		HalLcdWriteStringValue( "", (uint8)(*Notify_value), 10,  HAL_LCD_LINE_6 );
	#endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
 }
}

/*=============================================
				Handle Key function
===============================================*/
static void simpleBLEPeripheral_HandleKeys( uint8 shift, uint8 keys )
{
  uint8 SK_Keys = 0;

  VOID shift;  // Intentionally unreferenced parameter
  noti.handle = 0x0001;
  /*=========================================
  
              UP Key
  
  ===========================================*/
  if ( keys & HAL_KEY_UP ){
    
      if(setpowerflag){
		#if (defined HAL_LCD) && (HAL_LCD == TRUE)
			HalLcdWriteString( "TX power: -23DB",  HAL_LCD_LINE_4 );
		#endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
		
		HCI_EXT_SetTxPowerCmd(LL_EXT_TX_POWER_MINUS_23_DBM);
	  }
      
      /*------------------------------------------------------
                          Send Msg to Master
      ---------------------------------------------------------*/
      
      if ( simpleBLEDoWrite & !setpowerflag)
      {
		#if (defined HAL_LCD) && (HAL_LCD == TRUE)
			HalLcdWriteString( "UP Send",  HAL_LCD_LINE_4 );
		#endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
        
        SBP_BURST_EVT_PERIOD=RecvInterval;
        if (SBP_BURST_EVT_PERIOD<0x08)
          SBP_BURST_EVT_PERIOD=0x08;
        packetnum=1;
        testmode=0;
        osal_start_timerEx( simpleBLEPeripheral_TaskID, SBP_BURST_EVT, SBP_BURST_EVT_PERIOD );
        
      }
      
  }
  /*=========================================
  
              DOWN Key
  
  ===========================================*/
  if ( keys & HAL_KEY_DOWN ){
    
	  if(setpowerflag){
		#if (defined HAL_LCD) && (HAL_LCD == TRUE)
			HalLcdWriteString( "TX power: -6DB",  HAL_LCD_LINE_4 );
		#endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
		
		 HCI_EXT_SetTxPowerCmd(LL_EXT_TX_POWER_MINUS_6_DBM);
	  }
	  
      if ( simpleBLEDoWrite & !setpowerflag)
      {
		#if (defined HAL_LCD) && (HAL_LCD == TRUE)
			HalLcdWriteString( "DOWN Send",  HAL_LCD_LINE_4 );
		#endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
        SBP_BURST_EVT_PERIOD=RecvInterval;
        if (SBP_BURST_EVT_PERIOD<0x08)
          SBP_BURST_EVT_PERIOD=0x08;
        packetnum=1;
        testmode=1;
		osal_start_timerEx( simpleBLEPeripheral_TaskID, SBP_BURST_EVT, SBP_BURST_EVT_PERIOD );
      }
  }
  /*=========================================
  
              LEFT Key
  
  ===========================================*/
  if ( keys & HAL_KEY_LEFT ){
   
      if(setpowerflag){
		#if (defined HAL_LCD) && (HAL_LCD == TRUE)
			HalLcdWriteString( "TX power: 0DB",  HAL_LCD_LINE_4 );
		#endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
		
		 HCI_EXT_SetTxPowerCmd(LL_EXT_TX_POWER_0_DBM);
	  }
	  
      if ( simpleBLEDoWrite & !setpowerflag)
      {
	   #if (defined HAL_LCD) && (HAL_LCD == TRUE)
		HalLcdWriteString( "LEFT Send",  HAL_LCD_LINE_4 );
	   #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
          SBP_BURST_EVT_PERIOD=RecvInterval;
          if (SBP_BURST_EVT_PERIOD<0x08)
            SBP_BURST_EVT_PERIOD=0x08;
           //SBP_BURST_EVT_PERIOD=16;
           packetnum=3;
	   osal_start_timerEx( simpleBLEPeripheral_TaskID, SBP_BURST_EVT, SBP_BURST_EVT_PERIOD );
	   
      }
  }
  /*=========================================
  
              RIGHT Key
  
  ===========================================*/
  if ( keys & HAL_KEY_RIGHT ){
  
	if(setpowerflag){
		#if (defined HAL_LCD) && (HAL_LCD == TRUE)
			HalLcdWriteString( "TX power: +4DB",  HAL_LCD_LINE_4 );
		#endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
		
		 HCI_EXT_SetTxPowerCmd(LL_EXT_TX_POWER_4_DBM);
	  }
    
    if ( simpleBLEDoWrite & !setpowerflag)
    {
		#if (defined HAL_LCD) && (HAL_LCD == TRUE)
			HalLcdWriteString( "Right Send",  HAL_LCD_LINE_4 );
		#endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
                SBP_BURST_EVT_PERIOD=RecvInterval;
                if (SBP_BURST_EVT_PERIOD<0x08)
                  SBP_BURST_EVT_PERIOD=0x08;
                //SBP_BURST_EVT_PERIOD=100;
                packetnum=4;
		osal_start_timerEx( simpleBLEPeripheral_TaskID, SBP_BURST_EVT, SBP_BURST_EVT_PERIOD );
      }
  }
  
  if(keys & HAL_KEY_CENTER){
    
	  //center_mode switch
	  uint8 new_adv_enabled_status = TRUE;
	  
      if(center_mode==0){
		center_mode=1;
		#if (defined HAL_LCD) && (HAL_LCD == TRUE)
			HalLcdWriteString( "Master",  HAL_LCD_LINE_7 );
		#endif
		
		osal_start_timerEx( simpleBLEPeripheral_TaskID, SBP_START_DEVICE_EVT, 500 ); 
	  }else if(center_mode==1){
		center_mode=2;
		#if (defined HAL_LCD) && (HAL_LCD == TRUE)
			HalLcdWriteString( "Change TX Power",  HAL_LCD_LINE_4 );
		#endif
		
	  }else if(center_mode==2){
		center_mode=0;
		#if (defined HAL_LCD) && (HAL_LCD == TRUE)
			HalLcdWriteString( "Slave",  HAL_LCD_LINE_7 );
		#endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
		
		osal_start_timerEx( simpleBLEPeripheral_TaskID, SBP_START_DEVICE_EVT, 500 ); 
	  }
	  
	  if(center_mode==2){
		setpowerflag=1;
	  }else{		
		setpowerflag=0;
	  }
      
  }
 
}
static uint16 counter=0;
uint8 burstData[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

static void sendData(void )
{    
    burstData[0] = (counter & 0xFF00)>>8;
    burstData[1] = (counter & 0xFF);
        
    attHandleValueNoti_t nData;
    nData.len = 20;
    nData.handle = 20;
    
    osal_memcpy( &nData.value, &burstData,20);

    // Send the Notification
    if (GATT_Notification( gapConnHandle, &nData, FALSE )==SUCCESS){
      counter++;
    }
    
    //Suspend 
    if(burstData[0]==0x02){
      counter=0;
      osal_stop_timerEx(simpleBLEPeripheral_TaskID,SBP_BURST_EVT);
    }
}
/*********************************************************************
 * @fn      Central CallBack
 *
 * @brief   simpleBLECentralEventCB,simpleBLECentralRssiCB,simpleBLECentralPasscodeCB,simpleBLECentralPairStateCB
 *
 * @param   
 *
 */
 static void simpleBLECentralEventCB( gapCentralRoleEvent_t *pEvent )
{
  
	uint8 i;  
	HalLcdWriteString( "Central CB",  HAL_LCD_LINE_7 );
	
  switch ( pEvent->gap.opcode )
  {
      
   case GAP_LINK_ESTABLISHED_EVENT:
      {
       
        if ( pEvent->gap.hdr.status == SUCCESS )
        {          
          for(i=0; i<MAX_CONNECTIONS;i++)
          {
            //Check against addresses we want to connect to
            if(deviceList[i].addr[5] == pEvent->linkCmpl.devAddr[5])
            {
              deviceList[i].state = BLE_STATE_CONNECTED;
              deviceList[i].connHandle = pEvent->linkCmpl.connectionHandle;
            }
          }
         }        
      }
        
      break;

      case GAP_LINK_TERMINATED_EVENT:
      {
          for(i=0; i<MAX_CONNECTIONS;i++)
          {
            //Check against addresses we want to connect to
            if(deviceList[i].connHandle == pEvent->linkTerminate.connectionHandle)
            {
              deviceList[i].state = BLE_STATE_IDLE;
            }
          }     
      }
      break;
	  
	  default:
	  break;
  }
}
static void simpleBLECentralRssiCB( uint16 connHandle, int8 rssi )
{
    //LCD_WRITE_STRING_VALUE( "RSSI -dB:", (uint8) (-rssi), 10, HAL_LCD_LINE_1 );
}


static void simpleBLECentralPasscodeCB( uint8 *deviceAddr, uint16 connectionHandle,
                                        uint8 uiInputs, uint8 uiOutputs )
{
#if (HAL_LCD == TRUE)

  uint32  passcode;
  uint8   str[7];

  // Create random passcode
  LL_Rand( ((uint8 *) &passcode), sizeof( uint32 ));
  passcode %= 1000000;
  
  // Display passcode to user
  if ( uiOutputs != 0 )
  {
	HalLcdWriteString( "Passcode:",  HAL_LCD_LINE_1 );
    
  }
  
  // Send passcode response
  GAPBondMgr_PasscodeRsp( connectionHandle, SUCCESS, passcode );
#endif
}
static void simpleBLECentralPairStateCB( uint16 connHandle, uint8 state, uint8 status )
{
  if ( state == GAPBOND_PAIRING_STATE_STARTED )
  {
	HalLcdWriteString( "Pairing started",  HAL_LCD_LINE_1 );
  }
  else if ( state == GAPBOND_PAIRING_STATE_COMPLETE )
  {
    if ( status == SUCCESS )
    {
		HalLcdWriteString( "Pairing success",  HAL_LCD_LINE_1 );
    }
    else
    {
		HalLcdWriteString( "Pairing fail",  HAL_LCD_LINE_1 );
    }
  }
  else if ( state == GAPBOND_PAIRING_STATE_BONDED )
  {
    if ( status == SUCCESS )
    {
		HalLcdWriteString( "Bonding success",  HAL_LCD_LINE_1 );
    }
  }
}
 
/*********************************************************************
 * @fn      peripheralStateNotificationCB
 *
 * @brief   Notification from the profile of a state change.
 *
 * @param   newState - new state
 *
 * @return  none
 */
static void peripheralStateNotificationCB( gaprole_States_t newState )
{
	HalLcdWriteString( "Slave CB",  HAL_LCD_LINE_6 );
  switch ( newState )
  {
    case GAPROLE_STARTED:
      {
        uint8 ownAddress[B_ADDR_LEN];
        uint8 systemId[DEVINFO_SYSTEM_ID_LEN];

        GAPRole_GetParameter(GAPROLE_BD_ADDR, ownAddress);

        // use 6 bytes of device address for 8 bytes of system ID value
        systemId[0] = ownAddress[0];
        systemId[1] = ownAddress[1];
        systemId[2] = ownAddress[2];

        // set middle bytes to zero
        systemId[4] = 0x00;
        systemId[3] = 0x00;

        // shift three bytes up
        systemId[7] = ownAddress[5];
        systemId[6] = ownAddress[4];
        systemId[5] = ownAddress[3];

        DevInfo_SetParameter(DEVINFO_SYSTEM_ID, DEVINFO_SYSTEM_ID_LEN, systemId);

        #if (defined HAL_LCD) && (HAL_LCD == TRUE)
          // Display device address
          HalLcdWriteString( bdAddr2Str( ownAddress ),  HAL_LCD_LINE_2 );
          HalLcdWriteString( "Initialized",  HAL_LCD_LINE_3 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
      }
	  
	  //Restart Advertisement
	  uint8 turnOnAdv = TRUE;
	  GAPRole_SetParameter( GAPROLE_ADVERT_ENABLED, sizeof( uint8 ), &turnOnAdv );
	  
      break;

    case GAPROLE_ADVERTISING:
      {
        #if (defined HAL_LCD) && (HAL_LCD == TRUE)
          HalLcdWriteString( "Advertising",  HAL_LCD_LINE_3 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
		
		simpleBLEDoWrite = FALSE;
		setpowerflag=FALSE;
      }
      break;

    case GAPROLE_CONNECTED:
      {
		//Get connection handle
		GAPRole_GetParameter( GAPROLE_CONNHANDLE, &gapConnHandle );
        
		#if (defined HAL_LCD) && (HAL_LCD == TRUE)
          HalLcdWriteString( "Connected",  HAL_LCD_LINE_3 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
        
        simpleBLEDoWrite=true;
		masterSlave_State = BLE_STATE_CONNECTED;

      }
    break;

    case GAPROLE_WAITING:
      {
        #if (defined HAL_LCD) && (HAL_LCD == TRUE)
          HalLcdWriteString( "Disconnected",  HAL_LCD_LINE_3 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
		
		masterSlave_State = BLE_STATE_ADVERTISING;
      }
      break;

    case GAPROLE_WAITING_AFTER_TIMEOUT:
      {
        #if (defined HAL_LCD) && (HAL_LCD == TRUE)
          HalLcdWriteString( "Timed Out",  HAL_LCD_LINE_3 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
		
		masterSlave_State = BLE_STATE_ADVERTISING;
      }
      break;

    case GAPROLE_ERROR:
      {
        #if (defined HAL_LCD) && (HAL_LCD == TRUE)
          HalLcdWriteString( "Error",  HAL_LCD_LINE_3 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
      }
      break;

    default:
      {
		masterSlave_State = BLE_STATE_ADVERTISING;
        #if (defined HAL_LCD) && (HAL_LCD == TRUE)
          HalLcdWriteString( "",  HAL_LCD_LINE_3 );
        #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
      }
      break;

  }

  gapProfileState = newState;

#if !defined( CC2540_MINIDK )
  VOID gapProfileState;     // added to prevent compiler warning with
                            // "CC2540 Slave" configurations
#endif


}

/*********************************************************************
 * @fn      performPeriodicTask
 *
 * @brief   Perform a periodic application task. This function gets
 *          called every five seconds as a result of the SBP_PERIODIC_EVT
 *          OSAL event. In this example, the value of the third
 *          characteristic in the SimpleGATTProfile service is retrieved
 *          from the profile, and then copied into the value of the
 *          the fourth characteristic.
 *
 * @param   none
 *
 * @return  none
 */
static void performPeriodicTask( void )
{
/*
    arrivalcount_t1++;
    HalLcdWriteStringValue( "Task1 arrival", (uint16)(arrivalcount_t1), 10 , HAL_LCD_LINE_4);
	if(arrivalcount_t1==100)
		arrivalcount_t1=0;
	*/
  uint8 valueToCopy;
  uint8 stat;

  // Call to retrieve the value of the third characteristic in the profile
  stat = SimpleProfile_GetParameter( SIMPLEPROFILE_CHAR3, &valueToCopy);

  if( stat == SUCCESS )
  {
    /*
     * Call to set that value of the fourth characteristic in the profile. Note
     * that if notifications of the fourth characteristic have been enabled by
     * a GATT client device, then a notification will be sent every time this
     * function is called.
     */
    SimpleProfile_SetParameter( SIMPLEPROFILE_CHAR4, sizeof(uint8), &valueToCopy);
  }
}

int Qpriority=0;
static void BufferQueue(void){
	
	//====================================INIT
	
	for(int i=0;i<AMOUNT_OF_EVENT;i++){
		PERIODIC_EVENT[i].findflag=0;
	}
	
	//====================================SORT & ASSIGN TO QUEUE
	while(Qpriority<AMOUNT_OF_EVENT){
	
		int Maxid=0;
		while(PERIODIC_EVENT[Maxid].findflag==1)
			Maxid++;
		
		//Find the high priority
		for (int i=Maxid; i<AMOUNT_OF_EVENT; i++){
			if(PERIODIC_EVENT[i].findflag==0){
				if(PERIODIC_EVENT[i].DEADLINE < PERIODIC_EVENT[Maxid].DEADLINE){
					Maxid=i;
				}
			}
		}
		PERIODIC_EVENT[Maxid].findflag=1;
		Queue[Qpriority]=Maxid;
		
		Qpriority++;
	}
	
	//====================================Ready setting
	for(int i=0;i<AMOUNT_OF_EVENT;i++){
		if(PERIODIC_EVENT[i].ARRIVAL <= TIMESLOT){
			PERIODIC_EVENT[i].READYFLAG=1;
		}else
			PERIODIC_EVENT[i].READYFLAG=0;
	}
}

static void SEND_PACKET(short int Handle )
{    
    burstData[0] = (counter & 0xFF00)>>8;
    burstData[1] = (counter & 0xFF);
        
    attHandleValueNoti_t nData;
    nData.len = 20;
    nData.handle = Handle;
    
    osal_memcpy( &nData.value, &burstData,20);

    // Send the Notification
    if (GATT_Notification( gapConnHandle, &nData, FALSE )==SUCCESS){
      counter++;
    }
    if(counter==1000)
      counter=0;
}


static void periodicCentralTask( void )
{
    uint8 numDevices=1;
    uint8 i;

    HalLedSet( HAL_LED_ALL, HAL_LED_MODE_OFF );
  

     if(deviceRole == ROLE_CENTRAL)
     {
        //Calc number of connections
        for(i=0;i<MAX_CONNECTIONS;i++)
        {
          if(deviceList[i].state == BLE_STATE_CONNECTED)
          {
            numDevices = numDevices + 1;
          }
        }
     }
    
    switch (masterSlave_State)
    {
 
       case BLE_STATE_CONNECTING:
       case BLE_STATE_CONNECTED:
         
         if(deviceRole == ROLE_CENTRAL)
         {
           
             //cancel current link request - only give each fob 1 sec to complet connection
             GAPCentralRole_TerminateLink( 0XFFFE );
            
                GAPCentralRole_EstablishLink( DEFAULT_LINK_HIGH_DUTY_CYCLE,
                                              DEFAULT_LINK_WHITE_LIST,
                                              ADDRTYPE_PUBLIC, deviceList[currentDevice].addr );
                
         }                
          //green
         //HalLedBlink (1, 3, HAL_LED_DEFAULT_DUTY_CYCLE, HAL_LED_DEFAULT_FLASH_TIME); 
         osal_pwrmgr_device( PWRMGR_BATTERY );
          HalLedBlink (1, numDevices, 50, 500);
         osal_pwrmgr_device( PWRMGR_ALWAYS_ON );
          
          break;
      case BLE_STATE_DISCONNECTING:
      case BLE_STATE_IDLE:

         if(deviceRole == ROLE_CENTRAL)
         {
           //are we finished disconnecting
           if(numDevices == 1)
           {  
              // set new role to peripheral           
              deviceRole = ROLE_PERIPHERAL;
              
              masterSlave_State = BLE_STATE_ADVERTISING;
              
              //restet to peripheral and advertise
              osal_start_timerEx( simpleBLEPeripheral_TaskID, SBP_START_DEVICE_EVT, 100 );
           }
           else  //continue to terminate links
           {
            if(deviceList[currentDevice].state == BLE_STATE_CONNECTED)
            {
              //terminate link
              GAPCentralRole_TerminateLink(deviceList[currentDevice].connHandle);
            }
           }
           
           //red
          HalLedBlink (2, 2, 10, 100);
         }
        break;
        
      case BLE_STATE_ADVERTISING: 

           // HalLedBlink (2, 1, HAL_LED_DEFAULT_DUTY_CYCLE, HAL_LED_DEFAULT_FLASH_TIME);    
           HalLedBlink (2, 4, 10, 100);
        
        break;
        
          
    default:
      
        break;
    }

    currentDevice = currentDevice +1;
    
    if(currentDevice >2)
      currentDevice = 0;
}

/*====================================================
				Profile Callback
====================================================*/

static void simpleProfileChangeCB( uint8 paramID )
{
  uint8 newValue;
  
  switch( paramID )
  {
    case SIMPLEPROFILE_CHAR1:
      SimpleProfile_GetParameter( SIMPLEPROFILE_CHAR1, &newValue );

	  
      #if (defined HAL_LCD) && (HAL_LCD == TRUE)
        HalLcdWriteStringValue( "Char 1:", (uint16)(newValue), 10,  HAL_LCD_LINE_3 );
      #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)
        
        
        if(newValue>=8){
          RecvInterval=newValue;
          SBP_BURST_EVT_PERIOD=RecvInterval;
          if (SBP_BURST_EVT_PERIOD<0x08)
              SBP_BURST_EVT_PERIOD=0x08;
        }
	  /*----------------------------
			Conneciotn interval 
				10ms 
	  ----------------------------*/
	  recevdata=newValue;
	  
	  if (newValue==1){
            osal_set_event( simpleBLEPeripheral_TaskID, SBP_PERIODIC_EVT );   //TimeSlot count
            osal_set_event( simpleBLEPeripheral_TaskID, SBP_PERIODIC_TASK1 ); //Event1
            osal_set_event( simpleBLEPeripheral_TaskID, SBP_PERIODIC_TASK2 ); //Event2
            osal_set_event( simpleBLEPeripheral_TaskID, SBP_PERIODIC_TASK3 ); //Event3
            osal_set_event( simpleBLEPeripheral_TaskID, SBP_BURST_EVT );      //TX Event
            /*
		  HalLcdWriteStringValue( "Get:", (uint16)(newValue), 10,  HAL_LCD_LINE_4 );
		  
		  SBP_BURST_EVT_PERIOD=RecvInterval;
		  packetnum=1;
		  osal_start_timerEx( simpleBLEPeripheral_TaskID, SBP_BURST_EVT, SBP_BURST_EVT_PERIOD );
            */
	  }
	  /*
	  if (newValue==2){
		  HalLcdWriteStringValue( "Get:", (uint16)(newValue), 10,  HAL_LCD_LINE_4 );
		  
		  SBP_BURST_EVT_PERIOD=RecvInterval;
		  packetnum=2;
		  osal_start_timerEx( simpleBLEPeripheral_TaskID, SBP_BURST_EVT, SBP_BURST_EVT_PERIOD );
	  }
	  if (newValue==3){
		  HalLcdWriteStringValue( "Get:", (uint16)(newValue), 10,  HAL_LCD_LINE_4 );
		  
		  SBP_BURST_EVT_PERIOD=RecvInterval;
		  packetnum=3;
		  osal_start_timerEx( simpleBLEPeripheral_TaskID, SBP_BURST_EVT, SBP_BURST_EVT_PERIOD );
	  }
	  if (newValue==4){
		  HalLcdWriteStringValue( "Get:", (uint16)(newValue), 10,  HAL_LCD_LINE_4 );
		  
		  SBP_BURST_EVT_PERIOD=RecvInterval;
		  packetnum=4;
		  osal_start_timerEx( simpleBLEPeripheral_TaskID, SBP_BURST_EVT, SBP_BURST_EVT_PERIOD );
	  }
	  if (newValue==5){
		  HalLcdWriteStringValue( "Get:", (uint16)(newValue), 10,  HAL_LCD_LINE_4 );
		  
		  SBP_BURST_EVT_PERIOD=RecvInterval;
		  packetnum=5;
		  osal_start_timerEx( simpleBLEPeripheral_TaskID, SBP_BURST_EVT, SBP_BURST_EVT_PERIOD );
	  }
	  if (newValue==6){
		  HalLcdWriteStringValue( "Get:", (uint16)(newValue), 10,  HAL_LCD_LINE_4 );
		  
		  SBP_BURST_EVT_PERIOD=RecvInterval;
		  packetnum=6;
		  osal_start_timerEx( simpleBLEPeripheral_TaskID, SBP_BURST_EVT, SBP_BURST_EVT_PERIOD );
	  }
	 */
	  /*----------------------------
			Conneciotn interval 
				20ms 
	  ----------------------------*/
	  
      break;

    case SIMPLEPROFILE_CHAR3:
      SimpleProfile_GetParameter( SIMPLEPROFILE_CHAR3, &newValue );

      #if (defined HAL_LCD) && (HAL_LCD == TRUE)
        HalLcdWriteStringValue( "Char 3:", (uint16)(newValue), 10,  HAL_LCD_LINE_3 );
      #endif // (defined HAL_LCD) && (HAL_LCD == TRUE)

      break;

    default:
	  break;
  }
}

#if (defined HAL_LCD) && (HAL_LCD == TRUE)
/*********************************************************************
 * @fn      bdAddr2Str
 *
 * @brief   Convert Bluetooth address to string. Only needed when
 *          LCD display is used.
 *
 * @return  none
 */
char *bdAddr2Str( uint8 *pAddr )
{
  uint8       i;
  char        hex[] = "0123456789ABCDEF";
  static char str[B_ADDR_STR_LEN];
  char        *pStr = str;

  *pStr++ = '0';
  *pStr++ = 'x';

  // Start from end of addr
  pAddr += B_ADDR_LEN;

  for ( i = B_ADDR_LEN; i > 0; i-- )
  {
    *pStr++ = hex[*--pAddr >> 4];
    *pStr++ = hex[*pAddr & 0x0F];
  }

  *pStr = 0;

  return str;
}
#endif // (defined HAL_LCD) && (HAL_LCD == TRUE)

/*********************************************************************
*********************************************************************/
