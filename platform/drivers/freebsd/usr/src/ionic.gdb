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

define _ionic_qe_posted
	set $_q = (struct ionic_queue *)$arg0
	set $_prod = $_q->prod
	set $_cons = $_q->cons
	set $_idx = $arg1

	# producer is farther ahead of consumer than index is from consumer
	set $arg2 = (($_idx - $_cons) & $_q->mask) <			\
				(($_prod - $_cons) & $_q->mask)
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
	printf "pgs %5d  size %8d  iova %p  pid %7d\n",			\
		$umem->npages, $umem->length,				\
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
		$ibd->ndev->if_xname, $ibd->lif_index,			\
		$ibd->rdma_version, $ibd->ibdev.node_desc
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
		printf "\n"
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
	printf "cqid %4d  spec %4d  seq %5d\n",				\
		$cq->cqid, $qp->sq_spec, $qp->sq_msn_cons
	_ionic_q_print &$qp->sq
	_ionic_res_print &$qp->sq_res
	if ($qp->sq_umem)
		_ionic_umem_print $qp->sq_umem
	end
end
document ionic_qp_print
Print information about the given QP.
Usage: ionic_qp_print [qp]
end

# Args: dev
define ionic_aq_list
	set $ibd = (struct ionic_ibdev *)$arg0
	set $i = 0
	while ($i < $ibd->aq_count)
		ionic_aq_print $ibd->aq_vec[$i]
		set $i = $i + 1
	end
end
document ionic_aq_list
Print a list of the AQs (Admin Queues) on the given IONIC RDMA device.
Usage: ionic_aq_list [device]
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

# Args: flags
define ionic_mr_print_flags
	set $_mr_flags = $arg0
	if ($_mr_flags & (1 << 0))
		printf "LW "
	end
	if ($_mr_flags & (1 << 1))
		printf "RW "
	end
	if ($_mr_flags & (1 << 2))
		printf "RR "
	end
	if ($_mr_flags & (1 << 3))
		printf "RA "
	end
	if ($_mr_flags & (1 << 4))
		printf "MW "
	end
	if ($_mr_flags & (1 << 5))
		printf "ZB "
	end
	if ($_mr_flags & (1 << 6))
		printf "OD "
	end
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
	ionic_mr_print_flags $mr->flags
	printf "\n"
	if ($is_mr)
		__indent
		printf "lkey 0x%08x  rkey 0x%08x\n",			\
			$mr->ibmr.lkey, $mr->ibmr.rkey
		__indent
		printf "size   %8d  iova %p\n",				\
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
		ionic_aq_list $ibd
		ionic_mr_list $ibd
		ionic_qp_list $ibd
		ionic_cq_list $ibd
		_ionic_ibdev_next $ibd
	end
end
document ionic_ibdev_list_all
Print a list of the current IONIC RDMA devices, MRs, QPs, and CQs.
Use ionic_eq_list to see the EQs.
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

define _ionic_cqe_posted
	set $_q = (struct ionic_queue *)$arg0
	set $_cqe = (struct ionic_v1_cqe *)$arg1
	set $_idx = $_cqe - (struct ionic_v1_cqe *)$_q->ptr

	_ionic_qe_posted $_q $_idx $arg2
end

define _ionic_cqe_print_admin
	set $_cqe = $arg0
	__be16tole $_cqe->admin.cmd_idx $_cmd_idx

	__indent
	_ionic_cqe_print_flags $_cqe
	printf "  cmd_idx %5d  ", $_cmd_idx
	output (enum ionic_v1_admin_op)$_cqe->admin.cmd_op
	printf "\n"
end

define _ionic_cqe_print_recv
	set $_cqe = $arg0
	set $_cqe_qp = $arg1
	set $_is_posted = $arg2
	_ionic_cqe_flags $_cqe $_flags
	__be32tole $_cqe->status_length $_st_len
	__be32tole $_cqe->recv.src_qpn_op $_src_qpn_op
	__be16tole $_cqe->recv.vlan_tag $_vlan

	set $_op = ($_src_qpn_op >> 24) & 0x1f
	set $_src_qpn = $_src_qpn_op & 0xffffff

	__indent
	output (enum ionic_v1_cqe_src_qpn_bits)$_op
	printf "  "
	_ionic_cqe_print_flags $_cqe

	if ($_flags & 0x2)
		# Error
		output (enum ionic_status)$_st_len
	else
		printf "\n"
		__indent
		printf "wqe_id %4d  sqpn %4d  len %5d  vlan %4d",	\
			$_cqe->recv.wqe_id, $_src_qpn, $_st_len, $_vlan
		if ($_op == IONIC_V1_CQE_RECV_OP_SEND_INV)
			__be32tole $_cqe->recv.imm_data_rkey $_inv_rkey
			printf "  rkey 0x%08x", $_inv_rkey
		end
		if ($_op == IONIC_V1_CQE_RECV_OP_SEND_IMM ||		\
		    $_op == IONIC_V1_CQE_RECV_OP_RDMA_IMM)
			printf "  imm 0x08x", $_cqe->recv.imm_data_rkey
		end

		set $_qp_type = $_cqe_qp->ibqp.qp_type
		if ($_qp_type == IB_QPT_UD || $_qp_type == IB_QPT_GSI)
			# UD or GSI
			printf "\n"
			__indent
			printf "src_mac %02x:%02x:%02x:%02x:%02x:%02x",	\
				$_cqe->recv.src_mac[0],			\
				$_cqe->recv.src_mac[1],			\
				$_cqe->recv.src_mac[2],			\
				$_cqe->recv.src_mac[3],			\
				$_cqe->recv.src_mac[4],			\
				$_cqe->recv.src_mac[5]
		end
	end
	printf "\n"

	if ($_is_posted == 1)
		set $_meta = &$_cqe_qp->rq_meta[$_cqe->recv.wqe_id]

		__indent
		printf "wrid 0x%016x\n", $_meta->wrid
	end
end

define _ionic_cqe_print_send_msn
	set $_cqe = $arg0
	set $_cqe_qp = $arg1
	set $_is_posted = $arg2
	__be32tole $_cqe->send.msg_msn $_cqe_msn
	set $_cqe_seq = $_cqe_msn & $_cqe_qp->sq.mask

	__indent
	printf "SEND_MSN  seq %7d  ", $_cqe_seq
	_ionic_cqe_print_flags $_cqe
	printf "\n"

	if ($_is_posted == 1)
		_ionic_cqe_flags $_cqe $_flags
		if (($_flags & 2) == 0)
			# Success case: subtract one more
			set $_cqe_seq = ($_cqe_seq - 1) & $_cqe_qp->sq.mask
		end

		set $_cqe_idx = $_cqe_qp->sq_msn_idx[$_cqe_seq]
		set $_meta = &$_cqe_qp->sq_meta[$_cqe_idx]

		__indent
		printf "wrid 0x%016x\n", $_meta->wrid
	end
end

define _ionic_cqe_print_send_npg
	set $_cqe = $arg0
	set $_cqe_qp = $arg1
	set $_is_posted = $arg2

	set $_cqe_idx = $_cqe->send.npg_wqe_id & $_cqe_qp->sq.mask

	__indent
	printf "SEND_NPG  wqe_id %4d  ", $_cqe->send.npg_wqe_id
	_ionic_cqe_print_flags $_cqe
	printf "\n"

	if ($_is_posted == 1)
		set $_meta = &$_cqe_qp->sq_meta[$_cqe_idx]

		__indent
		printf "wrid 0x%016x\n", $_meta->wrid
	end
end

define ionic_cqe_print
	set $_cqe = (struct ionic_v1_cqe *)$arg0
	set $_cqe_qp = (struct ionic_qp *)$arg1
	set $_is_posted = $arg2

	_ionic_cqe_type $_cqe $_type
	if ($_type == IONIC_V1_CQE_TYPE_ADMIN)
		_ionic_cqe_print_admin $_cqe
	end
	if ($_type == IONIC_V1_CQE_TYPE_RECV)
		_ionic_cqe_print_recv $_cqe $_cqe_qp $_is_posted
	end
	if ($_type == IONIC_V1_CQE_TYPE_SEND_MSN)
		_ionic_cqe_print_send_msn $_cqe $_cqe_qp $_is_posted
	end
	if ($_type == IONIC_V1_CQE_TYPE_SEND_NPG)
		_ionic_cqe_print_send_npg $_cqe $_cqe_qp $_is_posted
	end
end
document ionic_cqe_print
Print information about the given CQE.
The is_posted flag controls whether queue entry metadata is decoded.
Usage: ionic_cqe_print [cqe] [qp] [is_posted]
end

define _ionic_cq_dump_kernel
	set $_cq = $arg0
	set $_max = $_cq->q.mask + 1
	set $_cq_qp = 0
	set $_i = 0
	set $_aq = 0

	_ib_to_ionic_dev $_cq->ibcq.device $_cq_ibd

	while ($_i < $_cq_ibd->aq_count)
		if ($_cq_ibd->aq_vec[$_i].cq == $_cq)
			set $_aq = $_cq_ibd->aq_vec[$_i]
			break
		end
		set $_i = $_i + 1
	end
	if ($_aq)
		# This is an admin CQ
		printf "  link:  aq%-4d  (struct ionic_aq *)%p\n",	\
			$_aq->aqid, $_aq
	else
		# Search for the matching QP
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

			_ionic_cqe_posted &$_cq->q $_cqe $_is_posted
			printf "  %4d  %p", $_i, $_cqe
			if ($_is_posted)
				printf "  [posted]"
			end
			printf "\n"

			ionic_cqe_print $_cqe $_cq_qp $_is_posted
		end
		set $_i = $_i + 1
	end
end

define ionic_cq_dump
	set $_cq = (struct ionic_cq *)$arg0

	ionic_cq_print $_cq
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
	printf "sge %5d  0x%016x  len %5d  lkey 0x%08x\n",		\
		$arg0, $_va, $_len, $_lkey
end

define _ionic_wqe_posted
	set $_q = (struct ionic_queue *)$arg0
	set $_wqe = (struct ionic_v1_wqe *)$arg1
	set $_idx = $_wqe - (struct ionic_v1_wqe *)$_q->ptr

	_ionic_qe_posted $_q $_idx $arg2
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
	set $_is_posted = $arg2
	__be32tole $_wqe->base.imm_data_key $_len

	__indent
	printf "RECV      len %5d  ", $_len
	_ionic_wqe_print_flags $_wqe->base.flags
	printf "\n"

	if ($_is_posted == 1)
		set $_meta = $_qp->rq_meta[$_wqe->base.wqe_id]

		__indent
		printf "wrid 0x%016x  ", $_meta->wrid
		if ($_meta->next == 1)
			printf "LAST"
		end
		if ($_meta->next == 2)
			printf "POSTED"
		end
		printf "\n"
	end

	set $_j = 0
	while ($_j < $_wqe->base.num_sge_key)
		_ionic_wqe_print_sge $_j &$_wqe->recv.pld.sgl[$_j]
		set $_j = $_j + 1
	end
end
document ionic_rqe_print
Print information about the given receive WQE.
The is_posted flag controls whether queue entry metadata is decoded.
Usage: ionic_rqe_print [qp] [rqe] [is_posted]
end

define _ionic_wqe_print_send
	set $_wqe = (struct ionic_v1_wqe *)$arg0
	set $_qp = (struct ionic_qp *)$arg1
	__be32tole $_wqe->common.send.ah_id $_ahid
	__be32tole $_wqe->common.send.dest_qpn $_dest_qpn
	__be32tole $_wqe->common.send.dest_qkey $_dest_qkey
	__be32tole $_wqe->common.length $_len

	printf "len %5d  ahid %4d  ", $_len, $_ahid

	set $_qp_type = $_qp->ibqp.qp_type
	if ($_qp_type == IB_QPT_UD || $_qp_type == IB_QPT_GSI)
		printf "dqpn %4d  qkey 0x%08x", $_dest_qpn, $_dest_qkey
		if ($_dest_qkey & 0x80000000)
			printf "/PRIV"
		end
	end
	printf "\n"

	if ($_wqe->base.op == IONIC_V1_OP_SEND_IMM)
		__indent
		printf "imm 0x%08x\n", $_wqe->base.imm_data_key
	end
	if ($_wqe->base.op == IONIC_V1_OP_SEND_INV)
		__indent
		__be32tole $_wqe->base.imm_data_key $_inv_rkey
		printf "rkey 0x%08x\n", $_inv_rkey
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
	__be32tole $_wqe->base.imm_data_key $_imm_key

	printf "len %5d  rkey 0x%08x  rva 0x%08x%08x\n",		\
		$_len, $_rkey, $_rva_high, $_rva_low,

	if ($_wqe->base.op == IONIC_V1_OP_RDMA_WRITE_IMM)
		__indent
		printf "imm 0x%08x\n", $_wqe->base.imm_data_key
	end

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
	__be32tole $_wqe->reg_mr.map_count $_pages
	__be16tole $_wqe->reg_mr.flags $_flags
	set $_key = $_wqe->base.num_sge_key
	__be32tole $_wqe->base.imm_data_key $_lkey

	printf "len %5d  offs %4d  pgs %5d\n", $_len, $_off, $_pages

	__indent
	printf "key 0x%08x  lkey 0x%08x  flags 0x%04x ",		\
		$_key, $_lkey, $_flags
	ionic_mr_print_flags $_flags
	printf "\n"

	__indent
	printf "va 0x%016x  dma 0x%016x\n", $_va, $_dma
end

define _ionic_wqe_print_local_inv
	set $_wqe = (struct ionic_v1_wqe *)$arg0
	__be32tole $_wqe->base.imm_data_key $_inv_rkey

	printf "rkey 0x%08x\n", $_inv_rkey
end

define _ionic_wqe_print_bind_mw
	# Not possible to get here - bind_mw is a userspace op,
	# but userspace sqe are not present in the kernel core
	printf "\n"
end

define ionic_sqe_print
	set $_qp = (struct ionic_qp *)$arg0
	set $_wqe = (struct ionic_v1_wqe *)$arg1
	set $_is_posted = $arg2
	set $_op = $_wqe->base.op

	if ($_is_posted == 1)
		set $_meta = $_qp->sq_meta[$_wqe->base.wqe_id]

		__indent
		printf "wrid 0x%016x  len %5d  ", $_meta->wrid, $_meta->len
		_ionic_wqe_print_flags $_wqe->base.flags
		printf "\n"
		__indent
		output (enum ib_wc_opcode)$_meta->ibop
		printf "  "
		output (enum ib_wc_status)$_meta->ibsts
		printf "  seq %5d\n", $_meta->seq
	else
		__indent
		output (enum ionic_v1_op)$_op
		printf "\n"
	end

	__indent
	if ($_op == IONIC_V1_OP_SEND     ||				\
	    $_op == IONIC_V1_OP_SEND_INV ||				\
	    $_op == IONIC_V1_OP_SEND_IMM)
		_ionic_wqe_print_send $_wqe $_qp
	end
	if ($_op == IONIC_V1_OP_RDMA_READ  ||				\
	    $_op == IONIC_V1_OP_RDMA_WRITE ||				\
	    $_op == IONIC_V1_OP_RDMA_WRITE_IMM)
		_ionic_wqe_print_rdma $_wqe
	end
	if ($_op == IONIC_V1_OP_ATOMIC_CS ||				\
	    $_op == IONIC_V1_OP_ATOMIC_FA)
		_ionic_wqe_print_atomic $_wqe
	end
	if ($_op == IONIC_V1_OP_REG_MR)
		_ionic_wqe_print_reg_mr $_wqe
	end
	if ($_op == IONIC_V1_OP_LOCAL_INV)
		_ionic_wqe_print_local_inv $_wqe
	end
	if ($_op == IONIC_V1_OP_BIND_MW)
		_ionic_wqe_print_bind_mw $_wqe
	end
end
document ionic_sqe_print
Print information about the given send WQE.
The is_posted flag controls whether queue entry metadata is decoded.
TODO: ATOMIC
Usage: ionic_sqe_print [qp] [sqe] [is_posted]
end

define _ionic_rq_dump_kernel
	set $_qp = $arg0
	set $_max = $_qp->rq.mask + 1

	_ib_to_ionic_cq $_qp->ibqp.recv_cq $_cq
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

			_ionic_wqe_posted &$_qp->rq $_wqe $_is_posted
			printf "  %4d  %p", $_i, $_wqe
			if ($_is_posted)
				printf "  [posted]"
			end
			printf "\n"

			ionic_rqe_print $_qp $_wqe $_is_posted
		end
		set $_i = $_i + 1
	end
end

define _ionic_sq_dump_kernel
	set $_qp = $arg0
	set $_max = $_qp->sq.mask + 1

	_ib_to_ionic_cq $_qp->ibqp.send_cq $_cq
	printf "  link:  cq%-4d  (struct ionic_cq *)%p\n",		\
		$_cq->cqid, $_cq
	printf "    SQ:\n"

	set $_i = 0
	set $_skip = 0
	while ($_i < $_max)
		_ionic_q_at &$_qp->sq $_i $_e
		set $_wqe = (struct ionic_v1_wqe *)$_e

		if ($_wqe->base.op == 0 && $_wqe->base.num_sge_key == 0)
			if ($_skip == 0)
				printf "   ...\n"
				set $_skip = 1
			end
		else
			set $_skip = 0

			_ionic_wqe_posted &$_qp->sq $_wqe $_is_posted
			printf "  %4d  %p", $_i, $_wqe
			if ($_is_posted)
				printf "  [posted]"
			end
			printf "\n"

			ionic_sqe_print $_qp $_wqe $_is_posted
		end
		set $_i = $_i + 1
	end
end

define ionic_rq_dump
	set $_qp = (struct ionic_qp *)$arg0

	ionic_qp_print $_qp
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

	ionic_qp_print $_qp
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
Use ionic_sq_dump or ionic_rq_dump to dump only a single queue.
Usage: ionic_qp_dump [qp]
end

#######################################################################
# Admin Queue Entries
#

define ionic_aqe_print
	set $_aq = (struct ionic_aq *)$arg0
	set $_aqe = (struct ionic_v1_admin_wqe *)$arg1

	__indent
	output (enum ionic_v1_admin_op)$_aqe->op

	if ($_aqe->op == IONIC_V1_ADMIN_CREATE_MR ||			\
	    $_aqe->op == IONIC_V1_ADMIN_DESTROY_MR)
		printf "  mrid %4d", $_aqe->id_ver
	end
	if ($_aqe->op == IONIC_V1_ADMIN_CREATE_CQ ||			\
	    $_aqe->op == IONIC_V1_ADMIN_DESTROY_CQ)
		printf "  cqid %4d", $_aqe->id_ver
	end
	if ($_aqe->op == IONIC_V1_ADMIN_CREATE_QP  ||			\
	    $_aqe->op == IONIC_V1_ADMIN_MODIFY_QP  ||			\
	    $_aqe->op == IONIC_V1_ADMIN_DESTROY_QP ||			\
	    $_aqe->op == IONIC_V1_ADMIN_QUERY_QP)
		printf "  qpid %4d", $_aqe->id_ver
	end
	if ($_aqe->op == IONIC_V1_ADMIN_CREATE_AH ||			\
	    $_aqe->op == IONIC_V1_ADMIN_DESTROY_AH)
		printf "  ahid %4d", $_aqe->id_ver
	end
	printf "\n"
end
document ionic_aqe_print
Print information about the given admin WQE.
Usage: ionic_aqe_print [aq] [aqe]
end

define ionic_aq_dump
	set $_aq = (struct ionic_aq *)$arg0
	set $_max = $_aq->q.mask + 1
	set $ibd = $_aq->dev

	ionic_aq_print $_aq

	set $_cq = 0
	_ionic_cq_first $ibd $__tmp_cq
	while ($__tmp_cq != 0)
		if ($__tmp_cq->cqid == $aq->cqid)
			set $_cq = $__tmp_cq
		end
		_ionic_cq_next $ibd $__tmp_cq
	end
	if ($_cq != 0)
		printf "  link:  cq%-4d  (struct ionic_cq *)%p\n",	\
			$_cq->cqid, $_cq
	end
	printf "    AQ:\n"

	set $_i = 0
	set $_skip = 0
	while ($_i < $_max)
		_ionic_q_at &$_aq->q $_i $_e
		set $_aqe = (struct ionic_v1_admin_wqe *)$_e

		if ($_aqe->op == 0)
			if ($_skip == 0)
				printf "   ...\n"
				set $_skip = 1
			end
		else
			set $_skip = 0
			printf "  %4d  %p\n", $_i, $_aqe
			ionic_aqe_print $_aq $_aqe
		end
		set $_i = $_i + 1
	end
end
document ionic_aq_dump
Print information about each WQE in the given AQ (Admin Queue).
Usage: ionic_aq_dump [aq]
end

printf "Hint: start with ionic_ibdev_list_all\n"
