import { ChangeDetectorRef, Component, ViewEncapsulation, OnInit, OnDestroy } from '@angular/core';
import { Animations } from '@app/animations';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { Utility } from '@app/common/Utility';
import { TablevieweditAbstract } from '@app/components/shared/tableviewedit/tableviewedit.component';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { ControllerService } from '@app/services/controller.service';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { IApiStatus, IMonitoringArchiveRequest, MonitoringArchiveRequest, MonitoringArchiveRequestStatus_status, IMonitoringArchiveRequestStatus, IMonitoringCancelArchiveRequest } from '@sdk/v1/models/generated/monitoring';
import { Observable } from 'rxjs';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { TableCol, RowClickEvent, CustomExportMap } from '@app/components/shared/tableviewedit';
import { TableUtility } from '@app/components/shared/tableviewedit/tableutility';


@Component({
  selector: 'app-archivelog',
  templateUrl: './archivelog.component.html',
  styleUrls: ['./archivelog.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})

export class ArchivelogComponent extends TablevieweditAbstract<IMonitoringArchiveRequest, MonitoringArchiveRequest> implements OnInit, OnDestroy {
  public static AL_DOWNLOAD = 'archivelogsdownload'; // Will contain URL for archive request download
  dataObjects: ReadonlyArray<MonitoringArchiveRequest> = [];

  archiverequestsEventUtility: HttpEventUtility<MonitoringArchiveRequest>;

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
    { field: 'meta.name', header: 'Name', class: 'archiverequests-column-name', sortable: true, width: 15, notReorderable: true },
    { field: 'spec.type', header: 'Log Type', class: 'archiverequests-column-log-type', sortable: true, width: 15 },
    { field: 'meta.creation-time', header: 'Creation Time', class: 'archiverequests-column-date', sortable: true, width: '180px', notReorderable: true },
    { field: 'spec.query.start-time', header: 'Start Time', class: 'archiverequests-column-date', sortable: true, width: '180px', notReorderable: true },
    { field: 'spec.query.end-time', header: 'End Time', class: 'archiverequests-column-date', sortable: true, width: '180px', notReorderable: true },
    // { field: 'spec.query', header: 'Criteria', class: 'archiverequests-column-criteria', sortable: false, width: 45 },
    { field: 'status.status', header: 'Status', class: 'archiverequests-column-status_status', sortable: true, width: 15 }
  ];

  exportFilename: string = 'Venice-archive-logs-requests';
  exportMap: CustomExportMap = {};

  isTabComponent = false;
  disableTableWhenRowExpanded = true;
  tableLoading: boolean = false;
  archiveRequestDetail: any;

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
    // setTimeout(() => {this.getArchiveRequests(); });
    this.getArchiveRequests();
  }

  setDefaultToolbar() {
    const buttons = [];
    this.controllerService.setToolbarData({
      buttons: buttons,
      breadcrumb: [{ label: 'Archive Logs', url: Utility.getBaseUIUrl() + 'monitoring/archive' }]
    });
  }

  ngOnDestroy() {
    this.subscriptions.forEach((subscription) => {
      subscription.unsubscribe();
    });
  }

  /**
   * Watch Archive Requests
   */
  getArchiveRequests() {
    this.archiverequestsEventUtility = new HttpEventUtility<MonitoringArchiveRequest>(MonitoringArchiveRequest);
    this.dataObjects = this.archiverequestsEventUtility.array;
    const sub = this.monitoringService.WatchArchiveRequest().subscribe(
      response => {
        this.archiverequestsEventUtility.processEvents(response);
      },
      error => {
        this._controllerService.invokeRESTErrorToaster(Utility.UPDATE_FAILED_SUMMARY, error);
      }
    );
    this.subscriptions.push(sub);
  }

  displayArchiveRequest(): string {
    return JSON.stringify(this.expandedRowData, null, 1);
  }

  /**
   * Handle logics when user click the row
   * @param event
   */
  onArchiveRequestsTableRowClick(event: RowClickEvent) {
    if (this.expandedRowData === event.rowData) {
      // Click was on the same row
      this.closeRowExpand();
    } else {
      this.closeRowExpand();
      this.expandRowRequest(event.event, event.rowData);
    }
  }

  /**
   * Cancel Running Archive Request
   * @param event
   * @param object
   */
  onCancelRecord(event, object: MonitoringArchiveRequest) {
    if (event) {
      event.stopPropagation();
    }
    // Should not be able to cancel any record while we are editing
    if (this.isRowExpanded()) {
      return;
    }
    this.controllerService.invokeConfirm({
      header: this.generateCancelConfirmMsg(object),
      message: 'This action cannot be reversed',
      acceptLabel: 'Cancel Request',
      accept: () => {
        const cancelRequest: IMonitoringCancelArchiveRequest = {
          kind: object.kind,
          'api-version': object['api-version'],
          meta: object.meta
        };
        const sub = this.monitoringService.Cancel(object.meta.name, cancelRequest).subscribe(
          (response) => {
            // TODO: BETTER SOL: From backend if we have some status value saying cancellation in process!
            this.controllerService.invokeSuccessToaster(Utility.CANCEL_SUCCESS_SUMMARY, this.generateCancelSuccessMsg(object));
          },
          (err) => {
            if (err.body instanceof Error) {
              console.error('Service returned code: ' + err.statusCode + ' data: ' + <Error>err.body);
            } else {
              console.error('Service returned code: ' + err.statusCode + ' data: ' + <IApiStatus>err.body);
            }
            this.controllerService.invokeRESTErrorToaster(Utility.CANCEL_FAILED_SUMMARY, err);
          }
        );
        this.subscriptions.push(sub);
      }
    });
  }

  generateCancelSuccessMsg(object: MonitoringArchiveRequest): string {
    return 'Canceled archive request ' + object.meta.name;
  }

  generateCancelConfirmMsg(object: any): string {
    return 'Are you sure to cancel archive request: ' + object.meta.name;
  }

  displayColumn(data, col): any {
    const column = col.field;
    switch (column) {
      case 'spec.query':
        return JSON.stringify(data, null, 2);
      default:
        return TableUtility.displayColumn(data, col);
    }
  }

  isArchiveCompleted(rowData: MonitoringArchiveRequest, col): boolean {
    const fields = col.field.split('.');
    const value = Utility.getObjectValueByPropertyPath(rowData, fields);
    const isComplete = (value === MonitoringArchiveRequestStatus_status.completed);
    if (isComplete && !rowData[ArchivelogComponent.AL_DOWNLOAD]) {
      // when AL.status is complete, we build UI model to list available download links. We build only once
      this.buildALDownload(rowData);
    }
    return isComplete;
  }

  timeoutOrFailure(rowData: MonitoringArchiveRequest): boolean {
    return (this.isALFailure(rowData) || this.isALTimeout(rowData));
  }

  isALFailure(rowData: MonitoringArchiveRequest): boolean {
    return (rowData.status.status === MonitoringArchiveRequestStatus_status.failed) ? true : false;
  }

  isALTimeout(rowData: MonitoringArchiveRequest): boolean {
    return (rowData.status.status === MonitoringArchiveRequestStatus_status.timeout) ? true : false;
  }

  displayMessage(rowData: MonitoringArchiveRequest): any {
    const status: IMonitoringArchiveRequestStatus = rowData.status;
    return (status != null && this.timeoutOrFailure(rowData) && status.reason !== undefined && status.reason !== null) ? status.reason : '';
  }


  /**
   * Builds Archive Request Download URL
   *
   * Archive Request server response JSON object.status looks like:
   * {
   *  "status": "completed",
   *  "uri": "/objstore/v1/downloads/auditevents/xxxx.gz"
   * }
  */

  buildALDownload(rowData: MonitoringArchiveRequest) {
    const status: IMonitoringArchiveRequestStatus = rowData.status;
    if (status.uri !== undefined && status.uri !== null) {
      rowData._ui[ArchivelogComponent.AL_DOWNLOAD] = status.uri;
    } else {
      rowData._ui[ArchivelogComponent.AL_DOWNLOAD] = '';
    }
  }

  triggerBuildDownloadZip(rowData: MonitoringArchiveRequest): boolean {
    this.buildALDownload(rowData);
    return true;
  }

  displayColumn_status(fields, value): string {
    return (value === MonitoringArchiveRequestStatus_status.completed) ? '' : value;
  }

  deleteRecord(object: MonitoringArchiveRequest): Observable<{ body: IMonitoringArchiveRequest | IApiStatus | Error, statusCode: number }> {
    return this.monitoringService.DeleteArchiveRequest(object.meta.name);
  }

  generateDeleteConfirmMsg(object: IMonitoringArchiveRequest) {
    return 'Are you sure to delete archive request: ' + object.meta.name;
  }

  generateDeleteSuccessMsg(object: IMonitoringArchiveRequest) {
    return 'Deleted archive request ' + object.meta.name;
  }

  showCancelIcon(rowData: MonitoringArchiveRequest): boolean {
    return (rowData.status.status === MonitoringArchiveRequestStatus_status.running);
  }

  /**
   * This API serves html template
   * @param rowData
   */
  showDeleteIcon(rowData: MonitoringArchiveRequest): boolean {
    // when the status is null, the user shouldn't be able to perform any action (cancel/delete)
    return (rowData.status.status !== MonitoringArchiveRequestStatus_status.running && rowData.status.status !== null);
  }

}
