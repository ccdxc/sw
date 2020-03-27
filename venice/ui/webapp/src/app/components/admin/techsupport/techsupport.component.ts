import { ChangeDetectorRef, Component, ViewEncapsulation, OnInit, OnDestroy } from '@angular/core';
import { Animations } from '@app/animations';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { Utility } from '@app/common/Utility';
import { TablevieweditAbstract } from '@app/components/shared/tableviewedit/tableviewedit.component';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { ControllerService } from '@app/services/controller.service';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { IApiStatus, IMonitoringTechSupportRequest, MonitoringTechSupportRequest, MonitoringTechSupportRequestStatus_status, IMonitoringTechSupportRequestStatus } from '@sdk/v1/models/generated/monitoring';
import { Observable } from 'rxjs';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';
import { TableCol, RowClickEvent, CustomExportMap } from '@app/components/shared/tableviewedit';
import { TableUtility } from '@app/components/shared/tableviewedit/tableutility';



@Component({
  selector: 'app-techsupport',
  templateUrl: './techsupport.component.html',
  styleUrls: ['./techsupport.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class TechsupportComponent extends TablevieweditAbstract<IMonitoringTechSupportRequest, MonitoringTechSupportRequest> implements OnInit, OnDestroy {
  public static TS_DOWNLOAD = 'techsupportdownload';
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
    { field: 'meta.name', header: 'Name', class: 'techsupportrequests-column-name', sortable: true, width: 15, notReorderable: true },
    { field: 'meta.creation-time', header: 'Creation Time', class: 'techsupportrequests-column-date', sortable: true, width: '180px', notReorderable: true },
    { field: 'spec.node-selector', header: 'Selected Nodes', class: ' techsupportrequests-column-node_selector', sortable: false, width: 45 },
    { field: 'status.status', header: 'Status', class: ' techsupportrequests-column-status_status', sortable: true, width: 15 }
  ];

  exportFilename: string = 'Venice-tech-support-requests';
  exportMap: CustomExportMap = {};

  isTabComponent = false;
  disableTableWhenRowExpanded = true;
  tableLoading: boolean = false;

  constructor(protected controllerService: ControllerService,
    protected uiconfigsService: UIConfigsService,
    protected cdr: ChangeDetectorRef,
    protected monitoringService: MonitoringService) {
    super(controllerService, cdr, uiconfigsService);
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
    if (!this.checkPermissions()) {
      this.controllerService.invokeInfoToaster('Additional authorizaiton required', 'Cluster node and DSC read permissions are required to create tech-support request.');
    }
  }

  setDefaultToolbar() {
    let buttons = [];
    if (this.checkPermissions()) {
      buttons = [{
        cssClass: 'global-button-primary techsupportrequests-toolbar-button techsupportrequests-toolbar-button-ADD',
        text: 'ADD TECH-SUPPORT REQUEST',
        computeClass: () => this.shouldEnableButtons ? '' : 'global-button-disabled',
        callback: () => { this.createNewObject(); }
      }];
    }
    this.controllerService.setToolbarData({
      buttons: buttons,
      breadcrumb: [{ label: 'Tech Supports', url: Utility.getBaseUIUrl() + 'admin/techsupport' }]
    });
  }

  checkPermissions(): boolean {
    const boolClusterNodeRead = this.uiconfigsService.isAuthorized(UIRolePermissions.clusternode_read);
    const boolClusterDSCRead = this.uiconfigsService.isAuthorized(UIRolePermissions.clusterdistributedservicecard_read);
    const boolMonitoryTechSupportCreate = this.uiconfigsService.isAuthorized(UIRolePermissions.monitoringtechsupportrequest_create);
    return (boolClusterNodeRead && boolClusterDSCRead && boolMonitoryTechSupportCreate);

  }

  ngOnDestroy() {
    this.subscriptions.forEach((subscription) => {
      subscription.unsubscribe();
    });
  }

  getTechSupportRequests() {
    this.techsupportrequestsEventUtility = new HttpEventUtility<MonitoringTechSupportRequest>(MonitoringTechSupportRequest);
    this.dataObjects = this.techsupportrequestsEventUtility.array;
    const sub = this.monitoringService.WatchTechSupportRequest().subscribe(
      response => {
        this.techsupportrequestsEventUtility.processEvents(response);
      },
      () => {
        this.controllerService.webSocketErrorHandler('Failed to get Tech Support Requests');
      }
    );
    this.subscriptions.push(sub);
  }

  /**
   * This API serves html template
   */
  displayTechsupportRequest(): string {
    return JSON.stringify(this.expandedRowData, null, 1);
  }

  displayColumn(data: MonitoringTechSupportRequest, col: TableCol): any {
    const fields = col.field.split('.');
    const value = Utility.getObjectValueByPropertyPath(data, fields);
    const column = col.field;
    switch (column) {
      case 'spec.node-selector':
        return this.displayColumn_nodeselector(fields, value);
      case 'spec.collection-selector':
        return this.displayColumn_collectionselector(fields, value);
      default:
        return TableUtility.displayColumn(data, col);
    }
  }

  isTechsupportCompleted(rowData: MonitoringTechSupportRequest, col): boolean {
    const fields = col.field.split('.');
    const value = Utility.getObjectValueByPropertyPath(rowData, fields);
    const column = col.field;
    const isComplete = (value === MonitoringTechSupportRequestStatus_status.completed);
    if (isComplete && !rowData[TechsupportComponent.TS_DOWNLOAD]) {
      // when TS.status is complete, we build UI model to list available download links. We build only once
      this.buildTSDownload(rowData);
    }
    return isComplete;
  }

  timeoutOrFailure(rowData: MonitoringTechSupportRequest): boolean {
    return (this.isTSFailure(rowData) || this.isTSTimeout(rowData));
  }

  isTSFailure(rowData: MonitoringTechSupportRequest): boolean {
    if (rowData.status.status === MonitoringTechSupportRequestStatus_status.failed) {
      return true;
    }
    return false;
  }

  isTSTimeout(rowData: MonitoringTechSupportRequest): boolean {
    if (rowData.status.status === MonitoringTechSupportRequestStatus_status.timeout) {
      return true;
    }
    return false;
  }


  displayTimeOutMessage(rowData: MonitoringTechSupportRequest): string {
    const nodes = rowData.status['ctrlr-node-results'];
    const nics = rowData.status['dsc-results'];
    const reasonArray: string[] = [];
    if (nodes != null) {
      Object.keys(nodes).forEach((key) => {
        if (nodes != null && key != null && nodes[key].status != null && nodes[key].status === MonitoringTechSupportRequestStatus_status.timeout) {
          reasonArray.push(key + ' timed out during operation');
        }
      });
    }
    if (nics != null) {
      Object.keys(nics).forEach((key) => {
        if (key != null && nics[key].status != null && nics[key].status === MonitoringTechSupportRequestStatus_status.timeout) {
          reasonArray.push(key + ' timed out during operation');
        }
      });
    }

    if (reasonArray.length > 0) {
      return reasonArray.join('\n');
    }
    return '';
  }

  displayFailureMessage(rowData: MonitoringTechSupportRequest): string {
    const nodes = rowData.status['ctrlr-node-results'];
    const nics = rowData.status['dsc-results'];
    const reasonArray: string[] = [];
    if (nodes != null) {
      Object.keys(nodes).forEach((key) => {
        if (key != null && nodes[key].status != null && nodes[key].status === MonitoringTechSupportRequestStatus_status.failed) {
          reasonArray.push(key + ' ' + nodes[key].reason);
        }
      });
    }
    if (nics != null) {
      Object.keys(nics).forEach((key) => {
        if (key != null && nics[key].status != null && nics[key].status === MonitoringTechSupportRequestStatus_status.failed) {
          reasonArray.push(key + ' ' + nics[key].reason);
        }
      });
    }
    const nonNullArray = reasonArray.filter(function (el) {
      return el != null;
    });

    if (nonNullArray.length > 0) {
      return nonNullArray.join('\n');  // if the failing agents have a 'reason' attribute (ideally), then print that explanation as an error message
    } else {     // else, manually identify the agents that caused the operation to fail, and print a generic error message (eg: X is unhealthy)
      return this.whichNaplesFail(rowData);
    }
  }

  whichNaplesFail(rowData: MonitoringTechSupportRequest): string {
    const nodes = rowData.status['ctrlr-node-results'];
    const nics = rowData.status['dsc-results'];
    const reasonArray: string[] = [];
    if (nodes != null) {
      Object.keys(nodes).forEach((key) => {
        if (key != null && nodes[key].status != null && nodes[key].status === MonitoringTechSupportRequestStatus_status.failed) {
          reasonArray.push(key + 'is unhealthy and causing the operation to fail');
        }
      });
    }
    if (nics != null) {
      Object.keys(nics).forEach((key) => {
        if (key != null && nics[key].status != null && nics[key].status === MonitoringTechSupportRequestStatus_status.failed) {
          reasonArray.push(key + ' is unhealthy and causing the operation to fail');
        }
      });
    }

    if (reasonArray.length > 0) {
      return reasonArray.join('\n');
    }
    return '';
  }


  displayMessage(rowData: MonitoringTechSupportRequest): any {
    if (rowData.status.status === MonitoringTechSupportRequestStatus_status.timeout) {
      return this.displayTimeOutMessage(rowData);
    } else if (rowData.status.status === MonitoringTechSupportRequestStatus_status.failed) {
      return this.displayFailureMessage(rowData);
    }
  }


  /**
  Techsupport server response JSON object.status looks like:
  status: {
          "instance-id": "b91bd56b-060a-48dc-ad9b-fa2c62d7b621",
          "status": "Completed",
          "dsc-results": {
            "00ae.cd00.1142": {
              "end-time": "2019-04-15T19:14:24.435689118Z",
              "status": "Completed"
            }
            .. // another nic's MAC
          }
        }
   techsupport url looks like: https://10.7.100.21:10001/objstore/v1/downloads/tenant/default/techsupport/ts-1-b91bd56b-060a-48dc-ad9b-fa2c62d7b621-DistributedServiceCard-00ae.cd00.1142
  */


  buildTSDownload(rowData: MonitoringTechSupportRequest) {
    const tsName = rowData.meta.name;
    const status: IMonitoringTechSupportRequestStatus = rowData.status;
    const instanceId = status['instance-id'];
    const smartNICNodeResults = status['dsc-results'];
    const ctrlrNnodeResults = status['ctrlr-node-results'];
    const downloadObj = {
      nicNodes: [],
      controlnodes: []
    };
    downloadObj['bundleAll'] = this.buildTSBundleAllDownloadHelper(tsName, instanceId);
    if (smartNICNodeResults) {
      downloadObj['nicNodes'] = this.buildTSDownloadHelper(smartNICNodeResults, instanceId, tsName);
    }
    if (ctrlrNnodeResults) {
      downloadObj['controlnodes'] = this.buildTSDownloadHelper(ctrlrNnodeResults, instanceId, tsName);
    }
    rowData[TechsupportComponent.TS_DOWNLOAD] = downloadObj;
  }

  triggerBuildDownloaZip(rowData: MonitoringTechSupportRequest): boolean {
    this.buildTSDownload(rowData);
    return true;
  }

  /**
   * Per disussion with back-end (Barun). All-in-one download link will look like
   * https://10.7.100.24:10001/objstore/v1/downloads/all/@tenant/default/techsupport/@tsname.zip
   * https://10.7.100.24:10001/objstore/v1/downloads/all/tenant/default/techsupport/ts-1.zip  (tenant = default , tsname = ts-1)
   */
  buildTSBundleAllDownloadHelper(tsName: string, instanceId: string): string {
    const instantIDprefix = (instanceId.split('-').length > 0) ? instanceId.split('-')[0] : instanceId; // "instance-id": "c7b8c234-a453-4594-b98f-0890ddcf4cbf", we want c7b8c234
    return '/objstore/v1/downloads/all/tenant/' + Utility.getInstance().getTenant() + '/techsupport/' + tsName + '-' + instantIDprefix + '.zip';
  }

  buildTSDownloadHelper(results, instanceId: string, tsName: string): any {
    const list = [];
    const keys = Object.keys(results);
    keys.forEach(key => {
      const obj = {
        name: key,
        url: results[key].uri
      };
      list.push(obj);
    });
    return list;
  }

  displayColumn_status(fields, value): string {
    if (value === MonitoringTechSupportRequestStatus_status.completed) {
      return '';
    } else {
      return value;
    }
  }

  displayColumn_nodeselector(fields, value): string {
    const nodenames = 'Nodes: ' + value.names.join(', ');  // VS-719 make sure it is not a one big comma-seperated long word.
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

  /**
   * This API serves html template
   * @param rowData
   */
  showDeleteIcon(rowData: MonitoringTechSupportRequest): boolean {
    return (rowData.status.status !== MonitoringTechSupportRequestStatus_status.running);
  }

}
