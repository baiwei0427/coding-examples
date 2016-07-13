source "tcp-traffic-gen.tcl"

set ns [new Simulator]
set flowlog [open flow.tr w]
set debug_mode 1
set sim_start [clock seconds]
set sim_end 1000
set init_fid 0
set flow_gen 0
set flow_fin 0
set packet_size 1460

set flow_cdf CDF_dctcp.tcl
set mean_flow_size [expr 1138 * 1460]

set num_sender 40
set connections_per_pair 5
set link_rate 10
set load 0.8
set rtt 0.0001
set buffer_size 250
set rto_min 0.01

Agent/TCP set windowInit_ 10
Agent/TCP set packetSize_ $packet_size
Agent/TCP set window_ 1256
Agent/TCP set slow_start_restart_ false
Agent/TCP set tcpTick_ 0.000001 ; # 1us should be enough
Agent/TCP set minrto_ $rto_min
Agent/TCP set rtxcur_init_ $rto_min ; # initial RTO
Agent/TCP set windowOption_ 0

Agent/TCP/FullTcp set nodelay_ true; # disable Nagle
Agent/TCP/FullTcp set segsize_ $packet_size
Agent/TCP/FullTcp set segsperack_ 1 ; # ACK frequency
Agent/TCP/FullTcp set interval_ 0.000006 ; #delayed ACK interval

Queue set limit_ $buffer_size

set lambda [expr ($link_rate * $load * 1000000000)/($mean_flow_size * 8.0 / $packet_size * ($packet_size + 40))]
puts "Arrival: Poisson with inter-arrival [expr 1 / $lambda * 1000] ms"
puts "Average flow size: $mean_flow_size bytes"
puts "Setting up connections ..."; flush stdout

set switch [$ns node]
set receiver [$ns node]
$ns duplex-link $receiver $switch [set link_rate]Gb [expr $rtt / 4] DropTail

for {set i 0} {$i < $num_sender} {incr i} {
        set senders($i) [$ns node]
        $ns duplex-link $senders($i) $switch [set link_rate]Gb [expr $rtt/4] DropTail

        set agtagr($i) [new Agent_Aggr_pair]
        $agtagr($i) setup $senders($i) $receiver "$i" $connections_per_pair "TCP_pair" "Agent/TCP/FullTcp/Sack"
        $agtagr($i) set_PCarrival_process [expr $lambda / ($num_sender - 1)] $flow_cdf [expr 17 * $i] [expr 33 * $i]
        $agtagr($i) attach-logfile $flowlog

        $ns at 0.1 "$agtagr($i) warmup 0.5 $packet_size"
        $ns at 1 "$agtagr($i) init_schedule"
}

puts "Simulation starts!"
$ns run
