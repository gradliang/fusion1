/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2006, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************
*/
#include "rt_config.h"
#include "os_macros.h"

/*
    ==========================================================================
    Description:
        authentication state machine init procedure
    Parameters:
        Sm - the state machine
    Note:
        the state machine looks like the following 
        
                                    AUTH_RSP_IDLE                       AUTH_RSP_WAIT_CHAL
    MT2_AUTH_CHALLENGE_TIMEOUT      auth_rsp_challenge_timeout_action   auth_rsp_challenge_timeout_action
    MT2_PEER_AUTH_ODD               peer_auth_at_auth_rsp_idle_action   peer_auth_at_auth_rsp_wait_action
    MT2_PEER_DEAUTH                 peer_deauth_action                  peer_deauth_action
    ==========================================================================
 */
VOID AuthRspStateMachineInit(
    IN PRTMP_ADAPTER pAd, 
    IN PSTATE_MACHINE Sm, 
    IN STATE_MACHINE_FUNC Trans[]) 
{
    ULONG        Now;

    StateMachineInit(Sm, (STATE_MACHINE_FUNC*)Trans, MAX_AUTH_RSP_STATE, MAX_AUTH_RSP_MSG, (STATE_MACHINE_FUNC)Drop, AUTH_RSP_IDLE, AUTH_RSP_MACHINE_BASE);

    // column 1
    StateMachineSetAction(Sm, AUTH_RSP_IDLE, MT2_PEER_DEAUTH, (STATE_MACHINE_FUNC)PeerDeauthAction);

    // column 2
    StateMachineSetAction(Sm, AUTH_RSP_WAIT_CHAL, MT2_PEER_DEAUTH, (STATE_MACHINE_FUNC)PeerDeauthAction);

    
    // initialize the random number generator
    Now = jiffies;
    LfsrInit(pAd, Now);
}

/*
    ==========================================================================
    Description:
    ==========================================================================
*/
VOID PeerAuthSimpleRspGenAndSend(
    IN PRTMP_ADAPTER pAd, 
    IN PHEADER_802_11 pHdr80211,
    IN USHORT Alg, 
    IN USHORT Seq, 
    IN USHORT Reason, 
    IN USHORT Status) 
{
    HEADER_802_11   AuthHdr;
    UINT            FrameLen = 0;
    PUCHAR          pOutBuffer = NULL;
    USHORT          NStatus;
    
    // allocate and send out Auth_Rsp frame
    NStatus = MlmeAllocateMemory(pAd, (PVOID *)&pOutBuffer);  //Get an unused nonpaged memory
    if (NStatus != NDIS_STATUS_SUCCESS)
        return;

    if (Reason == MLME_SUCCESS)
    {
        DBGPRINT(RT_DEBUG_TRACE, "Send AUTH response (seq#2)...\n");
        MgtMacHeaderInit(pAd, &AuthHdr, SUBTYPE_AUTH, 0, pHdr80211->Addr2, pAd->MlmeAux.Bssid);
        MakeOutgoingFrame(pOutBuffer,               &FrameLen, 
                          sizeof(HEADER_802_11),    &AuthHdr, 
                          2,                        &Alg, 
                          2,                        &Seq, 
                          2,                        &Reason, 
                          END_OF_ARGS);
        MiniportMMRequest(pAd, QID_AC_BE, pOutBuffer, FrameLen);
    }
    else
    {
        DBGPRINT(RT_DEBUG_TRACE, "Peer AUTH fail...\n");
        return;
    }
}

/*
    ==========================================================================
    Description:
    ==========================================================================
*/
VOID PeerDeauthAction(
    IN PRTMP_ADAPTER pAd, 
    IN PMLME_QUEUE_ELEM Elem) 
{
    UCHAR       Addr2[MAC_ADDR_LEN];
    USHORT      Reason;
#ifdef RALINK_WPA_SUPPLICANT_SUPPORT
    union iwreq_data wrqu;
#endif

    if (PeerDeauthSanity(pAd, Elem->Msg, Elem->MsgLen, Addr2, &Reason))
    {
        if (INFRA_ON(pAd) && MAC_ADDR_EQUAL(Addr2, &pAd->PortCfg.Bssid))
        {
            DBGPRINT(RT_DEBUG_TRACE,"AUTH_RSP - receive DE-AUTH from our AP\n");
            LinkDown(pAd, TRUE);
            mpDebugPrint("%s", __FUNCTION__);
#ifdef PLATFORM_MPIXEL
            os_carrier_off(pAd);
#ifdef RALINK_WPA_SUPPLICANT_SUPPORT            /* TODO added billwang */
            if (pAd->PortCfg.WPA_Supplicant == TRUE) {
                // send disassoc event to wpa_supplicant 
                memset(&wrqu, 0, sizeof(wrqu));
                wrqu.data.flags = RT_DISASSOC_EVENT_FLAG;
                mpDebugPrint("%s: snd event", __FUNCTION__);
                wireless_send_event(pAd->net_dev, IWEVCUSTOM, &wrqu, NULL);
            }
#endif
#ifdef NATIVE_WPA_SUPPLICANT_SUPPORT
            // collect associate info and notify the wpa_supplicant. 
            if (pAd->PortCfg.bNativeWpa == TRUE)
                wext_notify_event_assoc(pAd, SIOCGIWAP, FALSE);
#endif // NATIVE_WPA_SUPPLICANT_SUPPORT
#endif
        }
    }
    else
    {
        DBGPRINT(RT_DEBUG_TRACE,"AUTH_RSP - PeerDeauthAction() sanity check fail\n");
    }
}

