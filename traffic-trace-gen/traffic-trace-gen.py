import sys
import random
import math

# print the usage of the program
def usage():
        print 'usage: python %s dist_file throughput num_flows out_file seed' % (sys.argv[0])
        print '         dist_file: file with flow size distribution' 
        print '         throughput: desired throughput in Mbps'
        print '         num_flows: number of flows to generate'
        print '         out_file: output flow trace file'
        print '         seed: seed for random number generator'

# read flow size distribution from the file
def load_dist_file(file_name):
        flow_dist = []
        dist_file = open(file_name, 'r')
	lines = dist_file.readlines()

	for line in lines:
                words = line.split()
		if len(words) == 2:
                        # (CDF, flow size)
		        flow_dist.append([float(words[1]), float(words[0])])
	dist_file.close()
        return flow_dist

# return the average flow size of a distribution
def dist_avg_size(dist):
        size = 0.0

        for i in range(1, len(dist)):
                prob = dist[i][0] - dist[i - 1][0]
                mean = (dist[i][1] + dist[i - 1][1]) / 2
                size = size + prob * mean

        return size        

# return next time interval in second 
def next_time(rate):
        return -math.log(1.0 - random.random()) / rate

# generate a flow size according to a distribution and a number x in [0, 1]
def gen_size(dist, x):
	i = 0
	for var in dist:
		if var[0] > x:
			x1 = dist[i-1][0]
			x2 = dist[i][0]
			y1 = dist[i-1][1]
			y2 = dist[i][1]
			val = (y2 - y1) / (x2 - x1) * x + (x2 * y1 - x1 * y2) / (x2 - x1)
			return int(val)
		elif var[0] == x:
			return int(var[1])
		else:
			i = i + 1
	return 0

# return the average flow size of flow size distribution
if __name__ == "__main__":
        if len(sys.argv) != 6:
                usage()
                sys.exit(1)
        
        dist_file_name = sys.argv[1]
        throughput = float(sys.argv[2])
        num_flows = int(sys.argv[3])
        out_file_name = sys.argv[4]
        seed = int(sys.argv[5])

        # read flow size distribution from the file
        flow_dist = load_dist_file(dist_file_name)
        print 'Load flow size distribution from file %s' % dist_file_name
        print flow_dist
        avg_size = dist_avg_size(flow_dist)
        print 'Average flow size %f B' % avg_size

        print 'Average throughput %f Mbps' % throughput

        # calculate average # of flows per second 
	rate = throughput * 1024 * 1024.0 / (avg_size * 8)
        print 'Average # of flows per second: %f' % rate

        # set seed for random number generator
        random.seed(seed)

        # generate flow trace and write it to a file
        print 'Write %d flows to %s' % (num_flows, out_file_name)
        out_file = open(out_file_name, 'w')
        start_time = 0.0
        for i in range(num_flows):
                start_time = start_time + next_time(rate)   
                out_file.write('%d %f\n' % (gen_size(flow_dist, random.random()), start_time))
        out_file.close() 




        
