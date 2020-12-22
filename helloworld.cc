//��ֻ�Ǹ�Helloworld�����˵���д��ע��

#include "ns3/netanim-module.h"//��������xml
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("FirstScriptExample");//������־

int
main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);
  
  Time::SetResolution (Time::NS);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

  NodeContainer nodes;
  nodes.Create (2);//���������ڵ�

  PointToPointHelper pointToPoint;
  //��������
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer devices;
  devices = pointToPoint.Install (nodes);//����豸���ŵ�������

  InternetStackHelper stack;
  stack.Install (nodes);//��nodes��װЭ��ջ

  Ipv4AddressHelper address;//��ַ����������
  address.SetBase ("10.1.1.0", "255.255.255.0");

  Ipv4InterfaceContainer interfaces = address.Assign (devices);//��ɵ�ַ����  Ipv4InterfaceContainer��Ipv4Interface������б�

  UdpEchoServerHelper echoServer (9);//�˿ں�

  ApplicationContainer serverApps = echoServer.Install (nodes.Get (1));//����һ���������ں�ָ�����б�������������Ӧ��ָ��
  serverApps.Start (Seconds (1.0));//��ʼͨ��ʱ��
  serverApps.Stop (Seconds (10.0));//����ͨ��ʱ��

  UdpEchoClientHelper echoClient (interfaces.GetAddress (1), 9);//1 ��IP
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = echoClient.Install (nodes.Get (0));//��װ��0�Žڵ���
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

  //��������xml
  AnimationInterface anim("first.xml");
  anim.SetConstantPosition(nodes.Get(0), 1.0, 2.0);
  anim.SetConstantPosition(nodes.Get(1), 2.0, 3.0);

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}