#include "nic/sdk/lib/pal/pal.hpp"
#include "lib/thread/thread.hpp"
#include "asicerrord.h"

using namespace sdk::asic::pd;

delphi::objects::dppintcreditmetrics_t                  dppintcreditmetrics[2];
delphi::objects::dppintfifometrics_t                    dppintfifometrics[2];
delphi::objects::dppintreg1metrics_t                    dppintreg1metrics[2];
delphi::objects::dppintreg2metrics_t                    dppintreg2metrics[2];
delphi::objects::intsparemetrics_t                      intsparemetrics[4];
delphi::objects::dppintsramseccmetrics_t                dppintsramseccmetrics[2];
delphi::objects::dprintcreditmetrics_t                  dprintcreditmetrics[2];
delphi::objects::dprintfifometrics_t                    dprintfifometrics[2];
delphi::objects::dprintflopfifometrics_t                dprintflopfifometrics[2];
delphi::objects::dprintreg1metrics_t                    dprintreg1metrics[2];
delphi::objects::dprintreg2metrics_t                    dprintreg2metrics[2];
delphi::objects::dprintsramseccmetrics_t                dprintsramseccmetrics[2];
delphi::objects::ssepicsintbadaddrmetrics_t             ssepicsintbadaddrmetrics[1];
delphi::objects::ssepicsintbgmetrics_t                  ssepicsintbgmetrics[1];
delphi::objects::ssepicsintpicsmetrics_t                ssepicsintpicsmetrics[1];
delphi::objects::dbwaintdbmetrics_t                     dbwaintdbmetrics[1];
delphi::objects::dbwaintlifqstatemapmetrics_t           dbwaintlifqstatemapmetrics[1];
delphi::objects::sgeteinterrmetrics_t                   sgeteinterrmetrics[6];
delphi::objects::sgeteintinfometrics_t                  sgeteintinfometrics[6];
delphi::objects::sgempuinterrmetrics_t                  sgempuinterrmetrics[6];
delphi::objects::sgempuintinfometrics_t                 sgempuintinfometrics[6];
delphi::objects::mdhensintaxierrmetrics_t               mdhensintaxierrmetrics[1];
delphi::objects::mdhensinteccmetrics_t                  mdhensinteccmetrics[3];
delphi::objects::mdhensintipcoremetrics_t               mdhensintipcoremetrics[1];
delphi::objects::mpmpnsintcryptometrics_t               mpmpnsintcryptometrics[1];
delphi::objects::pbpbcintcreditunderflowmetrics_t       pbpbcintcreditunderflowmetrics[1];
delphi::objects::inteccdescmetrics_t                    inteccdescmetrics[24];
delphi::objects::pbpbcintpbusviolationmetrics_t         pbpbcintpbusviolationmetrics[2];
delphi::objects::pbpbcintrplmetrics_t                   pbpbcintrplmetrics[1];
delphi::objects::pbpbcintwritemetrics_t                 pbpbcintwritemetrics[12];
delphi::objects::pbpbchbmintecchbmrbmetrics_t           pbpbchbmintecchbmrbmetrics[1];
delphi::objects::pbpbchbminthbmaxierrrspmetrics_t       pbpbchbminthbmaxierrrspmetrics[1];
delphi::objects::pbpbchbminthbmdropmetrics_t            pbpbchbminthbmdropmetrics[1];
delphi::objects::pbpbchbminthbmpbusviolationmetrics_t   pbpbchbminthbmpbusviolationmetrics[2];
delphi::objects::pbpbchbminthbmxoffmetrics_t            pbpbchbminthbmxoffmetrics[1];
delphi::objects::mcmchintmcmetrics_t                    mcmchintmcmetrics[8];

const char
*errortostring(etype errortype)
{
    switch (errortype) {
    case ERROR:
        return "Error";
    case INFO:
        return "Info";
    case FATAL:
        return "Fatal";
    default:
        return "Unknown";
    }
}

void
unravel_intr(uint32_t data)
{
    bool iscattrip = false;

    if (data) {
        asicpd_unravel_hbm_intrs(&iscattrip, true);
        if (iscattrip == false) {
            ERR("ECCERR observed on the system.");
        }
    }
}

void createTables() {
    delphi::objects::DppintcreditMetrics::CreateTable();
    delphi::objects::DppintfifoMetrics::CreateTable();
    delphi::objects::Dppintreg1Metrics::CreateTable();
    delphi::objects::Dppintreg2Metrics::CreateTable();
    delphi::objects::IntspareMetrics::CreateTable();
    delphi::objects::DppintsramseccMetrics::CreateTable();
    delphi::objects::DprintcreditMetrics::CreateTable();
    delphi::objects::DprintfifoMetrics::CreateTable();
    delphi::objects::DprintflopfifoMetrics::CreateTable();
    delphi::objects::Dprintreg1Metrics::CreateTable();
    delphi::objects::Dprintreg2Metrics::CreateTable();
    delphi::objects::DprintsramseccMetrics::CreateTable();
    delphi::objects::SsepicsintbadaddrMetrics::CreateTable();
    delphi::objects::SsepicsintbgMetrics::CreateTable();
    delphi::objects::SsepicsintpicsMetrics::CreateTable();
    delphi::objects::DbwaintdbMetrics::CreateTable();
    delphi::objects::DbwaintlifqstatemapMetrics::CreateTable();
    delphi::objects::SgeteinterrMetrics::CreateTable();
    delphi::objects::SgeteintinfoMetrics::CreateTable();
    delphi::objects::SgempuinterrMetrics::CreateTable();
    delphi::objects::SgempuintinfoMetrics::CreateTable();
    delphi::objects::MdhensintaxierrMetrics::CreateTable();
    delphi::objects::MdhensinteccMetrics::CreateTable();
    delphi::objects::MdhensintipcoreMetrics::CreateTable();
    delphi::objects::MpmpnsintcryptoMetrics::CreateTable();
    delphi::objects::PbpbcintcreditunderflowMetrics::CreateTable();
    delphi::objects::InteccdescMetrics::CreateTable();
    delphi::objects::PbpbcintpbusviolationMetrics::CreateTable();
    delphi::objects::PbpbcintrplMetrics::CreateTable();
    delphi::objects::PbpbcintwriteMetrics::CreateTable();
    delphi::objects::PbpbchbmintecchbmrbMetrics::CreateTable();
    delphi::objects::PbpbchbminthbmaxierrrspMetrics::CreateTable();
    delphi::objects::PbpbchbminthbmdropMetrics::CreateTable();
    delphi::objects::PbpbchbminthbmpbusviolationMetrics::CreateTable();
    delphi::objects::PbpbchbminthbmxoffMetrics::CreateTable();
    delphi::objects::McmchintmcMetrics::CreateTable();
}

int
main(int argc, char *argv[])
{
    sdk::lib::logger::init(asicerrord_logger, asicerrord_logger);

    // initialize the pal
#ifdef __x86_64__
    assert(sdk::lib::pal_init(platform_type_t::PLATFORM_TYPE_SIM) == sdk::lib::PAL_RET_OK);
#elif __aarch64__
    assert(sdk::lib::pal_init(platform_type_t::PLATFORM_TYPE_HAPS) == sdk::lib::PAL_RET_OK);
#endif
    createTables();
    while (1) {
        // Dont block context switches, let the process sleep for some time
        sleep(1);
        // Poll for interrupts

        poll_capri_intr();
    }
    return (0);
}
