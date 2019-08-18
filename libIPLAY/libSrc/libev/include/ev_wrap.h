/* DO NOT EDIT, automatically generated by update_ev_wrap */
#ifndef EV_WRAP_H
#define EV_WRAP_H
#define now_floor ((loop)->now_floor)
#define mn_now ((loop)->mn_now)
#define rtmn_diff ((loop)->rtmn_diff)
#define io_blocktime ((loop)->io_blocktime)
#define timeout_blocktime ((loop)->timeout_blocktime)
#define backend ((loop)->backend)
#define activecnt ((loop)->activecnt)
#define loop_done ((loop)->loop_done)
#define backend_fd ((loop)->backend_fd)
#define backend_mintime ((loop)->backend_mintime)
#define backend_modify ((loop)->backend_modify)
#define backend_poll ((loop)->backend_poll)
#define anfds ((loop)->anfds)
#define anfdmax ((loop)->anfdmax)
#define pendings ((loop)->pendings)
#define pendingmax ((loop)->pendingmax)
#define pendingcnt ((loop)->pendingcnt)
#define pending_w ((loop)->pending_w)
#define rfeeds ((loop)->rfeeds)
#define rfeedmax ((loop)->rfeedmax)
#define rfeedcnt ((loop)->rfeedcnt)
#define evfd ((loop)->evfd)
#define evpipe ((loop)->evpipe)
#define pipe_w ((loop)->pipe_w)
#define pipe_write_wanted ((loop)->pipe_write_wanted)
#define pipe_write_skipped ((loop)->pipe_write_skipped)
#define curpid ((loop)->curpid)
#define postfork ((loop)->postfork)
#define vec_ri ((loop)->vec_ri)
#define vec_ro ((loop)->vec_ro)
#define vec_wi ((loop)->vec_wi)
#define vec_wo ((loop)->vec_wo)
#define vec_eo ((loop)->vec_eo)
#define vec_max ((loop)->vec_max)
#define polls ((loop)->polls)
#define pollmax ((loop)->pollmax)
#define pollcnt ((loop)->pollcnt)
#define pollidxs ((loop)->pollidxs)
#define pollidxmax ((loop)->pollidxmax)
#define epoll_events ((loop)->epoll_events)
#define epoll_eventmax ((loop)->epoll_eventmax)
#define epoll_eperms ((loop)->epoll_eperms)
#define epoll_epermcnt ((loop)->epoll_epermcnt)
#define epoll_epermmax ((loop)->epoll_epermmax)
#define kqueue_changes ((loop)->kqueue_changes)
#define kqueue_changemax ((loop)->kqueue_changemax)
#define kqueue_changecnt ((loop)->kqueue_changecnt)
#define kqueue_events ((loop)->kqueue_events)
#define kqueue_eventmax ((loop)->kqueue_eventmax)
#define port_events ((loop)->port_events)
#define port_eventmax ((loop)->port_eventmax)
#define iocp ((loop)->iocp)
#define fdchanges ((loop)->fdchanges)
#define fdchangemax ((loop)->fdchangemax)
#define fdchangecnt ((loop)->fdchangecnt)
#define timers ((loop)->timers)
#define timermax ((loop)->timermax)
#define timercnt ((loop)->timercnt)
#define periodics ((loop)->periodics)
#define periodicmax ((loop)->periodicmax)
#define periodiccnt ((loop)->periodiccnt)
#define idles ((loop)->idles)
#define idlemax ((loop)->idlemax)
#define idlecnt ((loop)->idlecnt)
#define idleall ((loop)->idleall)
#define prepares ((loop)->prepares)
#define preparemax ((loop)->preparemax)
#define preparecnt ((loop)->preparecnt)
#define checks ((loop)->checks)
#define checkmax ((loop)->checkmax)
#define checkcnt ((loop)->checkcnt)
#define forks ((loop)->forks)
#define forkmax ((loop)->forkmax)
#define forkcnt ((loop)->forkcnt)
#define cleanups ((loop)->cleanups)
#define cleanupmax ((loop)->cleanupmax)
#define cleanupcnt ((loop)->cleanupcnt)
#define async_pending ((loop)->async_pending)
#define asyncs ((loop)->asyncs)
#define asyncmax ((loop)->asyncmax)
#define asynccnt ((loop)->asynccnt)
#define fs_fd ((loop)->fs_fd)
#define fs_w ((loop)->fs_w)
#define fs_2625 ((loop)->fs_2625)
#define fs_hash ((loop)->fs_hash)
#define sig_pending ((loop)->sig_pending)
#define sigfd ((loop)->sigfd)
#define sigfd_w ((loop)->sigfd_w)
#define sigfd_set ((loop)->sigfd_set)
#define origflags ((loop)->origflags)
#define loop_count ((loop)->loop_count)
#define loop_depth ((loop)->loop_depth)
#define userdata ((loop)->userdata)
#define release_cb ((loop)->release_cb)
#define acquire_cb ((loop)->acquire_cb)
#define invoke_cb ((loop)->invoke_cb)
#else
#undef EV_WRAP_H
#undef now_floor
#undef mn_now
#undef rtmn_diff
#undef io_blocktime
#undef timeout_blocktime
#undef backend
#undef activecnt
#undef loop_done
#undef backend_fd
#undef backend_mintime
#undef backend_modify
#undef backend_poll
#undef anfds
#undef anfdmax
#undef pendings
#undef pendingmax
#undef pendingcnt
#undef pending_w
#undef rfeeds
#undef rfeedmax
#undef rfeedcnt
#undef evfd
#undef evpipe
#undef pipe_w
#undef pipe_write_wanted
#undef pipe_write_skipped
#undef curpid
#undef postfork
#error dod1
#undef vec_ri
#undef vec_ro
#undef vec_wi
#undef vec_wo
#undef vec_eo
#undef vec_max
#undef polls
#undef pollmax
#undef pollcnt
#undef pollidxs
#undef pollidxmax
#undef epoll_events
#undef epoll_eventmax
#undef epoll_eperms
#undef epoll_epermcnt
#undef epoll_epermmax
#undef kqueue_changes
#undef kqueue_changemax
#undef kqueue_changecnt
#undef kqueue_events
#undef kqueue_eventmax
#undef port_events
#undef port_eventmax
#undef iocp
#undef fdchanges
#undef fdchangemax
#undef fdchangecnt
#undef timers
#undef timermax
#undef timercnt
#undef periodics
#undef periodicmax
#undef periodiccnt
#undef idles
#undef idlemax
#undef idlecnt
#undef idleall
#undef prepares
#undef preparemax
#undef preparecnt
#undef checks
#undef checkmax
#undef checkcnt
#undef forks
#undef forkmax
#undef forkcnt
#undef cleanups
#undef cleanupmax
#undef cleanupcnt
#undef async_pending
#undef asyncs
#undef asyncmax
#undef asynccnt
#undef fs_fd
#undef fs_w
#undef fs_2625
#undef fs_hash
#undef sig_pending
#undef sigfd
#undef sigfd_w
#undef sigfd_set
#undef origflags
#undef loop_count
#undef loop_depth
#undef userdata
#undef release_cb
#undef acquire_cb
#undef invoke_cb
#endif
