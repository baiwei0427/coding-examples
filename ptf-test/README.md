# How to run PFC/FC Tests

In the following example, we generate 10 global pause frames and 10 priority-based pause frames at priority 3. The interface is ens1d1.

<pre><code>sudo ptf --test-dir ./ --interfac 0@ens1d1 --test-params="src_port=0;count=10;priority=3"
</code></pre>

# How to run ICMP Test

In the following example, we send 10 ICMP request packets from 192.168.8.129 to 192.168.8.130. The interface is ens1d1.  

<pre><code>sudo ptf --test-dir ./ --interfac 0@ens1d1 --test-params="local_port=0;count=10;local_mac='e0:07:1b:70:0c:52';remote_mac='f4:52:14:61:a9:21';local_ip='192.168.8.129';remote_ip='192.168.8.130'"
</code></pre>
