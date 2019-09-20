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
    { field: 'meta.labels', header: 'Labels', class: 'workload-column-labels', sortable: false, width: 15},
    { field: 'spec.interfaces', header: 'Interfaces', class: 'workload-column-interfaces', sortable: false},
    { field: 'meta.mod-time', header: 'Modification Time', class: 'workload-column-date', sortable: true, width: '180px' },
    { field: 'meta.creation-time', header: 'Creation Time', class: 'workload-column-date', sortable: true, width: '180px' },
  ];

  // Name of the row we are hovering over
  // When we hover over a row we expand it to show more interface data
  rowHoverName: string;

  // Modal vars
  dialogRef: any;
  securityGroups: string[] = ['SG1', 'SG2'];
  labels: any = { 'Loc': ['NL', 'AMS'], 'Env': ['test', 'prod'] };

  isTabComponent: boolean = false;
  disableTableWhenRowExpanded: boolean  = true;
  dataObjects: ReadonlyArray<WorkloadWorkload> = [];
  exportFilename: string = 'Venice-workloads';



  constructor(
    private workloadService: WorkloadService,
    protected _controllerService: ControllerService,
    protected uiconfigsService: UIConfigsService,
    protected dialog: MatDialog,
    protected cdr: ChangeDetectorRef,
  ) {
    super(_controllerService, cdr, uiconfigsService);
  }

  postNgInit() {
    this.getWorkloads();
  }

  setDefaultToolbar() {
    let buttons = [];
    if (this.uiconfigsService.isAuthorized(UIRolePermissions.securitynetworksecuritypolicy_create) && this.uiconfigsService.isFeatureEnabled(Features.createWorkload)) {
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

  // Commenting out as modal isn't part of August release
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
    this.workloadEventUtility = new HttpEventUtility<WorkloadWorkload>(WorkloadWorkload);
    this.dataObjects = this.workloadEventUtility.array;
    const subscription = this.workloadService.WatchWorkload().subscribe(
      (response) => {
        this.workloadEventUtility.processEvents(response);
        this.dataObjects = Utility.sortDate(this.dataObjects, ['meta', 'mod-time'], -1);  // VS-791, sort workoad object by mod-time. UI table will match web-socket return.
      },
      this._controllerService.webSocketErrorHandler('Failed to get Workloads')
    );
    this.subscriptions.push(subscription);
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
}
