import { Component, OnDestroy, OnInit, ViewChild, ViewEncapsulation, ChangeDetectorRef } from '@angular/core';
import { MatDialog } from '@angular/material';
import { Animations } from '@app/animations';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { Utility } from '@app/common/Utility';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { ControllerService } from '@app/services/controller.service';
import { WorkloadService } from '@app/services/generated/workload.service';
import { UIConfigsService, Features } from '@app/services/uiconfigs.service';
import { WorkloadWorkload, IWorkloadWorkload, IApiStatus } from '@sdk/v1/models/generated/workload';
import { Table } from 'primeng/table';
import { Subscription, Observable } from 'rxjs';
import { TableCol } from '../shared/tableviewedit';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';
import { TablevieweditAbstract } from '../shared/tableviewedit/tableviewedit.component';
import { ClusterDistributedServiceCard, ClusterHost } from '@sdk/v1/models/generated/cluster';
import { ClusterService } from '@app/services/generated/cluster.service';
import { ObjectsRelationsUtility, WorkloadDSCHostSecurityTuple } from '@app/common/ObjectsRelationsUtility';
import { LabelEditorMetadataModel } from '../shared/labeleditor';
import { SelectItem } from 'primeng/primeng';
import { SecuritySecurityGroup } from '@sdk/v1/models/generated/security';
import { SecurityService } from '@app/services/generated/security.service';

/**
 * Creates the workload page. Uses workload widget for the hero stats
 * section and a PrimeNG data table to list the workloads.
 */
@Component({
  selector: 'app-workload',
  templateUrl: './workload.component.html',
  styleUrls: ['./workload.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class WorkloadComponent extends TablevieweditAbstract<IWorkloadWorkload, WorkloadWorkload> implements OnInit, OnDestroy {

  public static WORKLOAD_FIELD_DSCS: string = 'dscs';
  public static WORKLOAD_FIELD_SECURITYGROUPS: string = 'linkedsecuritygroups';
  // Feature Flags
  hideWorkloadWidgets: boolean = !this.uiconfigsService.isFeatureEnabled('workloadWidgets');

  @ViewChild('workloadTable') workloadTable: Table;

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
    { field: 'spec.host-name', header: 'Host name', class: 'workload-column-host-name', sortable: true, width: 15 },
    { field: 'meta.labels', header: 'Labels', class: 'workload-column-labels', sortable: false, width: 15 },
    { field: 'spec.interfaces', header: 'Interfaces', class: 'workload-column-interfaces', sortable: false },
    { field: 'meta.mod-time', header: 'Modification Time', class: 'workload-column-date', sortable: true, width: '180px' },
    { field: 'meta.creation-time', header: 'Creation Time', class: 'workload-column-date', sortable: true, width: '180px' },
  ];

  // Name of the row we are hovering over
  // When we hover over a row we expand it to show more interface data
  rowHoverName: string;

  // Modal vars
  dialogRef: any;

  isTabComponent: boolean = false;
  disableTableWhenRowExpanded: boolean = true;
  dataObjects: ReadonlyArray<WorkloadWorkload> = [];
  exportFilename: string = 'Venice-workloads';

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

  constructor(
    private workloadService: WorkloadService,
    protected _controllerService: ControllerService,
    protected uiconfigsService: UIConfigsService,
    protected dialog: MatDialog,
    protected cdr: ChangeDetectorRef,
    private clusterService: ClusterService,
    private securityService: SecurityService
  ) {
    super(_controllerService, cdr, uiconfigsService);
  }

  /**
   * Fetch data.
   */
  postNgInit() {
    this.getHosts() ; // prepare hostOptions needed by newworkload component.
    this.getNaples(); // get DSC cards
    this.getSecuritygroups(); // get security groups
    this.getWorkloads(); // Once workloads are available, it will build object-maps
  }

  getHosts() {
    this.hostsEventUtility = new HttpEventUtility<ClusterHost>(ClusterHost, true);
    this.hostObjects = this.hostsEventUtility.array as ReadonlyArray<ClusterHost>;
    const subscription = this.clusterService.WatchHost().subscribe(
      response => {
        this.hostOptions = this.hostsEventUtility.processEvents(response).map( x => {
          return { label: x.meta.name, value: x.meta.name };
        });
      },
      this.controllerService.webSocketErrorHandler('Failed to get Hosts info')
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
      this.controllerService.webSocketErrorHandler('Failed to get Naples')
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
      this.controllerService.webSocketErrorHandler('Failed to get Security Groups info')
    );
    this.subscriptions.push(subscription);
  }

  setDefaultToolbar() {
    let buttons = [];
    if (this.uiconfigsService.isAuthorized(UIRolePermissions.securitynetworksecuritypolicy_create)) {
      buttons = [{
        cssClass: 'global-button-primary global-button-padding',
        text: 'ADD WORKLOAD',
        computeClass: () => this.shouldEnableButtons ? '' : 'global-button-disabled',
        callback: () => { this.createNewObject(); }
      }];
    }
    this._controllerService.setToolbarData({
      buttons: buttons,
      breadcrumb: [{ label: 'Workloads Overview', url: Utility.getBaseUIUrl() + 'workload' }]
    });
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

  /**
   * Fetch workloads.
   */
  getWorkloads() {
    this.workloadEventUtility = new HttpEventUtility<WorkloadWorkload>(WorkloadWorkload);
    this.dataObjects = this.workloadEventUtility.array;
    const subscription = this.workloadService.WatchWorkload().subscribe(
      (response) => {
        this.workloadEventUtility.processEvents(response);
        this.buildObjectsMap();
      },
      this._controllerService.webSocketErrorHandler('Failed to get Workloads')
    );
    this.subscriptions.push(subscription);
  }

  public buildObjectsMap() {
    this.workloadDSCHostTupleMap = ObjectsRelationsUtility.buildWorkloadDscHostSecuritygroupMap(this.dataObjects, this.naples, this.hostObjects, this.securitygroups);
    this.buildWorkloadDSCS();
    this.buildWorkloadSecurityGroups();
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

  getLinkedSecuritygroups(workload: WorkloadWorkload): SecuritySecurityGroup [] {
    if (this.workloadDSCHostTupleMap[workload.meta.name]) {
      return this.workloadDSCHostTupleMap[workload.meta.name].securitygroups;
    } else {
      return [];
    }
  }

  buildWorkloadDSCS() {
    this.dataObjects.forEach( (workload) => {
      const dscs = this.getDSCs(workload);
      workload[WorkloadComponent.WORKLOAD_FIELD_DSCS] = (dscs || dscs.length > 0) ? dscs : [];
    });
  }

  buildWorkloadSecurityGroups() {
    this.dataObjects.forEach( (workload) => {
      const linkedSecuritygroups = this.getLinkedSecuritygroups(workload);
      workload[WorkloadComponent.WORKLOAD_FIELD_SECURITYGROUPS] = (linkedSecuritygroups && linkedSecuritygroups.length > 0) ? linkedSecuritygroups : [];
    });
  }

  editLabels() {
    this.labelEditorMetaData = {
      title: 'Editing worload objects',
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
     if (observables.length > 0 ) {
      const allSuccessSummary = 'Update';
      const partialSuccessSummary = 'Partially update';
      const msg = 'Marked selected ' + updatedWorkloads.length + '  updated.';
      const self = this;
      this.invokeAPIonMultipleRecords(observables, allSuccessSummary, partialSuccessSummary, msg,
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
      delete  workloadObj[WorkloadComponent.WORKLOAD_FIELD_DSCS];  // Remove the workload.dscs field as it is only needed in UI.
      const sub = this.workloadService.UpdateWorkload(name, workloadObj);
      observables.push(sub);
    }
    return observables;
  }


  handleEditCancel($event) {
    this.inLabelEditMode = false;
  }
}
