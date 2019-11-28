import binascii
import ptf 
import ptf.packet as scapy
import ptf.dataplane as dataplane

from ptf.base_tests import BaseTest
from ptf.testutils import *
from ptf.mask import Mask 

def icmp_filter(pkt_str):
    try:
        pkt = scapy.Ether(pkt_str)
        return (scapy.ICMP in pkt)
    except:
        return False 
    
class IcmpTest(BaseTest):
    def __init__(self):
        BaseTest.__init__(self)
        self.test_params = test_params_get()
    
    def setUp(self):
        add_filter(icmp_filter)
        self.dataplane = ptf.dataplane_instance
        self.dataplane.flush()
        
        # Index of local interface
        self.local_port = self.test_params['local_port']
        # Local MAC address
        self.local_mac = self.test_params['local_mac']
        # Remote MAC address
        self.remote_mac = self.test_params['remote_mac']
        # Local IP address
        self.local_ip = self.test_params['local_ip']
        # Remote IP address
        self.remote_ip = self.test_params['remote_ip']
        # Number of ICMP packets 
        self.count = self.test_params['count']
                            
    def runTest(self):
        pkt = simple_icmp_packet(
                eth_src = self.local_mac,
                eth_dst = self.remote_mac,
                ip_src = self.local_ip,
                ip_dst = self.remote_ip,
                icmp_type = 8, # Echo
                icmp_code = 0,
                icmp_data = '')
        
        exp_pkt = simple_icmp_packet(
                    eth_src = self.remote_mac,
                    eth_dst = self.local_mac,
                    ip_src = self.remote_ip,
                    ip_dst = self.local_ip,
                    icmp_type = 0, # Echo Reply
                    icmp_code = 0,
                    icmp_data = '')
        
        m = Mask(exp_pkt)
        m.set_do_not_care_scapy(IP, 'ip_ttl')
        m.set_do_not_care_scapy(IP, 'id')
        m.set_do_not_care_scapy(IP, 'chksum')
        #m.set_do_not_care_scapy(ICMP, 'chksum')
        #m.set_do_not_care_scapy(ICMP, 'icmp_data')
                        
        for i in range(self.count):
            send_packet(self, self.local_port, pkt)
            verify_packet(self, m, self.local_port)
    
    def tearDown(self):   
        reset_filters()
        BaseTest.tearDown(self)
        