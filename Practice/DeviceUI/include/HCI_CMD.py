import collections
import datetime,time
import threading
mutex=threading.Lock()

opcodes = {
        "fd8a": 'GATT_ReadCharValue',
        "fd8e": 'GATT_ReadMultipleCharValues',
        "fd92": 'GATT_WriteCharValue',
        "fd96": 'GATT_WriteLongCharValue',
        "fdb2": 'GATT_DiscAllChars',
        "fdb4": 'GATT_ReadUsingCharUUID',
        "fe00": 'GAP_DeviceInit',
        "fe03": 'GAP_ConfigureDeviceAddr',
        "fe04": 'GATT_DeviceDiscoveryRequest',
        "fe05": 'GATT_DeviceDiscoveryCancel',
        "fe09": 'GATT_EstablishLinkRequest',
        "fe0a": 'GATT_TerminateLinkRequest',
        "fe30": 'GAP_SetParam',
        "fe31": 'GAP_GetParam',
        "fd9b": 'GATT_Notification',
    }

'''=======================================
                Builder
======================================='''

hci_cmds = {
        "fd8a": [
            {'name': 'conn_handle', 'len': 2, 'default': '\x00\x00'},
            {'name': 'handle', 'len': 2, 'default': None}],
        "fd8e": [
            {'name': 'conn_handle', 'len': 2, 'default': '\x00\x00'},
            {'name': 'handles', 'len': None, 'default': None}],
        "fd92": [
            {'name': 'conn_handle', 'len': 2, 'default': '\x00\x00'},
            {'name': 'handle', 'len': 2, 'default': None},
            {'name': 'value', 'len': None, 'default': None}],
        "fd96": [
            {'name': 'handle', 'len': 2, 'default': '\x00\x00'},
            {'name': 'offset', 'len': 1, 'default': None},
            {'name': 'value', 'len': None, 'default': None}],
        "fdb2": [
            {'name': 'start_handle', 'len': 2, 'default': '\x00\x00'},
            {'name': 'end_handle', 'len': 2, 'default': '\xff\xff'}],
        "fdb4": [
            {'name': 'conn_handle', 'len': 2, 'default': '\x00\x00'},
            {'name': 'start_handle', 'len': 2, 'default': '\x01\x00'},
            {'name': 'end_handle', 'len': 2, 'default': '\xff\xff'},
            {'name': 'read_type', 'len': 2, 'default': None}],
        "fe00": [
            {'name': 'profile_role', 'len': 1, 'default': '\x08'},
            {'name': 'max_scan_rsps', 'len': 1, 'default': '\x05'},
            {'name': 'irk', 'len': 16, 'default':
                '\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'},
            {'name': 'csrk', 'len': 16, 'default':
                '\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'},
            {'name': 'sign_counter', 'len': 4, 'default': '\x01\x00\x00\x00'}],
        "fe03": [
            {'name': 'addr_type', 'len': 1, 'default': None},
            {'name': 'addr', 'len': 6, 'default': None}],
        "fe04": [
            {'name': 'mode', 'len': 1, 'default': None},
            {'name': 'active_scan', 'len': 1, 'default': '\x01'},
            {'name': 'white_list', 'len': 1, 'default': '\x00'}],
        "fe05": [],
        "fe09": [
            {'name': 'high_duty_cycle', 'len': 1, 'default': '\x00'},
            {'name': 'white_list', 'len': 1, 'default': '\x00'},
            {'name': 'addr_type_peer', 'len': 1, 'default': '\x00'},
            {'name': 'peer_addr', 'len': 6, 'default': None}],
        "fe0a": [
            {'name': 'conn_handle', 'len': 2, 'default': '\x00\x00'}],
        "fe30": [
            {'name': 'param_id', 'len': 1, 'default': None},
            {'name': 'param_value', 'len': 2, 'default': None}],
        "fe31": [
            {'name': 'param_id', 'len': 1, 'default': None}],
        
        "fd9b": [
            {'name': 'conn_handle', 'len': 2, 'default': '\x00\x00'},
            {'name': 'authenticated', 'len': 1, 'default': '\x00'},
            {'name': 'handle', 'len': 2, 'default': '\x00\x00'},
            {'name': 'value', 'len': None, 'default': None}],
    }

'''=======================================
                Parser
======================================='''
hci_events = {
    "ff": {
        'name': 'HCI_LE_ExtEvent',
        'structure': [{'name': 'ext_event', 'len': None}]},
	"1a": {
		'name': 'Data_Buffer_Overflow'},
    }

    # parameter formats for HCI_LE_ExtEvent
ext_events = {
    "0501": {
        'name': 'ATT_ErrorRsp',
        'structure': [
            {'name': 'conn_handle', 'len': 2},
            {'name': 'pdu_len', 'len': 1},
            {'name': 'req_op_code', 'len': 1},
            {'name': 'handle', 'len': 2},
            {'name': 'error_code', 'len': 1}]},
    "0509": {
        'name': 'ATT_ReadByTypeRsp',
        'structure': [
            {'name': 'conn_handle', 'len': 2},
            {'name': 'pdu_len', 'len': 1},
            {'name': 'length', 'len': 1},
            {'name': 'results', 'len': None}],
        'parsing': [
            ('results', lambda ble, original:
                ble._parse_read_results(original['results']))]},
    "050b": {
        'name': 'ATT_ReadRsp',
        'structure': [
            {'name': 'conn_handle', 'len': 2},
            {'name': 'pdu_len', 'len': 1},
            {'name': 'value', 'len': None}]},
    "050f": {
        'name': 'ATT_ReadMultiRsp',
        'structure': [
            {'name': 'conn_handle', 'len': 2},
            {'name': 'pdu_len', 'len': 1},
            {'name': 'results', 'len': None}]},
    "0513": {
        'name': 'ATT_WriteRsp',
        'structure': [
            {'name': 'conn_handle', 'len': 2},
            {'name': 'pdu_len', 'len': 1}]},
    "051b": {
        'name': 'ATT_HandleValueNotification',
        'structure': [
            {'name': 'conn_handle', 'len': 2},
            {'name': 'pdu_len', 'len': 1},
            {'name': 'handle', 'len': 2},
            {'name': 'values', 'len': None}]},
    "0600": {
        'name': 'GAP_DeviceInitDone',
        'structure': [
            {'name': 'dev_addr', 'len': 6},
            {'name': 'data_pkt_len', 'len': 2},
            {'name': 'num_data_pkts', 'len': 1},
            {'name': 'irk', 'len': 16},
            {'name': 'csrk', 'len': 16}]},
    "0601": {
        'name': 'GAP_DeviceDiscoveryDone',
        'structure': [
            {'name': 'num_devs', 'len': 1},
            {'name': 'devices', 'len': None}],
        'parsing': [
            ('devices', lambda ble, original:
                ble._parse_devices(original['devices']))]},
    "0605": {
        'name': 'GAP_EstablishLink',
        'structure': [
            {'name': 'dev_addr_type', 'len': 1},
            {'name': 'dev_addr', 'len': 6},
            {'name': 'conn_handle', 'len': 2},
            {'name': 'conn_interval', 'len': 2},
            {'name': 'conn_latency', 'len': 2},
            {'name': 'conn_timeout', 'len': 2},
            {'name': 'clock_accuracy', 'len': 1}]},
    "060d": {
        'name': 'GAP_DeviceInformation',
        'structure': [
            {'name': 'event_type', 'len': 1},
            {'name': 'addr_type', 'len': 1},
            {'name': 'addr', 'len': 6},
            {'name': 'rssi', 'len': 1},
            {'name': 'data_len', 'len': 1},
            {'name': 'data_field', 'len': None}]},
    "0606": {
        'name': 'GAP_LinkTerminated',
        'structure': [
            {'name': 'conn_handle', 'len': 2},
            {'name': 'reason', 'len': 1}]},
    "067f": {
        'name': 'GAP_HCI_ExtensionCommandStatus',
        'structure': [
            {'name': 'op_code', 'len': 2},
            {'name': 'data_len', 'len': 1},
            {'name': 'param_value', 'len': None}],
        'parsing': [
            ('op_code', lambda ble, original:
                ble._parse_opcodes(original['op_code']))]},
    "0607":{
	'name': 'GAP_LinkParamUpdate',
        'structure': [
            {'name': 'conn_handle', 'len': 2},
            {'name': 'conn_interval', 'len': 2},
	    {'name': 'conn_latency', 'len': 2},
	    {'name': 'conn_timeout', 'len': 2}]}
    }

'''======================================================
        print_output ==> Return the information list
        analyse_packet ==> Response from device &
                           print the print_output by ParseBLERSP function
======================================================'''
Sync="-1";

def ParseBLERSP(result,RecvPkt):
    global Sync
    sliceitem=result.split(')')
    RSPDict={}

    #------Parse value
    for i in sliceitem:
        i1=i.split(':')[0].split()
        key=' '.join(i1)
        value=i[i.find('(')+1:]

        RSPDict[key]=value
    
    #=====Event [GAP_DeviceInitDone, GAP_EstablishLink, ATT_HandleValueNotification]
    if RSPDict['Event']=="GAP_DeviceInitDone":
        print "GAP_DeviceInitDone"
        
    if RSPDict['Event']=="GAP_EstablishLink":
        print "GAP_EstablishLink","ADDR:",RSPDict['Dev Addr'],"Conn Handle:",RSPDict['Conn Handle']
    
    if RSPDict['Event']=="ATT_HandleValueNotification":
        
        print datetime.datetime.now().strftime("%H:%M:%S.%f"),"Conn:",RSPDict['Conn Handle'],"Handle:",RSPDict['Handle']," Values:",RSPDict['Values'],
        print " ",int(RSPDict['Values'][30:38],16)," ",int(RSPDict['Values'][22:30],16),
        print " ",int(RSPDict['Values'][22:30],16)-int(RSPDict['Values'][30:38],16)
        
        
        Pkt=int(RSPDict['Values'][22:30],16)
        RecvPkt.append(Pkt)
        
        '''
        if RSPDict['Handle'].find("9")>0:
            TDMAState[0]=True
        '''        
    del RSPDict
    
def pretty(hex_string, seperator=' '):
    """
    Prettify a hex string.

    >>> pretty("\x01\x02\x03\xff")
    '01 02 03 FF'
    """
    hex_string = hex_string.encode('hex')
    out = ''

    for i in range(len(hex_string)):
        if not i % 2:
            out = out + seperator
        out = out + hex_string[i].capitalize()

    return out


def print_ordered_dict(dictionary):
    result = ""
    for key in dictionary:
        if dictionary[key]:
            #convert e.g. "data_len" -> "Data Len"
            title = ' '.join(key.split("_")).title()
            if isinstance(dictionary[key], list):
                for idx2, _ in enumerate(dictionary[key]):
                    result += "{0} ({1})\n".format(title, idx2)
                    result += print_ordered_dict(dictionary[key][idx2])
            elif isinstance(dictionary[key], type(collections.OrderedDict())):
                result += '{0}\n{1}'.format(title, print_ordered_dict(
                    dictionary[key]))
            else:
                result += "{0:15}\t: {1}\n\t\t  ({2})\n".format(
                    title, pretty(dictionary[key][0], ':'), dictionary[key][1])
        else:
            result += "{0:15}\t: None".format(key)
    return result


def print_output((packet, dictionary)):
    result = print_ordered_dict(dictionary)
    result += 'Dump:\n{0}\n'.format(pretty(packet))
    return result

'''
def analyse_packet((packet, dictionary)):
    #print("EVENT: Response received from the device")
    ParseBLERSP(print_output((packet, dictionary)))
'''

    
