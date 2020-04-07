#include "Config.h"

int
_cdecl
wmain(int argc, wchar_t* argv[])
{
    command_info info;
    bool cmd_found = false;

    info.cmd = CmdUsage();
    info.cmds.push_back(CmdVersion());
    info.cmds.push_back(CmdInstall());
    info.cmds.push_back(CmdUninstall());
    info.cmds.push_back(CmdUpdate());
    info.cmds.push_back(CmdSetTrace());
    info.cmds.push_back(CmdGetTrace());
    info.cmds.push_back(CmdPort());
    info.cmds.push_back(CmdRxBudget());
    info.cmds.push_back(CmdDevStats());
    info.cmds.push_back(CmdLifStats());
    info.cmds.push_back(CmdPortStats());
    info.cmds.push_back(CmdPerfStats());
	info.cmds.push_back(CmdRegKeys());
    //info.cmds.push_back(CmdOidStats());
    //info.cmds.push_back(CmdFwcmdStats());
    info.cmds.push_back(CmdBistClient());
    info.cmds.push_back(CmdBistServer());

    po::command_line_style::style_t style =
        (po::command_line_style::style_t)
        (po::command_line_style::allow_short |
         po::command_line_style::allow_dash_for_short |
         po::command_line_style::allow_slash_for_short |
         po::command_line_style::short_allow_next |
         po::command_line_style::allow_long |
         po::command_line_style::allow_long_disguise |
         po::command_line_style::long_allow_adjacent |
         po::command_line_style::long_allow_next |
         po::command_line_style::case_insensitive);

    // Parse top-level options
    try {
        po::options_description opts = info.cmd.opts();
        po::wparsed_options parsed = po::wcommand_line_parser(argc, argv)
            .allow_unregistered()
            .options(opts)
            .style(style)
            .run();
        po::store(parsed, info.vm);
        info.cmd_args = po::collect_unrecognized(parsed.options, po::include_positional);
    }
    catch (std::exception& e) {
        std::cout << e.what() << std::endl;
        info.status = 1;
        info.usage = 1;
    }

    if (info.vm.count("Help") != 0) {
        info.usage = 1;
    }

    // Lookup subcommand
    if (!info.cmd_args.empty()) {
        info.cmd_name = to_bytes(info.cmd_args.front());
        info.cmd_args.erase(info.cmd_args.begin());

        // Remove prefix - -- or / if present
        std::string cmd_name = info.cmd_name;
        if (cmd_name[0] == '-') {
            if (cmd_name[1] == '-') {
                cmd_name.erase(0, 2);
            }
            else {
                cmd_name.erase(0, 1);
            }
        }
        else if (cmd_name[0] == '/') {
            cmd_name.erase(0, 1);
        }

        // Find in all commands, case insensitive
        for (auto& cmd_i : info.cmds) {
            if (boost::iequals(cmd_name, cmd_i.name)) {
                info.cmd = cmd_i;
                cmd_found = true;
            }
        }
    }

    // Parse subcommand options
    if (cmd_found && !info.usage) {
        try {
            po::options_description opts = info.cmd.opts();
            po::positional_options_description pos = info.cmd.pos();
            po::wparsed_options parsed = po::wcommand_line_parser(info.cmd_args)
                .options(opts)
                .positional(pos)
                .style(style)
                .run();
            po::store(parsed, info.vm);
            po::notify(info.vm);
            info.cmd_args = po::collect_unrecognized(parsed.options, po::include_positional);
        }
        catch (std::exception& e) {
            std::cout << e.what() << std::endl;
            info.status = 1;
            info.usage = true;
        }
    }

    try {
        return info.cmd.run(info);
    }
    catch (std::exception& e) {
        std::cout << e.what() << std::endl;
        return 1;
    }
}

//
// Top-level options and list of commands
//

static
po::options_description
CmdUsageOpts()
{
    po::options_description opts("IonicConfig.exe [-h] [Command options...]");

    opts.add_options()
        ("Help,?,h", optype_flag(), "Display command specific usage");

    return opts;
}

static
int
CmdUsageRun(command_info& info)
{
    if (!info.cmd_name.empty()) {
        std::cout << "unknown command: " << info.cmd_name << std::endl;
        info.status = 1;
    }

    std::cout << info.cmd.opts() << "Commands:" << std::endl;

    for (auto& cmd_i : info.cmds) {
        if (!cmd_i.hidden) {
            std::cout << "  " << std::left << std::setw(20)
                << cmd_i.name << "  " << cmd_i.desc << std::endl;
        }
    }

    return info.status;
}

command
CmdUsage()
{
    command cmd;

    cmd.name = "Help";
    cmd.desc = "Print usage";
    cmd.hidden = true;
    cmd.opts = CmdUsageOpts;
    cmd.run = CmdUsageRun;

    return cmd;
}

//
// -Version
//

static
po::options_description
CmdVersionOpts()
{
    po::options_description opts("IonicConfig.exe Version");

    return opts;
}

static
int
CmdVersionRun(command_info& info)
{
    if (info.usage) {
        std::cout << info.cmd.opts() << info.cmd.desc << std::endl;
        return info.status;
    }

    std::cout << "IonicConfig.exe version " << "TODO (from build)" << std::endl;

    return 0;
}

command
CmdVersion()
{
    command cmd;

    cmd.name = "Version";
    cmd.desc = "Print IonicConfig.exe version";
    cmd.opts = CmdVersionOpts;
    cmd.run = CmdVersionRun;

    return cmd;
}


//
// TODO: Cleanup, move to Stats.cpp
//

#include "stdio.h"
#include <shlwapi.h>
#include <winioctl.h>

#include "UserCommon.h"

#include "pci_ids.h"

#include "ionic_types.h"
#include "ionic_stats.h"

char *
GetLifTypeName(ULONG type);

void
DumpDevStats(void *Stats)
{
       
    struct dev_port_stats    *dev_stats = (struct dev_port_stats *)Stats;
    ULONG ulPortCnt = 0;
    ULONG ulLifCount = 0;
    ULONG ulRxCnt = 0;
    ULONG ulTxCnt = 0;

    while( dev_stats->device_id != 0)
    {
        printf("Port %d Mgmt %s RSS %s VMQ %s SRIOV %s Up %d Dwn %d\n", 
                        ulPortCnt, 
                        (dev_stats->device_id == PCI_DEVICE_ID_PENSANDO_IONIC_ETH_MGMT)?"Yes":"No",
                        (dev_stats->flags & IONIC_PORT_FLAG_RSS)?"Yes":"No",
                        (dev_stats->flags & IONIC_PORT_FLAG_VMQ)?"Yes":"No",
                        (dev_stats->flags & IONIC_PORT_FLAG_SRIOV)?"Yes":"No",
                        (ULONG)dev_stats->link_up,
                        (ULONG)dev_stats->link_dn);

        printf("\tVendor %04lX\n", dev_stats->vendor_id);
        printf("\tDevice %04lX\n", dev_stats->device_id);
        printf("\tLif Count %d\n", dev_stats->lif_count);

        ulLifCount = 0;
        while (ulLifCount < dev_stats->lif_count)
        {

            printf("\tLif %d\n", dev_stats->lif_stats[ ulLifCount].lif_id);
            printf("\t\tLif type: %s", GetLifTypeName( dev_stats->lif_stats[ ulLifCount].lif_type));

            printf("\t\tLif name: %s\n", dev_stats->lif_stats[ ulLifCount].lif_name);

            printf("\t\tRx Count: %d\n", dev_stats->lif_stats[ ulLifCount].rx_count);
            printf("\t\tTx Count: %d\n", dev_stats->lif_stats[ ulLifCount].tx_count);

            ulRxCnt = 0;
            while (ulRxCnt < dev_stats->lif_stats[ulLifCount].rx_count)
            {

                printf("\t\t\tRx %d Msi %d directed bytes 0x%I64X directed packets 0x%I64X Comp 0x%I64X Vlan 0x%I64X\n",
                                ulRxCnt,
                                dev_stats->lif_stats[ulLifCount].rx_ring[ ulRxCnt].msi_id,
                                dev_stats->lif_stats[ulLifCount].rx_ring[ ulRxCnt].directed_bytes,
                                dev_stats->lif_stats[ulLifCount].rx_ring[ ulRxCnt].directed_packets,
                                dev_stats->lif_stats[ulLifCount].rx_ring[ ulRxCnt].completion_count,
                                dev_stats->lif_stats[ulLifCount].rx_ring[ ulRxCnt].vlan_stripped);

                ulRxCnt++;
            }

            ulTxCnt = 0;
            while (ulTxCnt < dev_stats->lif_stats[ulLifCount].tx_count)
            {

                printf("\t\t\tTx %d directed bytes 0x%I64X directed packets 0x%I64X tso bytes 0x%I64X tso packets 0x%I64X db cnt 0x%I64X vlan 0x%I64X\n",
                            ulTxCnt,
                            dev_stats->lif_stats[ulLifCount].tx_ring[ ulTxCnt].directed_bytes,
                            dev_stats->lif_stats[ulLifCount].tx_ring[ ulTxCnt].directed_packets,
                            dev_stats->lif_stats[ulLifCount].tx_ring[ ulTxCnt].tso_bytes,
                            dev_stats->lif_stats[ulLifCount].tx_ring[ ulTxCnt].tso_packets,
                            dev_stats->lif_stats[ulLifCount].tx_ring[ ulTxCnt].doorbell_count,
                            dev_stats->lif_stats[ulLifCount].tx_ring[ ulTxCnt].vlan_inserted);

                ulTxCnt++;
            }

            ulLifCount++;
        }

        dev_stats = (struct dev_port_stats *)((char *)dev_stats + sizeof( struct dev_port_stats));
        ulPortCnt++;
    }

    return;
}

void
DumpMgmtStats(void *Stats)
{
    
    struct mgmt_port_stats *mgmt_stats = (struct mgmt_port_stats *)Stats;

    printf("Mgmt Port Stats\n");

    printf("\tRx frames all: 0x%I64X\n", mgmt_stats->frames_rx_all);


    return;
}

void
DumpPortStats(DWORD Port, void *Stats)
{
    
    struct port_stats *port_stats = (struct port_stats *)Stats;
    ULONGLONG ull9216Frames = 0;

    printf("Port %d Stats\n", Port);

    printf("\tRx frames ok:\t\t0x%I64X\n", port_stats->frames_rx_ok);
    printf("\tRx frames all:\t\t0x%I64X\n", port_stats->frames_rx_all);
    printf("\tRx fcs errors:\t\t0x%I64X\n", port_stats->frames_rx_bad_fcs);
    printf("\tRx crc errors:\t\t0x%I64X\n", port_stats->frames_rx_bad_all);
    printf("\tRx bytes ok:\t\t0x%I64X\n", port_stats->octets_rx_ok);
    printf("\tRx bytes all:\t\t0x%I64X\n", port_stats->octets_rx_all);
    printf("\tRx u-cast:\t\t0x%I64X\n", port_stats->frames_rx_unicast);
    printf("\tRx m-cast:\t\t0x%I64X\n", port_stats->frames_rx_multicast);
    printf("\tRx b-cast:\t\t0x%I64X\n", port_stats->frames_rx_broadcast);
    printf("\tRx pause:\t\t0x%I64X\n", port_stats->frames_rx_pause);
    printf("\tRx bad length:\t\t0x%I64X\n", port_stats->frames_rx_bad_length);
    printf("\tRx undersized:\t\t0x%I64X\n", port_stats->frames_rx_undersized);
    printf("\tRx oversized:\t\t0x%I64X\n", port_stats->frames_rx_oversized);
    printf("\tRx fragments:\t\t0x%I64X\n", port_stats->frames_rx_fragments);
    printf("\tRx jabber:\t\t0x%I64X\n", port_stats->frames_rx_jabber);
    printf("\tRx pfc frames:\t\t0x%I64X\n", port_stats->frames_rx_pripause);
    printf("\tRx crc stomped:\t\t0x%I64X\n", port_stats->frames_rx_stomped_crc);
    printf("\tRx too long:\t\t0x%I64X\n", port_stats->frames_rx_too_long);
    printf("\tRx vlan ok:\t\t0x%I64X\n", port_stats->frames_rx_vlan_good);
    printf("\tRx dropped frames:\t0x%I64X\n", port_stats->frames_rx_dropped);
    printf("\tRx frames < 64:\t\t0x%I64X\n", port_stats->frames_rx_less_than_64b);
    printf("\tRx frames = 64:\t\t0x%I64X\n", port_stats->frames_rx_64b);
    printf("\tRx frames 65-127:\t0x%I64X\n", port_stats->frames_rx_65b_127b);
    printf("\tRx frames 128-255:\t0x%I64X\n", port_stats->frames_rx_128b_255b);
    printf("\tRx frames 256-511:\t0x%I64X\n", port_stats->frames_rx_256b_511b);
    printf("\tRx frames 512-1023:\t0x%I64X\n", port_stats->frames_rx_512b_1023b);
    printf("\tRx frames 1024-1518:\t0x%I64X\n", port_stats->frames_rx_1024b_1518b);
    printf("\tRx frames 1519-2047:\t0x%I64X\n", port_stats->frames_rx_1519b_2047b);
    printf("\tRx frames 2048-4095:\t0x%I64X\n", port_stats->frames_rx_2048b_4095b);
    printf("\tRx frames 4096-8191:\t0x%I64X\n", port_stats->frames_rx_4096b_8191b);
    printf("\tRx frames 8192-9215:\t0x%I64X\n", port_stats->frames_rx_8192b_9215b);

    ull9216Frames = port_stats->frames_rx_all - ( port_stats->frames_rx_dropped +
                                                    port_stats->frames_rx_less_than_64b +
                                                    port_stats->frames_rx_64b +
                                                    port_stats->frames_rx_65b_127b +
                                                    port_stats->frames_rx_128b_255b +
                                                    port_stats->frames_rx_256b_511b +
                                                    port_stats->frames_rx_512b_1023b +
                                                    port_stats->frames_rx_1024b_1518b +
                                                    port_stats->frames_rx_1519b_2047b +
                                                    port_stats->frames_rx_2048b_4095b +
                                                    port_stats->frames_rx_4096b_8191b +
                                                    port_stats->frames_rx_8192b_9215b);

    printf("\tRx frames >= 9216:\t0x%I64X\n", ull9216Frames);

    printf("\tTx frames ok:\t\t0x%I64X\n", port_stats->frames_tx_ok);
    printf("\tTx frames all:\t\t0x%I64X\n", port_stats->frames_tx_all);
    printf("\tTx frames bad:\t\t0x%I64X\n", port_stats->frames_tx_bad);
    printf("\tTx bytes ok:\t\t0x%I64X\n", port_stats->octets_tx_ok);
    printf("\tTx bytes all:\t\t0x%I64X\n", port_stats->octets_tx_total);
    printf("\tTx u-cast:\t\t0x%I64X\n", port_stats->frames_tx_unicast);
    printf("\tTx m-cast:\t\t0x%I64X\n", port_stats->frames_tx_multicast);
    printf("\tTx b-cast:\t\t0x%I64X\n", port_stats->frames_tx_broadcast);
    printf("\tTx pause frames:\t0x%I64X\n", port_stats->frames_tx_pause);
    printf("\tTx pfc frames:\t\t0x%I64X\n", port_stats->frames_tx_pripause);
    printf("\tTx vlan frames:\t\t0x%I64X\n", port_stats->frames_tx_vlan);
    printf("\tTx frames < 64:\t\t0x%I64X\n", port_stats->frames_tx_less_than_64b);
    printf("\tTx frames = 64:\t\t0x%I64X\n", port_stats->frames_tx_64b);
    printf("\tTx frames 65-127:\t0x%I64X\n", port_stats->frames_tx_65b_127b);
    printf("\tTx frames 128-255:\t0x%I64X\n", port_stats->frames_tx_128b_255b);
    printf("\tTx frames 256-511:\t0x%I64X\n", port_stats->frames_tx_256b_511b);
    printf("\tTx frames 512-1023:\t0x%I64X\n", port_stats->frames_tx_512b_1023b);
    printf("\tTx frames 1024-1518:\t0x%I64X\n", port_stats->frames_tx_1024b_1518b);
    printf("\tTx frames 1519-2047:\t0x%I64X\n", port_stats->frames_tx_1519b_2047b);
    printf("\tTx frames 2048-4095:\t0x%I64X\n", port_stats->frames_tx_2048b_4095b);
    printf("\tTx frames 4096-8191:\t0x%I64X\n", port_stats->frames_tx_4096b_8191b);
    printf("\tTx frames 8192-9215:\t0x%I64X\n", port_stats->frames_tx_8192b_9215b);

    ull9216Frames = port_stats->frames_tx_all - ( port_stats->frames_tx_less_than_64b +
                                                    port_stats->frames_tx_64b +
                                                    port_stats->frames_tx_65b_127b +
                                                    port_stats->frames_tx_128b_255b +
                                                    port_stats->frames_tx_256b_511b +
                                                    port_stats->frames_tx_512b_1023b +
                                                    port_stats->frames_tx_1024b_1518b +
                                                    port_stats->frames_tx_1519b_2047b +
                                                    port_stats->frames_tx_2048b_4095b +
                                                    port_stats->frames_tx_4096b_8191b +
                                                    port_stats->frames_tx_8192b_9215b);

    printf("\tTx frames >= 9216:\t0x%I64X\n", ull9216Frames);

    printf("\tTx pri-0:\t\t0x%I64X\n", port_stats->frames_tx_pri_0);
    printf("\tTx pri-1:\t\t0x%I64X\n", port_stats->frames_tx_pri_1);
    printf("\tTx pri-2:\t\t0x%I64X\n", port_stats->frames_tx_pri_2);
    printf("\tTx pri-3:\t\t0x%I64X\n", port_stats->frames_tx_pri_3);
    printf("\tTx pri-4:\t\t0x%I64X\n", port_stats->frames_tx_pri_4);
    printf("\tTx pri-5:\t\t0x%I64X\n", port_stats->frames_tx_pri_5);
    printf("\tTx pri-6:\t\t0x%I64X\n", port_stats->frames_tx_pri_6);
    printf("\tTx pri-7:\t\t0x%I64X\n", port_stats->frames_tx_pri_7);

    printf("\tRx pri-0:\t\t0x%I64X\n", port_stats->frames_rx_pri_0);
    printf("\tRx pri-1:\t\t0x%I64X\n", port_stats->frames_rx_pri_1);
    printf("\tRx pri-2:\t\t0x%I64X\n", port_stats->frames_rx_pri_2);
    printf("\tRx pri-3:\t\t0x%I64X\n", port_stats->frames_rx_pri_3);
    printf("\tRx pri-4:\t\t0x%I64X\n", port_stats->frames_rx_pri_4);
    printf("\tRx pri-5:\t\t0x%I64X\n", port_stats->frames_rx_pri_5);
    printf("\tRx pri-6:\t\t0x%I64X\n", port_stats->frames_rx_pri_6);
    printf("\tRx pri-7:\t\t0x%I64X\n", port_stats->frames_rx_pri_7);

    printf("\tTx pri-0 pause:\t\t0x%I64X\n", port_stats->tx_pripause_0_1us_count);
    printf("\tTx pri-1 pause:\t\t0x%I64X\n", port_stats->tx_pripause_1_1us_count);
    printf("\tTx pri-2 pause:\t\t0x%I64X\n", port_stats->tx_pripause_2_1us_count);
    printf("\tTx pri-3 pause:\t\t0x%I64X\n", port_stats->tx_pripause_3_1us_count);
    printf("\tTx pri-4 pause:\t\t0x%I64X\n", port_stats->tx_pripause_4_1us_count);
    printf("\tTx pri-5 pause:\t\t0x%I64X\n", port_stats->tx_pripause_5_1us_count);
    printf("\tTx pri-6 pause:\t\t0x%I64X\n", port_stats->tx_pripause_6_1us_count);
    printf("\tTx pri-7 pause:\t\t0x%I64X\n", port_stats->tx_pripause_7_1us_count);

    printf("\tRx pri-0 pause:\t\t0x%I64X\n", port_stats->rx_pripause_0_1us_count);
    printf("\tRx pri-1 pause:\t\t0x%I64X\n", port_stats->rx_pripause_1_1us_count);
    printf("\tRx pri-2 pause:\t\t0x%I64X\n", port_stats->rx_pripause_2_1us_count);
    printf("\tRx pri-3 pause:\t\t0x%I64X\n", port_stats->rx_pripause_3_1us_count);
    printf("\tRx pri-4 pause:\t\t0x%I64X\n", port_stats->rx_pripause_4_1us_count);
    printf("\tRx pri-5 pause:\t\t0x%I64X\n", port_stats->rx_pripause_5_1us_count);
    printf("\tRx pri-6 pause:\t\t0x%I64X\n", port_stats->rx_pripause_6_1us_count);
    printf("\tRx pri-7 pause:\t\t0x%I64X\n", port_stats->rx_pripause_7_1us_count);
    printf("\tRx standard pause:\t0x%I64X\n", port_stats->rx_pause_1us_count);

    printf("\tFrames truncated:\t0x%I64X\n", port_stats->frames_tx_truncated);

    return;
}

void
DumpLifStats(DWORD Lif, void *Stats)
{

    struct lif_stats *lif_stats = (struct lif_stats *)Stats;

    printf("Lif %d Stats\n", Lif);

    printf("\tRx u-cast bytes: \t\t0x%I64X\n", lif_stats->rx_ucast_bytes);
    printf("\tRx u-cast packets: \t\t0x%I64X\n", lif_stats->rx_ucast_packets);
    printf("\tRx m-cast bytes: \t\t0x%I64X\n", lif_stats->rx_mcast_bytes);
    printf("\tRx m-cast packets: \t\t0x%I64X\n", lif_stats->rx_mcast_packets);
    printf("\tRx b-cast bytes: \t\t0x%I64X\n", lif_stats->rx_bcast_bytes);
    printf("\tRx b-cast packets: \t\t0x%I64X\n", lif_stats->rx_bcast_packets);

    printf("\tRx drop u-cast bytes: \t\t0x%I64X\n", lif_stats->rx_ucast_drop_bytes);
    printf("\tRx drop u-cast packets: \t0x%I64X\n", lif_stats->rx_ucast_drop_packets);
    printf("\tRx drop m-cast bytes: \t\t0x%I64X\n", lif_stats->rx_mcast_drop_bytes);
    printf("\tRx drop m-cast packets: \t0x%I64X\n", lif_stats->rx_mcast_drop_packets);
    printf("\tRx drop b-cast bytes: \t\t0x%I64X\n", lif_stats->rx_bcast_drop_bytes);
    printf("\tRx drop b-cast packets: \t0x%I64X\n", lif_stats->rx_bcast_drop_packets);
    printf("\tRx dma error: \t\t\t0x%I64X\n", lif_stats->rx_dma_error);

    printf("\tTx u-cast bytes: \t\t0x%I64X\n", lif_stats->tx_ucast_bytes);
    printf("\tTx u-cast packets: \t\t0x%I64X\n", lif_stats->tx_ucast_packets);
    printf("\tTx m-cast bytes: \t\t0x%I64X\n", lif_stats->tx_mcast_bytes);
    printf("\tTx m-cast packets: \t\t0x%I64X\n", lif_stats->tx_mcast_packets);
    printf("\tTx b-cast bytes: \t\t0x%I64X\n", lif_stats->tx_bcast_bytes);
    printf("\tTx b-cast packets: \t\t0x%I64X\n", lif_stats->tx_bcast_packets);
    
    printf("\tTx drop u-cast bytes: \t\t0x%I64X\n", lif_stats->tx_ucast_drop_bytes);
    printf("\tTx drop u-cast packets: \t0x%I64X\n", lif_stats->tx_ucast_drop_packets);
    printf("\tTx drop m-cast bytes: \t\t0x%I64X\n", lif_stats->tx_mcast_drop_bytes);
    printf("\tTx drop m-cast packets: \t0x%I64X\n", lif_stats->tx_mcast_drop_packets);
    printf("\tTx drop b-cast bytes: \t\t0x%I64X\n", lif_stats->tx_bcast_drop_bytes);
    printf("\tTx drop b-cast packets: \t0x%I64X\n", lif_stats->tx_bcast_drop_packets);
    printf("\tTx dma error: \t\t\t0x%I64X\n", lif_stats->tx_dma_error);

    printf("\tRx queue disabled drops: \t0x%I64X\n", lif_stats->rx_queue_disabled);
    printf("\tRx queue empty drops: \t\t0x%I64X\n", lif_stats->rx_queue_empty);
    printf("\tRx queue error count: \t\t0x%I64X\n", lif_stats->rx_queue_error);
    printf("\tRx descriptor fetch errors: \t0x%I64X\n", lif_stats->rx_desc_fetch_error);
    printf("\tRx descriptor data errors: \t0x%I64X\n", lif_stats->rx_desc_data_error);

    printf("\tTx queue disabled drops: \t0x%I64X\n", lif_stats->tx_queue_disabled);
    printf("\tTx queue error count: \t\t0x%I64X\n", lif_stats->tx_queue_error);
    printf("\tTx descriptor fetch errors: \t0x%I64X\n", lif_stats->tx_desc_fetch_error);
    printf("\tTx descriptor data errors: \t0x%I64X\n", lif_stats->tx_desc_data_error);

    return;
}

char *
GetLifTypeName(ULONG type)
{

    char    *pType = (char *)"Default";

    if (type == IONIC_LIF_TYPE_VMQ) {
        pType = (char *)"VM Queue";
    }
    else if (type == IONIC_LIF_TYPE_VPORT) {
        pType = (char *)"VPort";
    }

    return pType;
}

void
DumpPerfStats(void *Stats)
{

    struct _PERF_MON_CB *perf_stats = (struct _PERF_MON_CB *)Stats;
    struct _PERF_MON_ADAPTER_STATS *adapter_stats = NULL;
    struct _PERF_MON_LIF_STATS *lif_stats = NULL;
    struct _PERF_MON_TX_QUEUE_STATS *tx_stats = NULL;
    struct _PERF_MON_RX_QUEUE_STATS *rx_stats = NULL;
    ULONG adapter_cnt = 0;
    ULONG lif_cnt = 0;
    ULONG rx_cnt = 0;
    ULONG tx_cnt = 0;

    adapter_stats = (struct _PERF_MON_ADAPTER_STATS *)((char *)perf_stats + sizeof( struct _PERF_MON_CB));

    for (adapter_cnt = 0; adapter_cnt < perf_stats->adapter_count; adapter_cnt++) {

        printf("Adapter: %S Mgmt: %s Lif cnt: %d Core redirect cnt: %d\n", 
                        adapter_stats->name,
                        adapter_stats->mgmt_device?"Yes":"No",
                        adapter_stats->lif_count,
                        adapter_stats->core_redirection_count);

        lif_stats = (struct _PERF_MON_LIF_STATS *)((char *)adapter_stats + sizeof( struct _PERF_MON_ADAPTER_STATS));

        for (lif_cnt = 0; lif_cnt < adapter_stats->lif_count; lif_cnt++) {

            printf("\tLif: %s Rx cnt: %d Tx cnt: %d\n",
                            lif_stats->name,
                            lif_stats->rx_queue_count,
                            lif_stats->tx_queue_count);

            rx_stats = (struct _PERF_MON_RX_QUEUE_STATS *)((char *)lif_stats + sizeof( struct _PERF_MON_LIF_STATS));

            for (rx_cnt = 0; rx_cnt < lif_stats->rx_queue_count; rx_cnt++) {

                printf("\t\tRx: %d :Pool cnt: %d\n",
                                rx_cnt,
                                rx_stats->rx_pool_count);

                rx_stats = (struct _PERF_MON_RX_QUEUE_STATS *)((char *)rx_stats + sizeof( struct _PERF_MON_RX_QUEUE_STATS));
            }

            printf("\n");

            tx_stats = (struct _PERF_MON_TX_QUEUE_STATS *)rx_stats;

            for (tx_cnt = 0; tx_cnt < lif_stats->tx_queue_count; tx_cnt++) {

                printf("\t\tTx: %d nblps: %d nbps %d byteps: %d pending nbl: %d pending nb: %d\n",
                                tx_cnt,
                                tx_stats->nbl_per_sec,
                                tx_stats->nb_per_sec,
                                tx_stats->byte_per_sec,
                                tx_stats->pending_nbl_count,
                                tx_stats->pending_nb_count);

                printf("\t\tqueue len: %d max: %d ttq: %lld ttc: %lld qtc: %lld nbl ttc: %lld\n",
                                tx_stats->queue_len,
                                tx_stats->max_queue_len,
                                tx_stats->nb_time_to_queue,
                                tx_stats->nb_time_to_complete,
                                tx_stats->nb_time_queue_to_comp,
                                tx_stats->nbl_time_to_complete);

                tx_stats = (struct _PERF_MON_TX_QUEUE_STATS *)((char *)tx_stats + sizeof( struct _PERF_MON_TX_QUEUE_STATS));
            }

            lif_stats = (struct _PERF_MON_LIF_STATS *)tx_stats;
            printf("\n");
        }
       
        adapter_stats = (struct _PERF_MON_ADAPTER_STATS *)lif_stats;
        printf("\n");
    }

    return;
}

void
DumpRegKeys(void *KeyInfo, ULONG Size)
{

    struct _REG_KEY_INFO_HDR *key_info = (struct _REG_KEY_INFO_HDR *)KeyInfo;
	struct _REG_KEY_ENTRY *entry = NULL;
	ULONG processed_len = 0;
	ULONG reg_index = 0;
#define KEY_NAME_LEN		15
	WCHAR key_name[KEY_NAME_LEN];
	ULONG pad = 0;

	while (processed_len < Size)
	{

		if (key_info->name[0] == 0) {
			break;
		}

		printf("Adapter: %S\n", key_info->name);
		printf("Location: %S\n", key_info->device_location);

		printf("\tRegistry Key\t\t\t\tValue\t\tDefault\t\tMin\t\tMax\n\n");
		entry = (struct _REG_KEY_ENTRY *)((char *)key_info + sizeof( struct _REG_KEY_INFO_HDR));
		for (reg_index = 0; reg_index < key_info->entry_count; reg_index++) {

			wcscpy_s(key_name, 50, (WCHAR *)((char *)entry + entry->key_name_offset));
			/* pad with spaces so all looks the same */
			for (pad = (ULONG)wcslen(key_name); pad < KEY_NAME_LEN; pad++) {
				key_name[ pad] = L' ';
			}
			key_name[ KEY_NAME_LEN - 1] = NULL;

			printf("\t%S\t\t\t\t%d\t\t%d\t\t%d\t\t%d\n", 
						key_name,
						entry->current_value,
						entry->default_value,
						entry->min_value,
						entry->max_value);

			entry = (struct _REG_KEY_ENTRY *)((char *)entry + entry->next_entry);
		}

		processed_len += key_info->next_entry;
		key_info = (struct _REG_KEY_INFO_HDR *)((char *)key_info + key_info->next_entry);
		printf("\n");
	}

    return;
}

//
// TODO: move to Ioctl.cpp or something... after integration
//

BOOL
DoIoctl(DWORD dwIoControlCode,
        LPVOID lpInBuffer,
        DWORD nInBufferSize,
        LPVOID lpOutBuffer,
        DWORD nOutBufferSize)
{
    HANDLE hDevice = NULL;
    DWORD nBytesReturned = 0;
    BOOL bResult = FALSE;

    hDevice = CreateFile(IONIC_LINKNAME_STRING_USER,
                         GENERIC_READ | GENERIC_WRITE,
                         0,
                         NULL,
                         OPEN_EXISTING,
                         0,
                         NULL);

    if (hDevice == INVALID_HANDLE_VALUE) {
        std::cout << "IonicConfig Failed to open adapter Error " << GetLastError() << std::endl;
        return FALSE;
    }

    bResult = DeviceIoControl(hDevice,
                              dwIoControlCode,
                              lpInBuffer,
                              nInBufferSize,
                              lpOutBuffer,
                              nOutBufferSize,
                              &nBytesReturned,
                              NULL);

    if (!bResult) {
        std::cout << "Ioctl Failed Error " << GetLastError() << std::endl;
    }

    CloseHandle( hDevice);

    return bResult;
}

//
// TODO: move to Trace.cpp after integration
//

//
// -SetTrace
//

static
po::options_description
CmdSetTraceOpts()
{
    po::options_description opts("IonicConfig.exe [-h] SetTrace [options ...]");

    opts.add_options()
        ("Level,l", optype_long(), "Set debug level")
        ("Flags,f", optype_long(), "Set degut flags")
        ("Component,c", optype_long(), "Set component mask")
        ("Length,s", optype_long(), "Set trace buffer length in KB");

    return opts;
}

static
int
CmdSetTraceRun(command_info& info)
{
    if (info.usage) {
        std::cout << info.cmd.opts() << info.cmd.desc << std::endl;
        return info.status;
    }

    TraceConfigCB stTrace;
    stTrace.Level = -1;
    stTrace.TraceFlags = -1;
    stTrace.Component = -1;
    stTrace.TraceBufferLength = -1;

    if (info.vm.count("Level")) {
        stTrace.Level = opval_long(info.vm, "Level");
    }

    if (info.vm.count("Flags")) {
        stTrace.TraceFlags = opval_long(info.vm, "Flags");
    }

    if (info.vm.count("Component")) {
        stTrace.Component = opval_long(info.vm, "Component");
    }

    if (info.vm.count("Length")) {
        stTrace.TraceBufferLength = opval_long(info.vm, "Length");
        if (stTrace.TraceBufferLength < 0 ||
            stTrace.TraceBufferLength > MAXIMUM_TRACE_BUFFER_SIZE) {

            std::cout << "buffer length not in range 0 to "
                << MAXIMUM_TRACE_BUFFER_SIZE << std::endl;
            return 1;
        }
    }

    return DoIoctl(IOCTL_IONIC_CONFIGURE_TRACE, &stTrace, sizeof(stTrace), NULL, 0) != TRUE;
}

command
CmdSetTrace()
{
    command cmd;

    cmd.name = "SetTrace";
    cmd.desc = "Set debug trace parameters";
    cmd.hidden = true;

    cmd.opts = CmdSetTraceOpts;
    cmd.run = CmdSetTraceRun;

    return cmd;
}

//
// -GetTrace
//

static
po::options_description
CmdGetTraceOpts()
{
    po::options_description opts("IonicConfig.exe [-h] GetTrace");

    return opts;
}

static
int
CmdGetTraceRun(command_info& info)
{
    if (info.usage) {
        std::cout << info.cmd.opts() << info.cmd.desc << std::endl;
        return info.status;
    }

    DWORD Size = MAXIMUM_TRACE_BUFFER_SIZE * 1024;
    char *pTraceBuffer = (char *)malloc(Size + 1);

    memset(pTraceBuffer, 0, Size + 1);

    if (!DoIoctl(IOCTL_IONIC_GET_TRACE, NULL, 0, pTraceBuffer, Size)) {
        info.status = 1;
        goto out;
    }

    std::cout << pTraceBuffer << std::endl;

out:
    free(pTraceBuffer);
    return info.status;
}

command
CmdGetTrace()
{
    command cmd;

    cmd.name = "GetTrace";
    cmd.desc = "Get debug trace buffer content";
    cmd.hidden = true;

    cmd.opts = CmdGetTraceOpts;
    cmd.run = CmdGetTraceRun;

    return cmd;
}

//
// TODO: move to Stats.cpp after integration
//

//
// -ResetStats
//

static
po::options_description
CmdResetStatsOpts()
{
    po::options_description opts("IonicConfig.exe [-h] ResetStats");

    return opts;
}

static
int
CmdResetStatsRun(command_info& info)
{
    int status = 0;

    if (info.usage) {
        std::cout << info.cmd.opts() << info.cmd.desc << std::endl;
        return info.status;
    }

    return DoIoctl(IOCTL_IONIC_RESET_STATS, NULL, 0, NULL, 0) != TRUE;
}

command
CmdResetStats()
{
    command cmd;

    cmd.name = "ResetStats";
    cmd.desc = "Reset counters";
    cmd.hidden = true;

    cmd.opts = CmdResetStatsOpts;
    cmd.run = CmdResetStatsRun;

    return cmd;
}

//
// -PortStats
//

static
po::options_description
CmdPortStatsOpts()
{
    po::options_description opts("IonicConfig.exe [-h] PortStats [-p] <port>");

    opts.add_options()
        ("Port,p", optype_long()->required(), "Port number");

    return opts;
}

static
po::positional_options_description
CmdPortStatsPos()
{
    po::positional_options_description pos;

    pos.add("Port", 1);

    return pos;
}

static
int
CmdPortStatsRun(command_info& info)
{
    int status = 0;

    if (info.usage) {
        std::cout << info.cmd.opts() << info.cmd.desc << std::endl;
        return info.status;
    }

    DWORD dwPort = opval_long(info.vm, "Port");

    DWORD Size = 10 * 1024 * 1024;
    char *pStatsBuffer = (char *)malloc(Size);

    if (!DoIoctl(IOCTL_IONIC_GET_PORT_STATS, &dwPort, sizeof(dwPort), pStatsBuffer, Size)) {
        status = 1;
        goto out;
    }

    // Dump* use printf
    std::cout.flush();

    DumpPortStats( dwPort, pStatsBuffer);

out:
    free(pStatsBuffer);
    return status;
}

command
CmdPortStats()
{
    command cmd;

    cmd.name = "PortStats";
    cmd.desc = "Read port mac counters from device";

    cmd.opts = CmdPortStatsOpts;
    cmd.pos = CmdPortStatsPos;
    cmd.run = CmdPortStatsRun;

    return cmd;
}

//
// -LifStats
//

static
po::options_description
CmdLifStatsOpts()
{
    po::options_description opts("IonicConfig.exe [-h] LifStats [-p] <port> [-l] <lif>");

    opts.add_options()
        ("Port,p", optype_long()->required(), "Port number")
        ("Lif,l", optype_long()->required(), "Lif index");

    return opts;
}

static
po::positional_options_description
CmdLifStatsPos()
{
    po::positional_options_description pos;

    pos.add("Port", 1);
    pos.add("Lif", 2);

    return pos;
}

static
int
CmdLifStatsRun(command_info& info)
{
    int status = 0;

    if (info.usage) {
        std::cout << info.cmd.opts() << info.cmd.desc << std::endl;
        return info.status;
    }

    DWORD dwPort = opval_long(info.vm, "Port");
    DWORD dwLif = opval_long(info.vm, "Lif");
    DWORD dwPortLif = (dwPort << 16) | dwLif;

    DWORD Size = 10 * 1024 * 1024;
    char *pStatsBuffer = (char *)malloc(Size);

    if (!DoIoctl(IOCTL_IONIC_GET_LIF_STATS, &dwPortLif, sizeof(dwPortLif), pStatsBuffer, Size)) {
        status = 1;
        goto out;
    }

    // Dump* use printf
    std::cout.flush();

    DumpLifStats(dwLif, pStatsBuffer);

out:
    free(pStatsBuffer);
    return status;
}

command
CmdLifStats()
{
    command cmd;

    cmd.name = "LifStats";
    cmd.desc = "Read interface counters from device";

    cmd.opts = CmdLifStatsOpts;
    cmd.pos = CmdLifStatsPos;
    cmd.run = CmdLifStatsRun;

    return cmd;
}

//
// -MgmtStats
//

static
po::options_description
CmdMgmtStatsOpts()
{
    po::options_description opts("IonicConfig.exe [-h] MgmtStats");

    return opts;
}

static
int
CmdMgmtStatsRun(command_info& info)
{
    int status = 0;

    if (info.usage) {
        std::cout << info.cmd.opts() << info.cmd.desc << std::endl;
        return info.status;
    }

    DWORD Size = 10 * 1024 * 1024;
    char *pStatsBuffer = (char *)malloc(Size);

    if (!DoIoctl(IOCTL_IONIC_GET_LIF_STATS, NULL, 0, pStatsBuffer, Size)) {
        status = 1;
        goto out;
    }

    // Dump* use printf
    std::cout.flush();

    DumpMgmtStats(pStatsBuffer);

out:
    free(pStatsBuffer);
    return status;
}

command
CmdMgmtStats()
{
    command cmd;

    cmd.name = "MgmtStats";
    cmd.desc = "Read management interface counters";
    cmd.hidden = true;

    cmd.opts = CmdMgmtStatsOpts;
    cmd.run = CmdMgmtStatsRun;

    return cmd;
}

//
// -DevStats
//

static
po::options_description
CmdDevStatsOpts()
{
    po::options_description opts("IonicConfig.exe [-h] DevStats");

    return opts;
}

static
int
CmdDevStatsRun(command_info& info)
{
    int status = 0;

    if (info.usage) {
        std::cout << info.cmd.opts() << info.cmd.desc << std::endl;
        return info.status;
    }

    DWORD Size = 10 * 1024 * 1024;
    char *pStatsBuffer = (char *)malloc(Size);

    if (!DoIoctl(IOCTL_IONIC_GET_LIF_STATS, NULL, 0, pStatsBuffer, Size)) {
        status = 1;
        goto out;
    }

    // Dump* use printf
    std::cout.flush();

    DumpDevStats(pStatsBuffer);

out:
    free(pStatsBuffer);
    return status;
}

command
CmdDevStats()
{
    command cmd;

    cmd.name = "DevStats";
    cmd.desc = "Read device counters";
    cmd.hidden = true;

    cmd.opts = CmdDevStatsOpts;
    cmd.run = CmdDevStatsRun;

    return cmd;
}

//
// -PerfStats
//

static
po::options_description
CmdPerfStatsOpts()
{
    po::options_description opts("IonicConfig.exe [-h] PerfStats");

    return opts;
}

static
int
CmdPerfStatsRun(command_info& info)
{
    int status = 0;

    if (info.usage) {
        std::cout << info.cmd.opts() << info.cmd.desc << std::endl;
        return info.status;
    }

    DWORD Size = 10 * 1024 * 1024;
    char *pStatsBuffer = (char *)malloc(Size);

    if (!DoIoctl(IOCTL_IONIC_GET_LIF_STATS, NULL, 0, pStatsBuffer, Size)) {
        status = 1;
        goto out;
    }

    // Dump* use printf
    std::cout.flush();

    DumpPerfStats(pStatsBuffer);

out:
    free(pStatsBuffer);
    return status;
}

command
CmdPerfStats()
{
    command cmd;

    cmd.name = "PerfStats";
    cmd.desc = "Read performance counters";
    cmd.hidden = true;

    cmd.opts = CmdPerfStatsOpts;
    cmd.run = CmdPerfStatsRun;

    return cmd;
}

//
// TODO: move to Registry.cpp after integration
//

//
// -RxBudget
//

static
po::options_description
CmdRxBudgetOpts()
{
    po::options_description opts("IonicConfig.exe [-h] RxBudget [-r] <rxbudget>");

    opts.add_options()
        ("RxBudget,r", optype_long()->required(), "Receive polling budget");

    return opts;
}

static
po::positional_options_description
CmdRxBudgetPos()
{
    po::positional_options_description pos;

    pos.add("RxBudget", 1);

    return pos;
}

static
int
CmdRxBudgetRun(command_info& info)
{
    if (info.usage) {
        std::cout << info.cmd.opts() << info.cmd.desc << std::endl;
        return info.status;
    }

    DWORD dwRxBudget = opval_long(info.vm, "RxBudget");

    return DoIoctl(IOCTL_IONIC_SET_RX_BUDGET, &dwRxBudget, sizeof(dwRxBudget), NULL, 0) != TRUE;
}

command
CmdRxBudget()
{
    command cmd;

    cmd.name = "RxBudget";
    cmd.desc = "Set receive polling budget";

    cmd.opts = CmdRxBudgetOpts;
    cmd.pos = CmdRxBudgetPos;
    cmd.run = CmdRxBudgetRun;

    return cmd;
}

//
// -RegKeys
//

static
po::options_description
CmdRegKeysOpts()
{
    po::options_description opts("IonicConfig.exe [-h] RegKeys");

    return opts;
}

static
int
CmdRegKeysRun(command_info& info)
{
    int status = 0;

    if (info.usage) {
        std::cout << info.cmd.opts() << info.cmd.desc << std::endl;
        return info.status;
    }

    DWORD Size = 10 * 1024 * 1024;
    void *pBuffer = malloc(Size);
	if (pBuffer == NULL) {
		status = 1;
		goto out;
	}

	memset( pBuffer, 0x00, Size);

    if (!DoIoctl(IOCTL_IONIC_GET_REG_KEY_INFO, NULL, 0, pBuffer, Size)) {
        status = 1;
        goto out;
    }

    // Dump* use printf
    std::cout.flush();

    DumpRegKeys( pBuffer, Size);

out:
	if( pBuffer != NULL) {
	    free(pBuffer);
	}
    return status;
}

command
CmdRegKeys()
{
    command cmd;

    cmd.name = "RegKeys";
    cmd.desc = "List registry keys accessible in Advanced Properties";

    cmd.opts = CmdRegKeysOpts;
    cmd.run = CmdRegKeysRun;

    return cmd;
}
