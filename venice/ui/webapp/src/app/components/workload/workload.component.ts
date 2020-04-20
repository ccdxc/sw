import { ChangeDetectorRef, Component, OnInit, ViewChild, ViewEncapsulation } from '@angular/core';
import { FormArray } from '@angular/forms';
import { MatDialog } from '@angular/material';
import { Animations } from '@app/animations';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { ObjectsRelationsUtility, WorkloadDSCHostSecurityTuple } from '@app/common/ObjectsRelationsUtility';
import { Utility, VeniceObjectCache } from '@app/common/Utility';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { ControllerService } from '@app/services/controller.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { SearchService } from '@app/services/generated/search.service';
import { SecurityService } from '@app/services/generated/security.service';
import { WorkloadService } from '@app/services/generated/workload.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { ClusterDistributedServiceCard, ClusterHost } from '@sdk/v1/models/generated/cluster';
import { FieldsRequirement, SearchSearchRequest, ISearchSearchResponse } from '@sdk/v1/models/generated/search';
import { SecuritySecurityGroup } from '@sdk/v1/models/generated/security';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';
import { IApiStatus, IWorkloadWorkload, WorkloadWorkload } from '@sdk/v1/models/generated/workload';
import * as _ from 'lodash';
import { SelectItem } from 'primeng/primeng';
import { Observable, Subscription } from 'rxjs';
import { SearchUtil } from '../search/SearchUtil';
import { AdvancedSearchComponent } from '../shared/advanced-search/advanced-search.component';
import { LabelEditorMetadataModel } from '../shared/labeleditor';
import { CustomExportMap, TableCol } from '../shared/tableviewedit';
import { TableUtility } from '../shared/tableviewedit/tableutility';
import { PentableComponent } from '../shared/pentable/pentable.component';
import { BaseComponent } from '../base/base.component';
import { Eventtypes } from '@app/enum/eventtypes.enum';

interface WorkloadUiModel {
  dscs: ClusterDistributedServiceCard[];
  linkedsecuritygroups?: SecuritySecurityGroup[];
}

/**
 * Creates the workload page. Uses workload widget for the hero stats
 * section and a PrimeNG data table to list the workloads.
 *
 * There is a AdvancedSearchComponent widget included in this page. User can specify search criteria.
 * As workload page has all the workload records, this.onSearchWorkloads() searches records locally.  It uses TableUtlity.ts searchTable() api
 *
 * Pay attention to  this.buildAdvSearchCols()
 *
 * buildAdvSearchCols() specifis how to feed data advance-search widget and search types. User can these 3 types of search
 *
 *  a. search with table meta data  (e.g search workload.meta.name = xxx)
 *  b. customize search  (e.g workload table contains DSC record which is not in workload object.  We must use customize search)
 *  c. text search  (free form text search)
 */
@Component({
  selector: 'app-workload',
  templateUrl: './workload.component.html',
  styleUrls: ['./workload.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class WorkloadComponent extends BaseComponent implements OnInit {
  // Feature Flags
  hideWorkloadWidgets: boolean = !this.uiconfigsService.isFeatureEnabled('workloadWidgets');

  @ViewChild('workloadTable') workloadTable: PentableComponent;
  @ViewChild('advancedSearchComponent') advancedSearchComponent: AdvancedSearchComponent;
  maxSearchRecords: number = 8000;

  subscriptions: Subscription[] = [];
  // Workload Widget vars
  heroStatsToggled = true;

  totalworkloadsWidget: any;
  newworkloadsWidget: any;
  unprotectedworkloadsWidget: any;
  workloadalertsWidget: any;
  widgets: string[] = ['totalworkloads',
    'newworkloads',
    'unprotectedworkloads',
    'workloadalerts'];
  totalworkload: any;
  selectedWorkloadWidget: string;
  bodyicon: any = {
    margin: {
      top: '9px',
      left: '8px'
    },
    url: '/assets/images/icons/workload/icon-workloads.svg'
  };

  tableIcon: Icon = {
    margin: {
      top: '0px',
      left: '0px',
    },
    matIcon: 'grid_on'
  };

  // Workload table vars

  // Used for the table - when true there is a loading icon displayed
  tableLoading: boolean = false;

  // Used for processing watch events
  workloadEventUtility: HttpEventUtility<WorkloadWorkload>;

  cols: TableCol[] = [
    { field: 'meta.name', header: 'Workload Name', class: 'workload-column-name', sortable: true, width: 15 },
    { field: 'spec.host-name', header: 'Host Name', class: 'workload-column-host-name', sortable: true, width: 15 },
    { field: 'meta.labels', header: 'Labels', class: 'workload-column-labels', sortable: false, width: 15 },
    { field: 'spec.interfaces', header: 'Interfaces', class: 'workload-column-interfaces', sortable: false },
    { field: 'meta.mod-time', header: 'Modification Time', class: 'workload-column-date', sortable: true, width: '180px' },
    { field: 'meta.creation-time', header: 'Creation Time', class: 'workload-column-date', sortable: true, width: '180px' },
  ];

  // advance search variables
  advSearchCols: TableCol[] = [];
  fieldFormArray = new FormArray([]);

  // Name of the row we are hovering over
  // When we hover over a row we expand it to show more interface data
  rowHoverName: string;

  // Modal vars
  dialogRef: any;

  isTabComponent: boolean = false;
  disableTableWhenRowExpanded: boolean = true;
  dataObjects: ReadonlyArray<WorkloadWorkload> = [];
  dataObjectsBackUp: ReadonlyArray<WorkloadWorkload> = null;
  exportFilename: string = 'PSM-workloads';
  exportMap: CustomExportMap = {
    'spec.interfaces': (opts): string => {
      const outputs = [];
      const rowData = opts.data;
      const keys = this.getKeys(rowData.spec.interfaces);
      for (let index = 0; index < keys.length; index++) {
        if (rowData.spec.interfaces[index]['ip-addresses'] &&
          rowData.spec.interfaces[index]['ip-addresses'].length > 0) {
          outputs.push(rowData.spec.interfaces[index]['mac-address'] + '-' + rowData.spec.interfaces[index]['ip-addresses'].join(', '));
        } else if (!rowData.spec.interfaces[index]['ip-addresses'] || rowData.spec.interfaces[index]['ip-addresses'].length === 0) {
          outputs.push(rowData.spec.interfaces[index]['mac-address']);
        }
      }
      return outputs.join(' ');
    },
    'meta.labels': (opts): string => {
      return this.formatLabels(opts.data.meta.labels);
    }
  };

  naplesEventUtility: HttpEventUtility<ClusterDistributedServiceCard>;
  naples: ReadonlyArray<ClusterDistributedServiceCard> = [];
  workloadDSCHostTupleMap: { [key: string]: WorkloadDSCHostSecurityTuple } = {};
  labelEditorMetaData: LabelEditorMetadataModel;
  inLabelEditMode: boolean = false;

  hostsEventUtility: HttpEventUtility<ClusterHost>;
  hostObjects: ReadonlyArray<ClusterHost>;
  hostOptions: SelectItem[] = [];

  securitygroupsEventUtility: HttpEventUtility<SecuritySecurityGroup>;
  securitygroups: ReadonlyArray<SecuritySecurityGroup>;
  searchWorkloadCount: number = 0;
  starttimeWatchWorkload: number;

  dataMappingTimer: NodeJS.Timer;

  /**
   * This API is to assist searchWorkloadInterfaces().  We use public static function is to avoid "this" keyword confusion
   * @param requirement
   */
  public static getsearchWorkloadInterfaceKey(requirement: FieldsRequirement): string[] {
    const key = requirement.key;
    if (key === 'interface.mac') {
      return ['mac-address'];
    }
    if (key === 'interface.micro_seg_vlan') {
      return ['micro-seg-vlan'];
    }
    if (key === 'interface.external_vlan') {
      return ['external-vlan'];
    }
    if (key === 'interface.ipaddress') {
      return ['ip-addresses'];
    }
  }



  constructor(
    private workloadService: WorkloadService,
    protected _controllerService: ControllerService,
    protected uiconfigsService: UIConfigsService,
    protected dialog: MatDialog,
    protected cdr: ChangeDetectorRef,
    private clusterService: ClusterService,
    private securityService: SecurityService,
    protected searchService: SearchService
  ) {
    super(_controllerService, uiconfigsService);
  }

  /**
   * Fetch data.
   */
  ngOnInit() {
    this._controllerService.publish(Eventtypes.COMPONENT_INIT, {
      'component': this.getClassName(), 'state':
        Eventtypes.COMPONENT_INIT
    });
    this.setDefaultToolbar();
    this.buildAdvSearchCols();
    this.tableLoading = true;
    this.getHosts(); // prepare hostOptions needed by newworkload component.
    this.getNaples(); // get DSC cards
    this.getSecuritygroups(); // get security groups
    this.getWorkloads(); // Once workloads are available, it will build object-maps
  }

  getHosts() {
    this.hostsEventUtility = new HttpEventUtility<ClusterHost>(ClusterHost, true);
    this.hostObjects = this.hostsEventUtility.array as ReadonlyArray<ClusterHost>;
    const subscription = this.clusterService.WatchHost().subscribe(
      response => {
        this.hostOptions = this.hostsEventUtility.processEvents(response).map(x => {
          return { label: x.meta.name, value: x.meta.name };
        });
      },
      this._controllerService.webSocketErrorHandler('Failed to get Hosts info')
    );
    this.subscriptions.push(subscription);
  }

  getNaples() {
    this.naplesEventUtility = new HttpEventUtility<ClusterDistributedServiceCard>(ClusterDistributedServiceCard);
    this.naples = this.naplesEventUtility.array as ReadonlyArray<ClusterDistributedServiceCard>;
    const subscription = this.clusterService.WatchDistributedServiceCard().subscribe(
      response => {
        this.naplesEventUtility.processEvents(response);
      },
      this._controllerService.webSocketErrorHandler('Failed to get Naples')
    );
    this.subscriptions.push(subscription); // add subscription to list, so that it will be cleaned up when component is destroyed.
  }

  getSecuritygroups() {
    this.securitygroupsEventUtility = new HttpEventUtility<SecuritySecurityGroup>(SecuritySecurityGroup, true);
    this.securitygroups = this.securitygroupsEventUtility.array as ReadonlyArray<SecuritySecurityGroup>;
    const subscription = this.securityService.WatchSecurityGroup().subscribe(
      response => {
        this.securitygroupsEventUtility.processEvents(response);
      },
      this._controllerService.webSocketErrorHandler('Failed to get Security Groups info')
    );
    this.subscriptions.push(subscription);
  }

  setDefaultToolbar() {
    let buttons = [];
    if (this.uiconfigsService.isAuthorized(UIRolePermissions.securitynetworksecuritypolicy_create)) {
      buttons = [{
        cssClass: 'global-button-primary global-button-padding',
        text: 'ADD WORKLOAD',
        callback: () => { this.workloadTable.createNewObject(); },
        computeClass: () => !this.workloadTable.showRowExpand ? '' : 'global-button-disabled'
      }];
    }
    this._controllerService.setToolbarData({
      buttons: buttons,
      breadcrumb: [{ label: 'Workloads Overview', url: Utility.getBaseUIUrl() + 'workload' }]
    });
  }

  buildAdvSearchCols() {
    this.advSearchCols = this.cols.filter((col: TableCol) => {
      return (col.field !== 'spec.interfaces');
    });
    this.advSearchCols.push(
      {
        field: 'DSC', header: 'DSC Name', localSearch: true, kind: 'DistributedServiceCard',
        filterfunction: this.searchDSC,
        advancedSearchOperator: SearchUtil.stringOperators
      }
    );
    this.advSearchCols.push(
      {
        field: 'interface.mac', header: 'Interface MAC', localSearch: true, kind: 'Workload',
        filterfunction: this.searchWorkloadInterfaces,
        advancedSearchOperator: SearchUtil.stringOperators
      }
    );
    this.advSearchCols.push(
      {
        field: 'interface.micro_seg_vlan', header: 'Interface Micro Seg-VLAN', localSearch: true, kind: 'Workload',
        filterfunction: this.searchWorkloadInterfaces,
        advancedSearchOperator: SearchUtil.stringOperators
      }
    );
    this.advSearchCols.push(
      {
        field: 'interface.external_vlan', header: 'Interface External-VLAN', localSearch: true, kind: 'Workload',
        filterfunction: this.searchWorkloadInterfaces,
        advancedSearchOperator: SearchUtil.stringOperators
      }
    );
    this.advSearchCols.push(
      {
        field: 'interface.ipaddress', header: 'Interface IP Address', localSearch: true, kind: 'Workload',
        filterfunction: this.searchWorkloadInterfaces,
        advancedSearchOperator: SearchUtil.stringOperators
      }
    );
  }

  // Commenting out as modal isn't part of 2018-August release
  //
  // Component variables
  // securityGroups: string[] = ['SG1', 'SG2'];
  // labels: any = { 'Loc': ['NL', 'AMS'], 'Env': ['test', 'prod'] };
  //
  // generateModalAddToGroup() {
  //   this.dialogRef = this.dialog.open(WorkloadModalComponent, {
  //     panelClass: 'workload-modal',
  //     width: '898px',
  //     hasBackdrop: true,
  //     data: {
  //       securityGroups: this.securityGroups,
  //       selectedWorkloads: this.selectedWorkloads,
  //       labels: this.labels
  //     }
  //   });
  // }
  //

  toggleHeroStats() {
    this.heroStatsToggled = !this.heroStatsToggled;
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

  /**
   * TODO: This API is not used. We may want to delete it. Let Rohan check it out first
   * @param interfacesObj
   */
  formatInterfaces(interfacesObj) {
    const interfaces = [];
    Object.keys(interfacesObj).forEach((key) => {
      let ret = key + '  -  ' + interfacesObj[key]['ip'].join(',    ') + ' \n';
      const network = interfacesObj[key]['network'];
      if (network != null) {
        ret += '    Network: ' + network + '    ';
      }
      const ipAddress = interfacesObj[key]['ip-addresses'];
      if (ipAddress != null) {
        ret += '    IP Addresses: ' + ipAddress.split(',');
      }
      const microSegVlan = interfacesObj[key]['micro-seg-vlan'];
      if (microSegVlan != null) {
        ret += '    Micro-seg VLAN: ' + microSegVlan + '    ';
      }
      const externalVlan = interfacesObj[key]['external-vlan'];
      if (microSegVlan != null) {
        ret += '    External VLAN: ' + microSegVlan;
      }
      interfaces.push(ret);
    });
    return interfaces.join('\n');
  }


  displayColumn(data, col): any {
    const fields = col.field.split('.');
    const value = Utility.getObjectValueByPropertyPath(data, fields);
    const column = col.field;
    switch (column) {
      case 'meta.labels':
        return this.formatLabels(data.meta.labels);
      default:
        return Array.isArray(value) ? JSON.stringify(value, null, 2) : value;
    }
  }

  /**
   * Overide super's API
   * It will return this Component name
   */
  getClassName(): string {
    return this.constructor.name;
  }

  /**
   * Hook called by html when user mouses over a row
   */
  rowHover(rowData) {
    this.rowHoverName = rowData.meta.name;
  }

  /**
   * Hook called by html when user's mouse leaves a row
   */
  resetHover(rowData) {
    // We check if the  row that we are leaving
    // is the row that is saved so that if the rowhover
    // fires for another row before this leave we don't unset it.
    if (this.rowHoverName === rowData.meta.name) {
      this.rowHoverName = null;
    }
  }

  getWorkloads() {
    const query: SearchSearchRequest = Utility.buildObjectTotalSearchQuery('Workload');
    const searchDSCTotalSubscription = this.searchService.PostQuery(query).subscribe(
      resp => {
        if (resp) {
          const body = resp.body as ISearchSearchResponse;
          const dscTotal = parseInt(body['total-hits'], 10);
          if (dscTotal > 0) {
            this.searchWorkloadCount = dscTotal;
          } else {
            this.tableLoading = false;
          }
          this.starttimeWatchWorkload = (new Date()).getTime();
          this.watchWorkloads();
        }
      },
      this._controllerService.webSocketErrorHandler('Failed to get Distributed Services Cards'),
    );
    this.subscriptions.push(searchDSCTotalSubscription);
  }

  watchWorkloads() {
    this.workloadEventUtility = new HttpEventUtility<WorkloadWorkload>(WorkloadWorkload);
    this.dataObjects = this.workloadEventUtility.array;
    const subscription = this.workloadService.WatchWorkload().subscribe(
      (response) => {
        this.workloadEventUtility.processEvents(response);
        const currenttimeWatchWorkload = (new Date()).getTime();
        const timeDiff = currenttimeWatchWorkload - this.starttimeWatchWorkload;
        const timeOut = (timeDiff > 2 * 60 * 1000);
        // wait up to 2 * 60 seconds, then turn off tableLoading.  Or current # of  workloads is up to 90% of searchWorkloadCount.
        if (timeOut || (this.dataObjects && this.dataObjects.length > 0.9 * this.searchWorkloadCount)) {
          this.tableLoading = false;
        }

        // if there are too many workloads in Venice, take longer time so table has time to render.
        if (this.searchWorkloadCount > 300) {
          clearTimeout(this.dataMappingTimer);
          this.dataMappingTimer = setTimeout(() => {
            this.mapData();
          }, 300);
        } else {
          this.mapData();
        }

        // once we have get more workload objects than searchWorkloadCount, we reset this.searchWorkloadCount. It is need in DestroyHook()
        if (this.dataObjects.length >= this.searchWorkloadCount) {
          this.searchWorkloadCount = this.dataObjects.length;
        }
      },
      (error) => {
        this.tableLoading = false;
        this._controllerService.invokeRESTErrorToaster('Failed to get workloads', error);
      }
    );
    this.subscriptions.push(subscription);
  }

  mapData() {
    this.buildObjectsMap();
    this.dataObjectsBackUp = Utility.getLodash().cloneDeepWith(this.dataObjects); // make a copy of server provided data
  }

  public buildObjectsMap() {
    this.workloadDSCHostTupleMap = ObjectsRelationsUtility.buildWorkloadDscHostSecuritygroupMap(this.dataObjects, this.naples, this.hostObjects, this.securitygroups);
    this.buildUiModel();
  }

  buildUiModel() {
    this.dataObjects.forEach((workload, idx) => {
      const dscs = this.getDSCs(workload);
      const linkedSecuritygroups = this.getLinkedSecuritygroups(workload);

      const workloadUiModel: WorkloadUiModel = {
        dscs: (dscs || dscs.length > 0) ? dscs : [],
        linkedsecuritygroups: (linkedSecuritygroups && linkedSecuritygroups.length > 0) ? linkedSecuritygroups : [],
      };

      workload._ui = workloadUiModel;
    });
  }

  deleteRecord(object: WorkloadWorkload): Observable<{ body: IWorkloadWorkload | IApiStatus | Error, statusCode: number }> {
    return this.workloadService.DeleteWorkload(object.meta.name);
  }

  generateDeleteConfirmMsg(object: IWorkloadWorkload) {
    return 'Are you sure you want to delete workload ' + object.meta.name;
  }

  generateDeleteSuccessMsg(object: IWorkloadWorkload) {
    return 'Deleted workload ' + object.meta.name;
  }

  /**
   * Used by html to get an object's keys for iterating over.
   */
  getKeys(obj) {
    if (obj != null) {
      return Object.keys(obj);
    } else {
      return [];
    }
  }

  /**
   * This API serves html template
   */
  getDSCs(workload: WorkloadWorkload): ClusterDistributedServiceCard[] {
    if (this.workloadDSCHostTupleMap[workload.meta.name]) {
      return this.workloadDSCHostTupleMap[workload.meta.name].dscs;
    } else {
      return [];
    }
  }

  getLinkedSecuritygroups(workload: WorkloadWorkload): SecuritySecurityGroup[] {
    if (this.workloadDSCHostTupleMap[workload.meta.name]) {
      return this.workloadDSCHostTupleMap[workload.meta.name].securitygroups;
    } else {
      return [];
    }
  }

  editLabels() {
    this.labelEditorMetaData = {
      title: 'Edit workload labels',
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

  handleEditSave(updatedWorkloads: WorkloadWorkload[]) {
    this.updateWithForkjoin(updatedWorkloads);
  }


  updateWithForkjoin(updatedWorkloads: WorkloadWorkload[]) {
    const observables = this.getObservables(updatedWorkloads);
    if (observables.length > 0) {
      const allSuccessSummary = 'Update';
      const partialSuccessSummary = 'Partially update';
      const msg = 'Marked selected ' + updatedWorkloads.length + '  updated.';
      const self = this;
      this.workloadTable.invokeAPIonMultipleRecords(observables, allSuccessSummary, partialSuccessSummary, msg,
        () => {
          self.handleEditCancel(null);
        }, // onSuccess callback
        () => {
          self.handleEditCancel(null);
        }  // onFailure call back
      );
    }
  }

  getObservables(updatedWorkloads: WorkloadWorkload[]): Observable<any>[] {
    const observables: Observable<any>[] = [];
    for (const workloadObj of updatedWorkloads) {
      const name = workloadObj.meta.name;
      const sub = this.workloadService.UpdateWorkload(name, workloadObj);
      observables.push(sub);
    }
    return observables;
  }


  handleEditCancel($event) {
    this.inLabelEditMode = false;
  }

  // advance search APIs
  onCancelSearch($event) {
    this._controllerService.invokeInfoToaster('Information', 'Cleared search criteria, Table refreshed.');
    this.dataObjects = this.dataObjectsBackUp;
  }

  /**
   * Execute table search
   * @param field
   * @param order
   */
  onSearchWorkloads(field = this.workloadTable.sortField, order = this.workloadTable.sortOrder) {
    const searchResults = this.workloadTable.onSearchDataObjects(field, order, 'Workload', this.maxSearchRecords, this.advSearchCols, this.dataObjectsBackUp, this.advancedSearchComponent);
    if (searchResults && searchResults.length > 0) {
      this.dataObjects = [];
      this.dataObjects = searchResults;
    }
  }


  /* commont it out for now
    private _callSearchRESTAPI(searchSearchRequest: SearchSearchRequest) {

    // TODO: search on workload has problem.
    // request JSON
    //    {"query-string":null,"from":0,"max-results":8000,"sort-by":"meta.mod-time","sort-order":"descending","mode":null,"query":{"texts":[{"text":[]}],"categories":[],"kinds":["Workload"],"fields":{"requirements":[{"key":"meta.name","operator":"in","values":["wl2"]}]},"labels":{"requirements":[]}},"tenants":[],"aggregate":true}
    //    return nothing
    //    If I change "operator": "in", to "operator": In",
    //    it returns  "/venice/config/workload/workloads/default/vcenter-vm-53".  Should return "wl2"

    const subscription = this.searchService.PostQuery(searchSearchRequest).subscribe(
      response => {
        const data: SearchSearchResponse = response.body as SearchSearchResponse;
        let objects = data.entries;
        if (!objects || objects.length === 0) {
          this.controllerService.invokeInfoToaster('Information', 'No workload found. Please change search criteria.');
          objects = [];
        }
        const entries = [];
        const remoteSearchResult = [];
        for (let k = 0; k < objects.length; k++) {
          entries.push(objects[k].object); // objects[k] is a SearchEntry object
          remoteSearchResult.push(entries[k].meta.name);
        }
      },
      (error) => {
        this.controllerService.invokeRESTErrorToaster('Failed to search workloads', error);
      }
    );
    this.subscriptions.push(subscription);
  } */

  searchDSC(requirement: FieldsRequirement, data = this.dataObjects): any[] {
    const outputs: any[] = [];
    for (let i = 0; data && i < data.length; i++) {
      const dscs = data[i]['dscs'];
      for (let k = 0; k < dscs.length; k++) {
        const recordValue = _.get(dscs[k], ['spec', 'id']);
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

  searchWorkloadInterfaces(requirement: FieldsRequirement, data = this.dataObjects): any[] {
    const outputs: any[] = [];
    for (let i = 0; data && i < data.length; i++) {
      const interfaces = data[i].spec.interfaces;
      for (let k = 0; k < interfaces.length; k++) {
        const recordValue = _.get(interfaces[k], WorkloadComponent.getsearchWorkloadInterfaceKey(requirement));
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

  isWorkloadSystemGenerated(rowData: WorkloadWorkload): boolean {
    if (rowData && rowData.meta.labels) {
      if (rowData.meta.labels['io.pensando.namespace']) {
        return true;
      }
      if (rowData.meta.labels['io.pensando.orch-name']) {
        return true;
      }
      if (rowData.meta.labels['io.pensando.vcenter.display-name']) {
        return true;
      }
    }
    return false;
  }

  disableMultiDelIcons() {
    if (this.getSelectedDataObjects() && this.getSelectedDataObjects().length > 0) {
      if (Utility.getInstance().isAdmin()) {
        return false;
      }
      const generatedWorkload: WorkloadWorkload = this.getSelectedDataObjects().find(
        item => this.isWorkloadSystemGenerated(item)
      );
      if (generatedWorkload) {
        return true;
      }
    }
    return false;
  }

  onDeleteSelectedWorkloads(event) {
    if (!this.disableMultiDelIcons()) {
      this.onDeleteSelectedRows(event);
    }
  }

  ngOnDestroyHook() {
    // we save workloads to host only we have full set of workloads.
    if (this.dataObjects && this.dataObjects.length >= this.searchWorkloadCount) {
      const ts = (new Date()).getTime();
      const hour = Utility.DEFAULT_CACHE_DURATION;

      const workloadVeniceObjectCache: VeniceObjectCache = {
        timestamp: ts,
        duration: hour,
        data: this.dataObjects as any[]
      };

      Utility.getInstance().setVeniceObjectCache('Workload', workloadVeniceObjectCache);
    }
  }

  creationFormClose() {
    this.workloadTable.creationFormClose();
  }

  editFormClose(rowData) {
    if (this.workloadTable.showRowExpand) {
      this.workloadTable.toggleRow(rowData);
    }
  }

  expandRowRequest(event, rowData) {
    if (!this.workloadTable.showRowExpand) {
      this.workloadTable.toggleRow(rowData, event);
    }
  }

  getSelectedDataObjects() {
    return this.workloadTable.getSelectedDataObjects();
  }

  hasSelectedRows(): boolean {
    return this.workloadTable.getSelectedDataObjects().length > 0;
  }

  onColumnSelectChange(event) {
    this.workloadTable.onColumnSelectChange(event);
  }

  onDeleteRecord(event, object) {
    this.workloadTable.onDeleteRecord(
      event,
      object,
      this.generateDeleteConfirmMsg(object),
      this.generateDeleteSuccessMsg(object),
      this.deleteRecord.bind(this)
    );
  }

  onDeleteSelectedRows(event) {
    this.workloadTable.onDeleteSelectedRows(event, this.deleteRecord.bind(this));
  }
}
