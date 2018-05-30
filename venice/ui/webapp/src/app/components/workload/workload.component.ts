import { Component, OnChanges, OnDestroy, OnInit, ViewChild, ViewEncapsulation } from '@angular/core';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { ControllerService } from '@app/services/controller.service';
import { WorkloadService } from '@app/services/workload.service';
import { Table } from 'primeng/table';
import { Subscription } from 'rxjs/Subscription';

import { BaseComponent } from '../base/base.component';
import { WorkloadModalComponent } from '@app/components/workload/workloadmodal/workloadmodal.component';
import { MatDialog } from '@angular/material';

/**
 * Creates the workload page. Uses workload widget for the hero stats
 * section and a PrimeNG data table to list the workloads.
 */
@Component({
  selector: 'app-workload',
  templateUrl: './workload.component.html',
  styleUrls: ['./workload.component.scss'],
  encapsulation: ViewEncapsulation.None
})
export class WorkloadComponent extends BaseComponent implements OnInit, OnDestroy, OnChanges {
  @ViewChild('workloadtable') workloadTable: Table;

  private subscription: Subscription;
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

  // Workload table vars
  globalFilterFields: string[] = ['name'];
  items: any;
  workloadCount: any = {
    shown: 10,
    total: 200
  };
  idToFilter: any = {
    newworkloads: 'neutral',
    unprotectedworkloads: 'deleted',
    workloadalerts: 'alert',
    savedChangesToggle: 'changed',
  };
  selectedWorkloads: any[] = [];
  loading = false;

  workloads: any;
  cols: any[] = [
    { field: 'name', header: 'Workload Name' },
    { field: 'labels', header: 'Labels' },
    { field: 'securityGroups', header: 'Security groups' },
    { field: 'orchestration', header: 'Orchestration' },
    { field: 'loadBalancer', header: 'Load Balancer' },
    { field: 'appId', header: 'App ID' },
  ];

  // Modal vars
  dialogRef: any;
  securityGroups: string[] = ['SG1', 'SG2'];
  labels: any = { 'Loc': ['NL', 'AMS'], 'Env': ['test', 'prod'] };

  constructor(
    private _workloadService: WorkloadService,
    protected _controllerService: ControllerService,
    protected dialog: MatDialog,
  ) {
    super(_controllerService);
  }

  ngOnInit() {
    if (!this._controllerService.isUserLogin()) {
      this._controllerService.publish(Eventtypes.NOT_YET_LOGIN, {});
    } else {
      this._controllerService.publish(Eventtypes.COMPONENT_INIT, {
        'component': 'WorkloadComponent', 'state':
          Eventtypes.COMPONENT_INIT
      });
      // Setting the toolbar of the app
      this._controllerService.setToolbarData({
        buttons: [
          {
            cssClass: 'global-button-primary workload-button',
            text: 'NEW WORKLOAD',
            callback: () => { this.buttoncallback('new workload'); }
          },
          {
            cssClass: 'global-button-primary workload-button workload-toolbar-button',
            text: 'COMMIT CHANGES',
            callback: () => { this.buttoncallback('commit changes'); }
          }],
        breadcrumb: [{ label: 'Workloads Overview', url: '' }]
      });
      // Fetching workload items
      this.getItems();
      // Default selected workloadwidget
      this.selectedWorkloadWidget = 'totalworkloads';
    }
  }

  buttoncallback(text) {
    console.log(text);
  }

  generateModalAddToGroup() {
    this.dialogRef = this.dialog.open(WorkloadModalComponent, {
      panelClass: 'workload-modal',
      width: '898px',
      hasBackdrop: true,
      data: {
        securityGroups: this.securityGroups,
        selectedWorkloads: this.selectedWorkloads,
        labels: this.labels
      }
    });
  }

  ngOnDestroy() {
    if (this.subscription != null) {
      this.subscription.unsubscribe();
    }
    this._controllerService.publish(Eventtypes.COMPONENT_DESTROY, {
      'component': 'WorkloadComponent', 'state':
        Eventtypes.COMPONENT_DESTROY
    });
  }


  ngOnChanges() {
  }

  hasWorkloadsSelected() {
    return this.selectedWorkloads.length > 0;
  }

  workloadTableRowStyle(rowData) {
    const classMapping = {
      neutral: 'workload-item-neutral ',
      alert: 'workload-item-alert ',
      deleted: 'workload-item-deleted',
      changed: 'workload-item-changed'
    };
    return classMapping[rowData.state];
  }

  /**
   * Overide super's API
   * It will return this Component name
   */
  getClassName(): string {
    return this.constructor.name;
  }

  getItems() {
    this.subscription = this._workloadService.getItems().subscribe(data => {
      this.workloads = data;
      this.workloadCount.total = this.workloads.length;
    });
  }

  workloadClickHandler(id) {
    this.selectedWorkloadWidget = id;
    this.applyFilterById(id);
  }

  workloadTableToggleHandler($event) {
    if ($event.checked) {
      this.workloadTable.filter(this.idToFilter['savedChangesToggle'], 'state', 'equals');
    } else {
      this.applyFilterById(this.selectedWorkloadWidget);
    }
  }

  applyFilterById(id) {
    this.workloadTable.filter(this.idToFilter[id], 'state', 'equals');
  }

  onWorkloadtableDeleteRecord($event, record) {
    console.log('WorkloadComponent.onWorkloadtableDeleteRecord()', record);
  }

  workloadTableAddToGroup($event) {
    if (this.selectedWorkloads.length !== 0) {
      this.generateModalAddToGroup();
    }
  }

  workloadTableAddLabel($event) {
    if (this.selectedWorkloads.length !== 0) {
      console.log('add label');
    }
  }

  workloadTableDeleteWorkload($event) {
    if (this.selectedWorkloads.length !== 0) {
      console.log('add label');
    }
  }

  workloadTableMoreActions($event) {
    if (this.selectedWorkloads.length !== 0) {
      console.log('more actions clicked', $event);
    }
  }

  onWorkloadTableArchiveRecord($event) {
    console.log('archive', $event);
  }

  onWorkloadTableDeleteRecord($event) {
    console.log('delete', $event);
  }

  toggleHeroStats() {
    this.heroStatsToggled = !this.heroStatsToggled;
  }

}
