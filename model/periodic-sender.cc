/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2017 University of Padova
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#include "ns3/periodic-sender.h"
#include "ns3/pointer.h"
#include "ns3/log.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/lora-net-device.h"
#include "aes.hpp"
//#include "aes.h"
#include "aes.c"
#include "ns3/string.h" 


namespace ns3 {
namespace lorawan {

NS_LOG_COMPONENT_DEFINE ("PeriodicSender");

NS_OBJECT_ENSURE_REGISTERED (PeriodicSender);

TypeId
PeriodicSender::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PeriodicSender")
    .SetParent<Application> ()
    .AddConstructor<PeriodicSender> ()
    .SetGroupName ("lorawan")
    .AddAttribute ("Interval", "The interval between packet sends of this app",
                   TimeValue (Seconds (0)),
                   MakeTimeAccessor (&PeriodicSender::GetInterval,
                                     &PeriodicSender::SetInterval),
                   MakeTimeChecker ());
  // .AddAttribute ("PacketSizeRandomVariable", "The random variable that determines the shape of the packet size, in bytes",
  //                StringValue ("ns3::UniformRandomVariable[Min=0,Max=10]"),
  //                MakePointerAccessor (&PeriodicSender::m_pktSizeRV),
  //                MakePointerChecker <RandomVariableStream>());
  return tid;
}

PeriodicSender::PeriodicSender ()
  : m_interval (Seconds (10)),
  m_initialDelay (Seconds (1)),
  m_basePktSize (10),
  m_pktSizeRV (0),
  m_encryption(0)

{
  NS_LOG_FUNCTION_NOARGS ();
}

PeriodicSender::~PeriodicSender ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
PeriodicSender::SetInterval (Time interval)
{
  NS_LOG_FUNCTION (this << interval);
  m_interval = interval;
}

Time
PeriodicSender::GetInterval (void) const
{
  NS_LOG_FUNCTION (this);
  return m_interval;
}

//+++++++++++++++++++++++++++++++++++++++++++++++
void
PeriodicSender::SetAesKey (int encryption, unsigned char* msg,size_t input)
{
   m_encryption = encryption;
   unsigned char* cipher = encrypt(msg,input); 
   m_encryptedMsg = std::string((char*)cipher);
   
}

unsigned char*
PeriodicSender::encrypt(unsigned char* shellcode,size_t input)
{
    // beginning timestamp  ++++++++++++++++++++++++++++++
    struct timespec begin;    timespec_get(&begin, TIME_UTC);
    struct timespec begin2;    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &begin2); 
    // end beginning timestamp ---------------------------
/**
 * ++++++   code for cesar encryption ++++
 * std::string str = "";
 * for (int i = 0; i < m_basePktSize; i++)
 * str.Append(m_basePktSize,"a"); 
 * std::string(m_basePktSize,'a');
 * 
 */ 

    printf("============= plaintext =================\n");
    
    for (size_t i = 0; i < input -1; i++) {
        printf("%02x", shellcode[i]);
    }
    printf("\n+++++++++++++++++++++++++++++++++++++++++\n");

    unsigned char key[] = "2b7e151628aed2a6abf7158809cf4f3c";
    unsigned char iv[] = "\x9d\x02\x35\x3b\xa3\x4b\xec\x26\x13\x88\x58\x51\x11\x47\xa5\x98";    

    struct AES_ctx ctx;
    AES_init_ctx_iv(&ctx, key,iv);
    AES_CBC_encrypt_buffer(&ctx, shellcode,input);
    //AES_init_ctx(&ctx, key);
    //AES_ECB_encrypt(&ctx, shellcode);   
    

    printf("============= enrypted =================\n");
    for (size_t i = 0; i < input - 1; ++i) {
        printf("%02x", shellcode[i]);
    }
    printf("\n======================================\n");


    // ending timestamp ++++++++++++++++++++++++++++++++++++
    struct timespec end;    timespec_get(&end, TIME_UTC);
    struct timespec end2;    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end2);

    // display the difference between the 2 timestamps
    double time_spent = (end.tv_sec - begin.tv_sec) + (end.tv_nsec - begin.tv_nsec) / 1000000000.0;
    double time_spent2 = (end2.tv_sec - begin2.tv_sec) + (end2.tv_nsec - begin2.tv_nsec) / 1000000000.0;

    printf("Time it took to execute (WALL TIME): %lf\n", time_spent);
    printf("Time it took to execute (CPU TIME): %lf\n", time_spent2);
    // end of ending timestamp ----------------------------
  return shellcode;
}
//---------------------------------------------
std::string
PeriodicSender::hexToASCII(std::string hex)
{
    // initialize the ASCII code string as empty.
    std::string ascii = "";
    for (size_t i = 0; i < hex.length(); i += 2)
    {
        // extract two characters from hex string
        std::string part = hex.substr(i, 2);
 
        // change it into base 16 and 
        // typecast as the character
        char ch = stoul(part, nullptr, 16);
 
        // add this char to final ASCII string
        ascii += ch;
    }
    return ascii;
}

std::string 
PeriodicSender::array_to_string(const unsigned char* buffer, std::size_t size) 
{ 
    std::ostringstream oss; 
  
    // Afficher en hexadécimal et en majuscule 
    oss << std::hex << std::uppercase; 
  
    // Injecter tous les caractères sous forme d'entiers dans le flux 
    std::copy(buffer, buffer + size, std::ostream_iterator<int>(oss, "")); 
  
    return oss.str(); 
}

void
PeriodicSender::SetInitialDelay (Time delay)
{
  NS_LOG_FUNCTION (this << delay);
  m_initialDelay = delay;
}


void
PeriodicSender::SetPacketSizeRandomVariable (Ptr <RandomVariableStream> rv)
{
  m_pktSizeRV = rv;
}


void
PeriodicSender::SetPacketSize (uint8_t size)
{
  m_basePktSize = size;
}


void
PeriodicSender::SendPacket (void)
{
  NS_LOG_FUNCTION (this);

  // Create and send a new packet
  Ptr<Packet> packet;
  if (m_pktSizeRV)
    {
      int randomsize = m_pktSizeRV->GetInteger ();
      packet = Create<Packet> (m_basePktSize + randomsize);
    }
  else
    {
      packet = Create<Packet> (m_basePktSize);
    }

  if (m_encryption != 0)
    {
      packet = Create<Packet>((uint8_t*)m_encryptedMsg.data(), m_encryptedMsg.length());
    }

  m_mac->Send (packet);
  

  

  // Schedule the next SendPacket event
  m_sendEvent = Simulator::Schedule (m_interval, &PeriodicSender::SendPacket,
                                     this);

  //NS_LOG_DEBUG ("Sent a packet of size " << packet->GetSize ());
}

void
PeriodicSender::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

  // Make sure we have a MAC layer
  if (m_mac == 0)
    {
      // Assumes there's only one device
      Ptr<LoraNetDevice> loraNetDevice = m_node->GetDevice (0)->GetObject<LoraNetDevice> ();

      m_mac = loraNetDevice->GetMac ();
      NS_ASSERT (m_mac != 0);
    }

  // Schedule the next SendPacket event
  Simulator::Cancel (m_sendEvent);
  NS_LOG_DEBUG ("Starting up application with a first event with a " <<
                m_initialDelay.GetSeconds () << " seconds delay");
  m_sendEvent = Simulator::Schedule (m_initialDelay,
                                     &PeriodicSender::SendPacket, this);
  NS_LOG_DEBUG ("Event Id: " << m_sendEvent.GetUid ());
}

void
PeriodicSender::StopApplication (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  Simulator::Cancel (m_sendEvent);
}

}
}
