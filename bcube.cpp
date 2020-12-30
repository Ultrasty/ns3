#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

#include "ns3/netanim-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/bridge-helper.h"
#include "ns3/bridge-net-device.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/random-variable-stream.h"
#include "ns3/csma-module.h"
#include "ns3/ipv4-nix-vector-helper.h"

#define EXPORT_STATS
using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE("BCube-Architecture");

// 从数字创建IP地址
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
//主函数
int
main(int argc, char* argv[])
{

	LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
	LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

//根据k定义一些参数
#ifdef EXPORT_STATS
	ofstream sfile;
	sfile.open("statistics/stats.csv", ios::out | ios::app);
#endif
	int n;//每个Bcube中的服务器的数量
	CommandLine cmd;
	cmd.AddValue("n", "Number of servers per rack", n);
	cmd.Parse(argc, argv);
	int k = 2;			//BCube level，为了控制变量这里是三层
	int num_sw = pow(n, k);		//每层的交换机数量 = n^k;
	int num_host = num_sw * n;	//全部的主机数量
	char filename[] = "statistics/BCube";
	char traceFile[] = "statistics/BCube";
	char buf[4];
	std::sprintf(buf, "-%d", k);
	strcat(filename, buf);
	strcat(filename, ".xml");// Flow Monitor xml output file
	strcat(traceFile, buf);
	strcat(traceFile, ".tr");// for Flow Monitor xml output file
	int i = 0;
	int j = 0;
	int temp = 0;

	int levelRand = 0;
	int swRand = 0;		// Random values for servers' address
	int hostRand = 0;

	int randHost = 0;	// Random values for clients' address


	int port = 9;
	int packetSize = 1024;	
	char dataRate_OnOff[] = "1Mbps";
	char maxBytes[] = "0";	

	char dataRate[] = "1000Mbps";	// 1Gbps
	int delay = 0.001;		// 0.001 ms


	std::cout << "Number of BCube level =  " << k + 1 << "\n";
	std::cout << "Number of switch in each BCube level =  " << num_sw << "\n";
	std::cout << "Number of host under each switch =  " << n << "\n";
	std::cout << "Total number of host =  " << num_host << "\n";

	InternetStackHelper internet;
	Ipv4NixVectorHelper nixRouting;
	Ipv4StaticRoutingHelper staticRouting;
	Ipv4ListRoutingHelper list;
	list.Add(staticRouting, 0);
	list.Add(nixRouting, 10);
	internet.SetRoutingHelper(list);

	//创建Node Containers

	NodeContainer host;				// NodeContainer for hosts;  				
	host.Create(num_host);
	internet.Install(host);

	NodeContainer swB0;				// NodeContainer for B0 switches 
	swB0.Create(num_sw);
	internet.Install(swB0);

	NodeContainer bridgeB0;			// NodeContainer for B0 bridges
	bridgeB0.Create(num_sw);
	internet.Install(bridgeB0);

	NodeContainer swB1;				// NodeContainer for B1 switches
	swB1.Create(num_sw);
	internet.Install(swB1);

	NodeContainer bridgeB1;			// NodeContainer for B1 bridges
	bridgeB1.Create(num_sw);
	internet.Install(bridgeB1);

	NodeContainer swB2;				// NodeContainer for B2 switches
	swB2.Create(num_sw);
	internet.Install(swB2);

	NodeContainer bridgeB2;			// NodeContainer for B2 bridges
	bridgeB2.Create(num_sw);
	internet.Install(bridgeB2);



	// 创建traceFile
	ApplicationContainer app[num_host];
	for (i = 0; i < num_host; i++) {
		// 随机选择服务器
		levelRand = 0;
		swRand = rand() % num_sw + 0;
		hostRand = rand() % n + 0;
		hostRand = hostRand + 2;
		char* add;
		add = toString(10, levelRand, swRand, hostRand);

		// Initialize On/Off Application with addresss of server
		OnOffHelper oo = OnOffHelper("ns3::UdpSocketFactory", Address(InetSocketAddress(Ipv4Address(add), port))); // ip address of server
		oo.SetAttribute("OnTime", StringValue("ns3::ExponentialRandomVariable[Mean=1.0|Bound=0.0]"));
		oo.SetAttribute("OffTime", StringValue("ns3::ExponentialRandomVariable[Mean=1.0|Bound=0.0]"));
		oo.SetAttribute("PacketSize", UintegerValue(packetSize));
		oo.SetAttribute("DataRate", StringValue(dataRate_OnOff));
		oo.SetAttribute("MaxBytes", StringValue(maxBytes));

		// 随机选择客户端
		randHost = rand() % num_host + 0;
		int temp = n * swRand + (hostRand - 2);
		while (temp == randHost) {
			randHost = rand() % num_host + 0;
		}
		// 确保服务器和客户端不同

		// Install On/Off Application to the client
		// OnOffApplication根据OnOff模式为单个目的地生成流量。
		NodeContainer onoff;
		onoff.Add(host.Get(randHost));
		app[i] = oo.Install(onoff);
	}

	std::cout << "Finished creating On/Off traffic" << "\n";
	// 初始化 Address Helper

	Ipv4AddressHelper address;

	// 初始化 Csma helper

	CsmaHelper csma;
	csma.SetChannelAttribute("DataRate", StringValue(dataRate));
	csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(delay)));

	// Connect BCube 0 switches to hosts

	NetDeviceContainer hostSwDevices0[num_sw];
	NetDeviceContainer bridgeDevices0[num_sw];
	Ipv4InterfaceContainer ipContainer0[num_sw];

	temp = 0;
	for (i = 0; i < num_sw; i++) {
		NetDeviceContainer link1 = csma.Install(NodeContainer(swB0.Get(i), bridgeB0.Get(i)));
		hostSwDevices0[i].Add(link1.Get(0));
		bridgeDevices0[i].Add(link1.Get(1));
		temp = j;
		for (j = temp; j < temp + n; j++) {
			NetDeviceContainer link2 = csma.Install(NodeContainer(host.Get(j), bridgeB0.Get(i)));
			hostSwDevices0[i].Add(link2.Get(0));
			bridgeDevices0[i].Add(link2.Get(1));
		}
		BridgeHelper bHelper0;
		bHelper0.Install(bridgeB0.Get(i), bridgeDevices0[i]);
		//分配地址
		char* subnet;
		subnet = toString(10, 0, i, 0);
		address.SetBase(subnet, "255.255.255.0");
		ipContainer0[i] = address.Assign(hostSwDevices0[i]);
	}
	std::cout << "Fininshed BCube 0 connection" << "\n";

	//Connect BCube 1 switches to hosts

	NetDeviceContainer hostSwDevices1[num_sw];
	NetDeviceContainer bridgeDevices1[num_sw];
	Ipv4InterfaceContainer ipContainer1[num_sw];

	j = 0; temp = 0;

	for (i = 0; i < num_sw; i++) {
		NetDeviceContainer link1 = csma.Install(NodeContainer(swB1.Get(i), bridgeB1.Get(i)));
		hostSwDevices1[i].Add(link1.Get(0));
		bridgeDevices1[i].Add(link1.Get(1));

		if (i == 0) {
			j = 0;
			temp = j;
		}

		if (i % n != 0) {
			j = temp + 1;
			temp = j;
		}

		if ((i % n == 0) && (i != 0)) {
			j = temp - n + 1;
			j = j + n * n;
			temp = j;
		}

		for (j = temp; j < temp + n * n; j = j + n) {
			NetDeviceContainer link2 = csma.Install(NodeContainer(host.Get(j), bridgeB1.Get(i)));
			hostSwDevices1[i].Add(link2.Get(0));
			bridgeDevices1[i].Add(link2.Get(1));
		}
		BridgeHelper bHelper1;
		bHelper1.Install(bridgeB1.Get(i), bridgeDevices1[i]);
		//Assign address
		char* subnet;
		subnet = toString(10, 1, i, 0);
		address.SetBase(subnet, "255.255.255.0");
		ipContainer1[i] = address.Assign(hostSwDevices1[i]);
	}
	std::cout << "Fininshed BCube 1 connection" << "\n";

	// Connect BCube 2 switches to hosts
	//
	NetDeviceContainer hostSwDevices2[num_sw];
	NetDeviceContainer bridgeDevices2[num_sw];
	Ipv4InterfaceContainer ipContainer2[num_sw];

	j = 0; temp = 0;
	int temp2 = n * n;
	int temp3 = n * n * n;

	for (i = 0; i < num_sw; i++) {
		NetDeviceContainer link1 = csma.Install(NodeContainer(swB2.Get(i), bridgeB2.Get(i)));
		hostSwDevices2[i].Add(link1.Get(0));
		bridgeDevices2[i].Add(link1.Get(1));

		if (i == 0) {
			j = 0;
			temp = j;
		}

		if (i % temp2 != 0) {
			j = temp + 1;
			temp = j;
		}

		if ((i % temp2 == 0) && (i != 0)) {
			j = temp - temp2 + 1;
			j = j + temp3;
			temp = j;
		}

		for (j = temp; j < temp + temp3; j = j + temp2) {
			NetDeviceContainer link2 = csma.Install(NodeContainer(host.Get(j), bridgeB2.Get(i)));
			hostSwDevices2[i].Add(link2.Get(0));
			bridgeDevices2[i].Add(link2.Get(1));
		}
		BridgeHelper bHelper2;
		bHelper2.Install(bridgeB2.Get(i), bridgeDevices2[i]);
		//分配地址
		char* subnet;
		subnet = toString(10, 2, i, 0);
		address.SetBase(subnet, "255.255.255.0");
		ipContainer2[i] = address.Assign(hostSwDevices2[i]);

	}
	std::cout << "Fininshed BCube 2 connection" << "\n";
	std::cout << "------------- " << "\n";

	//开始模拟


	std::cout << "Start ... " << "\n";
	for (i = 0; i < num_host; i++) {
		app[i].Start(Seconds(0.0));
		app[i].Stop(Seconds(100.0));
	}
	Ipv4GlobalRoutingHelper::PopulateRoutingTables();
	// 计算 Throughput

	FlowMonitorHelper flowmon;
	Ptr<FlowMonitor> monitor = flowmon.InstallAll();
	// Run simulation


	//生成traceFile，生成的traceFile很大，有1.4个G
	//AsciiTraceHelper ascii;
	//csma.EnableAsciiAll(ascii.CreateFileStream(traceFile));




	NS_LOG_INFO("Run Simulation.");
	Simulator::Stop(Seconds(101.0));

	//输出xml
	/*AnimationInterface anim("first.xml");
	for (int i = 0; i < num_host; i++) {
		anim.SetConstantPosition(host.Get(i), 2.0, i);
	}*/
	/*for (int i = 0; i < num_sw; i++) {
		anim.SetConstantPosition(swB0.Get(i), 4.0, i);
	}
	for (int i = 0; i < num_sw; i++) {
		anim.SetConstantPosition(bridgeB0.Get(i), 6.0, i);
	}
	for (int i = 0; i < num_sw; i++) {
		anim.SetConstantPosition(swB1.Get(i), 8.0, i);
	}
	for (int i = 0; i < num_sw; i++) {
		anim.SetConstantPosition(bridgeB1.Get(i), 10.0, i);
	}
	for (int i = 0; i < num_sw; i++) {
		anim.SetConstantPosition(swB2.Get(i), 12.0, i);
	}
	for (int i = 0; i < num_sw; i++) {
		anim.SetConstantPosition(bridgeB2.Get(i), 14.0, i);
	}*/
	//------------------//
	//------------------//
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
		throughput += iter->second.rxBytes * 8.0 / 1024 /1024;
		std::cout << nFlows<<"\n";
		std::cout << iter->second.rxBytes << " " << iter->second.timeLastRxPacket.GetSeconds() <<" "<< iter->second.timeFirstTxPacket.GetSeconds()<<"\n";
		std::cout <<"txPackets:"<< iter->second.txPackets<<" rxPackets:"<<iter->second.rxPackets<<"\n";
	}

#ifdef EXPORT_STATS
	sfile << "BCube" << "," << n << "," << nFlows << "," << txPackets << "," << rxPackets << "," << delaySum << "," << jitterSum << "," << lastDelay;
	sfile << "," << lostPackets << "," << timesForwarded << "," << averageDelay / nFlows << "," << throughput / delaySum *1000000000 << endl;

#endif
	std::cout << "BCube" << "," << "n" << "," << "txPackets" << "," << "rxPackets" << "," << "delaySum" << "," << "jitterSum" << "," << "lastDelay";
	std::cout << "," << "lostPackets" << "," << "timesForwarded" << "," << "averageDelay" << "," << "throughput" << endl;
	std::cout << "BCube" << "," << n << "," << txPackets << "," << rxPackets << "," << delaySum << "," << jitterSum << "," << lastDelay;
	std::cout << "," << lostPackets << "," << timesForwarded << "," << averageDelay / nFlows << "," << throughput / delaySum *1000000000 << endl;
	monitor->SerializeToXmlFile(filename, true, false);

	std::cout << "Simulation finished " << "\n";

	Simulator::Destroy();
	NS_LOG_INFO("Done.");

	return 0;
}