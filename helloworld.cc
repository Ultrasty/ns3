//这只是个Helloworld，加了点我写的注释

#include "ns3/netanim-module.h"//用来生成xml
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("FirstScriptExample");//开启日志

int
main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);
  
  Time::SetResolution (Time::NS);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

  NodeContainer nodes;
  nodes.Create (2);//创建两个节点

  PointToPointHelper pointToPoint;
  //设置属性
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer devices;
  devices = pointToPoint.Install (nodes);//完成设备和信道的配置

  InternetStackHelper stack;
  stack.Install (nodes);//给nodes安装协议栈

  Ipv4AddressHelper address;//地址生成器对象
  address.SetBase ("10.1.1.0", "255.255.255.0");

  Ipv4InterfaceContainer interfaces = address.Assign (devices);//完成地址配置  Ipv4InterfaceContainer是Ipv4Interface对象的列表

  UdpEchoServerHelper echoServer (9);//端口号

  ApplicationContainer serverApps = echoServer.Install (nodes.Get (1));//返回一个容器，内含指向所有被生成器创建的应用指针
  serverApps.Start (Seconds (1.0));//开始通信时间
  serverApps.Stop (Seconds (10.0));//结束通信时间

  UdpEchoClientHelper echoClient (interfaces.GetAddress (1), 9);//1 的IP
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = echoClient.Install (nodes.Get (0));//安装到0号节点上
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

  //配置生成xml
  AnimationInterface anim("first.xml");
  anim.SetConstantPosition(nodes.Get(0), 1.0, 2.0);
  anim.SetConstantPosition(nodes.Get(1), 2.0, 3.0);

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}