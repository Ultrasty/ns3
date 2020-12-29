Instructions to run the code and see the results.

(a) Install ns-3 (version 3.22) by following the instructions 
from https://www.nsnam.org/

(b) Place the three code files Fat-Tree.cc, BCube.cc and 
HyScale.cc into the scratch folder of ns3. Create a statistics
folder if not already present.

(c) At the ns-3 root directory, execute the following commands
to run the implementation

    ns3-dev> ./waf --run "scratch/Fat-tree --k=10"
    ns3-dev> ./waf --run "scratch/BCube --n=3"
    ns3-dev> ./waf --run "scratch/HyScale --k=2"

(d) For help the following can be used:

   ns3-dev> ./waf --run "scratch/Fat-tree --help"

    and similarly for other topologies

(e) Alternatively, the following scripts provided can be tweaked 
   and run:
	run-fat-tree.sh
	run-bcube.sh
	run-hyscale.sh

(f) Results and log-files are generated in the statistics directory 
    under the ns3 main folder. Important files are as follows:

       Logfiles: fat-tree.log,bcube.log,hyscale.log 
       Statistic files: fat-tree-stats.csv,bcube-stats.csv,hyscale-stats.csv
