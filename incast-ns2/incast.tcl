Class TCP_pair

TCP_pair instproc init {args} {
        $self instvar tcps tcpr; # Sender TCP,  Receiver TCP
        eval $self next $args
        $self set tcps [new Agent/TCP/FullTcp/Sack]
        $self set tcpr [new Agent/TCP/FullTcp/Sack]
}

TCP_pair instproc setup {snode dnode} {
        global ns
        $self instvar tcps tcpr; # Sender TCP,  Receiver TCP

        $ns attach-agent $snode $tcps;
        $ns attach-agent $dnode $tcpr;

        $tcpr listen
        $ns connect $tcps $tcpr
}

TCP_pair instproc send { num_bytes } {
        $self instvar tcps

        $tcps set signal_on_empty_ TRUE
        $tcps advance-bytes $num_bytes

        #puts "Run send"
}

TCP_pair instproc set_incast_callback { incast_group } {
        $self instvar tcps
        $tcps set_incast_callback $incast_group
}

Agent/TCP/FullTcp instproc set_incast_callback {incast_group} {
        $self instvar incast_ctrl
        $self set incast_ctrl $incast_group
}

Agent/TCP/FullTcp instproc done_data {} {
        $self instvar incast_ctrl

        #puts "Run done_data"

        if { [info exists incast_ctrl] } {
                $incast_ctrl fin_notify
        }
}

Class Incast_group

Incast_group instproc init {args} {
        $self instvar num_pairs

        $self set num_pairs 0
        eval $self next $args
}

Incast_group instproc add_pair { tcp_pair } {
        $self instvar num_pairs tcp_pairs

        $self set tcp_pairs($num_pairs) $tcp_pair
        $self set num_pairs [expr $num_pairs + 1]

        $tcp_pair set_incast_callback $self
}

set jitter_rng [new RNG]
$jitter_rng seed 5251

Incast_group instproc send_per_pair { jitter_period num_bytes } {
        global ns jitter_rng
        $self instvar fin_pairs num_pairs tcp_pairs start_time

        $self set fin_pairs 0
        $self set start_time [$ns now]

        #puts "Run send_per_pair"
        for {set i 0} {$i < $num_pairs} {incr i} {
                set next_time [expr [$ns now] + [$jitter_rng uniform 0.0 $jitter_period]]
                $ns at $next_time "$tcp_pairs($i) send $num_bytes"
        }
}

Incast_group instproc fin_notify {} {
        global ns start_time

        $self instvar fin_pairs num_pairs warmup_mode num_iter tot_num_bytes jitter_time start_time
        $self set fin_pairs [expr $fin_pairs + 1]

        if { $fin_pairs == $num_pairs } {
                set duration [expr [$ns now] - $start_time]
                set goodput [expr [$self set tot_num_bytes] * 8 / $duration / 1000000000]

                if { $warmup_mode != 1} {
                        puts "($num_iter) Time $duration seconds Goodput $goodput Gbps"
                        $self set num_iter [expr $num_iter - 1]

                        if { $num_iter > 0} {
                                $self send_per_pair $jitter_time [expr $tot_num_bytes / $num_pairs]
                        } else {
                                exit 0
                        }
                }
        }
}

Incast_group instproc warmup { jitter_period num_bytes } {
        $self instvar warmup_mode tot_num_bytes num_pairs
        $self set warmup_mode 1
        $self set tot_num_bytes [expr $num_pairs * $num_bytes]; # total size in bytes

        $self send_per_pair $jitter_period $num_bytes
}

Incast_group instproc schedule { jitter_period num_bytes iter } {
        $self instvar warmup_mode jitter_time num_iter num_pairs tot_num_bytes
        $self set warmup_mode 0
        $self set num_iter $iter; # iterations
        $self set tot_num_bytes [expr $num_pairs * $num_bytes]; # total size in bytes
        $self set jitter_time $jitter_period

        $self send_per_pair $jitter_period $num_bytes
}

set num_sender [lindex $argv 0] ; # number of senders
set bytes_per_sender [lindex $argv 1] ; # size of traffic in bytes per sender
set link_rate [lindex $argv 2] ; # link rate (Gbps)
set rtt [lindex $argv 3] ;
set buffer_size [lindex $argv 4] ; # buffer size in packets
set rto_min [lindex $argv 5] ; # RTOmin

set ns [new Simulator]
set packet_size 1460

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

set switch [$ns node]
set receiver [$ns node]
$ns duplex-link $receiver $switch [set link_rate]Gb [expr $rtt/4] DropTail

set incast [new Incast_group]

for {set i 0} {$i < $num_sender} {incr i} {
        set senders($i) [$ns node]
        $ns duplex-link $senders($i) $switch [set link_rate]Gb [expr $rtt/4] DropTail

        set tcp_pairs($i) [new TCP_pair]
        $tcp_pairs($i) setup $senders($i) $receiver

        $incast add_pair $tcp_pairs($i)
}

$ns at 0.1 "$incast warmup 0.00000 100"
$ns at 1 "$incast schedule 0.00000 $bytes_per_sender 10"
$ns run
