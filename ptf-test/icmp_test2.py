import binascii
import ptf 
import ptf.packet as scapy
import ptf.dataplane as dataplane

from ptf.base_tests import BaseTest
from ptf.testutils import *
from ptf.mask import Mask 

def icmp_filter(pkt_str):
    try:
        pkt = scapy.ICMP(pkt_str)
        return True 
    
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
        
        # Index of interface to send packets
        self.tx_port = self.test_params['tx_port']
        # Index of interface to receive packets
        self.rx_port = self.test_params['rx_port']
        
        # Source MAC address of the transmitted packets
        self.src_mac = self.test_params['src_mac']
        # Destination MAC address of the transmitted packets
        self.dst_mac = self.test_params['dst_mac']
                
        # Source IP of the transmitted packets
        self.src_ip = self.test_params['src_ip']
        # Destination IP of the transmitted packets
        self.dst_ip = self.test_params['dst_ip']
        
        # Number of ICMP packets 
        self.count = self.test_params['count']
                            
    def runTest(self):
        pkt = simple_icmp_packet(
                eth_src = self.src_mac,
                eth_dst = self.dst_mac,
                ip_src = self.src_ip,
                ip_dst = self.dst_ip,
                icmp_type = 8, # Echo
                icmp_code = 0,
                icmp_data = '')
        
        exp_pkt = simple_icmp_packet(
                    eth_src = self.src_mac,
                    eth_dst = self.dst_mac,
                    ip_src = self.src_ip,
                    ip_dst = self.dst_ip,
                    icmp_type = 8, # Echo
                    icmp_code = 0,
                    icmp_data = '')
        
        m = Mask(exp_pkt)
        m.set_do_not_care_scapy(scapy.Ether, 'src')
        m.set_do_not_care_scapy(scapy.Ether, 'dst')
        m.set_do_not_care_scapy(scapy.IP, 'ttl')
        m.set_do_not_care_scapy(scapy.IP, 'id')
        m.set_do_not_care_scapy(scapy.IP, 'chksum')
                                        
        for i in range(self.count):
            send_packet(self, self.tx_port, pkt)
            verify_packet(self, m, self.rx_port)
    
    def tearDown(self):   
        reset_filters()
        BaseTest.tearDown(self)
        