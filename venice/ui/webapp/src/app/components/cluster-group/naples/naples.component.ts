import { ChangeDetectorRef, Component, OnDestroy, OnInit, ViewChild, ViewEncapsulation } from '@angular/core';
import { FormArray } from '@angular/forms';
import { EventTypes, HttpEventUtility } from '@app/common/HttpEventUtility';
import { MetricsUtility } from '@app/common/MetricsUtility';
import { DSCWorkloadsTuple, HandleWatchItemResult, ObjectsRelationsUtility } from '@app/common/ObjectsRelationsUtility';
import { Utility, VeniceObjectCache } from '@app/common/Utility';
import { CardStates, StatArrowDirection } from '@app/components/shared/basecard/basecard.component';
import { HeroCardOptions } from '@app/components/shared/herocard/herocard.component';
import { CustomExportMap, TableCol } from '@app/components/shared/tableviewedit';
import { TableUtility } from '@app/components/shared/tableviewedit/tableutility';
import { TablevieweditAbstract } from '@app/components/shared/tableviewedit/tableviewedit.component';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { ControllerService } from '@app/services/controller.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { SearchService } from '@app/services/generated/search.service';
import { WorkloadService } from '@app/services/generated/workload.service';
import { MetricsPollingQuery, MetricsqueryService, TelemetryPollingMetricQueries } from '@app/services/metricsquery.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { SearchUtil } from '@components/search/SearchUtil';
import { AdvancedSearchComponent } from '@components/shared/advanced-search/advanced-search.component';
import { LabelEditorMetadataModel } from '@components/shared/labeleditor';
import { ClusterDistributedServiceCard, ClusterDistributedServiceCardSpec_mgmt_mode, ClusterDistributedServiceCardStatus_admission_phase, IClusterDistributedServiceCard } from '@sdk/v1/models/generated/cluster';
import { IApiStatus } from '@sdk/v1/models/generated/monitoring';
import { FieldsRequirement, ISearchSearchResponse, SearchSearchRequest, SearchSearchRequest_mode, SearchSearchResponse } from '@sdk/v1/models/generated/search';
import { IWorkloadAutoMsgWorkloadWatchHelper, WorkloadWorkload, WorkloadWorkloadList } from '@sdk/v1/models/generated/workload';
import { ITelemetry_queryMetricsQueryResponse, ITelemetry_queryMetricsQueryResult } from '@sdk/v1/models/telemetry_query';
import * as _ from 'lodash';
import { forkJoin, Observable, Subscription } from 'rxjs';
import { debounceTime } from 'rxjs/operators';
import { RepeaterData, ValueType } from 'web-app-framework';
import { NaplesCondition, NaplesConditionValues } from '.';


@Component({
  selector: 'app-naples',
  encapsulation: ViewEncapsulation.None,
  templateUrl: './naples.component.html',
  styleUrls: ['./naples.component.scss']
})

/**
 * Added advanced search to naples table.
 *
 * When WatchDistributedServiceCard sends a response, we create a map mapping naples name (spec.id) to the naples object.
 * Whenever advanced search is used, a new searchrequest is created using this.advancedSearchComponent.getSearchRequest
 * Then we make an api call to get all the matching DSCs using _callSearchRESTAPI.
 * These results do not contain spec information, so we lookup the original naples object from the naplesMap.
 * The matching naples objects are added to this.filteredNaples which is used to render the table.
 */

export class NaplesComponent extends TablevieweditAbstract<IClusterDistributedServiceCard, ClusterDistributedServiceCard> implements OnInit, OnDestroy {

  public static NAPLES_FIELD_WORKLOADS: string = 'associatedWorkloads';
  public static NAPLES_FIELD_CONDITIONSTATUS: string = 'associatedConditionStatus';

  @ViewChild('advancedSearchComponent') advancedSearchComponent: AdvancedSearchComponent;

  // naples: ReadonlyArray<ClusterDistributedServiceCard> = [];
  dataObjects: ReadonlyArray<ClusterDistributedServiceCard> = [];
  dataObjectsBackUp: ReadonlyArray<ClusterDistributedServiceCard> = null;
  inLabelEditMode: boolean = false;
  labelEditorMetaData: LabelEditorMetadataModel;

  // Used for processing the stream events
  naplesEventUtility: HttpEventUtility<ClusterDistributedServiceCard>;
  naplesMap: { [napleName: string]: ClusterDistributedServiceCard };
  searchObject: { [field: string]: any } = {};
  conditionNaplesMap: { [condition: string]: Array<string> };

  fieldFormArray = new FormArray([]);
  maxSearchRecords: number = 8000;

  workloadEventUtility: HttpEventUtility<WorkloadWorkload>;
  dscsWorkloadsTuple: { [dscKey: string]: DSCWorkloadsTuple; };
  workloads: ReadonlyArray<WorkloadWorkload> = [];
  maxWorkloadsPerRow: number = 10;

  isTabComponent: boolean = false;
  // Used for the table - when true there is a loading icon displayed
  tableLoading: boolean = false;
  hasDSC: boolean = false;

  cols: TableCol[] = [
    { field: 'spec.id', header: 'Name/Spec.id', class: '', sortable: true, width: 10 },
    { field: 'status.primary-mac', header: 'MAC Address', class: '', sortable: true, width: 10 },
    { field: 'status.DSCVersion', header: 'Version', class: '', sortable: true, width: '80px' },
    { field: 'status.ip-config.ip-address', header: 'Management IP Address', class: '', sortable: false, width: '160px' },
    { field: 'spec.admit', header: 'Admit', class: '', sortable: false, localSearch: true, width: 5, filterfunction: this.searchAdmits },
    { field: 'status.admission-phase', header: 'Phase', class: '', sortable: false, width: '120px' },
    {
      field: 'status.conditions', header: 'Condition', class: '', sortable: true, localSearch: true, width: 10,
      filterfunction: this.searchConditions
    },
    { field: 'status.host', header: 'Host', class: '', sortable: true, width: 10 },
    { field: 'meta.labels', header: 'Labels', class: '', sortable: true, width: 7 },
    { field: 'workloads', header: 'Workloads', class: '', sortable: false, localSearch: true, width: 10 },
    { field: 'meta.mod-time', header: 'Modification Time', class: '', sortable: true, width: '160px' },
    { field: 'meta.creation-time', header: 'Creation Time', class: '', sortable: true, width: '180px' },
  ];
  exportMap: CustomExportMap = {};

  advSearchCols: TableCol[] = [];

  subscriptions: Subscription[] = [];

  bodyicon: any = {
    margin: {
      top: '9px',
      left: '8px',
    },
    url: '/assets/images/icons/cluster/naples/ico-dsc-black.svg',
  };

  cardColor = '#b592e3';

  cardIcon: Icon = {
    margin: {
      top: '10px',
      left: '10px'
    },
    svgIcon: 'naples'
  };

  naplesIcon: Icon = {
    margin: {
      top: '0px',
      left: '0px',
    },
    matIcon: 'grid_on'
  };

  lastUpdateTime: string = '';

  cpuChartData: HeroCardOptions = MetricsUtility.clusterLevelCPUHeroCard(this.cardColor, this.cardIcon);

  memChartData: HeroCardOptions = MetricsUtility.clusterLevelMemHeroCard(this.cardColor, this.cardIcon);

  diskChartData: HeroCardOptions = MetricsUtility.clusterLevelDiskHeroCard(this.cardColor, this.cardIcon);

  heroCards = [
    this.cpuChartData,
    this.memChartData,
    this.diskChartData
  ];

  timeSeriesData: ITelemetry_queryMetricsQueryResult;
  avgData: ITelemetry_queryMetricsQueryResult;
  avgDayData: ITelemetry_queryMetricsQueryResult;
  maxObjData: ITelemetry_queryMetricsQueryResult;

  telemetryKind: string = 'DistributedServiceCard';

  customQueryOptions: RepeaterData[];

  cancelSearch: boolean = false;

  disableTableWhenRowExpanded: boolean = false;
  exportFilename: string = 'Venice-DistributedServiceCards';

  workloadList: WorkloadWorkload[] = [];
  searchDSCsCount: number = 0;

  constructor(private clusterService: ClusterService,
    protected controllerService: ControllerService,
    protected metricsqueryService: MetricsqueryService,
    protected searchService: SearchService,
    protected workloadService: WorkloadService,
    protected cdr: ChangeDetectorRef,
    protected uiconfigsService: UIConfigsService

  ) {
    super(controllerService, cdr, uiconfigsService);
  }


  deleteRecord(object: ClusterDistributedServiceCard): Observable<{ body: IClusterDistributedServiceCard | IApiStatus | Error; statusCode: number; }> {
    throw new Error('Method not implemented.');
  }
  generateDeleteConfirmMsg(object: ClusterDistributedServiceCard): string {
    throw new Error('Method not implemented.');
  }
  generateDeleteSuccessMsg(object: ClusterDistributedServiceCard): string {
    throw new Error('Method not implemented.');
  }
  setDefaultToolbar(): void {
  }

  /**
   * 2020-01-17 Release-A note:
   *  Flow:
   *  1. User search API to check DSC total number
   *     2. If there are DSC in Venice, this.launch()
   *        3. Fetch all workloads
   *           4. watchWorkloads, watchDSCs.
   */
  postNgInit() {
    this.buildAdvSearchCols();
    this.provideCustomOptions();
    this.controllerService.setToolbarData({
      buttons: [],
      breadcrumb: [{ label: 'Distributed Services Cards', url: Utility.getBaseUIUrl() + 'cluster/dscs' }]
    });
    this.getDSCTotalCount();  // start retrieving data.
  }

  getDSCTotalCount() {
    const query: SearchSearchRequest = Utility.buildObjectTotalSearchQuery('DistributedServiceCard');
    const searchDSCTotalSubscription = this.searchService.PostQuery(query).subscribe(
      resp => {
        if (resp) {
          const body = resp.body as ISearchSearchResponse;
          const dscTotal = parseInt(body['total-hits'], 10);
          // To test VS-1129, hard code -- const  dscTotal = 0;
          if (dscTotal > 0) {
            this.hasDSC = true;
            this.searchDSCsCount = dscTotal;
            this.launch();
          } else {
            this.hasDSC = false;
            this.controllerService.invokeInfoToaster('Information', 'There is no DSC record found in Venice');
            this.fetechAll();
          }
        }
      },
      this._controllerService.webSocketErrorHandler('Failed to get Distributed Services Cards'),
    );
    this.subscriptions.push(searchDSCTotalSubscription);
  }

  launch(): void {
    this.getMetrics();
    setTimeout(() => {
      // delay initiating websocket watches until metrics polling begins
      const wlVeniceObjectCacheData = Utility.getInstance().getVeniceObjectCacheData('Workload');
      if (wlVeniceObjectCacheData && wlVeniceObjectCacheData.length > 0) {
        this.workloadList = wlVeniceObjectCacheData;
        console.log(this.getClassName() + ' .launch(), use cached data');
        this.invokeWatch();
      } else {
        console.log(this.getClassName() + ' .launch(), use fetechAll()');
        this.fetechAll();
      }
    }, 2000);
  }

  fetechAll() {
    this.tableLoading = true;
    const observables: Observable<any>[] = [];
    observables.push(this.workloadService.ListWorkload()); // workloads
    const forkJoinSub = forkJoin(observables).subscribe((results: any[]) => {
      for (let i = 0; i < results.length; i++) {
        if (i === 0) {
          const body0 = results[0].body as WorkloadWorkloadList;
          this.workloadList = (body0.items) ? body0.items : [];
          this.workloads = this.workloadList;
        }
      }
    },
      (error) => {
        this.controllerService.invokeRESTErrorToaster('Error', 'Failed to get Fetch Hosts');
      },
      () => {
        this.invokeWatch();
      }
    );
    this.subscriptions.push(forkJoinSub);
  }

  invokeWatch() {
    this.watchWorkloads();
    this.watchNaples();
  }

  buildAdvSearchCols() {
    this.advSearchCols = this.cols.filter((col: TableCol) => {
      return (col.field !== 'meta.mod-time' && col.field !== 'meta.creation-time' && col.field !== 'workloads');
    });
    this.advSearchCols.push(
      {
        field: 'workloads', header: 'Workloads', localSearch: true, kind: 'Workload',
        filterfunction: this.searchWorkloads,
        advancedSearchOperator: SearchUtil.stringOperators
      }
    );
  }

  /**
   * Fetch workloads.
   */
  watchWorkloads() {
    this.workloadEventUtility = new HttpEventUtility<WorkloadWorkload>(WorkloadWorkload);
    this.workloads = this.workloadEventUtility.array;
    const debounceDeley = (this.workloadList && this.workloadList.length > 100) ? 300 : 1;
    const subscription = this.workloadService.WatchWorkload().pipe(debounceTime(debounceDeley)).subscribe(
      (response) => {
        this.workloadEventUtility.processEvents(response);

        const updatedMap: { [type: string]: Array<WorkloadWorkload>  } = this.workloadEventUtility.updateRecordMap;
        const addedWLItems =  updatedMap[EventTypes.create];
        const deletedWLIems = updatedMap[EventTypes.delete];
        const updatedWLIems = updatedMap[EventTypes.update];

        if (addedWLItems.length > 0) {
          this.handleAddedWorkloads(addedWLItems);
        }
        if (deletedWLIems.length > 0) {
          this.handleDeletededWorkloads(deletedWLIems);
        }
        if (updatedWLIems.length > 0) {
          this.handleUpdatedWorkloads(updatedWLIems);
        }
      },
      this._controllerService.webSocketErrorHandler('Failed to watch Workloads')
    );
    this.subscriptions.push(subscription);
  }

  /**
   * Remove objects from workloadList and recompute host-workloads map
   * @param deletedIems
   */
  handleDeletededWorkloads(deletedIems: any[]) {
    const handleWatchItemResult: HandleWatchItemResult = ObjectsRelationsUtility.handleDeletedItemsFromWatch(deletedIems, this.workloadList);
    if (handleWatchItemResult.hasChange) {
      this.workloadList = handleWatchItemResult.list;
      this.buildDSCWorkloadsMap(this.workloadList, this.dataObjects);
    }
  }

  /**
   * Update objects to workloadList and recompute host-workloads map
   * @param deletedIems
   */
  handleUpdatedWorkloads(updatedItems: any[]) {
    const handleWatchItemResult: HandleWatchItemResult = ObjectsRelationsUtility.handleUpdatedItemsFromWatch(updatedItems, this.workloadList, (item) => new WorkloadWorkload(item));
    if (handleWatchItemResult.hasChange) {
      this.workloadList = handleWatchItemResult.list;
      this.buildDSCWorkloadsMap(this.workloadList, this.dataObjects);
    }
  }

  /**
   * Add newly created objects to workloadList and recompute the host-workloads map
   * @param addedItems
   */
  handleAddedWorkloads(addedItems: any[]) {

    const handleWatchItemResult: HandleWatchItemResult = ObjectsRelationsUtility.handleAddedItemsFromWatch(addedItems, this.workloadList, (item) => new WorkloadWorkload(item));
    if (handleWatchItemResult.hasChange) {
      this.workloadList = handleWatchItemResult.list;
      this.buildDSCWorkloadsMap(this.workloadList, this.dataObjects);
    }
  }

  /**
   * This is a key API.  It computes the DSC-Workloads map and updates each DSC's workload list
   * @param myworkloads
   * @param dscs
   */
  buildDSCWorkloadsMap(myworkloads: ReadonlyArray<WorkloadWorkload> | WorkloadWorkload[],
    dscs: ReadonlyArray<ClusterDistributedServiceCard> | ClusterDistributedServiceCard[]) {
    if (myworkloads && dscs) {
      this.dscsWorkloadsTuple = ObjectsRelationsUtility.buildDscWorkloadsMaps(myworkloads, dscs);
      this.dataObjects.map(naple => {
        naple[NaplesComponent.NAPLES_FIELD_WORKLOADS] = this.getDSCWorkloads(naple);
      });
      // backup dataObjects
      this.dataObjectsBackUp = Utility.getLodash().cloneDeepWith(this.dataObjects);
    }
  }


  formatLabels(labelObj) {
    const labels = [];
    if (labelObj != null) {
      Object.keys(labelObj).forEach((key) => {
        labels.push(key + ': ' + labelObj[key]);
      });
    }
    return labels.join(', ');
  }

  provideCustomOptions() {
    this.customQueryOptions = [
      {
        key: { label: 'Condition', value: 'Condition' },
        operators: SearchUtil.stringOperators,
        valueType: ValueType.multiSelect,
        values: [
          { label: 'Healthy', value: NaplesConditionValues.HEALTHY },
          { label: 'Unhealthy', value: NaplesConditionValues.UNHEALTHY },
          { label: 'Unknown', value: NaplesConditionValues.UNKNOWN },
          { label: 'Empty', value: NaplesConditionValues.EMPTY },
          { label: 'Reboot Needed', value: NaplesConditionValues.REBOOT_NEEDED }
        ],
      }
    ];
  }

  /**
   * We use DSC maps for local search. In the backend, DSC status may changed. So we must clean up map every time we get updated data from watch.api
   * This is a helper function to clear up maps.
   */
  _clearDSCMaps() {
    this.naplesMap = {};
    this.conditionNaplesMap = {};
  }

  /**
   * Watches NapDistributed Services Cards data on KV Store and fetch new DSC data
   * Generates column based search object, currently facilitates condition search
   */
  watchNaples() {
    this._clearDSCMaps();
    this.naplesEventUtility = new HttpEventUtility<ClusterDistributedServiceCard>(ClusterDistributedServiceCard);
    this.dataObjects = this.naplesEventUtility.array as ReadonlyArray<ClusterDistributedServiceCard>;
    // this.naples = this.naplesEventUtility.array as ReadonlyArray<ClusterDistributedServiceCard>;
    const subscription = this.clusterService.WatchDistributedServiceCard().subscribe(
      response => {
        this.naplesEventUtility.processEvents(response);
        this.tableLoading = false;
        this._clearDSCMaps(); // VS-730.  Want to clear maps when we get updated data.
        this.buildDSCWorkloadsMap(this.workloadList, this.dataObjects);
        if (this.dataObjects && this.dataObjects.length > 0 && !this.hasDSC) {
          this.hasDSC = true;  // In a scenario where Venice has no DSC, this page is in idle.  We receive any new DSC, it will turn on hero-cards.
          this.getMetrics();
        }
        if (this.dataObjects.length >= this.searchDSCsCount) {
          this.searchDSCsCount = this.dataObjects.length;
        }
        for (const naple of this.dataObjects) {
          this.naplesMap[naple.meta.name] = naple;
          const dscHealthCond: NaplesCondition = Utility.getNaplesConditionObject(naple);
          // Create search object for condition
          switch (dscHealthCond.condition.toLowerCase()) {
            case NaplesConditionValues.HEALTHY:
              (this.conditionNaplesMap[NaplesConditionValues.HEALTHY] || (this.conditionNaplesMap[NaplesConditionValues.HEALTHY] = [])).push(naple.meta.name);
              break;
            case NaplesConditionValues.UNHEALTHY:
              (this.conditionNaplesMap[NaplesConditionValues.UNHEALTHY] || (this.conditionNaplesMap[NaplesConditionValues.UNHEALTHY] = [])).push(naple.meta.name);
              break;
            case NaplesConditionValues.UNKNOWN:
              (this.conditionNaplesMap[NaplesConditionValues.UNKNOWN] || (this.conditionNaplesMap[NaplesConditionValues.UNKNOWN] = [])).push(naple.meta.name);
              break;
            case NaplesConditionValues.NOTADMITTED:
              (this.conditionNaplesMap[NaplesConditionValues.NOTADMITTED] || (this.conditionNaplesMap[NaplesConditionValues.NOTADMITTED] = [])).push(naple.meta.name);
              break;
            case NaplesConditionValues.REBOOT_NEEDED:
              (this.conditionNaplesMap[NaplesConditionValues.REBOOT_NEEDED] || (this.conditionNaplesMap[NaplesConditionValues.REBOOT_NEEDED] = [])).push(naple.meta.name);
              break;
            case NaplesConditionValues.EMPTY:
              (this.conditionNaplesMap['empty'] || (this.conditionNaplesMap['empty'] = [])).push(naple.meta.name);
              break;
          }
          naple[NaplesComponent.NAPLES_FIELD_CONDITIONSTATUS] = {
            dscCondStr: dscHealthCond.condition.toLowerCase(),
            dscNeedReboot: dscHealthCond.rebootNeeded
          };
          naple[NaplesComponent.NAPLES_FIELD_WORKLOADS] = this.getDSCWorkloads(naple);
        }
        this.searchObject['status.conditions'] = this.conditionNaplesMap;
        this.tryGenCharts();
      },
      (error) => {
        this.tableLoading = false;
        this.controllerService.invokeRESTErrorToaster('Failed to get Naples', error);
      }
    );
    this.subscriptions.push(subscription); // add subscription to list, so that it will be cleaned up when component is destroyed.
  }
  getDSCWorkloads(naple: ClusterDistributedServiceCard): WorkloadWorkload[] {
    if (this.dscsWorkloadsTuple[naple.meta.name]) {
      return this.dscsWorkloadsTuple[naple.meta.name].workloads;
    } else {
      return [];
    }
  }

  displayColumn(data, col): any {
    const fields = col.field.split('.');
    const value = Utility.getObjectValueByPropertyPath(data, fields);
    const column = col.field;
    switch (column) {
      default:
        return Array.isArray(value) ? JSON.stringify(value, null, 2) : value;
    }
  }

  isNICHealthy(data: ClusterDistributedServiceCard): boolean {
    return data[NaplesComponent.NAPLES_FIELD_CONDITIONSTATUS] &&
      data[NaplesComponent.NAPLES_FIELD_CONDITIONSTATUS].dscCondStr
      === NaplesConditionValues.HEALTHY;
  }

  isNICUnhealthy(data: ClusterDistributedServiceCard): boolean {
    return data[NaplesComponent.NAPLES_FIELD_CONDITIONSTATUS] &&
      data[NaplesComponent.NAPLES_FIELD_CONDITIONSTATUS].dscCondStr
      === NaplesConditionValues.UNHEALTHY;
  }

  isNICHealthUnknown(data: ClusterDistributedServiceCard): boolean {
    return data[NaplesComponent.NAPLES_FIELD_CONDITIONSTATUS] &&
      data[NaplesComponent.NAPLES_FIELD_CONDITIONSTATUS].dscCondStr
      === NaplesConditionValues.UNKNOWN;
  }

  isNicNeedReboot(data: ClusterDistributedServiceCard): boolean {
    return data[NaplesComponent.NAPLES_FIELD_CONDITIONSTATUS] &&
      data[NaplesComponent.NAPLES_FIELD_CONDITIONSTATUS].dscNeedReboot;
  }

  displayReasons(data: ClusterDistributedServiceCard): any {
    return Utility.displayReasons(data);
  }

  isNICNotAdmitted(data: ClusterDistributedServiceCard): boolean {
    return Utility.isNICConditionNotAdmitted(data);
  }

  /**
   * We start 3 metric polls.
   * Time series poll
   *  - Fetches the past 24 hours, averaging in
   *    5 min buckets (starting at the hour) across all nodes
   *  - Used for line graph as well as the current value stat
   * AvgPoll
   *   - Fetches the average over the past 24 hours across all nodes
   * MaxNode poll
   *   - Fetches the average over the past 5 min bucket. Grouped by node
   *     - Rounds down to the nearest 5 min increment.
   *       Ex. 11:52 -> fetches data from 11:50 till current time
   *
   * When you query for 5 min intervals, they are automatically aligned by the hour
   * We always round down to the last completed 5 min interval.
   *
   * Distributed Services Cards overview level display
   *  - Time series graph of all the nodes averaged together, avg into 5 min buckets
   *  - Current avg of last 5 min
   *  - Average of past day
   *  - Naple using the most in the last 5 min
   */
  getMetrics() {
    const queryList: TelemetryPollingMetricQueries = {
      queries: [],
      tenant: Utility.getInstance().getTenant()
    };
    queryList.queries.push(this.timeSeriesQuery());
    queryList.queries.push(this.avgQuery());
    queryList.queries.push(this.avgDayQuery());
    queryList.queries.push(this.maxNaplesQuery());

    const sub = this.metricsqueryService.pollMetrics('naplesCards', queryList).subscribe(
      (data: ITelemetry_queryMetricsQueryResponse) => {
        if (data && data.results && data.results.length === 4) {
          this.timeSeriesData = data.results[0];
          this.avgData = data.results[1];
          this.avgDayData = data.results[2];
          this.maxObjData = data.results[3];
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
    return MetricsUtility.timeSeriesQueryPolling(this.telemetryKind, ['CPUUsedPercent', 'MemUsedPercent', 'DiskUsedPercent']);
  }

  avgQuery(): MetricsPollingQuery {
    return MetricsUtility.pastFiveMinAverageQueryPolling(this.telemetryKind);
  }

  avgDayQuery(): MetricsPollingQuery {
    return MetricsUtility.pastDayAverageQueryPolling(this.telemetryKind);
  }

  maxNaplesQuery(): MetricsPollingQuery {
    return MetricsUtility.maxObjQueryPolling(this.telemetryKind);
  }

  private tryGenCharts() {
    this.genCharts('CPUUsedPercent', this.cpuChartData);
    this.genCharts('MemUsedPercent', this.memChartData);
    this.genCharts('DiskUsedPercent', this.diskChartData);
  }

  private genCharts(fieldName: string, heroCard: HeroCardOptions) {
    const timeSeriesDataLoaded = MetricsUtility.resultHasData(this.timeSeriesData);
    const avgDataLoaded = MetricsUtility.resultHasData(this.avgData);
    const avgDayDataLoaded = MetricsUtility.resultHasData(this.avgDayData);
    const maxObjDataLoaded = MetricsUtility.resultHasData(this.maxObjData);

    // Time series graph
    if (timeSeriesDataLoaded) {
      const timeSeriesData = this.timeSeriesData;

      const data = MetricsUtility.transformToChartjsTimeSeries(timeSeriesData.series[0], fieldName);
      heroCard.lineData.data = data;
    }

    // Current stat calculation - we take the last point
    if (avgDataLoaded) {
      const index = MetricsUtility.findFieldIndex(this.avgData.series[0].columns, fieldName);
      heroCard.firstStat.value = Math.round(this.avgData.series[0].values[0][index]) + '%';
      heroCard.firstStat.numericValue = Math.round(this.avgData.series[0].values[0][index]);
    }

    // Avg
    if (avgDayDataLoaded) {
      const avgDayData = this.avgDayData;
      if (avgDayData.series[0].values.length !== 0) {
        const index = MetricsUtility.findFieldIndex(this.avgDayData.series[0].columns, fieldName);
        heroCard.secondStat.value = Math.round(avgDayData.series[0].values[0][index]) + '%';
        heroCard.secondStat.numericValue = Math.round(avgDayData.series[0].values[0][index]);
      }
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

    // Max naples
    if (maxObjDataLoaded) {
      const maxNaples = MetricsUtility.maxObjUtility(this.maxObjData, fieldName);
      if (maxNaples == null || maxNaples.max === -1) {
        heroCard.thirdStat.value = null;
      } else {
        const thirdStatNaples = this.getNaplesByKey(maxNaples.name);
        if ((thirdStatNaples) != null) {
          const thirdStatName = thirdStatNaples.spec.id;
          let thirdStat: string = thirdStatName;
          if (thirdStat.length > 0) {
            // VS-736 start
            thirdStat = Utility.getHeroCardDisplayValue(thirdStat);
            Utility.customizeHeroCardThirdStat(thirdStat, heroCard, thirdStatName);
            // VS-736 end
            thirdStat += ' (' + Math.round(maxNaples.max) + '%)';
            heroCard.thirdStat.value = thirdStat;
            heroCard.thirdStat.url = '/cluster/dscs/' + thirdStatNaples.meta.name;
            heroCard.thirdStat.numericValue = Math.round(maxNaples.max);
          }
        } else {
          heroCard.thirdStat.value = null;
        }
      }
    }

    // begin showing card when any of the metrics are available
    if (heroCard.cardState !== CardStates.READY && (timeSeriesDataLoaded || avgDataLoaded || avgDayDataLoaded || maxObjDataLoaded)) {
      heroCard.cardState = CardStates.READY;
    }
  }

  getNaplesByKey(name: string): ClusterDistributedServiceCard {
    for (let index = 0; index < this.dataObjects.length; index++) {
      const naple = this.dataObjects[index];
      if (naple.meta.name === name) {
        return naple;
      }
    }
    return null;
  }

  /**
   * Generates Local and Remote search queries. Later, calls Local and Remote Search Function
   * @param field Field by which to sort data
   * @param order Sort order (ascending = 1/descending = -1)
   *
   * plug in <app-advanced-search  (searchEmitter)="getDistributedServiceCards()"
   */
  getDistributedServiceCards(field = this.tableContainer.sortField,
    order = this.tableContainer.sortOrder) {
    let searchSearchRequest = this.advancedSearchComponent.getSearchRequest(field, order, 'DistributedServiceCard', true, this.maxSearchRecords);
    // VS-1008, we customize searchRequest.
    searchSearchRequest = this.customizeSearchRequest(searchSearchRequest);
    const localSearchResult = this.advancedSearchComponent.getLocalSearchResult(field, order, this.searchObject);
    if (localSearchResult.err) {
      this.controllerService.invokeInfoToaster('Invalid', 'Length of search values don\'t match with accepted length');
      return;
    }
    if ((searchSearchRequest.query != null && (searchSearchRequest.query.fields != null && searchSearchRequest.query.fields.requirements != null
      && searchSearchRequest.query.fields.requirements.length > 0) || (searchSearchRequest.query.texts != null
        && searchSearchRequest.query.texts[0].text.length > 0)) || this.cancelSearch) {
      if (this.cancelSearch) { this.cancelSearch = false; }
      this._callSearchRESTAPI(searchSearchRequest, localSearchResult.searchRes);
    } else {
      // This case executed when only local search is required
      this.dataObjects = this.generateFilteredNaples(localSearchResult.searchRes);
      if (this.dataObjects.length === 0) {
        this.controllerService.invokeInfoToaster('Information', 'No record found. Please change search criteria.');
      }
    }
  }

  /**
   * Per VS-1008, we want to change
   * status.DSCVersion → status.DSCVersion.keyword
   * spec.id → spec.id.keyword
   */
  // "fields": {
  //   "requirements": [
  //     {
  //       "key": "meta.name",  <-- change this per VS-1008
  //       "operator": "notIn",
  //       "values": [
  //         "hw",
  //         "u1"
  //       ]
  //     }
  //   ]
  // }
  customizeSearchRequest(searchSearchRequest: SearchSearchRequest): SearchSearchRequest {
    const requirements = (searchSearchRequest.query.fields && searchSearchRequest.query.fields.requirements) ? searchSearchRequest.query.fields.requirements : null;
    for (let i = 0; requirements && i < requirements.length; i++) {
      const requirement = requirements[i];
      const key = requirement.key;
      if (key === 'spec.id') {
        requirement.key = 'spec.id.keyword';
      }
      if (key === 'status.DSCVersion') {
        requirement.key = 'status.DSCVersion.keyword';
      }
    }
    return searchSearchRequest;
  }

  /**
   * Generates aggregate of local and/or remote advanced search filtered naples
   * @param tempNaples Array of local and/or remote advanced search filtered naples meta.name
   */
  generateFilteredNaples(tempNaples: string[]): ClusterDistributedServiceCard[] {
    let resultNics: ClusterDistributedServiceCard[] = [];
    this.lastUpdateTime = new Date().toISOString();
    if (tempNaples != null && tempNaples.length > 0) {
      resultNics = this.filterNaplesByName(tempNaples);
    }
    return resultNics;
  }

  populateFieldSelector() {
    this.fieldFormArray = new FormArray([]);
  }

  /**
 * This serves HTML API. It clear naples search and refresh data.
 * @param $event
 */
  onCancelSearch($event) {
    /*  Previous remote search code. Comment out for now.
     this.populateFieldSelector();
     this.cancelSearch = true;
     this.getDistributedServiceCards();
      */
    this.controllerService.invokeInfoToaster('Information', 'Cleared search criteria, DSCs refreshed.');
    this.dataObjects = this.dataObjectsBackUp;
  }

  /**
   * API to perform remote search
   * @param searchSearchRequest remote search query
   * @param localSearchResult Array of filtered naples name after local search
   */
  private _callSearchRESTAPI(searchSearchRequest: SearchSearchRequest, localSearchResult: Array<string>) {
    const subscription = this.searchService.PostQuery(searchSearchRequest).subscribe(
      response => {
        const data: SearchSearchResponse = response.body as SearchSearchResponse;
        let objects = data.entries;
        if (!objects || objects.length === 0) {
          this.controllerService.invokeInfoToaster('Information', 'No record found. Please change search criteria.');
          objects = [];
        }
        const entries = [];
        const remoteSearchResult = [];
        for (let k = 0; k < objects.length; k++) {
          entries.push(objects[k].object); // objects[k] is a SearchEntry object
          remoteSearchResult.push(entries[k].meta.name);
        }
        const searchResult = this.advancedSearchComponent.generateAggregateResults(remoteSearchResult, localSearchResult);
        this.dataObjects = this.generateFilteredNaples(searchResult);
      },
      (error) => {
        this.controllerService.invokeRESTErrorToaster('Failed to get naples', error);
      }
    );
    this.subscriptions.push(subscription);
  }

  /**
   * Provides DSC objects for DSC meta.name
   * @param entries Array of DSC meta.name
   */
  filterNaplesByName(entries: string[]): ClusterDistributedServiceCard[] {
    const tmpMap = {};
    entries.forEach(ele => {
      tmpMap[ele] = this.naplesMap[ele];
    });
    return Object.values(tmpMap);
  }

  editLabels() {
    this.labelEditorMetaData = {
      title: 'Editing naples objects',
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

  updateWithForkjoin(updatedNaples: ClusterDistributedServiceCard[]) {
    const observables: Observable<any>[] = [];
    for (const naplesObject of updatedNaples) {
      const name = naplesObject.meta.name;
      if (naplesObject[NaplesComponent.NAPLES_FIELD_CONDITIONSTATUS]) {  // remove UI fields
        delete naplesObject[NaplesComponent.NAPLES_FIELD_CONDITIONSTATUS];
      }
      if (naplesObject[NaplesComponent.NAPLES_FIELD_WORKLOADS]) {  // remove UI fields
        delete naplesObject[NaplesComponent.NAPLES_FIELD_WORKLOADS];
      }
      const sub = this.clusterService.UpdateDistributedServiceCard(name, naplesObject, '', this.naplesMap[name].$inputValue);
      observables.push(sub);
    }

    const summary = 'Distributed Services Card update';
    const objectType = 'naples';
    this.handleForkJoin(observables, summary, objectType);
  }

  private handleForkJoin(observables: Observable<any>[], summary: string, objectType: string) {
    forkJoin(observables).subscribe((results: any[]) => {
      let successCount: number = 0;
      let failCount: number = 0;
      const errors: string[] = [];
      for (let i = 0; i < results.length; i++) {
        if (results[i]['statusCode'] === 200) {
          successCount += 1;
        } else {
          failCount += 1;
          errors.push(results[i].body.message);
        }
      }
      if (successCount > 0) {
        const msg = 'Successfully updated ' + successCount.toString() + ' ' + objectType + '.';
        this._controllerService.invokeSuccessToaster(summary, msg);
        this.inLabelEditMode = false;
      }
      if (failCount > 0) {
        this._controllerService.invokeRESTErrorToaster(summary, errors.join('\n'));
      }
    },
      this._controllerService.restErrorHandler(summary + ' Failed'));
  }

  // The save emitter from labeleditor returns the updated objects here.
  // We use forkjoin to update all the naples.
  handleEditSave(updatedNaples: ClusterDistributedServiceCard[]) {
    this.updateWithForkjoin(updatedNaples);
  }

  handleEditCancel($event) {
    this.inLabelEditMode = false;
  }

  ngOnDestroyHook() {
    const ts = (new Date()).getTime();
    const hour = Utility.DEFAULT_CACHE_DURATION;

    const workloadVeniceObjectCache: VeniceObjectCache = {
      timestamp: ts,
      duration: hour,
      data: this.workloadList
    };
    Utility.getInstance().setVeniceObjectCache('Workload', workloadVeniceObjectCache);

    if (this.dataObjects && this.dataObjects.length >= this.searchDSCsCount) {
      const dscsVeniceObjectCache: VeniceObjectCache = {
        timestamp: ts,
        duration: hour,
        data: this.dataObjects as any[]
      };

      Utility.getInstance().setVeniceObjectCache('DistributedServiceCard', dscsVeniceObjectCache);
    }
  }

  buildMoreWorkloadTooltip(dsc: ClusterDistributedServiceCard): string {
    const wltips = [];
    const workloads = dsc[NaplesComponent.NAPLES_FIELD_WORKLOADS];
    for (let i = 0; i < workloads.length; i++) {
      if (i >= this.maxWorkloadsPerRow) {
        const workload = workloads[i];
        wltips.push(workload.meta.name);
      }
    }
    return wltips.join(' , ');
  }

  onDecommissionCard(event, object: ClusterDistributedServiceCard) {
    if (event) {
      event.stopPropagation();
    }
    this.controllerService.invokeConfirm({
      header: 'Are you sure that you want to decommission the card?',
      message: 'Once the card is decommissioned, it can not be brought back through UI.',
      acceptLabel: 'Decommission',
      accept: () => {
        const updatedObject: ClusterDistributedServiceCard = new ClusterDistributedServiceCard(object);
        updatedObject.spec['mgmt-mode'] = ClusterDistributedServiceCardSpec_mgmt_mode.host;
        this.invokeUpdateCard(updatedObject, object,
          'Successfully decommissioned ' + object.meta.name, 'Decommision Service');
      }
    });
  }

  onAdmitCard(event, object: ClusterDistributedServiceCard) {
    if (event) {
      event.stopPropagation();
    }
    this.controllerService.invokeConfirm({
      header: 'Are you sure that you want to admit the card?',
      message: 'Once the card is admitted, This action can not be undone.',
      acceptLabel: 'Admit',
      accept: () => {
        const updatedObject: ClusterDistributedServiceCard = new ClusterDistributedServiceCard(object);
        updatedObject.spec.admit = true;
        this.invokeUpdateCard(updatedObject, object,
          'Successfully admitted ' + object.meta.name, 'Admit Service');
      }
    });
  }

  invokeUpdateCard(updatedObject: ClusterDistributedServiceCard,
    oldObject: ClusterDistributedServiceCard,
    successMsg: string,
    actionType: string) {
    const sub = this.clusterService.UpdateDistributedServiceCard(updatedObject.meta.name, updatedObject, null,
      oldObject, false).subscribe(
        () => {
          this.controllerService.invokeSuccessToaster(Utility.UPDATE_SUCCESS_SUMMARY,
            successMsg);
        },
        (error) => {
          if (error.body instanceof Error) {
            console.error(actionType + ' returned code: ' + error.statusCode + ' data: ' + <Error>error.body);
          } else {
            console.error(actionType + ' returned code: ' + error.statusCode + ' data: ' + <IApiStatus>error.body);
          }
          this.controllerService.invokeRESTErrorToaster(Utility.UPDATE_FAILED_SUMMARY, error);
        }
      );
    this.subscriptions.push(sub);
  }

  showAdmissionButton(rowData: ClusterDistributedServiceCard): boolean {
    return !rowData.spec.admit;
  }

  showDecommissionButton(rowData: ClusterDistributedServiceCard): boolean {
    return rowData.spec.admit && rowData.status['admission-phase'] ===
      ClusterDistributedServiceCardStatus_admission_phase.admitted;
  }

  onSearchDSCs(field = this.tableContainer.sortField, order = this.tableContainer.sortOrder) {
    const searchResults = this.onSearchDataObjects(field, order, 'DistributedServiceCard', this.maxSearchRecords, this.advSearchCols, this.dataObjects, this.advancedSearchComponent);
    if (searchResults && searchResults.length > 0) {
      this.dataObjects = [];
      this.dataObjects = searchResults;
    }
  }

  searchWorkloads(requirement: FieldsRequirement, data = this.dataObjects): any[] {
    const outputs: any[] = [];
    for (let i = 0; data && i < data.length; i++) {
      const workloads = data[i][NaplesComponent.NAPLES_FIELD_WORKLOADS];
      // workloads[i] is a full object
      for (let k = 0; k < workloads.length; k++) {
        const recordValue = _.get(workloads[k], ['meta', 'name']);
        const searchValues = requirement.values;
        let operator = String(requirement.operator);
        operator = TableUtility.convertOperator(operator);
        for (let j = 0; j < searchValues.length; j++) {
          const activateFunc = TableUtility.filterConstraints[operator];
          if (activateFunc && activateFunc(recordValue, searchValues[j])) {
            outputs.push(data[i]);
          }
        }
      }
    }
    return outputs;
  }

  searchConditions(requirement: FieldsRequirement, data = this.dataObjects): any[] {
    const outputs: any[] = [];
    for (let i = 0; data && i < data.length; i++) {
      let found = false;
      const conditions = data[i].status.conditions;
      for (let k = 0; k < conditions.length; k++) {
        // datat[i].associatedConditionStatus is  {dscCondStr: "healthy", dscNeedReboot: true}
        const recordValueStr = data[i][NaplesComponent.NAPLES_FIELD_CONDITIONSTATUS].dscCondStr;
        const recordValueReboot = data[i][NaplesComponent.NAPLES_FIELD_CONDITIONSTATUS].dscNeedReboot;
        const searchValues = requirement.values;
        let operator = String(requirement.operator);
        operator = TableUtility.convertOperator(operator);
        for (let j = 0; j < searchValues.length; j++) {
          const searchValue = searchValues[j];
          // special case empty string
          if (searchValue === NaplesConditionValues.EMPTY) {
            if (operator === 'contains' && recordValueStr === searchValue) {
              found = true;
            } else if (operator === 'notcontains') {
              found = recordValueStr !== searchValue;
            }
          } else {
            const activateFunc = TableUtility.filterConstraints[operator];
            const activateFuncValue = activateFunc && activateFunc(recordValueStr, searchValue);

            if (operator === 'contains' && activateFuncValue) {
              found = true;
            } else if (operator === 'notcontains') {
              found = activateFuncValue;
            }
          }

          // check reboot condition
          if (searchValues[j] === NaplesConditionValues.REBOOT_NEEDED) {
            if (operator === 'contains' && recordValueReboot) {
              found = true;
            } else if (operator === 'notcontains') {
              found = !recordValueReboot;
            }
          }
        }
      }
      if (found) {
        outputs.push(data[i]);
      }
    }
    return outputs;
  }

  searchAdmits(requirement: FieldsRequirement, data = this.dataObjects): any[] {
    const outputs: any[] = [];
    for (let i = 0; data && i < data.length; i++) {
      const recordValue = String(data[i].spec.admit);
      const searchValues = requirement.values;
      let operator = String(requirement.operator);
      operator = TableUtility.convertOperator(operator);
      for (let j = 0; j < searchValues.length; j++) {
        const activateFunc = TableUtility.filterConstraints[operator];
        if (activateFunc && activateFunc(recordValue, searchValues[j])) {
          outputs.push(data[i]);
        }
      }
    }
    return outputs;
  }
}
