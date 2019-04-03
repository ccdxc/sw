import { ChangeDetectorRef, Component, ViewEncapsulation, OnInit } from '@angular/core';
import { Animations } from '@app/animations';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { Utility } from '@app/common/Utility';
import { TableCol, TablevieweditAbstract } from '@app/components/shared/tableviewedit/tableviewedit.component';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { ControllerService } from '@app/services/controller.service';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { IApiStatus, IMonitoringTechSupportRequest, MonitoringTechSupportRequest } from '@sdk/v1/models/generated/monitoring';
import { Observable } from 'rxjs';


@Component({
  selector: 'app-techsupport',
  templateUrl: './techsupport.component.html',
  styleUrls: ['./techsupport.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class TechsupportComponent extends TablevieweditAbstract<IMonitoringTechSupportRequest, MonitoringTechSupportRequest> implements OnInit {
  dataObjects: ReadonlyArray<MonitoringTechSupportRequest> = [];

  techsupportrequestsEventUtility: HttpEventUtility<MonitoringTechSupportRequest>;

  bodyicon: any = {
    margin: {
      top: '9px',
      left: '8px',
    },
    url: '/assets/images/icons/monitoring/ic_tech_support-black.svg',   // TODO: wait for new svg file
  };
  headerIcon: Icon = {
    margin: {
      top: '0px',
      left: '10px',
    },
    matIcon: 'grid_on'
  };

  cols: TableCol[] = [
    { field: 'meta.name', header: 'Name', class: 'techsupportrequests-column-name', sortable: true, width: 15 },
    { field: 'meta.mod-time', header: 'Time', class: 'techsupportrequests-column-date', sortable: true, width: 15 },
    { field: 'spec.node-selector', header: 'Selected Nodes', class: ' techsupportrequests-column-node_selector', sortable: false, width: 25 },
    { field: 'spec.collection-selector', header: 'Collection-Selector', class: ' techsupportrequests-column-collection_selector', sortable: false, width: 25 },
    { field: 'status.status', header: 'Status', class: ' techsupportrequests-column-status_status', sortable: true, width: 10 },
    { field: 'spec.verbosity', header: 'Verbosity', class: ' techsupportrequests-column-spec_verbosity', sortable: true, width: 10 }
  ];

  isTabComponent = false;
  disableTableWhenRowExpanded = true;

  constructor(protected controllerService: ControllerService,
    protected cdr: ChangeDetectorRef,
    protected monitoringService: MonitoringService) {
    super(controllerService, cdr);
  }

  /**
  * Overide super's API
  * It will return this Component name
  */
  getClassName(): string {
    return this.constructor.name;
  }

  postNgInit() {
    this.getTechSupportRequests();
  }

  setDefaultToolbar() {
    this.controllerService.setToolbarData({
      buttons: [
        {
          cssClass: 'global-button-primary techsupportrequests-toolbar-button techsupportrequests-toolbar-button-ADD',
          text: 'ADD TECH-SUPPORT REQUEST',
          computeClass: () => this.shouldEnableButtons ? '' : 'global-button-disabled',
          callback: () => { this.createNewObject(); }
        },
      ],
      breadcrumb: [{ label: 'Tech Supports', url: Utility.getBaseUIUrl() + 'monitoring/techsupport' }]
    });
  }

  getTechSupportRequests() {
    this.techsupportrequestsEventUtility = new HttpEventUtility<MonitoringTechSupportRequest>(MonitoringTechSupportRequest);
    this.dataObjects = this.techsupportrequestsEventUtility.array;
    const sub = this.monitoringService.WatchTechSupportRequest().subscribe(
      response => {
        this.techsupportrequestsEventUtility.processEvents(response);
      },
    );
    this.subscriptions.push(sub);
  }

  /**
   * This API serves html template
   */
  displayTechsupportRequest(): string {
    return JSON.stringify(this.expandedRowData, null, 1);
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
        return Utility.displayColumn(data, col);
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

  deleteRecord(object: MonitoringTechSupportRequest): Observable<{ body: IMonitoringTechSupportRequest | IApiStatus | Error, statusCode: number }> {
    return this.monitoringService.DeleteTechSupportRequest(object.meta.name);
  }

  generateDeleteConfirmMsg(object: IMonitoringTechSupportRequest) {
    return 'Are you sure to delete tech-support-request: ' + object.meta.name;
  }

  generateDeleteSucessMsg(object: IMonitoringTechSupportRequest) {
    return 'Deleted tech-support-request ' + object.meta.name;
  }

}
