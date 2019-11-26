import binascii
import ptf 
import ptf.packet as scapy
import ptf.dataplane as dataplane

from ptf.base_tests import BaseTest
from ptf.testutils import *

"""
 Return a Priority-based Flow Control (PFC) pause frame
 
 Supports a few parameters:
 @param priority: priority to pause [0, 7]
 @param pause_dur: pause duration [0, 65535]
 @param source_mac: source mac address   
    
 -----------------
 01:80:c2:00:00:01 
 -----------------
    Station MAC 
      Address
 -----------------
      0x8808
 -----------------
      0x0001
 -----------------
       Time
 -----------------
       Pad
 -----------------  
"""
def fc_packet(pause_dur = 65535, source_mac = '00:11:22:33:44:55'):   
    if pause_dur < 0 or pause_dur > 65535:
		return None 
        
    eth_hdr = scapy.Ether(dst = '01:80:c2:00:00:01', src = source_mac, type = 0x8808)  
    
    opcode = '\x00\x01'
    classtime = binascii.unhexlify(format(pause_dur, '04x'))     
    pad = '\x00' * 42
    
    payload = opcode + classtime + pad 
    pkt = eth_hdr / payload
    
    return pkt 

class FcTest(BaseTest):
    def __init__(self):
        BaseTest.__init__(self)
        self.test_params = test_params_get()
    
    def setUp(self):
        self.dataplane = ptf.dataplane_instance
        # Index of source interface
        self.src_port = self.test_params['src_port']
        # Number of PFC pause frames to send
        self.count = self.test_params['count']
  
    def runTest(self):
        fc_pkt = fc_packet()
        if fc_pkt == None:
            return False 
		
        send_packet(self, self.src_port, fc_pkt, self.count)
        return True 