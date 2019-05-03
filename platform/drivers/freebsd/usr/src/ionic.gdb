#
# Script to work with ionic structures under FreeBSD kgdb
#

define __indent
	printf "                 "
end

define __be16tole
	set $__be16 = (uint16_t)$arg0
	set $arg1 = (($__be16 & 0xff00) >> 8 ) |			\
		    (($__be16 & 0x00ff) << 8 )
end

define __be32tole
	set $__be32 = (uint32_t)$arg0
	set $arg1 = (($__be32 & 0xff000000) >> 24) |			\
		    (($__be32 & 0x00ff0000) >> 8 ) |			\
		    (($__be32 & 0x0000ff00) << 8 ) |			\
		    (($__be32 & 0x000000ff) << 24)
end

define __be64tole
       set $__be64 = (uint64_t)$arg0
       set $__be32u = (uint32_t)(($__be64 & 0xffffffff00000000) >> 32)
       set $__be32l = (uint32_t) ($__be64 & 0x00000000ffffffff)
       __be32tole $__be32u $__le32l
       __be32tole $__be32l $__le32u
       set $arg1 = ($__le32u << 32) | ($__le32l)
end

# Args: queue
define _ionic_q_print
	set $__q = (struct ionic_queue *)$arg0
	__indent
	printf "wr %6d  prod %4d  cons %4d  ",				\
		$__q->mask + 1, $__q->prod, $__q->cons
	if ($__q->ptr != 0)
		printf "KERNEL\n"
	else
		printf "USER\n"
	end
end

# Args: res
define _ionic_res_print
	set $res = (struct ionic_tbl_res *)$arg0
	if ($res->tbl_order != -1)
		printf "             res "
		printf "ord %5d  pos %5d\n", $res->tbl_order, $res->tbl_pos
	end
end

# Args umem
define _ionic_umem_print
	set $umem = (struct ib_umem *)$arg0
	printf "            umem "
	printf "pgs %5d  maps %4d  size %8d  iova %p  pid %7d\n",	\
		$umem->npages, $umem->nmap, $umem->length,		\
		(void *)$umem->address, $umem->pid
end

define _ionic_q_at
	set $_q = (struct ionic_queue *)$arg0
	set $_idx = $arg1
	set $arg2 = ((uintptr_t)$_q->ptr +				\
			((unsigned long)$_idx << $_q->stride_log2))
end

define _ionic_q_at_prod
	set $_q = (struct ionic_queue *)$arg0
	_ionic_q_at $_q $_q->prod $arg1
end

define _ionic_q_at_cons
	set $_q = (struct ionic_queue *)$arg0
	_ionic_q_at $_q $_q->cons $arg1
end

#######################################################################
# Devices (ibdevs)
#

# Args: devlist dev
define _to_ionic_dev
	set $_offset = (uintptr_t)&((struct ionic_ibdev *) 0)->driver_ent
	set $arg1 = (struct ionic_ibdev *)((uintptr_t)$arg0 - $_offset)
end

# Args: ibdev dev
define _ib_to_ionic_dev
	set $_offset = (uintptr_t)&((struct ionic_ibdev *) 0)->ibdev
	set $arg1 = (struct ionic_ibdev *)((uintptr_t)$arg0 - $_offset)
end

# Args: dev
define _ionic_ibdev_first
	set $nxt = ionic_ibdev_list->next
	if ($nxt != &ionic_ibdev_list)
		_to_ionic_dev $nxt $arg0
	else
		set $arg0 = 0
	end
end

# Args: dev
define _ionic_ibdev_next
	set $ibd = (struct ionic_ibdev *)$arg0
	set $nxt = $ibd->driver_ent.next
	if ($nxt != &ionic_ibdev_list)
		_to_ionic_dev $nxt $arg0
	else
		set $arg0 = 0
	end
end

# Args: dev
define ionic_ibdev_print
	set $ibd = (struct ionic_ibdev *)$arg0
	printf "%8s         (struct ionic_ibdev *)%p\n",		\
		$ibd->ibdev.name, $ibd
	__indent
	printf "net %s lif %d rev %1d %s\n",				\
		$ibd->ndev->if_xname, $ibd->lif_id,			\
		$ibd->rdma_version, $ibd->ibdev.node_desc
	printf "\n"
end
document ionic_ibdev_print
Print information about the given ibdev.
Usage: ionic_ibdev_print [ibdev]
end

# Args: None
define ionic_ibdev_list
	_ionic_ibdev_first $ibd
	while ($ibd != 0)
		ionic_ibdev_print $ibd
		_ionic_ibdev_next $ibd
	end
end
document ionic_ibdev_list
Print a list of the current IONIC RDMA devices.
Usage: ionic_ibdev_list
end

#######################################################################
# Completion Queues
#

# Args: cqlist cq
define _to_ionic_cq
	set $_offset = (uintptr_t)&((struct ionic_cq *) 0)->cq_list_ent
	set $arg1 = (struct ionic_cq *)((uintptr_t)$arg0 - $_offset)
end

# Args: ibcq cq
define _ib_to_ionic_cq
	set $_offset = (uintptr_t)&((struct ionic_cq *) 0)->ibcq
	set $arg1 = (struct ionic_cq *)((uintptr_t)$arg0 - $_offset)
end

# Args: dev cq
define _ionic_cq_first
	set $ibd = (struct ionic_ibdev *)$arg0
	set $fst = $ibd->cq_list.next
	if ($fst != &$ibd->cq_list)
		_to_ionic_cq $fst $arg1
	else
		set $arg1 = 0
	end
end

# Args: dev cq
define _ionic_cq_next
	set $ibd = (struct ionic_ibdev *)$arg0
	set $cq = (struct ionic_cq *)$arg1
	set $nxt = $cq->cq_list_ent.next
	if ($nxt != &$ibd->cq_list)
		_to_ionic_cq $nxt $arg1
	else
		set $arg1 = 0
	end
end

# Args: cq
define ionic_cq_print
	set $cq = (struct ionic_cq *)$arg0
	_ib_to_ionic_dev $cq->ibcq.device $ibd
	printf "%8s cq%-4d  (struct ionic_cq *)%p\n",			\
		$ibd->ibdev.name, $cq->cqid, $cq
	__indent
	printf "eqid %4d  color %3d  flush %3d\n",			\
		$cq->eqid, $cq->color, $cq->flush
	_ionic_q_print &$cq->q
	printf "\n"
end
document ionic_cq_print
Print information about the given CQ.
Usage: ionic_cq_print [cq]
end

# Args: dev
define ionic_cq_list
	set $ibd = (struct ionic_ibdev *)$arg0
	_ionic_cq_first $ibd $cq
	while ($cq != 0)
		ionic_cq_print $cq
		_ionic_cq_next $ibd $cq
	end
end
document ionic_cq_list
Print a list of the CQs on the given IONIC RDMA device.
Usage: ionic_cq_list [device]
end

#######################################################################
# Queue Pairs
#

# Args: qplist qp
define _to_ionic_qp
	set $_offset = (uintptr_t)&((struct ionic_qp *) 0)->qp_list_ent
	set $arg1 = (struct ionic_qp *)((uintptr_t)$arg0 - $_offset)
end

# Args: ibqp qp
define _ib_to_ionic_qp
	set $_offset = (uintptr_t)&((struct ionic_qp *) 0)->ibqp
	set $arg1 = (struct ionic_qp *)((uintptr_t)$arg0 - $_offset)
end

# Args: dev qp
define _ionic_qp_first
	set $ibd = (struct ionic_ibdev *)$arg0
	set $fst = $ibd->qp_list.next
	if ($fst != &$ibd->qp_list)
		_to_ionic_qp $fst $arg1
	else
		set $arg1 = 0
	end
end

# Args: dev qp
define _ionic_qp_next
	set $ibd = (struct ionic_ibdev *)$arg0
	set $qp = (struct ionic_qp *)$arg1
	set $nxt = $qp->qp_list_ent.next
	if ($nxt != &$ibd->qp_list)
		_to_ionic_qp $nxt $arg1
	else
		set $arg1 = 0
	end
end

# Args: qp
define ionic_qp_print
	set $qp = (struct ionic_qp *)$arg0
	_ib_to_ionic_dev $qp->ibqp.device $ibd
	printf "%8s qp%-4d  (struct ionic_qp *)%p\n",			\
		$ibd->ibdev.name, $qp->qpid, $qp
	__indent
	printf "ahid %4d  dcqcn %3d  ", $qp->ahid, $qp->dcqcn_profile
	output $qp->ibqp.qp_type
	printf "  "
	output $qp->state
	printf "\n"

	printf "              rq "
	_ib_to_ionic_cq $qp->ibqp.recv_cq $cq
	printf "cqid %4d  spec %4d\n", $cq->cqid, $qp->rq_spec
	_ionic_q_print &$qp->rq
	_ionic_res_print &$qp->rq_res
	if ($qp->rq_umem)
		_ionic_umem_print $qp->rq_umem
	end

	printf "              sq "
	_ib_to_ionic_cq $qp->ibqp.recv_cq $cq
	printf "cqid %4d  spec %4d\n", $cq->cqid, $qp->sq_spec
	_ionic_q_print &$qp->sq
	_ionic_res_print &$qp->sq_res
	if ($qp->sq_umem)
		_ionic_umem_print $qp->sq_umem
	end
	printf "\n"
end
document ionic_qp_print
Print information about the given QP.
Usage: ionic_qp_print [qp]
end

# Args: aq
define ionic_aq_print
	set $aq = (struct ionic_aq *)$arg0
	set $ibd = $aq->dev
	printf "%8s aq%-4d  (struct ionic_aq *)%p\n",			\
		$ibd->ibdev.name, $aq->aqid, $aq
	__indent
	printf "cqid %4d  ", $aq->cqid
	output $ibd->admin_state
	printf "\n"
	_ionic_q_print &$aq->q
	printf "\n"
end
document ionic_aq_print
Print information about the given AQ (Admin Queue).
Usage: ionic_aq_print [aq]
end

# Args: dev
define ionic_qp_list
	set $ibd = (struct ionic_ibdev *)$arg0
	_ionic_qp_first $ibd $qp
	while ($qp != 0)
		ionic_qp_print $qp
		_ionic_qp_next $ibd $qp
	end
end
document ionic_qp_list
Print a list of the QPs on the given IONIC RDMA device.
Usage: ionic_qp_list [device]
end

#######################################################################
# Event Queues
#

# Args: eq
define ionic_eq_print
	set $eq = (struct ionic_eq *)$arg0
	set $ibd = $eq->dev
	printf "%8s eq%-4d  (struct ionic_eq *)%p\n",			\
		$ibd->ibdev.name, $eq->eqid, $eq
	__indent
	printf "intr %4d  irq %5d  enabled %1d  armed %3d  %s\n",	\
		$eq->intr, $eq->irq, $eq->enable, $eq->armed, $eq->name
	_ionic_q_print &$eq->q
end
document ionic_eq_print
Print information about the given EQ.
Usage: ionic_eq_print [eq]
end

# Args: dev
define ionic_eq_list
	set $ibd = (struct ionic_ibdev *)$arg0
	set $i = 0
	while ($i < $ibd->eq_count)
		ionic_eq_print $ibd->eq_vec[$i]
		set $i = $i + 1
	end
	printf "\n"
end
document ionic_eq_list
Print a list of the EQs on the given IONIC RDMA device.
Usage: ionic_eq_list [device]
end

#######################################################################
# Memory Regions
#

# Args: mrid mr
define _to_ionic_mr
	set $_offset = (uintptr_t)&((struct ionic_mr *) 0)->mrid
	set $arg1 = (struct ionic_mr *)((uintptr_t)$arg0 - $_offset)
end

# Args: mr
define ionic_mr_print
	set $mr = (struct ionic_mr *)$arg0
	set $mridx = $mr->debug->oid_name
	_ib_to_ionic_dev $mr->ibmr.device $ibd

	printf "%8s mr%-4s  (struct ionic_mr *)%p\n",			\
		$ibd->ibdev.name, $mridx, $mr
	__indent
	printf "mrid %4d  flags 0x%x  ", $mr->mrid, $mr->flags
	set $is_mw = 0
	set $is_mr = 0
	if ($mr->flags & (1 << 13))
		if ($mr->flags & (1 << 14))
			set $is_mw = 1
			if ($mr->flags & (1 << 15))
				printf "MW2 "
			else
				printf "MW1 "
			end
		else
			if ($mr->flags & (1 << 15))
				set $is_mr = 1
				printf "PHYS "
			else
				printf "???? "
			end
		end
	else
		if (($mr->flags & (1 << 14)) || ($mr->flags & (1 << 15)))
			printf "???? "
		else
			set $is_mr = 1
			printf "USER "
		end
	end
	if ($mr->flags & (1 << 0))
		printf "LW "
	end
	if ($mr->flags & (1 << 1))
		printf "RW "
	end
	if ($mr->flags & (1 << 2))
		printf "RR "
	end
	if ($mr->flags & (1 << 3))
		printf "RA "
	end
	if ($mr->flags & (1 << 4))
		printf "MW "
	end
	if ($mr->flags & (1 << 5))
		printf "ZB "
	end
	if ($mr->flags & (1 << 6))
		printf "OD "
	end
	printf "\n"
	if ($is_mr)
		__indent
		printf "lkey %4d  rkey %4d  size %8d  iova %p\n",	\
			$mr->ibmr.lkey, $mr->ibmr.rkey,			\
			$mr->ibmr.length, (void *)$mr->ibmr.iova
		__indent
		printf "pd (struct ib_pd *)%p\n", $mr->ibmr.pd
	end
	if ($is_mw)
		__indent
		printf "pd (struct ib_pd *)%p\n", $mr->ibmw.pd
		__indent
		printf "pd (struct ib_pd *)%p\n", $mr->ibmr.pd
	end
	_ionic_res_print &$mr->res
	if ($mr->umem)
		_ionic_umem_print $mr->umem
	end
	printf "\n"
end
document ionic_mr_print
Print information about the given MR.
Usage: ionic_mr_print [mr]
end

define _ionic_mr_child_list
	set $mr_oid = $arg1
	while ($mr_oid)
		set $child_oid = $mr_oid->oid_children.slh_first
		while ($child_oid)
			if ($child_oid->oid_name[0] == 'm' &&		\
			    $child_oid->oid_name[1] == 'r' &&		\
			    $child_oid->oid_name[2] == 'i' &&		\
			    $child_oid->oid_name[3] == 'd')
				_to_ionic_mr $child_oid->oid_arg1 $mr
				ionic_mr_print $mr
			end
			set $child_oid = $child_oid->oid_link.sle_next
		end

		set $mr_oid = $mr_oid->oid_link.sle_next
	end
end

define ionic_mr_list
	set $ibd = (struct ionic_ibdev *)$arg0
	set $debug_mr = $ibd->debug_mr
	if ($debug_mr)
		_ionic_mr_child_list $ibd $debug_mr->oid_children.slh_first
	end
end
document ionic_mr_list
Print a list of the MRs on the given IONIC RDMA device.
Usage: ionic_mr_list [device]
end

#######################################################################
# Master print command
#

define ionic_ibdev_list_all
	_ionic_ibdev_first $ibd
	while ($ibd != 0)
		ionic_ibdev_print $ibd
		ionic_aq_print $ibd->adminq
		ionic_mr_list $ibd
		ionic_qp_list $ibd
		ionic_cq_list $ibd
		ionic_eq_list $ibd
		_ionic_ibdev_next $ibd
	end
end
document ionic_ibdev_list_all
Print a list of the current IONIC RDMA devices, MRs, QPs, CQs, and EQs.
Usage: ionic_ibdev_list_all
end

#######################################################################
# Completion Queue Entries
#

define _ionic_cqe_type
	__be32tole $arg0->qid_type_flags $__le32
	set $arg1 = ($__le32 >> 5) & 0x7
end

define _ionic_cqe_flags
	__be32tole $arg0->qid_type_flags $__le32
	set $arg1 = $__le32 & 0x3
end

define _ionic_cqe_print_flags
	_ionic_cqe_flags $arg0 $_flags
	printf "clr %1d  err %1d  ", $_flags & 0x1, $_flags & 0x2
end

define _ionic_cqe_print_admin
	set $_cqe = $arg0
	set $_cqe_aq = (struct ionic_aq *)$arg1
	__be16tole $_cqe->admin.cmd_idx $_cmd_idx

	printf "ADMIN     "
	_ionic_cqe_print_flags $_cqe
	printf "cmd_idx %5d  ", $_cmd_idx
	output (enum ionic_v1_admin_op)$_cqe->admin.cmd_op
	printf "\n"
end

define _ionic_cqe_print_recv
	set $_cqe = $arg0
	set $_cqe_qp = $arg1
	__be32tole $_cqe->recv.src_qpn_op $_op
	__be16tole $_cqe->recv.vlan_tag $_vlan
	__be32tole $_cqe->recv.imm_data_rkey $_rkey

	set $_meta = &$_cqe_qp->rq_meta[$_cqe->recv.wqe_id]

	printf "RECV      "
	_ionic_cqe_print_flags $_cqe
	printf "wqe_id %4d  wrid 0x%016x  sqpn %d  ",			\
		$_cqe->recv.wqe_id, $_meta->wrid, $_op & 0x1f
	printf "mac %02x:%02x:%02x:%02x:%02x:%02x",			\
		$_cqe->recv.src_mac[0],					\
		$_cqe->recv.src_mac[1],					\
		$_cqe->recv.src_mac[2],					\
		$_cqe->recv.src_mac[3],					\
		$_cqe->recv.src_mac[4],					\
		$_cqe->recv.src_mac[5]
	if ($_vlan != 0)
		printf "  vlan %d", $_vlan
	end
	if ($_rkey != 0)
		printf "  imm_rkey %x", $_rkey
	end
	printf "\n"
end

define _ionic_cqe_print_send_msn
	set $_cqe = $arg0
	set $_cqe_qp = $arg1
	__be32tole $_cqe->send.msg_msn $_cqe_msn

	set $_cqe_seq = $_cqe_msn & $_cqe_qp->sq.mask

	_ionic_cqe_flags $_cqe $_flags
	if (($_flags & 2) == 0)
		# Success case: subtract one more
		set $_cqe_seq = ($_cqe_seq - 1) & $_cqe_qp->sq.mask
	end
	set $_cqe_idx = $_cqe_qp->sq_msn_idx[$_cqe_seq]
	set $_meta = &$_cqe_qp->sq_meta[$_cqe_idx]

	printf "SEND_MSN  "
	_ionic_cqe_print_flags $_cqe
	printf "msn %7d  wrid 0x%016x\n", $_cqe_msn, $_meta->wrid
end

define _ionic_cqe_print_send_npg
	set $_cqe = $arg0
	set $_cqe_qp = $arg1

	set $_cqe_idx = $_cqe->send.npg_wqe_id & $_cqe_qp->sq.mask
	set $_meta = &$_cqe_qp->sq_meta[$_cqe_idx]

	printf "SEND_NPG  "
	_ionic_cqe_print_flags $_cqe
	printf "wqe_id %4d  wrid 0x%016x\n",				\
		$_cqe->send.npg_wqe_id, $_meta->wrid
end

define ionic_cqe_print
	set $_cqe = (struct ionic_v1_cqe *)$arg0
	set $_cqe_qp = (struct ionic_qp *)$arg1

	_ionic_cqe_type $_cqe $_type
	if ($_type == 0)
		_ionic_cqe_print_admin $_cqe $_cqe_qp
	end
	if ($_type == 1)
		_ionic_cqe_print_recv $_cqe $_cqe_qp
	end
	if ($_type == 2)
		_ionic_cqe_print_send_msn $_cqe $_cqe_qp
	end
	if ($_type == 3)
		_ionic_cqe_print_send_npg $_cqe $_cqe_qp
	end
end
document ionic_cqe_print
Print information about the given CQE.
Usage: ionic_cqe_print [cqe] [qp]
end

define _ionic_cq_dump_kernel
	set $_cq = $arg0
	set $_max = $_cq->q.mask + 1

	_ib_to_ionic_dev $_cq->ibcq.device $ibd
	printf "%8s cq%-4d  (struct ionic_cq *)%p\n",			\
		$ibd->ibdev.name, $_cq->cqid, $_cq
	_ionic_q_print &$_cq->q

	_ib_to_ionic_dev $_cq->ibcq.device $_cq_ibd
	if ($_cq_ibd->admincq == $_cq)
		# This is the admin CQ; use the admin QP
		set $_cq_qp = $_cq_ibd->admincq
		set $_cq_aq = (struct ionic_aq *)$_cq_qp
		printf "  link:  aq%-4d  (struct ionic_aq *)%p\n",	\
			$_cq_aq->aqid, $_cq_aq
	else
		# Search for the matching QP
		set $_cq_qp = 0

		_ionic_qp_first $_cq_ibd $_cq_qp_tmp
		while ($_cq_qp_tmp != 0)
			set $_cq_qp_tmp_scq = $_cq_qp_tmp->ibqp.send_cq
			set $_cq_qp_tmp_rcq = $_cq_qp_tmp->ibqp.recv_cq
			if ($_cq_qp_tmp_scq == &$_cq->ibcq ||		\
			    $_cq_qp_tmp_rcq == &$_cq->ibcq)
				set $_cq_qp = $_cq_qp_tmp
			end
			_ionic_qp_next $_cq_ibd $_cq_qp_tmp
		end
		printf "  link:  qp%-4d  (struct ionic_qp *)%p\n",	\
			$_cq_qp->qpid, $_cq_qp
	end

	printf "    CQ:\n"

	set $_i = 0
	set $_skip = 0
	while ($_i < $_max)
		_ionic_q_at &$_cq->q $_i $_e
		set $_cqe = (struct ionic_v1_cqe *)$_e

		_ionic_cqe_type $_cqe $_type
		if ($_cqe->admin.cmd_op == 0 && $_type == 0)
			if ($_skip == 0)
				printf "   ...\n"
				set $_skip = 1
			end
		else
			set $_skip = 0
			printf "  %4d ", $_i
			ionic_cqe_print $_cqe $_cq_qp
		end
		set $_i = $_i + 1
	end
end

define ionic_cq_dump
	set $_cq = (struct ionic_cq *)$arg0
	if ($_cq->q.ptr)
		_ionic_cq_dump_kernel $_cq
	else
		printf "user-space cq, no cqe in kernel\n"
	end
end
document ionic_cq_dump
Print information about each CQE in the given CQ.
Usage: ionic_cq_dump [cq]
end

#######################################################################
# Work Queue Entries
#

# Args: idx sge
define _ionic_wqe_print_sge
	set $_sge = (struct ionic_sge *)$arg1
	__be64tole $_sge->va $_va
	__be32tole $_sge->len $_len
	__be32tole $_sge->lkey $_lkey
	__indent
	printf "sge %3d  0x%016x  len %5d  lkey 0x%08x\n",		\
		$arg0, $_va, $_len, $_lkey
end

define _ionic_wqe_print_flags
	__be16tole $arg0 $_flags

	printf "flags 0x%04x ", $_flags
	if ($_flags & 0x1)
		printf "FENCE "
	end
	if ($_flags & 0x2)
		printf "SOL "
	end
	if ($_flags & 0x4)
		printf "INL "
	end
	if ($_flags & 0x8)
		printf "SIG "
	end
	if ($_flags & (1 << 12))
		printf "SPEC32 "
	end
	if ($_flags & (1 << 13))
		printf "SPEC16 "
	end
end

define ionic_rqe_print
	set $_qp = (struct ionic_qp *)$arg0
	set $_wqe = (struct ionic_v1_wqe *)$arg1
	__be32tole $_wqe->base.imm_data_key $_len

	set $_meta = $_qp->rq_meta[$_wqe->base.wqe_id]

	printf "wrid 0x%016x  len %5d  ",				\
		$_meta->wrid, $_len
	_ionic_wqe_print_flags $_wqe->base.flags
	printf "\n"

	__indent
	printf "RECV      "
	if ($_meta->next == 1)
		printf "LAST"
	end
	if ($_meta->next == 2)
		printf "POSTED"
	end
	printf "\n"

	set $_j = 0
	while ($_j < $_wqe->base.num_sge_key)
		_ionic_wqe_print_sge $_j &$_wqe->recv.pld.sgl[$_j]
		set $_j = $_j + 1
	end
end
document ionic_rqe_print
Print information about the given receive WQE.
Usage: ionic_rqe_print [qp] [rqe]
end

define _ionic_wqe_print_send
	set $_wqe = (struct ionic_v1_wqe *)$arg0
	__be32tole $_wqe->common.send.ah_id $_ahid
	__be32tole $_wqe->common.send.dest_qpn $_dest_qpn
	__be32tole $_wqe->common.send.dest_qkey $_dest_qkey
	__be32tole $_wqe->common.length $_len

	printf "len %5d  ahid %4d  dqpn %4d  qkey 0x%08x",		\
		$_len, $_ahid, $_dest_qpn, ($_dest_qkey & ~0x80000000)
	if ($_dest_qkey & 0x80000000)
		printf "/PRIV\n"
	else
		printf "\n"
	end

	set $_j = 0
	while ($_j < $_wqe->base.num_sge_key)
		_ionic_wqe_print_sge $_j &$_wqe->common.pld.sgl[$_j]
		set $_j = $_j + 1
	end
end

define _ionic_wqe_print_rdma
	set $_wqe = (struct ionic_v1_wqe *)$arg0
	__be32tole $_wqe->common.rdma.remote_va_high $_rva_low
	__be32tole $_wqe->common.rdma.remote_va_low $_rva_high
	__be32tole $_wqe->common.rdma.remote_rkey $_rkey
	__be32tole $_wqe->common.length $_len

	printf "len %5d  rkey 0x%08x  rva 0x%08x%08x\n",		\
		$_len, $_rkey, $_rva_high, $_rva_low,

	set $_j = 0
	while ($_j < $_wqe->base.num_sge_key)
		_ionic_wqe_print_sge $_j &$_wqe->common.pld.sgl[$_j]
		set $_j = $_j + 1
	end
end

define _ionic_wqe_print_atomic
	printf "TBD\n"
end

define _ionic_wqe_print_reg_mr
	set $_wqe = (struct ionic_v1_wqe *)$arg0
	__be64tole $_wqe->reg_mr.va $_va
	__be64tole $_wqe->reg_mr.length $_len
	__be64tole $_wqe->reg_mr.offset $_off
	set $_dma = $_wqe->reg_mr.dma_addr
	__be32tole $_wqe->reg_mr.map_count $_map
	__be16tole $_wqe->reg_mr.flags $_flags

	printf "len %5d  offs %4d  mapc %4d  flags 0x%04x  ",		\
		$_len, $_off, $_map, $_flags
	printf "va 0x%016x  dma 0x%016x\n", $_va, $_dma

	set $_j = 0
	while ($_j < $_wqe->base.num_sge_key)
		_ionic_wqe_print_sge $_j &$_wqe->common.pld.sgl[$_j]
		set $_j = $_j + 1
	end
end

define _ionic_wqe_print_bind_mw
	printf "TBD\n"
end

define ionic_sqe_print
	set $_qp = (struct ionic_qp *)$arg0
	set $_wqe = (struct ionic_v1_wqe *)$arg1
	set $_op = $_wqe->base.op
	__be32tole $_wqe->base.imm_data_key $_imm_key

	set $_meta = $_qp->sq_meta[$_wqe->base.wqe_id]

	printf "wrid 0x%016x  len %5d  seq %5d  ",			\
		$_meta->wrid, $_meta->len, $_meta->seq
	printf "op %6d  sts %5d  imkey %3d  ",				\
		$_meta->ibop, $_meta->ibsts, $_imm_key
	_ionic_wqe_print_flags $_wqe->base.flags
	printf "\n"

	__indent
	if ($_op == 0)
		printf "SEND      "
		_ionic_wqe_print_send $_wqe
	end
	if ($_op == 1)
		printf "SEND_INV  "
		_ionic_wqe_print_send $_wqe
	end
	if ($_op == 2)
		printf "SEND_IMM  "
		_ionic_wqe_print_send $_wqe
	end
	if ($_op == 3)
		printf "READ      "
		_ionic_wqe_print_rdma $_wqe
	end
	if ($_op == 4)
		printf "WRITE     "
		_ionic_wqe_print_rdma $_wqe
	end
	if ($_op == 5)
		printf "WR_IMM    "
		_ionic_wqe_print_rdma $_wqe
	end
	if ($_op == 6)
		printf "ATOM_CS   "
		_ionic_wqe_print_atomic $_wqe
	end
	if ($_op == 7)
		printf "ATOM_FA   "
		_ionic_wqe_print_atomic $_wqe
	end
	if ($_op == 8)
		printf "REG_MR    "
		_ionic_wqe_print_reg_mr $_wqe
	end
	if ($_op == 9)
		printf "LOCAL_INV\n"
	end
	if ($_op == 10)
		printf "BIND_MW   "
		_ionic_wqe_print_bind_mw $_wqe
	end
	if ($_op >= 11)
		printf "UNKNOWN\n"
	end
end
document ionic_sqe_print
Print information about the given send WQE.
TODO: ATOMIC, BIND_MW
Usage: ionic_sqe_print [qp] [sqe]
end

define _ionic_rq_dump_kernel
	set $_qp = $arg0
	set $_max = $_qp->rq.mask + 1

	_ib_to_ionic_dev $_qp->ibqp.device $ibd
	_ib_to_ionic_cq $_qp->ibqp.recv_cq $_cq
	printf "%8s qp%-4d  (struct ionic_qp *)%p\n",			\
		$ibd->ibdev.name, $_qp->qpid, $_qp
	_ionic_q_print &$_qp->rq
	printf "  link:  cq%-4d  (struct ionic_cq *)%p\n",		\
		$_cq->cqid, $_cq
	printf "    RQ:\n"

	set $_i = 0
	set $_skip = 0
	while ($_i < $_max)
		_ionic_q_at &$_qp->rq $_i $_e
		set $_wqe = (struct ionic_v1_wqe *)$_e

		if ($_wqe->base.num_sge_key == 0)
			if ($_skip == 0)
				printf "   ...\n"
				set $_skip = 1
			end
		else
			set $_skip = 0
			printf "  %4d ", $_i
			ionic_rqe_print $_qp $_wqe
		end
		set $_i = $_i + 1
	end
end

define _ionic_sq_dump_kernel
	set $_qp = $arg0
	set $_max = $_qp->sq.mask + 1

	_ib_to_ionic_dev $_qp->ibqp.device $ibd
	_ib_to_ionic_cq $_qp->ibqp.send_cq $_cq
	printf "%8s qp%-4d  (struct ionic_qp *)%p\n",			\
		$ibd->ibdev.name, $_qp->qpid, $_qp
	_ionic_q_print &$_qp->sq
	printf "  link:  cq%-4d  (struct ionic_cq *)%p\n",		\
		$_cq->cqid, $_cq
	printf "    SQ:\n"

	set $_i = 0
	set $_skip = 0
	while ($_i < $_max)
		_ionic_q_at &$_qp->sq $_i $_e
		set $_wqe = (struct ionic_v1_wqe *)$_e

		if ($_wqe->base.num_sge_key == 0)
			if ($_skip == 0)
				printf "   ...\n"
				set $_skip = 1
			end
		else
			set $_skip = 0
			printf "  %4d ", $_i
			ionic_sqe_print $_qp $_wqe
		end
		set $_i = $_i + 1
	end
end

define ionic_rq_dump
	set $_qp = (struct ionic_qp *)$arg0
	if ($_qp->rq.ptr)
		_ionic_rq_dump_kernel $_qp
	else
		printf "user-space qp, no rq wqe in kernel\n"
	end
end
document ionic_rq_dump
Print information about each WQE in the given QP's RQ.
Usage: ionic_rq_dump [qp]
end

define ionic_sq_dump
	set $_qp = (struct ionic_qp *)$arg0
	if ($_qp->sq.ptr)
		_ionic_sq_dump_kernel $_qp
	else
		printf "user-space qp, no sq wqe in kernel\n"
	end
end
document ionic_sq_dump
Print information about each WQE in the given QP's SQ.
Usage: ionic_sq_dump [qp]
end

define ionic_qp_dump
	set $_qp = (struct ionic_qp *)$arg0
	ionic_rq_dump $_qp
	ionic_sq_dump $_qp
end
document ionic_qp_dump
Print information about each WQE in the given QP's RQ and SQ.
Usage: ionic_qp_dump [qp]
end
