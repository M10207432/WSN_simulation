import collections
import os,sys

os.sys.path.append('./')
from HCI_CMD import *

class GW_Builder(object):
    def __init__(self,ser=None):
        self.serial_port = ser
        
    def BLECMD(self, cmd, **kwargs):
        serial_port = self.serial_port
        
        packet_type = "\x01"
        op_code = cmd.decode('hex')[::-1]  # command code was human-readable
        data_len = "\x00"  # insert dummy value for length

        # check for matching command codes in dictionary and store the matching
        # packet format
        try:
                packet_structure = hci_cmds[cmd]
        except AttributeError:
                raise NotImplementedError("Command spec could not be found")

        packet_type_parsed = "Command"
        op_code_parsed = opcodes[cmd]
        data_len_parsed = "0"  # insert dummy value for length

        # command match found, hence start storing result
        built_packet = collections.OrderedDict()
        built_packet['type'] = (packet_type, packet_type_parsed)
        built_packet['op_code'] = (op_code, op_code_parsed)
        built_packet['data_len'] = (data_len, data_len_parsed)

        packet = ''
        packet += packet_type
        packet += op_code
        packet += data_len

        # build the packet in the order specified, by processing each
        # required value as needed
        for field in packet_structure:
                field_name = field['name']
                    
                field_len = field['len']
                # try to read this field's name from the function arguments dict
                try:
                        field_data = kwargs[field_name]
                # data wasn't given
                except KeyError:
                        # only a problem is the field has a specific length...
                        if field_len is not None:
                                #...or a default value
                                default_value = field['default']
                                if default_value:
                                        field_data = default_value
                                else:
                                        raise KeyError(
                                                    "The data provided for '%s' was not %d bytes long"
                                                    % (field_name, field_len))
                        # no specific length, hence ignore it
                        else:
                                field_data = None

                # ensure that the correct number of elements will be written
                if field_len and len(field_data) != field_len:
                        raise ValueError(
                                "The data provided for '%s' was not %d bytes long"
                                % (field_name, field_len))

                # add the data to the packet if it has been specified (otherwise
                # the parameter was of variable length and not given)
                if field_data:
                        packet += field_data
                        built_packet[field_name] = (
                                field_data, field_data.encode('hex'))
                        
        # finally, replace the dummy length value in the string
        length = hex(len(packet) - 4)  # get length of bytes after 4th (length)
        data_len = length[2:].zfill(2).decode('hex')  # change 0x2 -> \x02

        modified_packet = list(packet)
        modified_packet[3] = data_len
        packet = "".join(modified_packet)
        
        # and the dictionary
        data_len_parsed = data_len.encode('hex')
        built_packet['data_len'] = (data_len, data_len_parsed)

        serial_port.write(packet)
        return (packet, built_packet)
