#ifndef TCPPW_H
#define TCPPW_H

#include "tcp-congestion-ops.h"

namespace ns3 {

  class TcpPW : public TcpCongestionOps {


    public:
      static uint16_t nextId;
      static TypeId GetTypeId (void);
      TcpPW (void);

      TcpPW (const TcpPW& sock);
      virtual ~TcpPW (void);

      virtual std::string GetName () const;
      virtual void PktsAcked (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked,
                             const Time& rtt);
      virtual void CongestionStateSet (Ptr<TcpSocketState> tcb,
                                       const TcpSocketState::TcpCongState_t newState);

      virtual void IncreaseWindow (Ptr<TcpSocketState> tcb, uint32_t segmentsAcked);
      virtual void ReduceCwnd (Ptr<TcpSocketState> tcb);
      virtual uint32_t GetSsThresh (Ptr<const TcpSocketState> tcb,
                                    uint32_t bytesInFlight);
      virtual Ptr<TcpCongestionOps> Fork ();



    protected:
      void updateWindowByNPackets(Ptr<TcpSocketState> tcb, int n);
      double getPower(Ptr<TcpSocketState> tcb);
      double getPreviousPower();
      double powerTrend(Ptr<TcpSocketState> tcb);
      double getRTTEstimate();
      bool explore();
    private:
      uint16_t id;
      uint32_t rttCount;
      Time currentRTT;
      Time lastPolicyUpdated;
      Time nextPolicyUpdate;
      Time prevRTT;
      uint32_t prevOutstandingPackets;
      uint32_t nPacketsAcked;



  };


}

#endif // TCPPW_H
