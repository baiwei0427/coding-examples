# Global Variables
# - ns: network Simulator
# - flowlog: log file to record flow comlpetion information
# - debug_mode: 1 (output necessary information for debug) or 0 (no output)
# - sim_start: simulation start time
# - flow_tot: total number of flows to generate
# - init_fid: flow ID
# - flow_gen: number of flows that have been generated
# - flow_fin: number of flows that have finished

Agent/TCP/FullTcp instproc set_callback {tcp_pair func} {
        $self instvar ctrl cb_func
        $self set ctrl $tcp_pair
        $self set cb_func $func
}

######### This function is called when the TCP send buffer becomes empty =======
Agent/TCP/FullTcp instproc done_data {} {
        $self instvar ctrl cb_func

        if { [info exists ctrl] } {
                $ctrl $cb_func
        }
}

#
# TCP_pair has
#
# Variables
# - groud_id (gid) : 'src, dst'
# - pair_id (pid) : index of connection among the group
# - flow_id (fid) : unique flow identifier for this connection (group_id, pair_id)
# - is_busy : whether the connection is busy transmitting data now
# - tcps: sender TCP agent
# - tcpr: receiver TCP agent
# - start_time: flow start time
# - bytes: flow size in bytes
# - fct: flow comlpetion time in seconds
# - timeouts: number of TCP timeouts
# - aggr_ctrl: Agent_Aggr_pair for callback
# - aggr_cb_func: Agent_Aggr_pair callback function
#
# Functions
# - set_up {snode dnode}: init TCP agents and attach them to two nodes
# - set_group_id {gid}: set group ID
# - set_pair_id {pid}: set pair ID
# - set_flow_id {fid}: set flow ID
# - send {nr_bytes}: generate a flow with nr_bytes
# - warmup {nr_bytes}: generate a flow with nr_bytes for warmup
# - set_callback {controller func}: register control Agent_Aggr_pair and callback function
# - fin_notify {}: this is called by Agent_Aggr_pair when the flow finishes

Class TCP_pair

TCP_pair instproc init {args} {
        $self instvar group_id pair_id flow_id is_busy timeouts

        eval $self next $args
        $self set group_id 0
        $self set pair_id 0
        $self set flow_id 0
        $self set is_busy 0
        $self set timeouts 0
}

######## Initialize TCP agents and attach them to sender/receiver nodes ########
TCP_pair instproc setup {snode dnode tcp_type} {
        global ns
        $self instvar tcps tcpr; # Sender TCP,  Receiver TCP

        $self set tcps [new $tcp_type]
        $self set tcpr [new $tcp_type]

        $ns attach-agent $snode $tcps
        $ns attach-agent $dnode $tcpr

        $tcpr listen
        $ns connect $tcps $tcpr

        $tcps set_callback $self fin_notify
}

TCP_pair instproc set_group_id { gid } {
        $self instvar group_id
        $self set group_id $gid
}

TCP_pair instproc set_pair_id { pid } {
        $self instvar pair_id
        $self set pair_id $pid
}

TCP_pair instproc set_flow_id { fid } {
        $self instvar flow_id tcps tcpr

        $self set flow_id $fid
        $tcps set fid_ $fid;
        $tcpr set fid_ $fid;
}

###################### Generate a flow with nr_bytes bytes #####################
TCP_pair instproc send { nr_bytes } {
        global ns flow_tot debug_mode
        $self instvar tcps tcpr is_busy group_id flow_id
        $self instvar start_time bytes

        $self set is_busy 1; # the connection is transmitting data now
        $self set start_time [$ns now]; # start time of this flow
        $self set bytes $nr_bytes; # flow size in bytes

        if {$debug_mode == 1} {
                puts "[$ns now] send group $group_id flow $flow_id $nr_bytes bytes"
        }

        $tcps set signal_on_empty_ TRUE; # call done_data() when the flow finishes
        $tcps advance-bytes $nr_bytes
}

TCP_pair instproc warmup { nr_bytes } {
        global ns debug_mode
        $self instvar tcps group_id flow_id

        if {$debug_mode == 1} {
                puts "[$ns now] warmup group $group_id flow $flow_id $nr_bytes bytes"
        }

        $tcps advance-bytes $nr_bytes
}

TCP_pair instproc set_callback { controller func } {
        $self instvar aggr_ctrl aggr_cb_func
        $self set aggr_ctrl $controller
        $self set aggr_cb_func $func
}

######### This function is called by done_data() of Agent/TCP/FullTcp ##########
TCP_pair instproc fin_notify {} {
        global ns debug_mode
        $self instvar tcps aggr_ctrl aggr_cb_func
        $self instvar is_busy group_id pair_id flow_id
        $self instvar start_time bytes; # flow start time, flow size
        $self instvar fct timeouts; # flow completion time, TCP timeouts

        $self set is_busy 0; # the connection becomes available

        set ct [$ns now]
        $self set fct [expr $ct - $start_time]

        set old_timeouts $timeouts
        $self set timeouts [$tcps set nrexmit_]
        set flow_timeouts [expr $timeouts - $old_timeouts]

        if {$debug_mode == 1} {
                puts "$ct finish group $group_id flow $flow_id $bytes bytes $fct sec $flow_timeouts timeouts"
        }

        if { [info exists aggr_ctrl] } {
                ## callback function    ######## Write flow information to a log file #############
                $aggr_ctrl $aggr_cb_func $pair_id $bytes $fct $flow_timeouts
        }
}

Class Agent_Aggr_pair

Agent_Aggr_pair instproc init {args} {
        eval $self next $args
}

Agent_Aggr_pair instproc attach-logfile { logf } {
        $self instvar logfile
        $self set logfile $logf
}

Agent_Aggr_pair instproc setup {s_node d_node gid nr pair_type tcp_type} {
        global init_fid
        $self instvar snode dnode; #sender and destination nodes
        $self instvar pairs; #connection pairs
        $self instvar group_id; #ID of this group
        $self instvar nr_pairs nr_busy_pairs; #number of pairs, number of busy pairs
        $self instvar agent_pair_type; #type of connection pairs
        $self instvar pair_tcp_type; #type of TCP used by connection pairs

        $self set snode $s_node
        $self set dnode $d_node
        $self set group_id $gid
        $self set nr_pairs $nr
        $self set nr_busy_pairs 0; #no connection is busy now
        $self set agent_pair_type $pair_type
        $self set pair_tcp_type $tcp_type

        for {set i 0} {$i < $nr_pairs} {incr i} {
                $self set pairs($i) [new $agent_pair_type]
                $pairs($i) setup $snode $dnode $pair_tcp_type
                $pairs($i) set_group_id $group_id; #let each pair know its group ID
                $pairs($i) set_pair_id $i; #assign pair ID
                $pairs($i) set_flow_id $init_fid; #assign global flow ID
                $pairs($i) set_callback $self fin_notify; #register controller and callback function

                incr init_fid
        }
}

#- PCarrival:
#flow arrival: poisson with rate $lambda
#flow size: custom defined expirical cdf
Agent_Aggr_pair instproc set_PCarrival_process {lambda cdffile rands1 rands2} {
        $self instvar rx_flow_interval rx_flow_size

        set rng1 [new RNG]
        $rng1 seed $rands1

        $self set rx_flow_interval [new RandomVariable/Exponential]
        $rx_flow_interval use-rng $rng1
        $rx_flow_interval set avg_ [expr 1.0/$lambda]

        set rng2 [new RNG]
        $rng2 seed $rands2

        $self set rx_flow_size [new RandomVariable/Empirical]
        $rx_flow_size use-rng $rng2
        $rx_flow_size set interpolation_ 2
        $rx_flow_size loadCDF $cdffile
}

set warmup_rng [new RNG]; #random number generator
$warmup_rng seed 5251

Agent_Aggr_pair instproc warmup {jitter_period nr_bytes} {
        global ns warmup_rng
        $self instvar pairs nr_pairs

        for {set i 0} {$i < $nr_pairs} {incr i} {
                $ns at [expr [$ns now] + [$warmup_rng uniform 0.0 $jitter_period]] "$pairs($i) warmup $nr_bytes"
        }
}

Agent_Aggr_pair instproc init_schedule {} {
        global ns
        $self instvar tnext rx_flow_interval

        set dt [$rx_flow_interval value]
        $self set tnext [expr [$ns now] + $dt]
        $ns at $tnext "$self schedule"
}

Agent_Aggr_pair instproc schedule {} {
        global ns flow_gen flow_tot debug_mode init_fid
        $self instvar group_id
        $self instvar snode dnode
        $self instvar tnext rx_flow_interval rx_flow_size
        $self instvar pairs nr_pairs nr_busy_pairs
        $self instvar agent_pair_type pair_tcp_type

        ## we have generate enough flows
        if {$flow_gen >= $flow_tot} {
                return
        }

        ## if no available connection
        if {$nr_busy_pairs == $nr_pairs} {
                if {$debug_mode == 1} {
                        puts "[$ns now]: create new connection $nr_pairs for group $group_id"
                }

                $self set pairs($nr_pairs) [new $agent_pair_type]
                $pairs($nr_pairs) setup $snode $dnode $pair_tcp_type
                $pairs($nr_pairs) set_group_id $group_id; #let each pair know its group ID
                $pairs($nr_pairs) set_pair_id $nr_pairs; #assign pair ID
                $pairs($nr_pairs) set_flow_id $init_fid; #assign global flow ID
                $pairs($nr_pairs) set_callback $self fin_notify; #register controller and callback function

                incr init_fid; #update global flow ID
                incr nr_pairs; #increase the number of pairs for this group

                if {$flow_gen < $flow_tot} {
                        incr flow_gen
                        incr nr_busy_pairs
                        set id [expr $nr_pairs - 1]
                        set nbytes [expr max([$rx_flow_size value], 2)]
                        $ns at-now "$pairs($id) send $nbytes"
                }
        ## if there are some available connections
        } else {
                set id [expr -1]
                for {set i 0} {$i < $nr_pairs} {incr i} {
                        if {[$pairs($i) set is_busy] == 0} {
                                set id $i
                                break
                        }
                }

                if {$id < 0 && $debug_mode == 1} {
                        puts "[$ns now]: cannot find available connection $snode -> $dnode"
                }

                if {$flow_gen < $flow_tot} {
                        incr flow_gen
                        incr nr_busy_pairs
                        set nbytes [expr max([$rx_flow_size value], 2)]
                        $ns at-now "$pairs($id) send $nbytes"
                }
        }

        if {$flow_gen < $flow_tot} {
                set dt [$rx_flow_interval value]
                $self set tnext [expr [$ns now] + $dt]
                $ns at $tnext "$self schedule"
        }
}


########### This function is called by fin_notify() of TCP_pair ############
# pid: pair id
# bytes: flow size in bytes
# fct: flow completion time in seconds
# timeouts: number of TCP timeouts this flow experiences
Agent_Aggr_pair instproc fin_notify {pid bytes fct timeouts} {
        global flow_fin flow_tot
        $self instvar logfile
        $self instvar nr_busy_pairs; # the number of busy pairs
        $self instvar group_id

        $self set nr_busy_pairs [expr $nr_busy_pairs - 1]

        ######## Write flow statistic information to a log file #############
        if { [info exists logfile] } {
                puts $logfile "$bytes $fct $timeouts $group_id"
        }

        incr flow_fin; # increase the total number of finished flows
        if {$flow_fin >= $flow_tot} {
                finish
        }
}

proc finish {} {
        global ns flowlog
        global sim_start

        $ns flush-trace
        close $flowlog

        set t [clock seconds]
        puts "Simulation Finished!"
        puts "Time [expr $t - $sim_start] sec"

        exit 0
}
