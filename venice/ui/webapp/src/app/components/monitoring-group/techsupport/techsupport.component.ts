import { Component, OnInit, ViewEncapsulation, OnDestroy, ViewChild } from '@angular/core';
import { Table } from 'primeng/table';
import { Subscription } from 'rxjs';
import { Animations } from '@app/animations';

import { ControllerService } from '@app/services/controller.service';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { BaseComponent } from '@app/components/base/base.component';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { Utility } from '@app/common/Utility';
import { MonitoringTechSupportRequest } from '@sdk/v1/models/generated/monitoring';
import { LazyrenderComponent } from '@app/components/shared/lazyrender/lazyrender.component';




@Component({
  selector: 'app-techsupport',
  templateUrl: './techsupport.component.html',
  styleUrls: ['./techsupport.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class TechsupportComponent extends BaseComponent implements OnInit, OnDestroy {

  @ViewChild('techSupportRequestsTable') techSupportRequestsTable: Table;
  @ViewChild(LazyrenderComponent) lazyRenderWrapper: LazyrenderComponent;

  selectedTechsupportRequest: MonitoringTechSupportRequest = null;
  subscriptions: Subscription[] = [];

  // techsupports related variables
  techsupportrequestsEventUtility: HttpEventUtility<MonitoringTechSupportRequest>;
  techsupportrequestsSubscription: Subscription;
  techsupportrequests: ReadonlyArray<MonitoringTechSupportRequest> = [];

  bodyicon: any = {
    margin: {
      top: '9px',
      left: '8px',
    },
    url: '/assets/images/icons/security/ico-app-black.svg',   // TODO: wait for new svg file
  };

  cols: any[] = [
    { field: 'meta.name', header: 'Name', class: 'techsupportrequests-column-common techsupportrequests-column-name', sortable: true },
    { field: 'meta.mod-time', header: 'Time', class: 'techsupportrequests-column-common techsupportrequests-column-date', sortable: true },
    { field: 'spec.node-selector', header: 'Selected Nodes', class: 'techsupportrequests-column-common techsupportrequests-column-node_selector', sortable: false },
    { field: 'spec.collection-selector', header: 'Collection-Selector', class: 'techsupportrequests-column-common techsupportrequests-column-collection_selector', sortable: false },
    { field: 'status.status', header: 'Status', class: 'techsupportrequests-column-common techsupportrequests-column-status_status', sortable: true },
    { field: 'spec.verbosity', header: 'Verbosity', class: 'techsupportrequests-column-common techsupportrequests-column-spec_verbosity', sortable: true }
  ];

  creatingMode: boolean = false;

  constructor(protected _controllerService: ControllerService,
    protected monitoringService: MonitoringService
  ) {
    super(_controllerService, null);
  }

  /**
  * Overide super's API
  * It will return this Component name
  */
  getClassName(): string {
    return this.constructor.name;
  }

  ngOnInit() {
    this._controllerService.publish(Eventtypes.COMPONENT_INIT, {
      'component': 'TechsupportComponent', 'state':
        Eventtypes.COMPONENT_INIT
    });
    this.setDefaultToolbar();
    this.getTechSupportRequests();
  }

  ngOnDestroy() {
    this.subscriptions.forEach(subscription => {
      subscription.unsubscribe();
    });
    this._controllerService.publish(Eventtypes.COMPONENT_DESTROY, {
      'component': 'TechsupportComponent', 'state':
        Eventtypes.COMPONENT_DESTROY
    });
  }

  setDefaultToolbar() {
    const currToolbar = this._controllerService.getToolbarData();
    currToolbar.buttons = [
      {
        cssClass: 'global-button-primary techsupportrequests-toolbar-button',
        text: 'ADD TECH-SUPPORT REQUEST',
        callback: () => { this.createNewTechSupprotRequest(); }
      },
    ];
    currToolbar.breadcrumb = [{ label: 'Tech Supports', url: Utility.getBaseUIUrl() + 'monitoring/techsupport' }];
    this._controllerService.setToolbarData(currToolbar);
  }

  createNewTechSupprotRequest() {
    this.creatingMode = true;
    this.resizeTable(500);
  }

  private resizeTable(num: number) {
    if (this.lazyRenderWrapper) {
      if (num === null) {
        this.lazyRenderWrapper.resizeTable();
      } else {
        this.lazyRenderWrapper.resizeTable(num);
      }
    }
  }

  creationFormClose() {
    this.creatingMode = false;
    this.resizeTable(500);
  }

  getTechSupportRequests() {
    this.techsupportrequestsEventUtility = new HttpEventUtility<MonitoringTechSupportRequest>(MonitoringTechSupportRequest);
    if (this.techsupportrequestsSubscription) {
      this.techsupportrequestsSubscription.unsubscribe();
    }
    this.techsupportrequestsSubscription = this.monitoringService.WatchTechSupportRequest().subscribe(
      response => {
        this.techsupportrequestsEventUtility.processEvents(response);
        this.techsupportrequests = this.techsupportrequestsEventUtility.array;
          this.resizeTable(500);
      },
      this._controllerService.restErrorHandler('Failed to get MonitoringTechSupportRequest')
    );
    this.subscriptions.push(this.techsupportrequestsSubscription);
  }

  onTechSupportRequestsTableRowClick(event, rowData: any) {
    this.selectedTechsupportRequest = rowData;
    this.techSupportRequestsTable.toggleRow(rowData, event);
    this.resizeTable(100);
    return false;
  }

  onTechSupportRequestsTableHeaderClick(event, col) {
    this.resizeTable(500);
  }

  /**
   * This API serves html template
   */
  displayTechsupportRequest(): string {
    return JSON.stringify(this.selectedTechsupportRequest, null, 1);
  }

  displayColumn(data, col): any {
    const fields = col.field.split('.');
    const value = Utility.getObjectValueByPropertyPath(data, fields);
    const column = col.field;
    switch (column) {
      case 'spec.node-selector':
        return this.displayColumn_nodeselector(fields, value);
      case 'spec.collection-selector':
        return this.displayColumn_collectionselector(fields, value);
      default:
        return super.displayColumn(data, col);
    }
  }

  displayColumn_nodeselector(fields, value): string {
    const nodenames = 'Nodes: ' + value.names.join(',');
    const labelselectors = [];
    value.labels.requirements.forEach((req) => {
      let ret = '';
      ret += req['key'] + ' ';
      ret += Utility.getFieldOperatorSymbol(req.operator);
      ret += ' ';

      if (req.values != null) {
        let values = [];
        const enumInfo = req.values;
        values = req.values.join(',');
        ret += values;
      }
      labelselectors.push(ret);
    });
    return nodenames + ' <br/> Labels:' + labelselectors.join(',');
  }

  displayColumn_collectionselector(fields, value): string {
    const labelselectors = [];
    value.requirements.forEach((req) => {
      let ret = '';
      ret += req['key'] + ' ';
      ret += Utility.getFieldOperatorSymbol(req.operator);
      ret += ' ';

      if (req.values != null) {
        let values = [];
        const enumInfo = req.values;
        values = req.values.join(',');
        ret += values;
      }
      labelselectors.push(ret);
    });
    return labelselectors.join(',');
  }

  onTableSort($event) {
    this.resizeTable(500);
  }

  onDeleteTechSupportRecord($event, techsupportRequest: MonitoringTechSupportRequest) {
    const r = confirm('Are you sure to delete tech-support-request: ' + techsupportRequest.meta.name);
    if (r === true ) {
      this.removeTechSupportRequest(techsupportRequest);
    }
  }

  removeTechSupportRequest (techsupportRequest: MonitoringTechSupportRequest) {
      this.monitoringService.DeleteTechSupportRequest(techsupportRequest.meta.name).subscribe(
        (response) => {
          this._controllerService.invokeSuccessToaster('Delete Successful', 'Deleted tech-support-request ' + techsupportRequest.meta.name);
          this.resizeTable(500);
        },
        this._controllerService.restErrorHandler('Failed to remove tech-support request: ' + techsupportRequest.meta.name)
      );
  }

}
