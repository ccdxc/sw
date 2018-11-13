import { Component, OnDestroy, OnInit, ViewChild, ViewEncapsulation } from '@angular/core';
import { MatDialog } from '@angular/material';
import { Animations } from '@app/animations';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { Utility } from '@app/common/Utility';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { ControllerService } from '@app/services/controller.service';
import { WorkloadService } from '@app/services/generated/workload.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { IApiStatus, WorkloadWorkload } from '@sdk/v1/models/generated/workload';
import { Table } from 'primeng/table';
import { Subscription } from 'rxjs';
import { BaseComponent } from '../base/base.component';
import { MessageService } from 'primeng/primeng';

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
export class WorkloadComponent extends BaseComponent implements OnInit, OnDestroy {
  // Feature Flags
  hideWorkloadWidgets: boolean = this.uiconfigsService.isFeatureDisabled('WorkloadWidgets');

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

  // Holds all the workloads
  workloads: ReadonlyArray<WorkloadWorkload>;

  // Used for the table - when true there is a loading icon displayed
  tableLoading: boolean = false;

  // Used for processing watch events
  workloadEventUtility: HttpEventUtility<WorkloadWorkload>;

  cols: any[] = [
    { field: 'meta.name', header: 'Workload Name', class: 'workload-column-name', sortable: false },
    { field: 'spec.host-name', header: 'Host name', class: 'workload-column-host-name', sortable: false },
    { field: 'meta.labels', header: 'Labels', class: 'workload-column-labels', sortable: false },
    { field: 'spec.interfaces', header: 'Interfaces', class: 'workload-column-interfaces', sortable: false },
    { field: 'meta.mod-time', header: 'Modification Time', class: 'workload-column-date', sortable: false },
    { field: 'meta.creation-time', header: 'Creation Time', class: 'workload-column-date', sortable: false },
  ];

  // Name of the row we are hovering over
  // When we hover over a row we expand it to show more interface data
  rowHoverName: string;

  // Modal vars
  dialogRef: any;
  securityGroups: string[] = ['SG1', 'SG2'];
  labels: any = { 'Loc': ['NL', 'AMS'], 'Env': ['test', 'prod'] };


  constructor(
    private workloadService: WorkloadService,
    protected _controllerService: ControllerService,
    protected uiconfigsService: UIConfigsService,
    protected dialog: MatDialog,
    protected messageService: MessageService
  ) {
    super(_controllerService, messageService);
  }

  ngOnInit() {
    this._controllerService.publish(Eventtypes.COMPONENT_INIT, {
      'component': 'WorkloadComponent', 'state':
        Eventtypes.COMPONENT_INIT
    });
    // Setting the toolbar of the app
    this._controllerService.setToolbarData({
      buttons: [],
      breadcrumb: [{ label: 'Workloads Overview', url: Utility.getBaseUIUrl() + 'workloads' }]
    });
    // Fetching workload items
    this.getWorkloads();
    // Default selected workloadwidget
    this.selectedWorkloadWidget = 'totalworkloads';
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

  ngOnDestroy() {
    if (this.subscriptions != null) {
      this.subscriptions.forEach(subscription => {
        subscription.unsubscribe();
      });
    }
    this._controllerService.publish(Eventtypes.COMPONENT_DESTROY, {
      'component': 'WorkloadComponent', 'state':
        Eventtypes.COMPONENT_DESTROY
    });
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

  formatInterfaces(interfacesObj) {
    const interfaces = [];
    Object.keys(interfacesObj).forEach((key) => {
      let ret = key + '  -  ' + interfacesObj[key]['ip'].join(',    ') + ' \n';
      const network = interfacesObj[key]['network'];
      if (network != null) {
        ret += '    Network: ' + network + '    ';
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
    this.workloads = this.workloadEventUtility.array;
    const subscription = this.workloadService.WatchWorkload().subscribe(
      (response) => {
        const body: any = response.body;
        this.workloadEventUtility.processEvents(body);
      },
      this.restErrorHandler('Failed to get workloads')
    );
    this.subscriptions.push(subscription);
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
