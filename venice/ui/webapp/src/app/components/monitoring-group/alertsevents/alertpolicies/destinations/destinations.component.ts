import { ChangeDetectorRef, Component, Input, ViewEncapsulation } from '@angular/core';
import { Animations } from '@app/animations';
import { Utility } from '@app/common/Utility';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { ControllerService } from '@app/services/controller.service';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { IApiStatus, IMonitoringAlertDestination, MonitoringAlertDestination, MonitoringAlertPolicy } from '@sdk/v1/models/generated/monitoring';
import { Observable } from 'rxjs';
import { TablevieweditAbstract } from '@app/components/shared/tableviewedit/tableviewedit.component';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';
import { TableCol, CustomExportMap } from '@app/components/shared/tableviewedit';


@Component({
  selector: 'app-destinations',
  templateUrl: './destinations.component.html',
  styleUrls: ['./destinations.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class DestinationpolicyComponent extends TablevieweditAbstract<IMonitoringAlertDestination, MonitoringAlertDestination> {

  @Input() dataObjects: MonitoringAlertDestination[] = [];
  @Input() eventPolices: MonitoringAlertPolicy[] = [];

  isTabComponent = true;
  disableTableWhenRowExpanded = true;

  headerIcon: Icon = {
    margin: {
      top: '0px',
      left: '10px'
    },
    matIcon: 'send'
  };

  cols: TableCol[] = [
    { field: 'meta.name', header: 'Policy Name', class: 'destinations-column-name', sortable: true, width: 30 },
    { field: 'spec.syslog-export', header: 'Syslog Exports', class: 'destinations-column-syslog', sortable: false, width: 40 },
    { field: 'status.total-notifications-sent', header: 'Notications Sent', class: 'destinations-column-notifications-sent', sortable: false, width: 30 },
    // Following fields are currently not supported
    // { field: 'spec.email-list', header: 'Email List', class: 'destinationpolicy-column-email-list', sortable: true },
    // { field: 'spec.snmp-trap-servers', header: 'SNMP TRAP Servers', class: 'destinationpolicy-column-snmp_trap_servers', sortable: false },
  ];

  exportFilename: string = 'Venice-alert-destinations';
  exportMap: CustomExportMap = {};

  constructor(protected controllerService: ControllerService,
    protected cdr: ChangeDetectorRef,
    protected uiconfigsService: UIConfigsService,
    protected monitoringService: MonitoringService) {
    super(controllerService, cdr, uiconfigsService);
  }

  postNgInit() { }

  getClassName(): string {
    return this.constructor.name;
  }

  setDefaultToolbar() {
    const currToolbar = this.controllerService.getToolbarData();
    currToolbar.buttons = [];
    if (this.uiconfigsService.isAuthorized(UIRolePermissions.monitoringalertdestination_create)) {
      currToolbar.buttons = [
        {
          cssClass: 'global-button-primary destinations-button',
          text: 'ADD DESTINATION',
          computeClass: () => this.shouldEnableButtons ? '' : 'global-button-disabled',
          callback: () => { this.createNewObject(); }
        },
      ];
    }
    this.controllerService.setToolbarData(currToolbar);
  }

  displayColumn(alerteventpolicies, col): any {
    const fields = col.field.split('.');
    const value = Utility.getObjectValueByPropertyPath(alerteventpolicies, fields);
    const column = col.field;
    switch (column) {
      case 'spec.syslog-export':
        return value.targets.map(item => item.destination).join(', ');
      case 'spec.email-list':
        return JSON.stringify(value, null, 2);
      case 'spec.snmp-trap-servers':
        return JSON.stringify(value, null, 2);
      default:
        return Array.isArray(value) ? JSON.stringify(value, null, 2) : value;
    }
  }

  deleteRecord(object: MonitoringAlertDestination): Observable<{ body: IMonitoringAlertDestination | IApiStatus | Error, statusCode: number }> {
    return this.monitoringService.DeleteAlertDestination(object.meta.name);
  }

  generateDeleteConfirmMsg(object: IMonitoringAlertDestination): string {
    return 'Are you sure you want to delete destination ' + object.meta.name;
  }

  generateDeleteSuccessMsg(object: IMonitoringAlertDestination): string {
    return 'Deleted destination ' + object.meta.name;
  }

  showDeleteButton(rowData: MonitoringAlertDestination): boolean {
    const isOKtoDelete: boolean  = true;
    for (let i = 0; i < this.eventPolices.length; i ++) {
      const policy: MonitoringAlertPolicy = this.eventPolices[i];
      const destinations: string[] = policy.spec.destinations;
      const matched  = destinations.find( (destination: string) =>  destination === rowData.meta.name);
      if (matched ) {
        return false;
      }
    }
     return isOKtoDelete;
  }

}
