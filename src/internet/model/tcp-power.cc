#include "tcp-power.h"
#include "tcp-socket-state.h"
#include "ns3/core-module.h"
#include <iostream>
#include "ns3/log.h"

namespace ns3 {

  NS_LOG_COMPONENT_DEFINE ("TcpPW");
  NS_OBJECT_ENSURE_REGISTERED (TcpPW);

  uint16_t TcpPW::nextId = 1;

  TypeId
  TcpPW::GetTypeId (void)
  {
    static TypeId tid = TypeId ("ns3::TcpPW")
      .SetParent<TcpCongestionOps> ()
      .AddConstructor<TcpPW> ()
      .SetGroupName ("Internet");
    return tid;
  }

  TcpPW::TcpPW(void)
    : TcpCongestionOps(),
      rttCount(0),
      currentRTT(Time::Max ()),
      lastPolicyUpdated(Time::Min()),
      nextPolicyUpdate(Time::Min()),
      prevRTT(Time::Max()),
      prevOutstandingPackets(0),
      nPacketsAcked(0) {

      id = TcpPW::nextId;
      TcpPW::nextId++;
      NS_LOG_FUNCTION (this);
      NS_LOG_INFO("Created Power client");

  }

  TcpPW::TcpPW (const TcpPW& sock)
    : TcpCongestionOps (sock),
      rttCount(sock.rttCount),
      currentRTT(sock.currentRTT),
      lastPolicyUpdated(sock.lastPolicyUpdated),
      nextPolicyUpdate(sock.nextPolicyUpdate),
      prevRTT(sock.prevRTT),
      prevOutstandingPackets(sock.prevOutstandingPackets),
      nPacketsAcked(sock.nPacketsAcked)
  {
    NS_LOG_FUNCTION (this);
  }

  TcpPW::~TcpPW (void)
  {
    NS_LOG_FUNCTION (this);
  }

  Ptr<TcpCongestionOps>
  TcpPW::Fork (void)
  {
    return CopyObject<TcpPW> (this);
  }

  std::string TcpPW::GetName() const {
    NS_LOG_FUNCTION(this);
    return "TcpPW";
  }

  uint32_t TcpPW::GetSsThresh(Ptr<const TcpSocketState> tcb, uint32_t bytesInFlight) {
    NS_LOG_FUNCTION(this << tcb << bytesInFlight);
    // NS_LOG_INFO(this << " Ignored. Returning max (65535).");
    return 65535;
  }

  void
  TcpPW::IncreaseWindow (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked)
  {
    NS_LOG_FUNCTION (this << tcb << segmentsAcked);
    // NS_LOG_INFO(this << " Ignored.");
  }


  void
  TcpPW::ReduceCwnd (Ptr<TcpSocketState> tcb)
  {
    NS_LOG_FUNCTION (this << tcb);
    // NS_LOG_INFO(this << " Ignored.");
  }

  void
  TcpPW::CongestionStateSet (Ptr<TcpSocketState> tcb,
                               const TcpSocketState::TcpCongState_t newState)
  {
    NS_LOG_FUNCTION (this << tcb << newState);
    // NS_LOG_INFO(this << " Ignored.");
  }

  double TcpPW::getPreviousPower() {
    return prevOutstandingPackets / (prevRTT.GetSeconds() * prevRTT.GetSeconds());
  }

  double TcpPW::getPower(Ptr<TcpSocketState> tcb) {
    if (rttCount > 1) {
      double power = tcb->GetCwndInSegments() / (currentRTT.GetSeconds() * currentRTT.GetSeconds());
      return power;
    }
    return 0.0;
  }

  double TcpPW::powerTrend(Ptr<TcpSocketState> tcb) {

    double prevPower = getPreviousPower();
    if (prevPower > 0.0) {

      double currentPower = getPower(tcb);

      double diff = (currentPower - prevPower); // percentage change is needed.

      return diff;
      // Now why is the diff

      // if (diff < 0) {
      //   // Is it due to the change in number of outstandingpackets or change in rtt
      //   // if changeInOP > changeInRTT ** 2, false alarm ?
      //
      //   double changeInOP = (prevOutstandingPackets - tcb->GetCwndInSegments()) / prevOutstandingPackets;
      //   double changeInRTT = abs(currentRTT.GetSeconds() -  prevRTT.GetSeconds()) / prevRTT.GetSeconds();
      //
      //   if (changeInOP >  changeInRTT) {
      //       NS_LOG_INFO("False decreasing alarm");
      //       return 1.0;
      //   }
      //
      //   return diff;
      //
      // } else if (diff > 0) {
      //
      //   // if changeInOP < changeInRTT ** 2, false alarm ?
      //   return diff;
      //
      // }

    }
    return 0.0;

  }
  void TcpPW::updateWindowByNPackets(Ptr<TcpSocketState> tcb, int n) {

    NS_LOG_INFO("Client " << id << " change in outstanding packets = " << n);

    tcb->m_cWnd += tcb->m_segmentSize * n;

    if (tcb->m_cWnd < tcb->m_segmentSize) {
      tcb->m_cWnd = tcb->m_segmentSize;
    }

  }

  bool TcpPW::explore() {
    if ((rand() % 10) > 8) {
      return true;
    }

    return false;
  }

  void
  TcpPW::PktsAcked (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked,
                               const Time& rtt)
  {
    nPacketsAcked += segmentsAcked;
    ++rttCount;
    currentRTT = rtt;
    uint32_t outstandingPacketsBeforePolicyUpdate = tcb->GetCwndInSegments(); // because this is volatile in this block

    NS_LOG_FUNCTION (this << tcb << segmentsAcked << rtt);

    // current power = outstandingPacketsBeforePolicyUpdate / currentRTT ** 2
    // previous power = previousPreviousOutstandingPackets / prevRTT

    // check if we need to update policity
    if (nextPolicyUpdate < Simulator::Now()) {
      NS_LOG_INFO("Client " << id << " nPacketsAcked = " << nPacketsAcked);
      NS_LOG_INFO("Client " << id << " rttCount = " << rttCount);
      NS_LOG_INFO("Client " << id << " currentRTT = " << currentRTT.GetMilliSeconds() << "ms");
      //NS_LOG_INFO("Client " << id << " m_segmentSize = " << tcb->m_segmentSize);
      //NS_LOG_INFO("Client " << id << " cwndWindow = " << tcb->m_cWnd.Get());
      NS_LOG_INFO("Client " << id << " outstandingPacketsBeforePolicyUpdate = " << outstandingPacketsBeforePolicyUpdate);
      NS_LOG_INFO("Client " << id << " prevRTT = " << prevRTT.GetMilliSeconds() << "ms");
      NS_LOG_INFO("Client " << id << " prevOutstandingPackets = " << prevOutstandingPackets);
      // NS_LOG_INFO("Client " << id << " nextPolicyUpdate = " << nextPolicyUpdate.GetMilliSeconds() << "ms");
      // Do update
      double trend = powerTrend(tcb);
      NS_LOG_INFO("Client " << id << " powerTrend = " << trend);

      bool forceExplore = false;

      if (trend > 0) {
        NS_LOG_INFO("Client " << id << " power increased");
        if (explore()) {
          forceExplore = true;
        } else {

          int n = static_cast<int>(outstandingPacketsBeforePolicyUpdate * 0.2);
          if (n==0) {
            n = 1;
          }
          updateWindowByNPackets(tcb, n);

        }

      } else {
        NS_LOG_INFO("Client " << id << " power decreased");
        forceExplore = true;
      }

      if (forceExplore) {
        NS_LOG_INFO("Client " << id << " exploring for better power!");
        int n = static_cast<int>(outstandingPacketsBeforePolicyUpdate * 0.2);
        if (n==0) {
          n = 1;
        }
        updateWindowByNPackets(tcb, -n);

      }

      NS_LOG_INFO("Client " << id << " policy updated");
      nextPolicyUpdate = Simulator::Now() + rtt + rtt;
      NS_LOG_INFO("Client " << id << " nextPolicyUpdate = " << nextPolicyUpdate.GetMilliSeconds() << "ms");

      prevOutstandingPackets = outstandingPacketsBeforePolicyUpdate;
      prevRTT = currentRTT;

    }


  }

}
