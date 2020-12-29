#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>

#include "ns3/flow-monitor-module.h"
#include "ns3/bridge-helper.h"
#include "ns3/bridge-net-device.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/csma-module.h"
#include "ns3/ipv4-nix-vector-helper.h"
#include "ns3/random-variable-stream.h"
#include "ns3/animation-interface.h"

using namespace ns3;
using namespace std;
NS_LOG_COMPONENT_DEFINE("Fat-Tree-Architecture");

void process_mem_usage(double& vm_usage, double& resident_set)
{
	vm_usage = 0.0;
	resident_set = 0.0;

	unsigned long vsize;
	long rss;
	{
		std::string ignore;
		std::ifstream ifs("/proc/self/stat", std::ios_base::in);
		ifs >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore
			>> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore
			>> ignore >> ignore >> vsize >> rss;
	}

	long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024; 
	vm_usage = vsize / 1024.0;
	resident_set = rss * page_size_kb;
}

void printTime()
{
	time_t t;
	time(&t);
	std::cout << "\nCurrent time: " << ctime(&t) << "\n" << endl;
}

char* toString(int a, int b, int c, int d) {

	int first = a;
	int second = b;
	int third = c;
	int fourth = d;

	char* address = new char[30];
	char firstOctet[30], secondOctet[30], thirdOctet[30], fourthOctet[30];

	bzero(address, 30);

	snprintf(firstOctet, 10, "%d", first);
	strcat(firstOctet, ".");
	snprintf(secondOctet, 10, "%d", second);
	strcat(secondOctet, ".");
	snprintf(thirdOctet, 10, "%d", third);
	strcat(thirdOctet, ".");
	snprintf(fourthOctet, 10, "%d", fourth);

	strcat(thirdOctet, fourthOctet);
	strcat(secondOctet, thirdOctet);
	strcat(firstOctet, secondOctet);
	strcat(address, firstOctet);

	return address;
}


int
main(int argc, char* argv[])
{

	int k;

#ifdef EXPORT_STATS
	ofstream sfile;
	sfile.open("statistics/fat-tree-stats.csv", ios::out | ios::app);
#endif
	CommandLine cmd;
	cmd.AddValue("k", "Number of ports per switch", k);
	cmd.Parse(argc, argv);
	int num_pod = k;		
	int num_host = (k / 2);		
	int num_edge = (k / 2);		
	int num_bridge = num_edge;	
	int num_agg = (k / 2);		
	int num_group = k / 2;		
	int num_core = (k / 2);		
	int total_host = k * k * k / 4;
	char filename[] = "statistics/Fat-tree";
	char traceFile[] = "statistics/Fat-tree";
	char buf[4];
	std::sprintf(buf, "-%d", k);
	strcat(filename, buf);
	strcat(filename, ".xml");
	strcat(traceFile, buf);
	strcat(traceFile, ".tr");

	int podRand = 0;
	int swRand = 0;	
	int hostRand = 0;	

	int rand1 = 0;	
	int rand2 = 0;	
	int rand3 = 0;		

	int i = 0;
	int j = 0;
	int h = 0;


	int port = 9;

	int packetSize = 1024;	
	char dataRate_OnOff[] = "1Mbps";

	char maxBytes[] = "0";	


	char dataRate[] = "1000Mbps";
	int delay = 0.001;		
	std::cout << "Value of k =  " << k << "\n";
	std::cout << "Total number of hosts =  " << total_host << "\n";
	std::cout << "Number of hosts under each switch =  " << num_host << "\n";
	std::cout << "Number of edge switch under each pod =  " << num_edge << "\n";
	std::cout << "------------- " << "\n";


	InternetStackHelper internet;
	Ipv4NixVectorHelper nixRouting;
	Ipv4StaticRoutingHelper staticRouting;
	Ipv4ListRoutingHelper list;
	list.Add(staticRouting, 0);
	list.Add(nixRouting, 10);
	internet.SetRoutingHelper(list);


	NodeContainer core[num_group];	
	for (i = 0; i < num_group; i++) {
		core[i].Create(num_core);
		internet.Install(core[i]);
	}
	NodeContainer agg[num_pod];			
	for (i = 0; i < num_pod; i++) {
		agg[i].Create(num_agg);
		internet.Install(agg[i]);
	}
	NodeContainer edge[num_pod];			
	for (i = 0; i < num_pod; i++) {
		edge[i].Create(num_bridge);
		internet.Install(edge[i]);
	}
	NodeContainer bridge[num_pod];			
	for (i = 0; i < num_pod; i++) {
		bridge[i].Create(num_bridge);
		internet.Install(bridge[i]);
	}
	NodeContainer host[num_pod][num_bridge];	
	for (i = 0; i < k; i++) {
		for (j = 0; j < num_bridge; j++) {
			host[i][j].Create(num_host);
			internet.Install(host[i][j]);
		}
	}

	ApplicationContainer app[total_host];
	for (i = 0; i < total_host; i++) {
		podRand = rand() % num_pod + 0;
		swRand = rand() % num_edge + 0;
		hostRand = rand() % num_host + 0;
		hostRand = hostRand + 2;
		char* add;
		add = toString(10, podRand, swRand, hostRand);

		OnOffHelper oo = OnOffHelper("ns3::UdpSocketFactory", Address(InetSocketAddress(Ipv4Address(add), port))); 

		oo.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
		oo.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));

		oo.SetAttribute("PacketSize", UintegerValue(packetSize));
		oo.SetAttribute("DataRate", StringValue(dataRate_OnOff));
		oo.SetAttribute("MaxBytes", StringValue(maxBytes));

		rand1 = rand() % num_pod + 0;
		rand2 = rand() % num_edge + 0;
		rand3 = rand() % num_host + 0;

		while (rand1 == podRand && swRand == rand2 && (rand3 + 2) == hostRand) {
			rand1 = rand() % num_pod + 0;
			rand2 = rand() % num_edge + 0;
			rand3 = rand() % num_host + 0;
		} 

		NodeContainer onoff;
		onoff.Add(host[rand1][rand2].Get(rand3));
		app[i] = oo.Install(onoff);
		std::cout << "Data transfer from " << (hostRand - 2) << " to " << rand3 << "\n";
	}
	std::cout << "Finished creating On/Off traffic" << "\n";

	Ipv4AddressHelper address;

	PointToPointHelper p2p;
	p2p.SetDeviceAttribute("DataRate", StringValue(dataRate));
	p2p.SetChannelAttribute("Delay", TimeValue(MilliSeconds(delay)));

	CsmaHelper csma;
	csma.SetChannelAttribute("DataRate", StringValue(dataRate));
	csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(delay)));

	NetDeviceContainer hostSw[num_pod][num_bridge];
	NetDeviceContainer bridgeDevices[num_pod][num_bridge];
	Ipv4InterfaceContainer ipContainer[num_pod][num_bridge];

	for (i = 0; i < num_pod; i++) {
		for (j = 0; j < num_bridge; j++) {
			NetDeviceContainer link1 = csma.Install(NodeContainer(edge[i].Get(j), bridge[i].Get(j)));
			hostSw[i][j].Add(link1.Get(0));
			bridgeDevices[i][j].Add(link1.Get(1));

			for (h = 0; h < num_host; h++) {
				NetDeviceContainer link2 = csma.Install(NodeContainer(host[i][j].Get(h), bridge[i].Get(j)));
				hostSw[i][j].Add(link2.Get(0));
				bridgeDevices[i][j].Add(link2.Get(1));
			}

			BridgeHelper bHelper;
			bHelper.Install(bridge[i].Get(j), bridgeDevices[i][j]);
			char* subnet;
			subnet = toString(10, i, j, 0);
			address.SetBase(subnet, "255.255.255.0");
			ipContainer[i][j] = address.Assign(hostSw[i][j]);
		}
	}
	std::cout << "Finished connecting edge switches and hosts  " << "\n";

	NetDeviceContainer ae[num_pod][num_agg][num_edge];
	Ipv4InterfaceContainer ipAeContainer[num_pod][num_agg][num_edge];
	for (i = 0; i < num_pod; i++) {
		for (j = 0; j < num_agg; j++) {
			for (h = 0; h < num_edge; h++) {
				ae[i][j][h] = p2p.Install(agg[i].Get(j), edge[i].Get(h));

				int second_octet = i;
				int third_octet = j + (k / 2);
				int fourth_octet;
				if (h == 0) fourth_octet = 1;
				else fourth_octet = h * 2 + 1;
				char* subnet;
				subnet = toString(10, second_octet, third_octet, 0);
				char* base;
				base = toString(0, 0, 0, fourth_octet);
				address.SetBase(subnet, "255.255.255.0", base);
				ipAeContainer[i][j][h] = address.Assign(ae[i][j][h]);
			}
		}
	}
	std::cout << "Finished connecting aggregation switches and edge switches  " << "\n";

	NetDeviceContainer ca[num_group][num_core][num_pod];
	Ipv4InterfaceContainer ipCaContainer[num_group][num_core][num_pod];
	int fourth_octet = 1;

	for (i = 0; i < num_group; i++) {
		for (j = 0; j < num_core; j++) {
			fourth_octet = 1;
			for (h = 0; h < num_pod; h++) {
				ca[i][j][h] = p2p.Install(core[i].Get(j), agg[h].Get(i));

				int second_octet = k + i;
				int third_octet = j;
				char* subnet;
				subnet = toString(10, second_octet, third_octet, 0);
				char* base;
				base = toString(0, 0, 0, fourth_octet);
				address.SetBase(subnet, "255.255.255.0", base);
				ipCaContainer[i][j][h] = address.Assign(ca[i][j][h]);
				fourth_octet += 2;
			}
		}
	}
	std::cout << "Finished connecting core switches and aggregation switches  " << "\n";
	std::cout << "------------- " << "\n";


	std::cout << "Start Simulation.. " << "\n";

	printTime();

	for (i = 0; i < total_host; i++) {
		app[i].Start(Seconds(0.0));
		app[i].Stop(Seconds(101.0));

	}
	Ipv4GlobalRoutingHelper::PopulateRoutingTables();

	FlowMonitorHelper flowmon;
	Ptr<FlowMonitor> monitor = flowmon.InstallAll();

	NS_LOG_INFO("Run Simulation.");

	Simulator::Stop(Seconds(100.0));

#ifdef NEED_TRACE
	AsciiTraceHelper ascii;
	csma.EnableAsciiAll(ascii.CreateFileStream(traceFile));
#endif

	Simulator::Run();
	monitor->CheckForLostPackets();
	Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier>(flowmon.GetClassifier());
	std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();

	int txPackets = 0, rxPackets = 0, lostPackets = 0;
	ns3::Time delaySum = NanoSeconds(0.0);
	ns3::Time jitterSum = NanoSeconds(0.0);
	ns3::Time lastDelay = NanoSeconds(0.0);
	int timesForwarded = 0;
	double averageDelay;
	double throughput;
	int nFlows = 0;
	for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator iter = stats.begin(); iter != stats.end(); ++iter)
	{
		nFlows++;
		txPackets += iter->second.txPackets;
		rxPackets += iter->second.rxPackets;
		lostPackets += iter->second.lostPackets;
		delaySum += iter->second.delaySum;
		jitterSum += iter->second.jitterSum;
		lastDelay += iter->second.lastDelay;
		timesForwarded += iter->second.timesForwarded;
		averageDelay += iter->second.delaySum.GetNanoSeconds() / iter->second.rxPackets;
		throughput += iter->second.rxBytes * 8.0 / (iter->second.timeLastRxPacket.GetSeconds() - iter->second.timeFirstTxPacket.GetSeconds()) / 1024;

	}

	std::cout << "Fat-Tree" << "," << "k" << "," << "txPackets" << "," << "rxPackets" << "," << "delaySum" << "," << "jitterSum" << "," << "lastDelay";
	std::cout << "," << "lostPackets" << "," << "timesForwarded" << "," << "averageDelay" << "," << "throughput(Kbps)" << endl;
	std::cout << "Fat-Tree" << "," << k << "," << txPackets << "," << rxPackets << "," << delaySum << "," << jitterSum << "," << lastDelay;
	std::cout << "," << lostPackets << "," << timesForwarded << "," << averageDelay / nFlows << "," << throughput / nFlows << endl;

	printTime();
	std::cout << "Simulation finished " << "\n";
	Simulator::Destroy();
	NS_LOG_INFO("Done.");
	return 0;
}
