set ns [new Simulator]

set link_rate [lindex $argv 0] ; # link rate (Gbps)
set mean_link_delay [lindex $argv 1]
set host_delay [lindex $argv 2]

set fattree_k [lindex $argv 3] ; # K for Fat-tree
set topology_x [lindex $argv 4] ; # over subscription ratio

set switchAlg DropTail

$ns color 0 Red
$ns color 1 Orange
$ns color 2 Yellow
$ns color 3 Green
$ns color 4 Blue
$ns color 5 Violet
$ns color 6 Brown
$ns color 7 Black

############# Topoplgy #########################
set topology_servers [expr int($fattree_k * $fattree_k * $fattree_k * $topology_x / 4)]
set topology_spt [expr int($fattree_k * $topology_x / 2)] ; # servers per ToR (spt)
set topology_edges [expr int($fattree_k * $fattree_k / 2)]
set topology_aggrs [expr int($fattree_k * $fattree_k / 2)]
set topology_cores [expr int($fattree_k * $fattree_k / 4)]

puts "Servers: $topology_servers"
puts "Servers per ToR: $topology_spt"
puts "Edge Switches: $topology_edges"
puts "Aggregation Switches: $topology_aggrs"
puts "Core Switches: $topology_cores"

######### Servers #########
for {set i 0} {$i < $topology_servers} {incr i} {
        set s($i) [$ns node]
}

######### Edge Switches #########
for {set i 0} {$i < $topology_edges} {incr i} {
        set edge($i) [$ns node]
        $edge($i) shape box
        $edge($i) color green
}

######### Aggregation Switches #########
for {set i 0} {$i < $topology_aggrs} {incr i} {
        set aggr($i) [$ns node]
        $aggr($i) shape box
        $aggr($i) color blue
}

######### Core Switches #########
for {set i 0} {$i < $topology_cores} {incr i} {
        set core($i) [$ns node]
        $core($i) shape box
        $core($i) color red
}

######### Links from Servers to Edge Switches #########
for {set i 0} {$i < $topology_servers} {incr i} {
        set j [expr $i / $topology_spt] ; # ToR ID
        $ns duplex-link $s($i) $edge($j) [set link_rate]Gb [expr $host_delay + $mean_link_delay] $switchAlg
}

######### Links from Edge to Aggregation Switches #########
for {set i 0} {$i < $topology_edges} {incr i} {

        set pod [expr $i / ($fattree_k / 2)] ; # pod ID
        set start [expr $pod * $fattree_k / 2]
        set end [expr $start + $fattree_k / 2]

        for {set j $start} {$j < $end} {incr j} {
                $ns duplex-link $edge($i) $aggr($j) [set link_rate]Gb [expr $mean_link_delay] $switchAlg
        }
}

######### Links from Aggregation to Core Switches #########
for {set i 0} {$i < $topology_aggrs} {incr i} {

        set index [expr $i % ($fattree_k / 2)] ; # index in pod
        set start [expr $index * $fattree_k / 2]
        set end [expr $start + $fattree_k / 2]

        for {set j $start} {$j < $end} {incr j} {
                $ns duplex-link $aggr($i) $core($j) [set link_rate]Gb [expr $mean_link_delay] $switchAlg
        }
}

############### NAM ###########################
set namfile [open out.nam w]
$ns namtrace-all $namfile

proc finish {} {
        global namfile
        close $namfile
}

$ns at 2.0 "finish"

$ns run
