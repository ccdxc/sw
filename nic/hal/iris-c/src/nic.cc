
#include <iostream>
#include <cmath>

#include "nic.hpp"

using namespace std;


const char qstate_64[64] = { 0 };
const char qstate_1024[1024] = { 0 };


sdk::lib::indexer *EthLif::allocator = sdk::lib::indexer::factory(EthLif::max_filters_per_lif, false, true);


struct queue_spec EthLif::qinfo [NUM_QUEUE_TYPES] = {
    [ETH_QTYPE_RX] = {
        .type_num = ETH_QTYPE_RX,
        .size = 1,
        .entries = 0,
        .purpose = ::intf::LIF_QUEUE_PURPOSE_RX,
        .prog = "rxdma_stage0.bin",
        .label = "eth_rx_stage0",
        .qstate = qstate_64
    },
    [ETH_QTYPE_TX] = {
        .type_num = ETH_QTYPE_TX,
        .size = 1,
        .entries = 0,
        .purpose = ::intf::LIF_QUEUE_PURPOSE_TX,
        .prog = "txdma_stage0.bin",
        .label = "eth_tx_stage0",
        .qstate = qstate_64
    },
    [ETH_QTYPE_ADMIN] = {
        .type_num = ETH_QTYPE_ADMIN,
        .size = 1,
        .entries = 0,
        .purpose = ::intf::LIF_QUEUE_PURPOSE_ADMIN,
        .prog = "txdma_stage0.bin",
        .label = "adminq_stage0",
        .qstate = qstate_64
    },
    // [ETH_QTYPE_SQ] = {
    //     .type_num = ETH_QTYPE_SQ,
    //     .size = 5,
    //     .entries = 0,
    //     .purpose = ::intf::LIF_QUEUE_PURPOSE_RDMA_SEND,
    //     .prog = "txdma_stage0.bin",
    //     .label = "rdma_req_tx_stage0",
    //     .qstate = qstate_1024
    // },
    // [ETH_QTYPE_RQ] = {
    //     .type_num = ETH_QTYPE_RQ,
    //     .size = 5,
    //     .entries = 0,
    //     .purpose = ::intf::LIF_QUEUE_PURPOSE_RDMA_SEND,
    //     .prog = "rxdma_stage0.bin",
    //     .label = "rdma_resp_rx_stage0",
    //     .qstate = qstate_1024
    // },
    // [ETH_QTYPE_CQ] = {
    //     .type_num = ETH_QTYPE_CQ,
    //     .size = 1,
    //     .entries = 0,
    //     .purpose = ::intf::LIF_QUEUE_PURPOSE_CQ,
    //     .prog = "txdma_stage0.bin", //hack
    //     .label = "rdma_req_tx_stage0", //hack
    //     .qstate = qstate_64
    // },
    // [ETH_QTYPE_EQ] = {
    //     .type_num = ETH_QTYPE_EQ,
    //     .size = 1,
    //     .entries = 0,
    //     .purpose = ::intf::LIF_QUEUE_PURPOSE_EQ,
    //     .prog = "txdma_stage0.bin", //hack
    //     .label = "rdma_req_tx_stage0", //hack
    //     .qstate = qstate_64
    // },
};

EthLif::EthLif(shared_ptr<Uplink> uplink, shared_ptr<Vrf> vrf,
               struct eth_spec &eth_spec, bool is_mgmt_lif)
{
    struct lif_spec lif_spec;

    memcpy(&lif_spec.queue_spec, &qinfo, sizeof(qinfo));

    lif_spec.queue_spec[ETH_QTYPE_RX].entries = (uint32_t)log2(eth_spec.rxq_count);
    lif_spec.queue_spec[ETH_QTYPE_TX].entries = (uint32_t)log2(eth_spec.txq_count);
    lif_spec.queue_spec[ETH_QTYPE_SQ].entries = (uint32_t)log2(eth_spec.rdma_sq_count);
    lif_spec.queue_spec[ETH_QTYPE_RQ].entries = (uint32_t)log2(eth_spec.rdma_rq_count);
    lif_spec.queue_spec[ETH_QTYPE_CQ].entries = (uint32_t)log2(eth_spec.rdma_cq_count);
    lif_spec.queue_spec[ETH_QTYPE_EQ].entries = (uint32_t)log2(eth_spec.rdma_eq_count);

    is_mgmt_lif = is_mgmt_lif;

    lif_ref = make_shared<Lif>(lif_spec);
    vrf_ref = vrf;
    if (!is_mgmt_lif) {
        uplink_ref = uplink;
    }
}

EthLif::~EthLif()
{
    mac_table.clear();
    vlan_table.clear();
    mac_vlan_table.clear();
    mac_vlan_filter_table.clear();

    lif_ref.reset();
    vrf_ref.reset();
    if (!is_mgmt_lif) {
        uplink_ref.reset();
    }
}

int
EthLif::AddMac(mac_t mac)
{
    uint32_t filter_id = 0;

    if (mac_table.find(mac) == mac_table.end()) {
        // Allocate a filter_id if the mac address is not registered
        if (allocator->alloc(&filter_id) != sdk::lib::indexer::SUCCESS) {
            cerr << "[ERROR] Failed to allocate MAC filter" << endl;
            return 0;
        }
        cout << "[INFO] mac " << mac << " registered! " << endl;
        mac_table[mac] = filter_id;
        // Register new mac across all existing vlans
        for (auto vlan_it = vlan_table.cbegin(); vlan_it != vlan_table.cend(); vlan_it++) {
            CreateMacVlanFilter(mac, vlan_it->first);
        }
    } else {
        cout << "[WARN] mac " << mac << " is already registered!" << endl;
        // Retrieve the filter_id
        filter_id = mac_table[mac];
    }

    return filter_id;
}

int
EthLif::DelMac(mac_t mac)
{
    // nop if mac is not registered
    if (mac_table.find(mac) == mac_table.end()) {
        cout << "[WARN] mac " << mac << " is not registered!" << endl;
        return 0;
    }

    // Deregister MAC across all existing vlans
    for (auto vlan_it = vlan_table.cbegin(); vlan_it != vlan_table.cend(); vlan_it++) {
        mac_vlan_t key(mac, vlan_it->first);
        if (mac_vlan_table.find(key) == mac_vlan_table.cend()) {
            DeleteMacVlanFilter(mac, vlan_it->first);
        }
    }
    allocator->free(mac_table[mac]);
    mac_table.erase(mac);
    cout << "[INFO] mac " << mac << " deregistered! " << endl;

    return 0;
}

int
EthLif::AddVlan(vlan_t vlan)
{
    uint32_t filter_id = 0;

    if (vlan_table.find(vlan) == vlan_table.end()) {
        // Allocate filter_id if VLAN is not registered
        if (allocator->alloc(&filter_id) != sdk::lib::indexer::SUCCESS) {
            cerr << "[ERROR] Failed to allocate VLAN filter" << endl;
            return 0;
        }
        cout << "[INFO] vlan " << vlan << " registered! " << endl;
        vlan_table[vlan] = filter_id;
        // Register this VLAN across all existing MACs
        for (auto mac_it = mac_table.cbegin(); mac_it != mac_table.cend(); mac_it++) {
            CreateMacVlanFilter(mac_it->first, vlan);
        }
    } else {
        cout << "[WARN] vlan " << vlan << " is already registered!" << endl;
        // Retrieve the filter_id
        filter_id = vlan_table[vlan];
    }

    return filter_id;
}

int
EthLif::DelVlan(vlan_t vlan)
{
    // nop if vlan is not registered
    if (vlan_table.find(vlan) == vlan_table.end()) {
        cout << "[WARN] vlan " << vlan << " is not registered!" << endl;
        return 0;
    }

    // Deregister this VLAN across all existing MACs
    for (auto mac_it = mac_table.cbegin(); mac_it != mac_table.cend(); mac_it++) {
        mac_vlan_t key(mac_it->first, vlan);
        if (mac_vlan_table.find(key) == mac_vlan_table.cend()) {
            DeleteMacVlanFilter(mac_it->first, vlan);
        }
    }
    allocator->free(vlan_table[vlan]);
    vlan_table.erase(vlan);
    cout << "[INFO] vlan " << vlan << " deregistered! " << endl;

    return 0;
}

int
EthLif::AddMacVlan(mac_t mac, vlan_t vlan)
{
    uint32_t filter_id = 0;
    mac_vlan_t key(mac, vlan);

    if (mac_vlan_table.find(key) == mac_vlan_table.end()) {
        // Allocate filter_id if (MAC, VLAN) is not registered
        if (allocator->alloc(&filter_id) != sdk::lib::indexer::SUCCESS) {
            cerr << "[ERROR] Failed to allocate MAC-VLAN filter" << endl;
            return -1;
        }
        cout << "[INFO] mac " << mac << " vlan " << vlan << " registered! " << endl;
        mac_vlan_table[key] = filter_id;
        CreateMacVlanFilter(mac, vlan);
    } else {
        cout << "[WARN] mac " << mac << " vlan " << vlan << " is already registered!" << endl;
        // Retrieve the filter_id
        filter_id = mac_vlan_table[key];
    }

    return 0;
}

int
EthLif::DelMacVlan(mac_t mac, vlan_t vlan)
{
    uint32_t filter_id = 0;
    mac_vlan_t key(mac, vlan);

    // nop if (mac, vlan) is not registered
    if (mac_vlan_table.find(key) == mac_vlan_table.end()) {
        cout << "[WARN] mac " << mac << " vlan " << vlan << " is not registered!" << endl;
        return 0;
    }

    if (mac_table.find(mac) == mac_table.cend() && vlan_table.find(vlan) == vlan_table.cend()) {
        DeleteMacVlanFilter(mac, vlan);
    }
    allocator->free(filter_id);
    mac_vlan_table.erase(key);
    cout << "[INFO] mac " << mac << " vlan " << vlan << " deregistered! " << endl;

    return 0;
}

int
EthLif::CreateMacVlanFilter(mac_t mac, vlan_t vlan)
{
    mac_vlan_t key(mac, vlan);

    // nop if (mac, vlan) is already registered
    if (mac_vlan_filter_table.find(key) != mac_vlan_filter_table.end()) {
        cout << "[WARN] mac " << mac << " vlan " << vlan << " filter is already created!" << endl;
        return 0;
    }

    mac_vlan_filter_table[key] = make_shared<MacVlanFilter>(lif_ref, vrf_ref, mac, vlan);
    cout << "[INFO] mac " << mac << " vlan " << vlan << " filter created!" << endl;

    return 0;
}

int
EthLif::DeleteMacVlanFilter(mac_t mac, vlan_t vlan)
{
    mac_vlan_t key(mac, vlan);

    // nop if (mac, vlan) is not registered
    if (mac_vlan_filter_table.find(key) == mac_vlan_filter_table.end()) {
        cout << "[WARN] mac " << mac << " vlan " << vlan << " filter is not created!" << endl;
        return 0;
    }

    mac_vlan_filter_table.erase(key);
    cout << "[INFO] mac " << mac << " vlan " << vlan << " filter deleted!" << endl;

    return 0;
}
