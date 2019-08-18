/******************************************************************************
 *
 * Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved.
 *                                        
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 
******************************************************************************/
#define _RTW_STA_MGT_C_

#include <drv_conf.h>
#include <osdep_service.h>
#include <drv_types.h>
#include <recv_osdep.h>
#include <xmit_osdep.h>
#include <mlme_osdep.h>


#if defined (PLATFORM_LINUX) && defined (PLATFORM_WINDOWS)

#error "Shall be Linux or Windows, but not both!\n"

#endif

#include <sta_info.h>

#ifdef PLATFORM_MPIXEL
BYTE pending_recvframe_queue_semId0 = 0, pending_recvframe_queue_semId1 = 0, pending_recvframe_queue_semId2 = 0, pending_recvframe_queue_semId3 = 0, pending_recvframe_queue_semId4 = 0, pending_recvframe_queue_semId5 = 0, pending_recvframe_queue_semId6 = 0, pending_recvframe_queue_semId7 = 0;
BYTE pending_recvframe_queue_semId8 = 0, pending_recvframe_queue_semId9 = 0, pending_recvframe_queue_semId10 = 0, pending_recvframe_queue_semId11 = 0, pending_recvframe_queue_semId12 = 0, pending_recvframe_queue_semId13 = 0, pending_recvframe_queue_semId14 = 0, pending_recvframe_queue_semId15 = 0;

_lock mpx_sta_lock;
_lock mpx_sta_sleep_q;
#endif

void _rtw_init_stainfo(struct sta_info *psta)
{

_func_enter_;
#ifdef PLATFORM_MPIXEL
	BYTE tmp_lock1 = 0, tmp_lock2 = 0;
	BYTE tmp_lock3 = 0, tmp_lock4 = 0, tmp_lock5 = 0, tmp_lock6 = 0, tmp_lock7 = 0;
	BYTE tmp_lock8 = 0, tmp_lock9 = 0, tmp_lock10 = 0;
	tmp_lock1 = (BYTE)psta->lock;
	tmp_lock2 = (BYTE)psta->sleep_q.lock;
	tmp_lock3 = (BYTE)psta->sta_xmitpriv.lock;
	tmp_lock4 = (BYTE)psta->sta_xmitpriv.be_q.sta_pending.lock;
	tmp_lock5 = (BYTE)psta->sta_xmitpriv.bk_q.sta_pending.lock;
	tmp_lock6 = (BYTE)psta->sta_xmitpriv.vi_q.sta_pending.lock;
	tmp_lock7 = (BYTE)psta->sta_xmitpriv.vo_q.sta_pending.lock;
	tmp_lock8 = (BYTE)psta->sta_recvpriv.lock;
	tmp_lock9 = (BYTE)psta->sta_recvpriv.defrag_q.lock;
#endif

	_rtw_memset((u8 *)psta, 0, sizeof (struct sta_info));

#ifdef PLATFORM_MPIXEL
	if(tmp_lock1)
		psta->lock = (_lock)tmp_lock1;
	if(tmp_lock2)
		psta->sleep_q.lock = (_lock)tmp_lock2;
	if(tmp_lock3)
		psta->sta_xmitpriv.lock = (_lock)tmp_lock3;
	if(tmp_lock4)
		psta->sta_xmitpriv.be_q.sta_pending.lock = (_lock)tmp_lock4;
	if(tmp_lock5)
		psta->sta_xmitpriv.bk_q.sta_pending.lock = (_lock)tmp_lock5;
	if(tmp_lock6)
		psta->sta_xmitpriv.vi_q.sta_pending.lock = (_lock)tmp_lock6;
	if(tmp_lock7)
		psta->sta_xmitpriv.vo_q.sta_pending.lock = (_lock)tmp_lock7;
	if(tmp_lock8)
		psta->sta_recvpriv.lock = (_lock)tmp_lock8;
	if(tmp_lock9)
		psta->sta_recvpriv.defrag_q.lock = (_lock)tmp_lock9;
	if(pending_recvframe_queue_semId0)
		psta->recvreorder_ctrl[0].pending_recvframe_queue.lock = (_lock)pending_recvframe_queue_semId0;
	if(pending_recvframe_queue_semId1)
		psta->recvreorder_ctrl[1].pending_recvframe_queue.lock = (_lock)pending_recvframe_queue_semId1;
	if(pending_recvframe_queue_semId2)
		psta->recvreorder_ctrl[2].pending_recvframe_queue.lock = (_lock)pending_recvframe_queue_semId2;
	if(pending_recvframe_queue_semId3)
		psta->recvreorder_ctrl[3].pending_recvframe_queue.lock = (_lock)pending_recvframe_queue_semId3;
	if(pending_recvframe_queue_semId4)
		psta->recvreorder_ctrl[4].pending_recvframe_queue.lock = (_lock)pending_recvframe_queue_semId4;
	if(pending_recvframe_queue_semId5)
		psta->recvreorder_ctrl[5].pending_recvframe_queue.lock = (_lock)pending_recvframe_queue_semId5;
	if(pending_recvframe_queue_semId6)
		psta->recvreorder_ctrl[6].pending_recvframe_queue.lock = (_lock)pending_recvframe_queue_semId6;
	if(pending_recvframe_queue_semId7)
		psta->recvreorder_ctrl[7].pending_recvframe_queue.lock = (_lock)pending_recvframe_queue_semId7;
	if(pending_recvframe_queue_semId8)
		psta->recvreorder_ctrl[8].pending_recvframe_queue.lock = (_lock)pending_recvframe_queue_semId8;
	if(pending_recvframe_queue_semId9)
		psta->recvreorder_ctrl[9].pending_recvframe_queue.lock = (_lock)pending_recvframe_queue_semId9;
	if(pending_recvframe_queue_semId10)
		psta->recvreorder_ctrl[10].pending_recvframe_queue.lock = (_lock)pending_recvframe_queue_semId10;
	if(pending_recvframe_queue_semId11)
		psta->recvreorder_ctrl[11].pending_recvframe_queue.lock = (_lock)pending_recvframe_queue_semId11;
	if(pending_recvframe_queue_semId2)
		psta->recvreorder_ctrl[12].pending_recvframe_queue.lock = (_lock)pending_recvframe_queue_semId12;
	if(pending_recvframe_queue_semId13)
		psta->recvreorder_ctrl[13].pending_recvframe_queue.lock = (_lock)pending_recvframe_queue_semId13;
	if(pending_recvframe_queue_semId14)
		psta->recvreorder_ctrl[14].pending_recvframe_queue.lock = (_lock)pending_recvframe_queue_semId14;
	if(pending_recvframe_queue_semId15)
		psta->recvreorder_ctrl[15].pending_recvframe_queue.lock = (_lock)pending_recvframe_queue_semId15;
#endif

#ifdef PLATFORM_MPIXEL
	if (!mpx_sta_lock)
		_rtw_spinlock_init(&mpx_sta_lock);
	psta->lock = mpx_sta_lock;
#else
	 _rtw_spinlock_init(&psta->lock);
#endif
	_rtw_init_listhead(&psta->list);
	_rtw_init_listhead(&psta->hash_list);
	//_rtw_init_listhead(&psta->asoc_list);
	//_rtw_init_listhead(&psta->sleep_list);
	//_rtw_init_listhead(&psta->wakeup_list);	

#ifdef PLATFORM_MPIXEL
	if (!mpx_sta_sleep_q)
		_rtw_spinlock_init(&mpx_sta_sleep_q);
	_rtw_init_listhead(&((&psta->sleep_q)->queue));
	(&psta->sleep_q)->lock = mpx_sta_sleep_q;
#else
	_rtw_init_queue(&psta->sleep_q);
#endif
	psta->sleepq_len = 0;

	_rtw_init_sta_xmit_priv(&psta->sta_xmitpriv);
	_rtw_init_sta_recv_priv(&psta->sta_recvpriv);
	
#ifdef CONFIG_AP_MODE

	_rtw_init_listhead(&psta->asoc_list);

	_rtw_init_listhead(&psta->auth_list);
	
	psta->expire_to = 0;
	
	psta->flags = 0;
	
	psta->capability = 0;


#ifdef CONFIG_NATIVEAP_MLME
	psta->nonerp_set = 0;
	psta->no_short_slot_time_set = 0;
	psta->no_short_preamble_set = 0;
	psta->no_ht_gf_set = 0;
	psta->no_ht_set = 0;
	psta->ht_20mhz_set = 0;
#endif	
	
#endif	
	
_func_exit_;	

}

u32	_rtw_init_sta_priv(struct	sta_priv *pstapriv)
{
	struct sta_info *psta;
	s32 i;
	
_func_enter_;	

	pstapriv->pallocated_stainfo_buf = rtw_zvmalloc (sizeof(struct sta_info) * NUM_STA+ 4);
	
	if(!pstapriv->pallocated_stainfo_buf)
		return _FAIL;

	pstapriv->pstainfo_buf = pstapriv->pallocated_stainfo_buf + 4 - 
		((SIZE_PTR)(pstapriv->pallocated_stainfo_buf ) & 3);

	_rtw_init_queue(&pstapriv->free_sta_queue);

	_rtw_spinlock_init(&pstapriv->sta_hash_lock);
	
	//_rtw_init_queue(&pstapriv->asoc_q);
	pstapriv->asoc_sta_count = 0;
	_rtw_init_queue(&pstapriv->sleep_q);
	_rtw_init_queue(&pstapriv->wakeup_q);

	psta = (struct sta_info *)(pstapriv->pstainfo_buf);

		
	for(i = 0; i < NUM_STA; i++)
	{
		_rtw_init_stainfo(psta);

		_rtw_init_listhead(&(pstapriv->sta_hash[i]));

		rtw_list_insert_tail(&psta->list, get_list_head(&pstapriv->free_sta_queue));

		psta++;
	}

#ifdef CONFIG_AP_MODE

	pstapriv->sta_dz_bitmap = 0;
	pstapriv->tim_bitmap = 0;

	_rtw_init_listhead(&pstapriv->asoc_list);
	_rtw_init_listhead(&pstapriv->auth_list);
	pstapriv->auth_to = 3; // 3*2 = 6 sec 
	pstapriv->assoc_to = 3;
	pstapriv->expire_to = 900;// 900*2 = 1800 sec = 30 min, expire after no any traffic.
	
	pstapriv->max_num_sta = NUM_STA;
	
#endif
	
_func_exit_;		

	return _SUCCESS;
	
}

void	_rtw_free_sta_xmit_priv_lock(struct sta_xmit_priv *psta_xmitpriv)
{
_func_enter_;

	_rtw_spinlock_free(&psta_xmitpriv->lock);

	_rtw_spinlock_free(&(psta_xmitpriv->be_q.sta_pending.lock));
	_rtw_spinlock_free(&(psta_xmitpriv->bk_q.sta_pending.lock));
	_rtw_spinlock_free(&(psta_xmitpriv->vi_q.sta_pending.lock));
	_rtw_spinlock_free(&(psta_xmitpriv->vo_q.sta_pending.lock));
_func_exit_;	
}

static void	_rtw_free_sta_recv_priv_lock(struct sta_recv_priv *psta_recvpriv)
{
_func_enter_;	

	_rtw_spinlock_free(&psta_recvpriv->lock);

	_rtw_spinlock_free(&(psta_recvpriv->defrag_q.lock));

_func_exit_;

}

void rtw_mfree_stainfo(struct sta_info *psta)
{
_func_enter_;

	if(&psta->lock != NULL)
		 _rtw_spinlock_free(&psta->lock);

	_rtw_free_sta_xmit_priv_lock(&psta->sta_xmitpriv);
	_rtw_free_sta_recv_priv_lock(&psta->sta_recvpriv);
	
_func_exit_;	
}


// this function is used to free the memory of lock || sema for all stainfos
void rtw_mfree_all_stainfo(struct sta_priv *pstapriv )
{
	_irqL	 irqL;
	_list	*plist, *phead;
	struct sta_info *psta = NULL;
	
_func_enter_;	

	_enter_critical_bh(&pstapriv->sta_hash_lock, &irqL);

	phead = get_list_head(&pstapriv->free_sta_queue);
	plist = get_next(phead);
		
	while ((rtw_end_of_queue_search(phead, plist)) == _FALSE)
	{
		psta = LIST_CONTAINOR(plist, struct sta_info ,list);
		plist = get_next(plist);

		rtw_mfree_stainfo(psta);
	}
	
	_exit_critical_bh(&pstapriv->sta_hash_lock, &irqL);

_func_exit_;	

}


void rtw_mfree_sta_priv_lock(struct	sta_priv *pstapriv)
{
	 rtw_mfree_all_stainfo(pstapriv); //be done before free sta_hash_lock

	_rtw_spinlock_free(&pstapriv->free_sta_queue.lock);

	_rtw_spinlock_free(&pstapriv->sta_hash_lock);
	_rtw_spinlock_free(&pstapriv->wakeup_q.lock);
	_rtw_spinlock_free(&pstapriv->sleep_q.lock);

}

u32	_rtw_free_sta_priv(struct	sta_priv *pstapriv)
{
_func_enter_;
	if(pstapriv){
		rtw_mfree_sta_priv_lock(pstapriv);

		if(pstapriv->pallocated_stainfo_buf) {
			rtw_vmfree(pstapriv->pallocated_stainfo_buf, sizeof(struct sta_info)*NUM_STA+4);
		}
	}
	
_func_exit_;
	return _SUCCESS;
}


//struct	sta_info *rtw_alloc_stainfo(_queue *pfree_sta_queue, unsigned char *hwaddr)
struct	sta_info *rtw_alloc_stainfo(struct	sta_priv *pstapriv, u8 *hwaddr) 
{	
	_irqL irqL, irqL2;
	uint tmp_aid;
	s32	index;
	_list	*phash_list;
	struct sta_info	*psta;
	_queue *pfree_sta_queue;
	struct recv_reorder_ctrl *preorder_ctrl;
	int i = 0;
	u16  wRxSeqInitialValue = 0xffff;
	
_func_enter_;	

	pfree_sta_queue = &pstapriv->free_sta_queue;
	
	_enter_critical_bh(&(pfree_sta_queue->lock), &irqL);

	if (_rtw_queue_empty(pfree_sta_queue) == _TRUE)
	{
		psta = NULL;
	}
	else
	{
		psta = LIST_CONTAINOR(get_next(&pfree_sta_queue->queue), struct sta_info, list);
		
		rtw_list_delete(&(psta->list));
		
		tmp_aid = psta->aid;	
	
		_rtw_init_stainfo(psta);

		_rtw_memcpy(psta->hwaddr, hwaddr, ETH_ALEN);

		index = wifi_mac_hash(hwaddr);

		RT_TRACE(_module_rtl871x_sta_mgt_c_,_drv_info_,("rtw_alloc_stainfo: index  = %x", index));

		if(index >= NUM_STA){
			RT_TRACE(_module_rtl871x_sta_mgt_c_,_drv_err_,("ERROR=> rtw_alloc_stainfo: index >= NUM_STA"));
			psta= NULL;	
			goto exit;
		}
		phash_list = &(pstapriv->sta_hash[index]);

		_enter_critical_bh(&(pstapriv->sta_hash_lock), &irqL2);

		rtw_list_insert_tail(&psta->hash_list, phash_list);

		pstapriv->asoc_sta_count ++ ;

		_exit_critical_bh(&(pstapriv->sta_hash_lock), &irqL2);

// Commented by Albert 2009/08/13
// For the SMC router, the sequence number of first packet of WPS handshake will be 0.
// In this case, this packet will be dropped by recv_decache function if we use the 0x00 as the default value for tid_rxseq variable.
// So, we initialize the tid_rxseq variable as the 0xffff.

		for( i = 0; i < 16; i++ )
		{
                     _rtw_memcpy( &psta->sta_recvpriv.rxcache.tid_rxseq[ i ], &wRxSeqInitialValue, 2 );
		}

		RT_TRACE(_module_rtl871x_sta_mgt_c_,_drv_info_,("alloc number_%d stainfo  with hwaddr = %x %x %x %x %x %x  \n", 
		pstapriv->asoc_sta_count , hwaddr[0], hwaddr[1], hwaddr[2],hwaddr[3],hwaddr[4],hwaddr[5]));

		init_addba_retry_timer(pstapriv->padapter, psta);

#ifdef CONFIG_TDLS
		psta->padapter = pstapriv->padapter;
		init_TPK_timer(pstapriv->padapter, psta);
		_init_workitem(&psta->option_workitem, TDLS_option_workitem_callback, psta);
		init_ch_switch_timer(pstapriv->padapter, psta);
		init_base_ch_timer(pstapriv->padapter, psta);
		_init_workitem(&psta->base_ch_workitem, base_channel_workitem_callback, psta);
		init_off_ch_timer(pstapriv->padapter, psta);
		_init_workitem(&psta->off_ch_workitem, off_channel_workitem_callback, psta);
#endif

		//for A-MPDU Rx reordering buffer control
		for(i=0; i < 16 ; i++)
		{
			preorder_ctrl = &psta->recvreorder_ctrl[i];

			preorder_ctrl->padapter = pstapriv->padapter;
		
			preorder_ctrl->enable = _FALSE;
		
			preorder_ctrl->indicate_seq = 0xffff;
			#ifdef DBG_RX_SEQ
			DBG_871X("DBG_RX_SEQ %s:%d IndicateSeq: %d\n", __FUNCTION__, __LINE__,
				preorder_ctrl->indicate_seq);
			#endif
			preorder_ctrl->wend_b= 0xffff;       
			//preorder_ctrl->wsize_b = (NR_RECVBUFF-2);
			preorder_ctrl->wsize_b = 64;//64;

			_rtw_init_queue(&preorder_ctrl->pending_recvframe_queue);

#ifdef PLATFORM_MPIXEL
			if(preorder_ctrl->pending_recvframe_queue.lock)
			{
				switch(i)
				{
					case 0:
						pending_recvframe_queue_semId0 = (BYTE)preorder_ctrl->pending_recvframe_queue.lock;
						break;
					case 1:
						pending_recvframe_queue_semId1 = (BYTE)preorder_ctrl->pending_recvframe_queue.lock;
						break;
					case 2:
						pending_recvframe_queue_semId2 = (BYTE)preorder_ctrl->pending_recvframe_queue.lock;
						break;
					case 3:
						pending_recvframe_queue_semId3 = (BYTE)preorder_ctrl->pending_recvframe_queue.lock;
						break;
					case 4:
						pending_recvframe_queue_semId4 = (BYTE)preorder_ctrl->pending_recvframe_queue.lock;
						break;
					case 5:
						pending_recvframe_queue_semId5 = (BYTE)preorder_ctrl->pending_recvframe_queue.lock;
						break;
					case 6:
						pending_recvframe_queue_semId6 = (BYTE)preorder_ctrl->pending_recvframe_queue.lock;
						break;
					case 7:
						pending_recvframe_queue_semId7 = (BYTE)preorder_ctrl->pending_recvframe_queue.lock;
						break;
					case 8:
						pending_recvframe_queue_semId8 = (BYTE)preorder_ctrl->pending_recvframe_queue.lock;
						break;
					case 9:
						pending_recvframe_queue_semId9 = (BYTE)preorder_ctrl->pending_recvframe_queue.lock;
						break;
					case 10:
						pending_recvframe_queue_semId10 = (BYTE)preorder_ctrl->pending_recvframe_queue.lock;
						break;
					case 11:
						pending_recvframe_queue_semId11 = (BYTE)preorder_ctrl->pending_recvframe_queue.lock;
						break;
					case 12:
						pending_recvframe_queue_semId12 = (BYTE)preorder_ctrl->pending_recvframe_queue.lock;
						break;
					case 13:
						pending_recvframe_queue_semId13 = (BYTE)preorder_ctrl->pending_recvframe_queue.lock;
						break;
					case 14:
						pending_recvframe_queue_semId14 = (BYTE)preorder_ctrl->pending_recvframe_queue.lock;
						break;
					case 15:
						pending_recvframe_queue_semId15 = (BYTE)preorder_ctrl->pending_recvframe_queue.lock;
						break;
				}
			}
#endif

			rtw_init_recv_timer(preorder_ctrl);
		}

	}
	
exit:

	_exit_critical_bh(&(pfree_sta_queue->lock), &irqL);
	
_func_exit_;	

	return psta;


}


// using pstapriv->sta_hash_lock to protect
u32	rtw_free_stainfo(_adapter *padapter , struct sta_info *psta)
{	
	int i;
	_irqL irqL0;
	_queue *pfree_sta_queue;
	struct recv_reorder_ctrl *preorder_ctrl;
	struct	sta_xmit_priv	*pstaxmitpriv;
	struct	xmit_priv	*pxmitpriv= &padapter->xmitpriv;
	struct	sta_priv *pstapriv = &padapter->stapriv;
	

_func_enter_;	
	
	if (psta == NULL)
		goto exit;

	pfree_sta_queue = &pstapriv->free_sta_queue;


	pstaxmitpriv = &psta->sta_xmitpriv;
	
	//rtw_list_delete(&psta->sleep_list);
	
	//rtw_list_delete(&psta->wakeup_list);
	
	rtw_free_xmitframe_queue(pxmitpriv, &psta->sleep_q);
	psta->sleepq_len = 0;
	
	_enter_critical_bh(&(pxmitpriv->vo_pending.lock), &irqL0);

	rtw_free_xmitframe_queue( pxmitpriv, &pstaxmitpriv->vo_q.sta_pending);

	rtw_list_delete(&(pstaxmitpriv->vo_q.tx_pending));

	_exit_critical_bh(&(pxmitpriv->vo_pending.lock), &irqL0);
	

	_enter_critical_bh(&(pxmitpriv->vi_pending.lock), &irqL0);

	rtw_free_xmitframe_queue( pxmitpriv, &pstaxmitpriv->vi_q.sta_pending);

	rtw_list_delete(&(pstaxmitpriv->vi_q.tx_pending));

	_exit_critical_bh(&(pxmitpriv->vi_pending.lock), &irqL0);


	_enter_critical_bh(&(pxmitpriv->bk_pending.lock), &irqL0);

	rtw_free_xmitframe_queue( pxmitpriv, &pstaxmitpriv->bk_q.sta_pending);

	rtw_list_delete(&(pstaxmitpriv->bk_q.tx_pending));

	_exit_critical_bh(&(pxmitpriv->bk_pending.lock), &irqL0);

	_enter_critical_bh(&(pxmitpriv->be_pending.lock), &irqL0);

	rtw_free_xmitframe_queue( pxmitpriv, &pstaxmitpriv->be_q.sta_pending);

	rtw_list_delete(&(pstaxmitpriv->be_q.tx_pending));

	_exit_critical_bh(&(pxmitpriv->be_pending.lock), &irqL0);
	
	
	rtw_list_delete(&psta->hash_list);
	RT_TRACE(_module_rtl871x_sta_mgt_c_,_drv_err_,("\n free number_%d stainfo  with hwaddr = 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x  \n",pstapriv->asoc_sta_count , psta->hwaddr[0], psta->hwaddr[1], psta->hwaddr[2],psta->hwaddr[3],psta->hwaddr[4],psta->hwaddr[5]));
	pstapriv->asoc_sta_count --;
	
	
	// re-init sta_info; 20061114
	_rtw_init_sta_xmit_priv(&psta->sta_xmitpriv);
	_rtw_init_sta_recv_priv(&psta->sta_recvpriv);

	_cancel_timer_ex(&psta->addba_retry_timer);

#ifdef CONFIG_TDLS
	_cancel_timer_ex(&psta->TPK_timer);
	_cancel_timer_ex(&psta->option_timer);
	_cancel_timer_ex(&psta->base_ch_timer);
	_cancel_timer_ex(&psta->off_ch_timer);
#endif

	//for A-MPDU Rx reordering buffer control, cancel reordering_ctrl_timer
	for(i=0; i < 16 ; i++)
	{
		preorder_ctrl = &psta->recvreorder_ctrl[i];
		
		_cancel_timer_ex(&preorder_ctrl->reordering_ctrl_timer);		
	}


#ifdef CONFIG_AP_MODE

	rtw_list_delete(&psta->asoc_list);	
	rtw_list_delete(&psta->auth_list);
	psta->expire_to = 0;
	
	psta->sleepq_ac_len = 0;
	psta->qos_info = 0;

	psta->max_sp_len = 0;
	psta->uapsd_bk = 0;
	psta->uapsd_be = 0;
	psta->uapsd_vi = 0;
	psta->uapsd_vo = 0;

	psta->has_legacy_ac = 0;

#ifdef CONFIG_NATIVEAP_MLME
	
	pstapriv->sta_dz_bitmap &=~BIT(psta->aid);
	pstapriv->tim_bitmap &=~BIT(psta->aid);	

	rtw_indicate_sta_disassoc_event(padapter, psta);

	if (pstapriv->sta_aid[psta->aid - 1] == psta)
	{
		pstapriv->sta_aid[psta->aid - 1] = NULL;
		psta->aid = 0;
	}	

#endif	

#endif	

	_enter_critical_bh(&(pfree_sta_queue->lock), &irqL0);
	rtw_list_insert_tail(&psta->list, get_list_head(pfree_sta_queue));
	_exit_critical_bh(&(pfree_sta_queue->lock), &irqL0);

exit:	
	
_func_exit_;	

	return _SUCCESS;
	
}

// free all stainfo which in sta_hash[all]
void rtw_free_all_stainfo(_adapter *padapter)
{
	_irqL	 irqL;
	_list	*plist, *phead;
	s32	index;
	struct sta_info *psta = NULL;
	struct	sta_priv *pstapriv = &padapter->stapriv;
	struct sta_info* pbcmc_stainfo =rtw_get_bcmc_stainfo( padapter);
	
_func_enter_;	

	if(pstapriv->asoc_sta_count==1)
		goto exit;

	_enter_critical_bh(&pstapriv->sta_hash_lock, &irqL);

	for(index=0; index< NUM_STA; index++)
	{
		phead = &(pstapriv->sta_hash[index]);
		plist = get_next(phead);
		
		while ((rtw_end_of_queue_search(phead, plist)) == _FALSE)
		{
			psta = LIST_CONTAINOR(plist, struct sta_info ,hash_list);

			plist = get_next(plist);

			if(pbcmc_stainfo!=psta)					
				rtw_free_stainfo(padapter , psta);
			
		}
	}
	
	_exit_critical_bh(&pstapriv->sta_hash_lock, &irqL);
	
exit:	
	
_func_exit_;	

}

/* any station allocated can be searched by hash list */
struct sta_info *rtw_get_stainfo(struct sta_priv *pstapriv, u8 *hwaddr)
{

	_irqL	 irqL;

	_list	*plist, *phead;

	struct sta_info *psta = NULL;
	
	u32	index;

	u8 *addr;

	u8 bc_addr[ETH_ALEN] = {0xff,0xff,0xff,0xff,0xff,0xff};

_func_enter_;

	if(hwaddr==NULL)
		return NULL;
		
	if(IS_MCAST(hwaddr))
	{
		addr = bc_addr;
	}
	else
	{
		addr = hwaddr;
	}

	index = wifi_mac_hash(addr);

	_enter_critical_bh(&pstapriv->sta_hash_lock, &irqL);
	
	phead = &(pstapriv->sta_hash[index]);
	plist = get_next(phead);


	while ((rtw_end_of_queue_search(phead, plist)) == _FALSE)
	{
	
		psta = LIST_CONTAINOR(plist, struct sta_info, hash_list);
		
		if ((_rtw_memcmp(psta->hwaddr, addr, ETH_ALEN))== _TRUE) 
		{ // if found the matched address
			break;
		}
		psta=NULL;
		plist = get_next(plist);
	}

	_exit_critical_bh(&pstapriv->sta_hash_lock, &irqL);
_func_exit_;	
	return psta;
	
}

u32 rtw_init_bcmc_stainfo(_adapter* padapter)
{

	struct sta_info 	*psta;
	struct tx_servq	*ptxservq;
	u32 res=_SUCCESS;
	NDIS_802_11_MAC_ADDRESS	bcast_addr= {0xff,0xff,0xff,0xff,0xff,0xff};
	
	struct	sta_priv *pstapriv = &padapter->stapriv;
	_queue	*pstapending = &padapter->xmitpriv.bm_pending; 
	
_func_enter_;

	psta = rtw_alloc_stainfo(pstapriv, bcast_addr);
	
	if(psta==NULL){
		res=_FAIL;
		RT_TRACE(_module_rtl871x_sta_mgt_c_,_drv_err_,("rtw_alloc_stainfo fail"));
		goto exit;
	}

	// default broadcast & multicast use macid 1
	psta->mac_id = 1;

	ptxservq= &(psta->sta_xmitpriv.be_q);

/*
	_enter_critical(&pstapending->lock, &irqL0);

	if (rtw_is_list_empty(&ptxservq->tx_pending))
		rtw_list_insert_tail(&ptxservq->tx_pending, get_list_head(pstapending));

	_exit_critical(&pstapending->lock, &irqL0);
*/
	
exit:
_func_exit_;		
	return _SUCCESS;

}


struct sta_info* rtw_get_bcmc_stainfo(_adapter* padapter)
{
	struct sta_info 	*psta;
	struct sta_priv 	*pstapriv = &padapter->stapriv;
	u8 bc_addr[ETH_ALEN] = {0xff,0xff,0xff,0xff,0xff,0xff};
_func_enter_;
	 psta = rtw_get_stainfo(pstapriv, bc_addr);
_func_exit_;		 
	return psta;

}

u8 rtw_access_ctrl(struct wlan_acl_pool* pacl_list, u8 * mac_addr)
{
	return _TRUE;
}

