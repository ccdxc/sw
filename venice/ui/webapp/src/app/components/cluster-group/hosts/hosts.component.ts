import { ChangeDetectorRef, Component, OnInit, ViewChild, ViewEncapsulation } from '@angular/core';
import { FormArray } from '@angular/forms';
import { Animations } from '@app/animations';
import { DSCsNameMacMap, HostWorkloadTuple, ObjectsRelationsUtility, HandleWatchItemResult } from '@app/common/ObjectsRelationsUtility';
import { SearchUtil } from '@app/components/search/SearchUtil';
import { AdvancedSearchComponent } from '@app/components/shared/advanced-search/advanced-search.component';
import { CustomExportMap, TableCol } from '@app/components/shared/tableviewedit';
import { TableUtility } from '@app/components/shared/tableviewedit/tableutility';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { ControllerService } from '@app/services/controller.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { SearchService } from '@app/services/generated/search.service';
import { WorkloadService } from '@app/services/generated/workload.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { EventTypes, HttpEventUtility } from '@common/HttpEventUtility';
import { Utility, VeniceObjectCache } from '@common/Utility';
import { TablevieweditAbstract } from '@components/shared/tableviewedit/tableviewedit.component';
import { ClusterDistributedServiceCard, ClusterDistributedServiceCardList, IApiStatus, IClusterAutoMsgDistributedServiceCardWatchHelper, IClusterAutoMsgHostWatchHelper } from '@sdk/v1/models/generated/cluster';
import { ClusterHost, IClusterHost } from '@sdk/v1/models/generated/cluster/cluster-host.model';
import { FieldsRequirement, ISearchSearchResponse, SearchSearchRequest_mode, SearchSearchRequest } from '@sdk/v1/models/generated/search';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';
import { IWorkloadAutoMsgWorkloadWatchHelper, WorkloadWorkload, WorkloadWorkloadList } from '@sdk/v1/models/generated/workload';
import * as _ from 'lodash';
import { forkJoin, Observable, Subscription } from 'rxjs';
import { debounceTime } from 'rxjs/operators';

export enum BuildHostWorkloadMapSourceType {
  init = 'init',
  watchHosts = 'watchHost',
  watchHostAdd = 'watchHostAdd',
  watchHostDelete = 'watchHostDelete',
  watchWorkloadAdd = ' watchWorkloadAdd',
  watchWorkloadDelete = ' watchWorkloadDelete',
  watchWorkloadUpdate = ' watchWorkloadUpdate',
}
/**
 * Hosts page.
 * UI fetches hosts, DSCs and Workloads objects and build relation map.
 * Each Host has extra UI fields HOST_FIELD_DSCS, and HOST_FIELD_WORKLOADS
 *
 * 2020-01-13 update:
 * https://10.30.2.173/#/login has 1000 hosts, 1000 DSC and 8000 workload
 * websocket's limit forces me to fetch all dscs/workloads, then watch them all.
 *
 * postNgInit() -> fetchAll() -> watch (DSCs, Workloads)
 *  populate
 *      naplesList, workloadList
 *                               --> watch (DSCs, Workloads) , watchHosts();
 *
 *  This design is only good for 2020 release-A
 *  Accumulating workloads/hosts/DSCs object through web-socket takes time and space. So I just user REST GetXXXList() APIs to fetch all records first.
 *
 *  A proper design should be load Top10-this, top10-that, let user see the big picture and use search feature.
 *
 *
 *  2020-02-13 update:
 *  postNgInit() -> getRecords() which fetch Hosts/DSCs/Workloads objects.
 *  As we use cluser.service and workload.service internally use GenUtility.ts to cache data and manage web-socket event, we just build object maps
 *
 */
@Component({
  selector: 'app-hosts',
  encapsulation: ViewEncapsulation.None,
  templateUrl: './hosts.component.html',
  styleUrls: ['./hosts.component.scss'],
  animations: [Animations]
})
export class HostsComponent extends TablevieweditAbstract<IClusterHost, ClusterHost> implements OnInit {

  public static HOST_FIELD_DSCS: string = 'processedSmartNics';
  public static HOST_FIELD_WORKLOADS: string = 'processedWorkloads';

  @ViewChild('advancedSearchComponent') advancedSearchComponent: AdvancedSearchComponent;
  maxSearchRecords: number = 8000;

  bodyicon: Icon = {
    margin: {
      top: '9px',
      left: '8px'
    },
    svgIcon: 'host'
  };

  headerIcon: Icon = {
    margin: {
      top: '0px',
      left: '10px',
    },
    matIcon: 'computer'
  };
  nameToMacMap: { [key: string]: string; } = {};
  macToNameMap: { [key: string]: string; } = {};
  subscriptions: Subscription[] = [];
  dataObjects: ReadonlyArray<ClusterHost> = [];
  dataObjectsBackUp: ReadonlyArray<ClusterHost> = [];
  disableTableWhenRowExpanded: boolean = true;
  isTabComponent: boolean = false;

  naplesWithoutHosts: ClusterDistributedServiceCard[] = [];
  notAdmittedCount: number = 0;

  // Used for the table - when true there is a loading icon displayed
  tableLoading: boolean = false;

  cols: TableCol[] = [
    { field: 'meta.name', header: 'Name', class: 'hosts-column-host-name', sortable: true, width: 15 },
    { field: 'spec.dscs', header: 'Distributed Services Cards', class: 'hosts-column-dscs', sortable: false, width: 25 },
    { field: 'workloads', header: 'Associated Workloads', class: 'hosts-column-workloads', sortable: false, width: 25 },
    { field: 'meta.mod-time', header: 'Modification Time', class: 'hosts-column-date', sortable: true, width: '180px' },
    { field: 'meta.creation-time', header: 'Creation Time', class: 'hosts-column-date', sortable: true, width: '180px' },
  ];

  // advance search variables
  advSearchCols: TableCol[] = [];
  fieldFormArray = new FormArray([]);

  exportFilename: string = 'Venice-hosts';

  exportMap: CustomExportMap = {
    'workloads': (opts): string => {
      return opts.data.processedWorkloads.map(wkld => wkld.meta.name).join(', ');
    }
  };

  hostWorkloadsTuple: { [hostKey: string]: HostWorkloadTuple; };

  maxWorkloadsPerRow: number = 8;

  naplesList: ClusterDistributedServiceCard[] = [];
  workloadList: WorkloadWorkload[] = [];
  searchHostsCount: number = 0;

  constructor(private clusterService: ClusterService,
    private workloadService: WorkloadService,
    protected cdr: ChangeDetectorRef,
    protected uiconfigsService: UIConfigsService,
    protected controllerService: ControllerService,
    protected searchService: SearchService) {
    super(controllerService, cdr, uiconfigsService);
  }


  /**
   * This API build host[i] -> workloads[] map
   * As # of workloads are much greater that of hosts, we invoke this api in both getHosts() and getWorkloads()
   * Backend keeps push workload records to UI when there is no more hosts received from web-sockets.
   * We have to keep building host[i] -> workloads[] map to show data in UI.
   *
   */
  buildHostWorkloadsMap(myworkloads: ReadonlyArray<WorkloadWorkload> | WorkloadWorkload[],
    hosts: ReadonlyArray<ClusterHost> | ClusterHost[], source: BuildHostWorkloadMapSourceType) {
    if (myworkloads && hosts) {
      this.hostWorkloadsTuple = ObjectsRelationsUtility.buildHostWorkloadsMap(myworkloads, hosts);
      this.dataObjects.forEach(host => {
        host._ui[HostsComponent.HOST_FIELD_DSCS] = this.processSmartNics(host);
        host._ui[HostsComponent.HOST_FIELD_WORKLOADS] = this.getHostWorkloads(host);
      });
      // backup dataObjects
      this.dataObjectsBackUp = Utility.getLodash().cloneDeepWith(this.dataObjects);
    }
  }



  /**
   *
   * @param oldMap
   * @param newMap
   */
  updateHostWorkloadMap(oldMap: { [key: string]: any; }, newMap: { [key: string]: any; }) {
    Object.keys(newMap).forEach((key) => {
      if (oldMap[key]) {
        oldMap[key] = newMap[key];
      }
    });
  }


  updateDSCsMaps(oldMap: { [key: string]: any; }, newMap: { [key: string]: any; }) {
    Object.keys(newMap).forEach((key) => {
      if (!oldMap[key]) {
        oldMap[key] = newMap[key];
      }
    });
  }



  getHostWorkloads(host: ClusterHost): WorkloadWorkload[] {
    if (this.hostWorkloadsTuple[host.meta.name]) {
      return this.hostWorkloadsTuple[host.meta.name].workloads;
    } else {
      return [];
    }
  }

  buildAdvSearchCols() {
    this.advSearchCols = this.cols.filter((col: TableCol) => {
      return (col.field !== 'spec.dscs' && col.field !== 'workloads');
    });
    this.advSearchCols.push(
      {
        field: 'Workload', header: 'Workloads', localSearch: true, kind: 'Workload',
        filterfunction: this.searchWorkloads,
        advancedSearchOperator: SearchUtil.stringOperators
      }
    );
    this.advSearchCols.push(
      {
        field: 'DSC', header: 'DSCs', localSearch: true, kind: 'DistributedServiceCard',
        filterfunction: this.searchDSCs,
        advancedSearchOperator: SearchUtil.stringOperators
      }
    );
  }

  setDefaultToolbar() {

    let buttons = [];

    if (this.uiconfigsService.isAuthorized(UIRolePermissions.clusterhost_create)) {
      buttons = [{
        cssClass: 'global-button-primary host-button newhost-button-ADD',
        text: 'ADD HOST',
        computeClass: () => this.shouldEnableButtons ? '' : 'global-button-disabled',
        callback: () => { this.createNewObject(); }
      }];
    }

    this.controllerService.setToolbarData({
      buttons: buttons,
      breadcrumb: [{ label: 'Hosts', url: Utility.getBaseUIUrl() + 'cluster/hosts' }]
    });
  }

  // This func is only working for when
  // spec.dscs and status.admitted-dscs will only be of length one, and that if status has an entry it's referring to the one in spec.
  isAdmitted(specValue, statusValue): boolean {
    return specValue && specValue.length === 1 && statusValue && statusValue.length === 1;
  }

  displayColumn(rowData, col): any {
    const fields = col.field.split('.');
    const value = Utility.getObjectValueByPropertyPath(rowData, fields);
    const column = col.field;
    switch (column) {
      default:
        return Array.isArray(value) ? JSON.stringify(value, null, 2) : value;
    }
  }

  processSmartNics(host: ClusterHost) {
    const fields = 'spec.dscs'.split('.');
    const value = Utility.getObjectValueByPropertyPath(host, fields);
    const statusValue = Utility.getObjectValueByPropertyPath(host, 'status.admitted-dscs'.split('.'));

    // We only have one entry at this point
    return value.map(v => {
      if (v.hasOwnProperty('id') && v['id']) {
        return {
          text: v['id'],
          mac: this.nameToMacMap[v['id']] || '',
          admitted: this.isAdmitted(value, statusValue)
        };
      } else if (v.hasOwnProperty('mac-address') && v['mac-address']) {
        let text = this.macToNameMap[v['mac-address']];
        if (text == null) {
          text = v['mac-address'];
        }
        return {
          text: text,
          mac: v['mac-address'],
          admitted: this.isAdmitted(value, statusValue)
        };
      } else {
        return {
          text: 'N/A',
          mac: '',
          admitted: this.isAdmitted(value, statusValue)
        };
      }
    });
  }

  postNgInit() {
    this.buildAdvSearchCols();
    this.getRecords();
  }

  /**
   * This API is used in getRecords().
   * Whenever, REST calls fetch host, dsc or workload, this function will be invoked.
   */
  handleDataReady() {
    // When naplesList is ready, build DSC-maps
    if (this.naplesList) {
      const _myDSCnameToMacMap: DSCsNameMacMap = ObjectsRelationsUtility.buildDSCsNameMacMap(this.naplesList);
      this.nameToMacMap = _myDSCnameToMacMap.nameToMacMap;
      this.macToNameMap = _myDSCnameToMacMap.macToNameMap;
    }
    // When workload and hostList are ready, build host-workload map
    if (this.workloadList && this.dataObjects) {
      this.buildHostWorkloadsMap(this.workloadList, this.dataObjects, BuildHostWorkloadMapSourceType.watchHosts);  // host[i] -> workloads[] map
    }
  }

  /**
   * This API fetch DSCs, Workloads and Hosts object.
   * xxxService.ListXXXXCache() has magic to return object list and websocket event.
   * {
   * "data": [
   * { .. } // DSC object
   * ],
   * "events": [], // websocket update record // Create, Update, Delete
   * "connIsErrorState": false
   * }
   *
   */
  getRecords() {
    const workloadSubscription = this.workloadService.ListWorkloadCache().subscribe(
      (response) => {
        if (response.connIsErrorState) {
          return;
        }
        this.workloadList = response.data as WorkloadWorkload[];
        this.handleDataReady();
      }
    );
    this.subscriptions.push(workloadSubscription);

    const dscSubscription = this.clusterService.ListDistributedServiceCardCache().subscribe(
      (response) => {
        if (response.connIsErrorState) {
          return;
        }
        this.naplesList = response.data as ClusterDistributedServiceCard[];
        this.handleDataReady();
      }
    );
    this.subscriptions.push(dscSubscription);


    const hostSubscription = this.clusterService.ListHostCache().subscribe(
      (response) => {
        if (response.connIsErrorState) {
          return;
        }
        this.dataObjects = response.data;
        this.handleDataReady();
      }
    );
    this.subscriptions.push(hostSubscription);
  }


  deleteRecord(object: ClusterHost): Observable<{ body: IClusterHost | IApiStatus | Error | IClusterHost; statusCode: number }> {
    return this.clusterService.DeleteHost(object.meta.name);
  }

  generateDeleteConfirmMsg(object: IClusterHost): string {
    return 'Are you sure you want to delete host ' + object.meta.name;
  }

  generateDeleteSuccessMsg(object: IClusterHost): string {
    return 'Deleted host ' + object.meta.name;
  }

  getClassName(): string {
    return this.constructor.name;
  }

  /**
   * This API serves HTML template. When there are many workloads in one host, we don't list all workloads. This API builds the tooltip text;
   * @param host
   */
  buildMoreWorkloadTooltip(host: ClusterHost): string {
    const wltips = [];
    const workloads = host[HostsComponent.HOST_FIELD_WORKLOADS];
    for (let i = 0; i < workloads.length; i++) {
      if (i >= this.maxWorkloadsPerRow) {
        const workload = workloads[i];
        wltips.push(workload.meta.name);
      }
    }
    return wltips.join(' , ');
  }

  // advance search APIs
  onCancelSearch($event) {
    this.controllerService.invokeInfoToaster('Information', 'Cleared search criteria, Table refreshed.');
    this.dataObjects = this.dataObjectsBackUp;
  }

  /**
   * Execute table search
   * @param field
   * @param order
   */
  onSearchHosts(field = this.tableContainer.sortField, order = this.tableContainer.sortOrder) {
    const searchResults = this.onSearchDataObjects(field, order, 'Host', this.maxSearchRecords, this.advSearchCols, this.dataObjectsBackUp, this.advancedSearchComponent);
    if (searchResults && searchResults.length > 0) {
      this.dataObjects = [];
      this.dataObjects = searchResults;
    }
  }


  searchWorkloads(requirement: FieldsRequirement, data = this.dataObjects): any[] {
    const outputs: any[] = [];
    for (let i = 0; data && i < data.length; i++) {
      const workloads = data[i][HostsComponent.HOST_FIELD_WORKLOADS];
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

  searchDSCs(requirement: FieldsRequirement, data = this.dataObjects): any[] {
    const outputs: any[] = [];
    for (let i = 0; data && i < data.length; i++) {
      const dscs = data[i]._ui[HostsComponent.HOST_FIELD_DSCS];
      // dsc looks like {text: "000c.2981.d8a0", mac: "000c.2981.d8a0", admitted: false}
      for (let k = 0; k < dscs.length; k++) {
        const recordValueID = _.get(dscs[k], ['text']);
        const recordValueMac = _.get(dscs[k], ['mac']);
        const searchValues = requirement.values;
        let operator = String(requirement.operator);
        operator = TableUtility.convertOperator(operator);
        for (let j = 0; j < searchValues.length; j++) {
          const activateFunc = TableUtility.filterConstraints[operator];
          if (activateFunc && (activateFunc(recordValueID, searchValues[j]) || activateFunc(recordValueMac, searchValues[j]))) {
            outputs.push(data[i]);
          }
        }
      }
    }
    return outputs;
  }

  // VS-1185.  UI blocks batch delete if selected hosts conttains any hosts that have associated workloads.
  areSelectedRowsDeletable(): boolean {
    const selectedRows = this.getSelectedDataObjects();
    if (selectedRows.length === 0) {
      return false;
    }
    const list = this.getSelectedDataObjects().filter((rowData) => (rowData._ui[HostsComponent.HOST_FIELD_WORKLOADS] && rowData._ui[HostsComponent.HOST_FIELD_WORKLOADS].length > 0));
    return (list.length === 0);
  }
}
