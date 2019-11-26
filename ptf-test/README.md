# How to run

In the following example, we generate 10 global pause frames and 10 priority-based pause frames at priority 3. The interface is ens1d1.

<pre><code>cd ptf-test
sudo ptf --test-dir ./ --interfac 0@ens1d1 --test-params="src_port=0;count=10;priority=3"
</code></pre>
