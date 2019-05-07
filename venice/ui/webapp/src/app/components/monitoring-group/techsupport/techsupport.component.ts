import { ChangeDetectorRef, Component, ViewEncapsulation, OnInit } from '@angular/core';
import { Animations } from '@app/animations';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { Utility } from '@app/common/Utility';
import { TableCol, TablevieweditAbstract } from '@app/components/shared/tableviewedit/tableviewedit.component';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { ControllerService } from '@app/services/controller.service';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { IApiStatus, IMonitoringTechSupportRequest, MonitoringTechSupportRequest, MonitoringTechSupportRequestStatus_status, IMonitoringTechSupportRequestStatus } from '@sdk/v1/models/generated/monitoring';
import { Observable } from 'rxjs';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';


@Component({
  selector: 'app-techsupport',
  templateUrl: './techsupport.component.html',
  styleUrls: ['./techsupport.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class TechsupportComponent extends TablevieweditAbstract<IMonitoringTechSupportRequest, MonitoringTechSupportRequest> implements OnInit {
  public static  TS_DOWNLOAD = 'techsupportdownload';
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
    { field: 'spec.node-selector', header: 'Selected Nodes', class: ' techsupportrequests-column-node_selector', sortable: false, width: 35 },
    { field: 'status.status', header: 'Status', class: ' techsupportrequests-column-status_status', sortable: true, width: 35 },
     ];

  isTabComponent = false;
  disableTableWhenRowExpanded = true;

  constructor(protected controllerService: ControllerService,
    protected uiconfigsService: UIConfigsService,
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
    let buttons = [];
    if (this.uiconfigsService.isAuthorized(UIRolePermissions.monitoringtechsupportrequest_create)) {
      buttons = [{
          cssClass: 'global-button-primary techsupportrequests-toolbar-button techsupportrequests-toolbar-button-ADD',
          text: 'ADD TECH-SUPPORT REQUEST',
          computeClass: () => this.shouldEnableButtons ? '' : 'global-button-disabled',
          callback: () => { this.createNewObject(); }
      }]
    }
    this.controllerService.setToolbarData({
      buttons: buttons,
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

  isTechsupportCompleted(rowData: MonitoringTechSupportRequest, col): boolean {
    const fields = col.field.split('.');
    const value = Utility.getObjectValueByPropertyPath(rowData, fields);
    const column = col.field;
    const isComplete = (value === MonitoringTechSupportRequestStatus_status.Completed);
    if (isComplete  && !rowData[TechsupportComponent.TS_DOWNLOAD]) {
      // when TS.status is complete, we build UI model to list available download links. We build only once
      this.buildTSDownload(rowData);
    }
    return isComplete;
  }

  /**
  Techsupport server response JSON object.status looks like:
  status: {
          "instance-id": "b91bd56b-060a-48dc-ad9b-fa2c62d7b621",
          "status": "Completed",
          "smartnic-node-results": {
            "00ae.cd00.1142": {
              "end-time": "2019-04-15T19:14:24.435689118Z",
              "status": "Completed"
            }
            .. // another nic's MAC
          }
        }
   techsupport url looks like: https://10.7.100.21:10001/objstore/v1/downloads/tenant/default/techsupport/ts-1-b91bd56b-060a-48dc-ad9b-fa2c62d7b621-SmartNIC-00ae.cd00.1142
  */
  buildTSDownload(rowData: MonitoringTechSupportRequest) {
    const tsName = rowData.meta.name;
    const status: IMonitoringTechSupportRequestStatus = rowData.status;
    const instanceId = status['instance-id'];
    const smartNICNodeResults = status['smartnic-node-results'];
    const ctrlrNnodeResults = status['ctrlr-node-results'];
    const downloadObj = {
      nicNodes: [],
      controlnodes: []
    };
    if (smartNICNodeResults) {
        downloadObj['nicNodes'] = this.buildTSDownloadHelper(smartNICNodeResults, instanceId, tsName);
    }
    if (ctrlrNnodeResults) {
      downloadObj['controlnodes'] = this.buildTSDownloadHelper(ctrlrNnodeResults, instanceId, tsName);
    }
    rowData[TechsupportComponent.TS_DOWNLOAD] = downloadObj;
  }

  buildTSDownloadHelper(results, instanceId: string, tsName: string): any {
    const list = [];
    const keys = Object.keys(results);
     keys.forEach( key => {
       const obj = {
         name : key,
         url :  results[key].uri
       };
       list.push(obj);
     });
    return list;
  }

  displayColumn_status(fields, value): string {
    if (value === MonitoringTechSupportRequestStatus_status.Completed) {
      return '';
    } else {
      return value;
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
    return nodenames; // comment it out as backend does not support label-selector // + ' <br/> Labels:' + labelselectors.join(',');
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

  generateDeleteSuccessMsg(object: IMonitoringTechSupportRequest) {
    return 'Deleted tech-support-request ' + object.meta.name;
  }

}
