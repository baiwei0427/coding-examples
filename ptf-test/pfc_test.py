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
      0x0101
 -----------------
   class vector
 -----------------
   Time (Class 0) 
 -----------------
   Time (Class 1)
 -----------------
   Time (Class 2)
 -----------------
   Time (Class 3)
 -----------------
   Time (Class 4)
 -----------------
   Time (Class 5)
 -----------------
   Time (Class 6)
 -----------------
   Time (Class 7)
 -----------------
       Pad
 -----------------  
"""
def pfc_packet(priority, pause_dur = 65535, source_mac = '00:11:22:33:44:55'):
    if priority < 0 or priority > 7:
		return None 
    
    if pause_dur < 0 or pause_dur > 65535:
		return None 
        
    eth_hdr = scapy.Ether(dst = '01:80:c2:00:00:01', src = source_mac, type = 0x8808)  
    
    opcode = '\x01\x01'
    # 04x = 4 * 4 bits = 2 bytes
    classvector = binascii.unhexlify(format(1 << priority, '04x')) 
    
    classtime = ''
    for prio in range(0, 8):
        if prio == priority:
            classtime = classtime + binascii.unhexlify(format(pause_dur, '04x')) 
        else:
            classtime = classtime + '\x00\x00'
    
    pad = '\x00' * 26
    payload = opcode + classvector + classtime + pad 
    pkt = eth_hdr / payload
    
    return pkt 

class PfcTest(BaseTest):
    def __init__(self):
        BaseTest.__init__(self)
        self.test_params = test_params_get()
    
    def setUp(self):
        self.dataplane = ptf.dataplane_instance
        # Priority to generate pause frame
        self.priority = self.test_params['priority']
        # Index of source interface
        self.src_port = self.test_params['src_port']
        # Number of PFC pause frames to send
        self.count = self.test_params['count']
  
    def runTest(self):
        pfc_pkt = pfc_packet(priority = self.priority)
        if pfc_pkt == None:
            return False 
		
        send_packet(self, self.src_port, pfc_pkt, self.count)
        return True 