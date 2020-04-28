import { Component, OnInit, OnDestroy, ViewEncapsulation } from '@angular/core';
import { BaseComponent } from '@app/components/base/base.component';
import { Animations } from '@app/animations';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { ClusterDistributedServiceCard, ClusterDistributedServiceCardStatus_admission_phase_uihint, IClusterDistributedServiceCard, ClusterDSCProfile } from '@sdk/v1/models/generated/cluster';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { HeroCardOptions } from '@app/components/shared/herocard/herocard.component';
import { MetricsUtility } from '@app/common/MetricsUtility';
import { ITelemetry_queryMetricsQueryResponse, Telemetry_queryMetricsResultSeries } from '@sdk/v1/models/telemetry_query';
import { ControllerService } from '@app/services/controller.service';

import { ActivatedRoute, Router } from '@angular/router';
import { ClusterService } from '@app/services/generated/cluster.service';
import { MetricsqueryService, MetricsPollingOptions, TelemetryPollingMetricQueries, MetricsPollingQuery } from '@app/services/metricsquery.service';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { Utility } from '@app/common/Utility';
import { Telemetry_queryMetricsQuerySpec, ITelemetry_queryMetricsQuerySpec, Telemetry_queryMetricsQuerySpec_function, Telemetry_queryMetricsQuerySpec_sort_order, Telemetry_queryMetricsQueryResult } from '@sdk/v1/models/generated/telemetry_query';
import { ITelemetry_queryMetricsQueryResult } from '@sdk/v1/models/telemetry_query';
import { AlertsEventsSelector } from '@app/components/shared/alertsevents/alertsevents.component';
import { StatArrowDirection, CardStates } from '@app/components/shared/basecard/basecard.component';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { LabelEditorMetadataModel } from '@components/shared/labeleditor';
import { WorkloadWorkload } from '@sdk/v1/models/generated/workload';
import { DSCWorkloadsTuple, ObjectsRelationsUtility } from '@app/common/ObjectsRelationsUtility';
import { WorkloadService } from '@app/services/generated/workload.service';
import { NetworkService } from '@app/services/generated/network.service';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';
import { NetworkNetworkInterface, INetworkNetworkInterfaceList, NetworkNetworkInterfaceSpec_type } from '@sdk/v1/models/generated/network';
import { SearchService } from '@app/services/generated/search.service';
import { SearchSearchRequest, IFieldsSelector } from '@sdk/v1/models/generated/search';
import { BrowserService } from '@app/services/generated/browser.service';
import { IBrowserBrowseRequestList, BrowserBrowseRequestList, BrowserBrowseResponseList } from '@sdk/v1/models/generated/browser';
import { SelectItem } from 'primeng/api';

interface IfStat {
  num: number;
  name: string;
  ifName: string;
  status: string;
}

export interface InterfaceTopos {
  // macs or ids of the dsc
  pif: IfStat[];
  // values of the card
  lif: {upper: IfStat[], lower: IfStat[]};
}

export interface InterfaceStats {
  ifname: string;
  shortname: string;
  stats: { status: string, rx: number, tx: number, rxDrop: number, txDrop: number};
}

/**
 * This component displays DSC detail information.
 * For performance's reason,  we employ different tricks
 * 1. ngOnInit (), we fetch DSC id from browser URL.
 * 2. init(), we fetch metrics before invoking getNaplesDetails().
 * 3. getNaplesDetails() -> watch DSC --> use browseDSCWorkload() to find workload associted with this DSC
 * 4. watchWorkload() will update DSC-workloads map in run time.
 *
 * In addition, we try to take advantage of data-cache. (If user first visit DSC-table page, then DSC-detail page, we may have complete list of workloads. See init()
 */

@Component({
  selector: 'app-naplesdetail',
  templateUrl: './naplesdetail.component.html',
  styleUrls: ['./naplesdetail.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class NaplesdetailComponent extends BaseComponent implements OnInit, OnDestroy {
  public static NAPLEDETAIL_FIELD_WORKLOADS: string = 'associatedWorkloads';

  detailLoaded: boolean = false;

  subscriptions = [];

  bodyicon: any = {
    margin: {
      top: '9px',
      left: '8px'
    },
    url: '/assets/images/icons/cluster/nodes/ico-node-black.svg'
  };

  cardColor = '#61b3a0';
  cpuColor: string = '#b592e3';
  memoryColor: string = '#e57553';
  storageColor: string = '#70bab4';
  cardIcon: Icon = {
    margin: {
      top: '10px',
      left: '10px'
    },
    svgIcon: 'naples'
  };
  // Id of the object the user has navigated to
  selectedId: string;
  selectedObj: Readonly<ClusterDistributedServiceCard>;

  showExpandedDetailsCard: boolean;

  // Holds all objects, should be only one item in the array
  objList: ReadonlyArray<ClusterDistributedServiceCard>;
  objEventUtility: HttpEventUtility<ClusterDistributedServiceCard>;

  // Whether we show a deletion overlay
  showDeletionScreen: boolean;

  // Whether we show a missing overlay
  showMissingScreen: boolean;

  lastUpdateTime: string = '';

  cpuChartData: HeroCardOptions = MetricsUtility.detailLevelCPUHeroCard(this.cpuColor, this.cardIcon);

  memChartData: HeroCardOptions = MetricsUtility.detailLevelMemHeroCard(this.memoryColor, this.cardIcon);

  diskChartData: HeroCardOptions = MetricsUtility.detailLevelDiskHeroCard(this.storageColor, this.cardIcon);

  admissionPhaseEnum = ClusterDistributedServiceCardStatus_admission_phase_uihint;

  heroCards = [
    this.cpuChartData,
    this.memChartData,
    this.diskChartData
  ];

  timeSeriesData: ITelemetry_queryMetricsQueryResult;
  avgData: ITelemetry_queryMetricsQueryResult;
  avgDayData: ITelemetry_queryMetricsQueryResult;
  clusterAvgData: ITelemetry_queryMetricsQueryResult;

  telemetryKind: string = 'DistributedServiceCard';

  alertseventsSelector: AlertsEventsSelector;

  inLabelEditMode: boolean = false;

  labelEditorMetaData: LabelEditorMetadataModel;

  workloadEventUtility: HttpEventUtility<WorkloadWorkload>;
  dscsWorkloadsTuple: { [dscKey: string]: DSCWorkloadsTuple; };

  networkInterfacesEventUtility: HttpEventUtility<NetworkNetworkInterface>;
  networkInterfaces: NetworkNetworkInterface[] = [];
  allNetworkInterfaces: NetworkNetworkInterface[] = [];
  // these stats are form backend
  lifInterfacePollingStats: ITelemetry_queryMetricsQueryResult = null;
  pifInterfacePollingStats: ITelemetry_queryMetricsQueryResult = null;
  // the next 3 stats are for gui display
  interfaceTopos: InterfaceTopos;
  interfaceStats: InterfaceStats[] = [];
  interfaceStatsUpdateTime: string = '';
  mouseOverInterface: string = '';
  showInterfaceTable: boolean = false;

  asicFrequency: string = '';

  inProfileAssigningMode: boolean = false;


  dscprofiles: ReadonlyArray<ClusterDSCProfile> = [];
  dscprofileOptions: SelectItem[] = [];

  constructor(protected _controllerService: ControllerService,
    private _route: ActivatedRoute,
    private _router: Router,
    protected clusterService: ClusterService,
    protected metricsqueryService: MetricsqueryService,
    protected uionfigService: UIConfigsService,
    protected workloadService: WorkloadService,
    protected networkService: NetworkService,
    protected searchService: SearchService,
    protected browserService: BrowserService
  ) {
    super(_controllerService, uionfigService);
  }

  /**
   * There are 2 ways for this component to be initialized, based on route /cluster/dscs/{id} and query param ?action=lookup.
   * On init, the component subscribes to changes in params {id}.
   * This can happen if user is on a DSC details page and clicks on a link (e.g. alert list item) to another DSC's details page.
   * If the {id} changes, we unsub all subscriptions and re-initialize everything.
   * Initialization flows:
   * 1. Normally, {id} is the mac address and the component can just go ahead and make API requests and initialize the page
   * 2. Sometimes, like in alert list items, the DSC name is passed in for {id}. In those situations, query param ?action=lookup
   *    is added to the URL. If the component sees that query param, then a lookup in searchDsc() is done to get the DSC object.
   *    The "name" is taken from the returned object, which is used to reroute the application to /cluster/dscs/{id}, bringing the
   *    component through flow 1.
   */
  ngOnInit() {
    this._controllerService.publish(Eventtypes.COMPONENT_INIT, { 'component': 'NapledetailComponent', 'state': Eventtypes.COMPONENT_INIT });
    this._route.paramMap.subscribe(params => {
      this.selectedId = params.get('id');
      this.subscriptions.forEach(subscription => {
        subscription.unsubscribe();
      });
      const queryAction = this._route.snapshot.queryParamMap.get('action');
      if (queryAction === 'lookup') {
        this.searchDsc();
      } else {
        this.init();
      }
    });
  }

  init() {
    this.initializeData();
    this.getNetworkInterfaces();
    this.getNaplesDetails();
    this.startInterfaceStatsPoll();
    this.setNapleDetailToolbar(this.selectedId); // Build the toolbar with naple.id first. Toolbar will be over-written when naple object is available.
  }

  searchDsc() {
    const query: SearchSearchRequest = new SearchSearchRequest({
      aggregate: false,
      query: {
        kinds: ['DistributedServiceCard'],
        fields: {
          requirements: [
            {
              key: 'spec.id.keyword',
              operator: 'equals',
              values: [this.selectedId]
            }
          ]
        }
      }
    });
    this.searchService.PostQuery(query).subscribe(
      resp => {
        const body = resp.body as any;
        const entries = body.entries || [];
        const entry = entries[0];

        if (!entry) {
          this.init();
        } else {
          const name = entry.object.meta.name;
          this._router.navigateByUrl(`cluster/dscs/${name}`);
        }
      },
      (error) => {
        this._controllerService.invokeRESTErrorToaster('Failed to search DSC ' + this.selectedId, error);
      },
      () => this.init()  // VS-1432.  If this.selectedId is not a valid DSC name, NaplesDatail will present DSC not found
    );
  }

  /**
   * This function find all NetworkInterfaces (NIs) belong to this.selectedObj (currrent DSC)
   */
  getThisDSCNetworkInterfaces() {
    if (this.selectedObj && this.allNetworkInterfaces) {
      const list: NetworkNetworkInterface[] = [];
      this.allNetworkInterfaces.forEach((networkNetworkInterface: NetworkNetworkInterface) => {
        // changed the next condition line for the backwards support issues
        // there are duplicated interfaces for example 00aecd000009-uplink-1-2 and
        // 00ae.cd00.0009-uplink-1-2
        // if (this.selectedObj && networkNetworkInterface.status.dsc === this.selectedObj.meta.name) {
        if (networkNetworkInterface.meta.name.startsWith(this.selectedObj.meta.name) &&
            !Utility.isInterfaceInValid(networkNetworkInterface)) {
          list.push(networkNetworkInterface);
        }
      });
      this.networkInterfaces = list;
      this.processInterfaceData();
      this.updateInterfacesStats();
    }
  }

  /**
   * Retrieve network interfaces objects ( whole list of NIs)
   */
  getNetworkInterfaces() {
    if (this.uiconfigsService.isAuthorized(UIRolePermissions.networknetworkinterface_read)) {
      const dscSubscription = this.networkService.ListNetworkInterfaceCache().subscribe(
        (response) => {
          if (response.connIsErrorState) {
            return;
          }
          this.allNetworkInterfaces = response.data as NetworkNetworkInterface[];
          if (this.allNetworkInterfaces && this.allNetworkInterfaces.length > 0) {
            this.getThisDSCNetworkInterfaces();  // find all NIs belong to current DSC
          }
        }
      );
      this.subscriptions.push(dscSubscription);
    }
  }

  processInterfaceData() {
    if (this.networkInterfaces && this.networkInterfaces.length > 0) {
      this.interfaceTopos = {
        pif: [],
        lif: {
          upper: [],
          lower: []
        }
      };
      const pifs: IfStat[] = [];
      this.networkInterfaces.forEach((itf: NetworkNetworkInterface) => {
        if (itf.spec && itf.spec.type === NetworkNetworkInterfaceSpec_type['uplink-eth']) {
          const arr: string[] = itf.meta.name.split('-uplink-1-');
          const num = arr[arr.length - 1];
          pifs.push({
            num: parseInt(num, 10),
            name: itf.meta.name,
            ifName: 'uplink-1-' + num,
            status: itf.spec['admin-status'] + '/' + itf.status['oper-status']
          });
        }
      });
      pifs.sort((a, b) => (a.name > b.name) ? 1 : -1);
      this.interfaceTopos.pif = pifs;

      const lifs: IfStat[] = [];
      this.networkInterfaces.forEach((itf: NetworkNetworkInterface) => {
        if (itf.spec && itf.spec.type === 'host-pf') {
          const arr: string[] = itf.meta.name.split('-');
          const num = arr[arr.length - 1];
          lifs.push({
            num: parseInt(num, 10),
            name: itf.meta.name,
            ifName: 'pf-' + num,
            status: itf.spec['admin-status'] + '/' + itf.status['oper-status']
          });
        }
      });
      lifs.sort((a, b) => (a.name > b.name) ? 1 : -1);
      lifs.forEach((itf, index) => {
        if (lifs.length > 8 && index % 2 === 1) {
          this.interfaceTopos.lif.lower.push(itf);
        } else {
          this.interfaceTopos.lif.upper.push(itf);
        }
      });

      // remove none existent intface stats
      this.interfaceStats = this.interfaceStats.filter((ifs: InterfaceStats) =>
        this.networkInterfaces.find((itf: NetworkNetworkInterface) =>
          itf.meta.name === ifs.ifname
        ) != null
      );

      this.processInterfacesStats(this.interfaceTopos.pif);
      this.processInterfacesStats(this.interfaceTopos.lif.upper);
      this.processInterfacesStats(this.interfaceTopos.lif.lower);

      if (this.interfaceTopos.pif.length > 0) {
        this.mouseOverInterface = this.interfaceTopos.pif[0].name;
      } else if (this.interfaceTopos.lif.upper.length > 0) {
        this.mouseOverInterface = this.interfaceTopos.lif.upper[0].name;
      }
    }
  }

  updateInterfacesStats() {
    const lifStats = this.lifInterfacePollingStats;
    const pifStats = this.pifInterfacePollingStats;
    const interfaceStats = this.interfaceStats;
    if (pifStats && pifStats.series && pifStats.series.length > 0 && interfaceStats.length > 0) {
      this.interfaceStatsUpdateTime = pifStats.series[0].values[0][0];
      pifStats.series.forEach(item => {
        const itf = this.interfaceStats.find(interf => {
          const tags = item.tags as any;
          const tagsName = tags.name;
          return interf.shortname === tagsName.substring(tagsName.length - 10);
        });
        if (itf) {
          itf.stats.rx = item.values[0][1];
          itf.stats.tx = item.values[0][2];
          itf.stats.rxDrop = -1000;
          itf.stats.txDrop = -1000;
        }
      });
    }
    if (lifStats && lifStats.series && lifStats.series.length > 0 && interfaceStats.length > 0) {
      if (!this.interfaceStatsUpdateTime) {
        this.interfaceStatsUpdateTime = lifStats.series[0].values[0][0];
      }
      lifStats.series.forEach(item => {
        const itf = this.interfaceStats.find(interf => {
          const tags = item.tags as any;
          return interf.ifname === tags.name;
        });
        if (itf) {
          itf.stats.rx = item.values[0][7];
          itf.stats.tx = item.values[0][8];
          itf.stats.rxDrop = item.values[0][1] + item.values[0][2] + item.values[0][3];
          itf.stats.txDrop = item.values[0][4] + item.values[0][5] + item.values[0][6];
        }
      });
    }
    // if current selected interfac having not stats, then  move to one having stats
    const selectedInterfaceStats: InterfaceStats = this.interfaceStats.find(
        (inf: InterfaceStats) => inf.ifname === this.mouseOverInterface);
    if (selectedInterfaceStats && selectedInterfaceStats.stats.rx === -1) {
      // find first available interface stats having value;
      const avialInf = this.interfaceStats.find(
          (inf: InterfaceStats) => inf.stats.rx !== -1);
      if (avialInf) {
        this.mouseOverInterface = avialInf.ifname;
      }
    }
  }

  processInterfacesStats(itfs: IfStat[]) {
    itfs.forEach(itf => {
      if (!this.interfaceStats.find((ifs: InterfaceStats) => ifs.ifname === itf.name)) {
        this.interfaceStats.push({
          ifname: itf.name,
          shortname: itf.ifName,
          stats: {
            status: itf.status,
            rx: -1,
            tx: -1,
            rxDrop: -1,
            txDrop: -1
          }
        });
      }
    });
  }

  getDSCProfiles() {
    const subscription = this.clusterService.ListDSCProfileCache().subscribe(
      response => {
        if (response.connIsErrorState) {
          return;
        }
        this.dscprofiles = response.data;
        this.processDSCProfiles(); // process DSCProfile. Note: At this this moment, this.selectedObj may not be available
      },
      this._controllerService.webSocketErrorHandler('Failed to get DSC Profile')
    );
    this.subscriptions.push(subscription); // add subscription to list, so that it will be cleaned up when component is destroyed.
  }

  private processDSCProfiles() {
    if (this.dscprofiles && this.dscprofiles.length > 0) {
      this.dscprofileOptions = [];
      this.dscprofiles.forEach((dscprofile: ClusterDSCProfile) => {
        if (!(this.selectedObj && this.selectedObj.spec.dscprofile === dscprofile.meta.name)) {
          const obj: SelectItem = {
            value: dscprofile.meta.name,
            label: dscprofile.meta.name
          };
          this.dscprofileOptions.push(obj);
        }
      });
    }
  }

  setNapleDetailToolbar(id: string) {
    const nicName = (this.selectedObj) ? this.selectedObj.spec.id : id;
    this._controllerService.setToolbarData({
      buttons: [],
      breadcrumb: [
        { label: 'Distributed Services Cards', url: Utility.getBaseUIUrl() + 'cluster/dscs' },
        { label: nicName, url: Utility.getBaseUIUrl() + 'cluster/dscs/' + id }
      ]
    });
  }

  initializeData() {
    // Initializing variables so that state is cleared between routing of different
    // sgpolicies
    // Ex. /cluster/naples/naples1-> /cluster/naples/naples2
    this.subscriptions.forEach(sub => {
      sub.unsubscribe();
    });
    this.subscriptions = [];
    this.objList = [];
    this.showDeletionScreen = false;
    this.showMissingScreen = false;
    this.timeSeriesData = null;
    this.avgData = null;
    this.avgDayData = null;
    this.clusterAvgData = null;
    this.selectedObj = null;
    this.alertseventsSelector = null;
    this.showExpandedDetailsCard = true;

    this.heroCards.forEach((card) => {
      card.firstStat.value = null;
      card.secondStat.value = null;
      card.thirdStat.value = null;
      card.lineData.data = [];
      card.cardState = CardStates.LOADING;
    });
  }

  /**
   *
  */
  browseDSCWorkload() {
    const hostname = this.selectedObj.status.host;
    const list = [];
    const obj = {
      'uri': '/configs/cluster/v1/hosts/' + hostname,
      'query-type': 'depended-by',
      'max-depth': 2,
      'count-only': null
    };
    list.push(obj);
    const body: BrowserBrowseRequestList = new BrowserBrowseRequestList(
      {
        'requestlist': list,
      }
    );
    this.browserService.PostQuery(body).subscribe(
      (response) => {
        const browserBrowseResponseList = response.body as BrowserBrowseResponseList;
        // browserBrowseResponseList looks like:
        // {"kind":"","meta":{"name":"","generation-id":"","creation-time":"","mod-time":""},"responselist":[{"root-uri":"/configs/cluster/v1/hosts/naples3-host","query-type":"depended-by","max-depth":2,"total-count":2,"objects":{"/configs/cluster/v1/hosts/naples3-host":{"kind":"Host","api-version":"v1","meta":{"name":"naples3-host","generation-id":"7","resource-version":"447663","uuid":"3e41523f-80ec-4f3a-97b3-18ac71ae654d","creation-time":"2020-01-16T22:56:04.310139027Z","mod-time":"2020-01-16T22:57:51.494345219Z","self-link":"/configs/cluster/v1/hosts/naples3-host"},"uri":"/configs/cluster/v1/hosts/naples3-host","reverse":"","query-type":"depended-by","links":{"spec.host-name":{"ref-type":"named-reference","uri":[{"tenant":"default","namespace":"default","kind":"Workload","name":"Saroj","uri":"/configs/workload/v1/tenant/default/workloads/Saroj"}]}}},"/configs/workload/v1/tenant/default/workloads/Saroj":{"kind":"Workload","api-version":"v1","meta":{"name":"Saroj","tenant":"default","namespace":"default","generation-id":"1","resource-version":"447663","uuid":"33e90343-56b2-4138-8734-e831e1d9ab26","creation-time":"2020-01-19T11:10:48.221308018Z","mod-time":"2020-01-19T11:10:48.22131401Z","self-link":"/configs/workload/v1/tenant/default/workloads/Saroj"},"uri":"/configs/workload/v1/tenant/default/workloads/Saroj","reverse":"","query-type":"depended-by","links":null}}}]}

        const hostWorkloadMap = {};
        for (let i = 0; browserBrowseResponseList && browserBrowseResponseList.responselist && i < browserBrowseResponseList.responselist.length; i++) {
          const responselistObject = browserBrowseResponseList.responselist[i].objects;
          const keys = Object.keys(responselistObject);
          let hostNameInList: string;
          for (let j = 0; j < keys.length; j++) {
            const objItem = responselistObject[keys[j]];
            if (objItem.uri === browserBrowseResponseList.responselist[i]['root-uri'] && objItem.kind === 'Host') {
              hostNameInList = objItem.meta.name;
              hostWorkloadMap[hostNameInList] = [];
            } else if (objItem.kind === 'Workload') {
              hostWorkloadMap[hostNameInList].push(objItem);
            }
          }
        }
        this.selectedObj[NaplesdetailComponent.NAPLEDETAIL_FIELD_WORKLOADS] = hostWorkloadMap[hostname];
      },
      (error) => {
        this._controllerService.invokeRESTErrorToaster('Failed to invoke browser api', error);
      }
    );
  }


  getNaplesDetails() {
    // We perform a get as well as a watch so that we can know if the object the user is
    // looking for exists or not.
    // Adding ':' as a temporary workaround of ApiGw not being able to
    // correctly parse smartNic names without it.
    const getSubscription = this.clusterService.GetDistributedServiceCard(this.selectedId + ':').subscribe(
      response => {
        this.detailLoaded = true;
      },
      error => {
        // If we receive any error code we display object is missing
        // TODO: Update to be more descriptive based on error message
        this.showMissingScreen = true;
        this.heroCards.forEach(card => {
          card.cardState = CardStates.READY;
        });
      }
    );
    this.subscriptions.push(getSubscription);
    this.objEventUtility = new HttpEventUtility<ClusterDistributedServiceCard>();
    this.objList = this.objEventUtility.array;
    const subscription = this.clusterService.WatchDistributedServiceCard({ 'field-selector': 'meta.name=' + this.selectedId }).subscribe(
      response => {
        this.objEventUtility.processEvents(response);
        if (this.objList.length > 1) {
          // because of the name selector, we should
          // have only got one object
          console.error(
            'Received more than one naples object. Expected '
            + this.selectedId + ', received ' +
            this.objList.map((naples) => naples.meta.name).join(', '));
        }
        if (this.selectedObj != null && this.objList.length > 0) {
          this.selectedObj = new ClusterDistributedServiceCard(this.objList[0]);
        } else if (this.selectedObj == null && this.objList.length > 0) {
          // In case object was deleted and then readded while we are on the same screen
          this.showDeletionScreen = false;
          // In case object wasn't created yet and then was added while we are on the same screen
          this.showMissingScreen = false;
          this.selectedObj = new ClusterDistributedServiceCard(this.objList[0]);
          this.alertseventsSelector = {
            eventSelector: {
              selector: 'object-ref.name=' + this.selectedObj.spec.id + ',object-ref.kind=DistributedServiceCard',
              name: this.selectedObj.spec.id,
            },
            alertSelector: {
              selector: 'status.object-ref.name=' + this.selectedObj.spec.id + ',status.object-ref.kind=DistributedServiceCard',
              name: this.selectedObj.spec.id
            }
          };
          this.startMetricPolls();
          this.setNapleDetailToolbar(this.selectedId); // VS-668. We have to wait for the Naple object.
        } else if (this.objList.length === 0) {
          // Must have received a delete event.
          this.showDeletionScreen = true;
          this.heroCards.forEach(card => {
            card.cardState = CardStates.READY;
          });
          this.selectedObj = null;
        }
        if (this.selectedObj) {
          // When we first get the DSC object. We try to user browser API to populate DSC's workload array.
          if (!this.selectedObj[NaplesdetailComponent.NAPLEDETAIL_FIELD_WORKLOADS] || this.selectedObj[NaplesdetailComponent.NAPLEDETAIL_FIELD_WORKLOADS].length === 0) {
            this.browseDSCWorkload();
          }
          // Wait until we got DSC object, then fetch DSC-profile and networkinterfaces or this DSC.
          // Since we use ListNetworkInterface()
          this.getDSCProfiles();
          this.getThisDSCNetworkInterfaces();
        }
      },
      this._controllerService.webSocketErrorHandler('Failed to get NAPLES')
    );
    this.subscriptions.push(subscription);
  }

  startInterfaceStatsPoll() {
    if (this.selectedId) {
      const queryList: TelemetryPollingMetricQueries = {
        queries: [],
        tenant: Utility.getInstance().getTenant()
      };
      const query1: MetricsPollingQuery = this.topologyInterfaceQuery(
        'LifMetrics', ['RxDropBroadcastBytes', 'RxDropMulticastBytes', 'RxDropUnicastBytes',
        'TxDropBroadcastBytes', 'TxDropMulticastBytes', 'TxDropUnicastBytes', 'RxTotalBytes', 'TxTotalBytes'],
        MetricsUtility.createReporterIDSelector(this.selectedId));
      queryList.queries.push(query1);

      const query2: MetricsPollingQuery = this.topologyInterfaceQuery(
        'MacMetrics', ['OctetsRxOk', 'OctetsTxOk'],
        MetricsUtility.createReporterIDSelector(this.selectedId));
      queryList.queries.push(query2);

      const query3: MetricsPollingQuery = this.topologyInterfaceQuery(
        'AsicFrequencyMetrics', ['Frequency'],
        MetricsUtility.createReporterIDSelector(this.selectedId));
      queryList.queries.push(query3);

      // this is the example of inetrface stats resutl
      /*
         {"tenant":"default","results":[{"statement_id":0,"series":[{"name":"LifMetrics",
         "tags":{"name":"00ae.cd01.0ed8-pf-70"},"columns":["time","last","last_1","last_2",
         "last_3","last_4","last_5","last_6","last_7"],"values":[["2020-04-24T06:26:33.000000001Z",
         1411652,26316,778696198369,24283200,0,849048307,27563196641199,17813711841860]]},
         {"name":"LifMetrics","tags":{"name":"00ae.cd01.0ed8-pf-69"},"columns":["time","last",
         "last_1","last_2","last_3","last_4","last_5","last_6","last_7"],"values":[
           ["2020-04-24T06:26:33.000000001Z",1422183,37678,835038121918,7353088,0,1508136395,67235370456757,
           30693719401173]]}]},{"statement_id":1,"series":[{"name":"MacMetrics","tags":{"name":
           "00ae.cd01.0ed8-00ae.cd01.0ed8-uplink-1-2"},"columns":["time","last","last_1"],"values":[
             ["2020-04-24T06:26:33.000000001Z",68674314086022,2743570971019
      */

      // refresh every 5 minutes
      const sub = this.metricsqueryService.pollMetrics('topologyInterfaces', queryList, MetricsUtility.FIVE_MINUTES).subscribe(
        (data: ITelemetry_queryMetricsQueryResponse) => {
          if (data && data.results && data.results.length === queryList.queries.length) {
            this.lifInterfacePollingStats = data.results[0];
            this.pifInterfacePollingStats = data.results[1];
            const asiqFreqStats: ITelemetry_queryMetricsQueryResult = data.results[2];
            if (asiqFreqStats && asiqFreqStats.series &&  asiqFreqStats.series.length > 0) {
              const series = asiqFreqStats.series;
              if (series[0].values && series[0].values.length > 0) {
                this.asicFrequency = series[0].values[0][1] + ' MHz';
              }
            }
          }
        },
        (err) => {
          this._controllerService.invokeErrorToaster('Error', 'Failed to load interface metrics.');
        }
      );
      this.subscriptions.push(sub);
    }
  }

  topologyInterfaceQuery(kind: string, fields: string[],
      selector: IFieldsSelector = null): MetricsPollingQuery {
    const query: ITelemetry_queryMetricsQuerySpec = {
      'kind': kind,
      'name': null,
      'selector': selector,
      'function': Telemetry_queryMetricsQuerySpec_function.last,
      'group-by-field': 'name',
      'group-by-time': null,
      'fields': fields != null ? fields : [],
      'sort-order': Telemetry_queryMetricsQuerySpec_sort_order.descending,
      'start-time': 'now() - 3m' as any,
      'end-time': 'now()' as any,
    };
    return { query: new Telemetry_queryMetricsQuerySpec(query), pollingOptions: {} };
  }

  startMetricPolls() {
    const queryList: TelemetryPollingMetricQueries = {
      queries: [],
      tenant: Utility.getInstance().getTenant()
    };
    queryList.queries.push(this.timeSeriesQuery());
    queryList.queries.push(this.avgQuery());
    queryList.queries.push(this.avgDayQuery());
    queryList.queries.push(this.clusterAvgQuery());
    const sub = this.metricsqueryService.pollMetrics('naplesDetailCards', queryList).subscribe(
      (data: ITelemetry_queryMetricsQueryResponse) => {
        if (data && data.results && data.results.length === 4) {
          this.timeSeriesData = data.results[0];
          this.avgData = data.results[1];
          this.avgDayData = data.results[2];
          this.clusterAvgData = data.results[3];
          this.lastUpdateTime = new Date().toISOString();
          this.tryGenCharts();
        }
      },
      (err) => {
        MetricsUtility.setCardStatesFailed(this.heroCards);
      }
    );
    this.subscriptions.push(sub);
  }

  timeSeriesQuery(): MetricsPollingQuery {
    let name = this.selectedId;
    if (this.selectedObj != null) {
      name = MetricsUtility.generateNaplesReporterId(this.selectedObj);
    }
    return MetricsUtility.timeSeriesQueryPolling(this.telemetryKind, ['CPUUsedPercent', 'MemUsedPercent', 'DiskUsedPercent'], MetricsUtility.createNameSelector(name));
  }

  avgQuery(): MetricsPollingQuery {
    let name = this.selectedId;
    if (this.selectedObj != null) {
      name = MetricsUtility.generateNaplesReporterId(this.selectedObj);
    }
    return MetricsUtility.pastFiveMinAverageQueryPolling(this.telemetryKind, MetricsUtility.createNameSelector(name));
  }

  avgDayQuery(): MetricsPollingQuery {
    let name = this.selectedId;
    if (this.selectedObj != null) {
      name = MetricsUtility.generateNaplesReporterId(this.selectedObj);
    }
    return MetricsUtility.pastDayAverageQueryPolling(this.telemetryKind, MetricsUtility.createNameSelector(name));
  }

  clusterAvgQuery(): MetricsPollingQuery {
    const clusterAvgQuery: ITelemetry_queryMetricsQuerySpec = {
      'kind': this.telemetryKind,
      name: null,
      function: Telemetry_queryMetricsQuerySpec_function.mean,
      'sort-order': Telemetry_queryMetricsQuerySpec_sort_order.ascending,
      // We don't specify the fields we need, as specifying more than one field
      // while using the average function isn't supported by the backend.
      // Instead we leave blank and get all fields
      fields: [],
      // We only look at the last 5 min bucket so that the max node reporting is never
      // lower than current
      'start-time': new Date(Utility.roundDownTime(5).getTime() - 1000 * 50 * 5).toISOString() as any,
      'end-time': Utility.roundDownTime(5).toISOString() as any
    };

    const query = new Telemetry_queryMetricsQuerySpec(clusterAvgQuery);
    const timeUpdate = (queryBody: ITelemetry_queryMetricsQuerySpec) => {
      queryBody['start-time'] = new Date(Utility.roundDownTime(5).getTime() - 1000 * 50 * 5).toISOString() as any,
        queryBody['end-time'] = Utility.roundDownTime(5).toISOString() as any;
    };

    const merge = (currData, newData) => {
      // since we round down to get the last 5 min bucket, there's a chance
      // that we can back null data, since no new metrics have been reported.
      if (!MetricsUtility.resultHasData(newData)) {
        // no new data, keep old value
        return currData;
      }
      return newData;
    };

    const pollOptions = {
      timeUpdater: timeUpdate,
      mergeFunction: merge
    };

    return { query: query, pollingOptions: pollOptions };
  }

  private tryGenCharts() {
    // Only require avg 5 min data and avg day data
    // before we show the cards
    if (MetricsUtility.resultHasData(this.avgData) &&
      MetricsUtility.resultHasData(this.avgDayData)) {
      this.genCharts('CPUUsedPercent', this.cpuChartData);
      this.genCharts('MemUsedPercent', this.memChartData);
      this.genCharts('DiskUsedPercent', this.diskChartData);
    } else {
      MetricsUtility.setCardStatesNoData(this.heroCards);
    }
  }

  genCharts(fieldName, heroCard: HeroCardOptions) {
    // Time series graph
    if (MetricsUtility.resultHasData(this.timeSeriesData)) {
      const timeSeriesData = this.timeSeriesData;

      const data = MetricsUtility.transformToChartjsTimeSeries(timeSeriesData.series[0], fieldName);
      heroCard.lineData.data = data;
    }

    // Current stat calculation - we take the last point
    if (MetricsUtility.resultHasData(this.avgData)) {
      const index = MetricsUtility.findFieldIndex(this.avgData.series[0].columns, fieldName);
      heroCard.firstStat.numericValue = Math.round(this.avgData.series[0].values[0][index]);
      heroCard.firstStat.value = Math.round(this.avgData.series[0].values[0][index]) + '%';
    }

    // Avg
    const avgDayData = this.avgDayData;
    if (avgDayData.series[0].values.length !== 0) {
      const index = MetricsUtility.findFieldIndex(this.avgDayData.series[0].columns, fieldName);
      heroCard.secondStat.numericValue = Math.round(avgDayData.series[0].values[0][index]);
      heroCard.secondStat.value = Math.round(avgDayData.series[0].values[0][index]) + '%';
    }

    // For determining arrow direction, we compare the current value to the average value
    if (heroCard.firstStat == null || heroCard.firstStat.value == null || heroCard.secondStat == null || heroCard.secondStat.value == null) {
      heroCard.arrowDirection = StatArrowDirection.HIDDEN;
    } else if (heroCard.firstStat.value > heroCard.secondStat.value) {
      heroCard.arrowDirection = StatArrowDirection.UP;
    } else if (heroCard.firstStat.value < heroCard.secondStat.value) {
      heroCard.arrowDirection = StatArrowDirection.DOWN;
    } else {
      heroCard.arrowDirection = StatArrowDirection.HIDDEN;
    }

    // Cluster average
    if (this.clusterAvgData && this.clusterAvgData.series && this.clusterAvgData.series.length > 0 &&
        this.clusterAvgData.series[0].values.length !== 0) {
      const index = MetricsUtility.findFieldIndex(this.clusterAvgData.series[0].columns, fieldName);
      heroCard.thirdStat.numericValue = Math.round(this.clusterAvgData.series[0].values[0][index]);
      heroCard.thirdStat.value = Math.round(this.clusterAvgData.series[0].values[0][index]) + '%';
    }

    if (heroCard.cardState !== CardStates.READY) {
      heroCard.cardState = CardStates.READY;
    }
  }

  /**
   * Component is about to exit
   */
  ngOnDestroy() {
    // publish event that AppComponent is about to be destroyed
    this._controllerService.publish(Eventtypes.COMPONENT_DESTROY, { 'component': 'NaplesdetailComponent', 'state': Eventtypes.COMPONENT_DESTROY });
    this.subscriptions.forEach(subscription => {
      subscription.unsubscribe();
    });
  }

  editLabels($event) {
    this.labelEditorMetaData = {
      title: this.selectedObj.meta.name,
      keysEditable: true,
      valuesEditable: true,
      propsDeletable: true,
      extendable: true,
      save: true,
      cancel: true,
    };

    if (!this.inLabelEditMode) {
      this.inLabelEditMode = true;
    }
  }

  handleEditCancel($event) {
    this.inLabelEditMode = false;
  }

  handleEditSave(dscs: ClusterDistributedServiceCard[]) {
    if (dscs.length > 0) {
      const naple = dscs[0];
      const dscNaple = naple.meta.name;
      if (naple[NaplesdetailComponent.NAPLEDETAIL_FIELD_WORKLOADS]) {
        delete naple[NaplesdetailComponent.NAPLEDETAIL_FIELD_WORKLOADS]; // remove UI field
      }
      const sub = this.clusterService.UpdateDistributedServiceCard(dscNaple, dscs[0], '', this.objList[0]).subscribe(
        response => {
          this._controllerService.invokeSuccessToaster(Utility.UPDATE_SUCCESS_SUMMARY, `Successfully updated ${dscNaple}'s labels`);
        },
        this._controllerService.restErrorHandler(Utility.UPDATE_FAILED_SUMMARY)
      );
      this.subscriptions.push(sub);
      this.inLabelEditMode = false;
    }
  }

  /**
   * TODO: have a local variable storing this to avoid repetitive computation
   */
  genKeys(): string[] {
    return Object.keys(this.selectedObj.meta.labels);
  }

  ifLabelExists(): boolean {
    if (this.selectedObj) {
      if (Utility.getLodash().isEmpty(this.selectedObj.meta.labels)) {
        return false;
      } else {
        return true;
      }
    }
    return false;
  }

  toggleDetailsCard() {
    this.showExpandedDetailsCard = !this.showExpandedDetailsCard;
  }

  helpDisplayCondition(data: Readonly<ClusterDistributedServiceCard>): string {
    return Utility.getNaplesCondition(data);
  }

  helpDisplayReasons(data: Readonly<ClusterDistributedServiceCard>): any {
    return Utility.displayReasons(data);
  }

  isNICHealthy(data: Readonly<ClusterDistributedServiceCard>): boolean {
    return Utility.isNaplesNICHealthy(data);
  }

  isNICNotAdmitted(data: Readonly<ClusterDistributedServiceCard>): boolean {
    return Utility.isNICConditionNotAdmitted(data);
  }

  // update dsc profile

  assignDSCProfile() {
    this.inProfileAssigningMode = !this.inProfileAssigningMode;
  }

  /**
   * Update DSC with new DSC-profile name
   * Only if DSC's existing profile is different from the newly chosen one.
   */
  handleSetDSCProfileSave(option: SelectItem) {
    const naplesObject = new ClusterDistributedServiceCard(this.selectedObj);
    const name = naplesObject.meta.name;
    if (naplesObject.spec.dscprofile !== option.value) {
      naplesObject.spec.dscprofile = option.value;
      // udpdate DSC without trimming
      const sub = this.clusterService.UpdateDistributedServiceCard(name, naplesObject, '', this.selectedObj, false).subscribe(
        response => {
          this._controllerService.invokeSuccessToaster(Utility.UPDATE_SUCCESS_SUMMARY, `Successfully updated ${name}'s profile`);
          this.inProfileAssigningMode = false;
        },
        error => {
          this._controllerService.invokeRESTErrorToaster(Utility.UPDATE_FAILED_SUMMARY, error);
          this.inProfileAssigningMode = false;
        }
      );
      this.subscriptions.push(sub);
    } else {
      this._controllerService.invokeInfoToaster('Info', naplesObject.spec.id + ' is already set to profile ' + option.value);
      this.inProfileAssigningMode = false;
    }
  }

  handleSetDSCProfileCancel($event) {
    this.inProfileAssigningMode = false;
  }

  onMouseOverInterface(itf: string) {
    this.mouseOverInterface = itf;
  }

  onCollapseExpandClick(action: string) {
    this.showInterfaceTable = action === 'expand';
  }

}
