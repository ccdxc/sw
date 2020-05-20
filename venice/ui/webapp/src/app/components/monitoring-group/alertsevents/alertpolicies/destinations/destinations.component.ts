import { ChangeDetectorRef, Component, Input, ViewEncapsulation, DoCheck, OnInit, IterableDiffer, IterableDiffers } from '@angular/core';
import { Animations } from '@app/animations';
import { Utility } from '@app/common/Utility';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { ControllerService } from '@app/services/controller.service';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { IApiStatus, IMonitoringAlertDestination, MonitoringAlertDestination, MonitoringAlertPolicy, IMonitoringExportConfig } from '@sdk/v1/models/generated/monitoring';
import { Observable } from 'rxjs';
import { TablevieweditAbstract } from '@app/components/shared/tableviewedit/tableviewedit.component';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';
import { TableCol, CustomExportMap } from '@app/components/shared/tableviewedit';
import { HttpEventUtility } from '@app/common/HttpEventUtility';


@Component({
  selector: 'app-destinations',
  templateUrl: './destinations.component.html',
  styleUrls: ['./destinations.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class DestinationpolicyComponent extends TablevieweditAbstract<IMonitoringAlertDestination, MonitoringAlertDestination> implements OnInit, DoCheck   {
  public static MAX_TARGETS_PER_POLICY = 2;
  public static MAX_TOTAL_TARGETS = 8;
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
    { field: 'meta.name', header: 'Policy Name', class: 'destinations-column-name', sortable: true, width: 25},
    { field: 'spec.syslog-export', header: 'Syslog Exports', class: 'destinations-column-syslog', sortable: false, width: 25 },
    { field: 'spec.syslog-export.targets', header: 'Targets', class: 'destinations-column-syslog', sortable: false, width: 30 },
    { field: 'status.total-notifications-sent', header: 'Notifications Sent', class: 'destinations-column-notifications-sent', sortable: false, width: 20 },
    // Following fields are currently not supported
    // { field: 'spec.email-list', header: 'Email List', class: 'destinationpolicy-column-email-list', sortable: true },
    // { field: 'spec.snmp-trap-servers', header: 'SNMP TRAP Servers', class: 'destinationpolicy-column-snmp_trap_servers', sortable: false },
  ];

  exportFilename: string = 'PSM-alert-destinations';
  exportMap: CustomExportMap = {};
  maxNewTargets: number = DestinationpolicyComponent.MAX_TARGETS_PER_POLICY;
  arrayDiffers: IterableDiffer<any>;


  constructor(protected controllerService: ControllerService,
    protected cdr: ChangeDetectorRef,
    protected uiconfigsService: UIConfigsService,
    protected monitoringService: MonitoringService,
    protected _iterableDiffers: IterableDiffers
    ) {
    super(controllerService, cdr, uiconfigsService);
    this.arrayDiffers = _iterableDiffers.find([]).create(HttpEventUtility.trackBy);
  }

  postNgInit() {
    this.maxNewTargets = this.computeTargets();
  }

  ngDoCheck() {
    const changes = this.arrayDiffers.diff(this.dataObjects);
    if (changes) {
      this.maxNewTargets = this.computeTargets();
    }
  }
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
          genTooltip: () => this.getTooltip(),
          computeClass: () => this.shouldEnableButtons && this.maxNewTargets > 0 ? '' : 'global-button-disabled',
          callback: () => { this.createNewObject(); }
        },
      ];
    }
    this.controllerService.setToolbarData(currToolbar);
  }
  getTooltip(): string {
    return this.maxNewTargets === 0 ? 'Cannot exceed 8 total targets across destination policies' : '';
  }
  displayColumn(alerteventpolicies, col): any {
    const fields = col.field.split('.');
    const value = Utility.getObjectValueByPropertyPath(alerteventpolicies, fields);
    const column = col.field;
    switch (column) {
      case 'spec.syslog-export':
        return this.formatSyslogExports(value);
      case 'spec.syslog-export.targets':
        return this.formatTargets(value);
      case 'spec.email-list':
        return JSON.stringify(value, null, 2);
      case 'spec.snmp-trap-servers':
        return JSON.stringify(value, null, 2);
      default:
        return Array.isArray(value) ? JSON.stringify(value, null, 2) : value;
    }
  }
  formatSyslogExports(data) {
    if (data == null) {
      return '';
    }
    let targetStr: string = '';
    if (data.format) {
      targetStr += 'Format:' +  data.format.replace('syslog-', '').toUpperCase() + ', ';
    }
    for (const k in data.config) {
      if (data.config.hasOwnProperty(k) && k !== '_ui') {
        if (data.config[k]) {
          targetStr += k.charAt(0).toUpperCase() + k.slice(1) + ':' +  data.config[k] + ', ';
        }
      }
    }
    if (targetStr.length === 0) {
      targetStr += '*';
    } else {
      targetStr = targetStr.slice(0, -2);
    }
    return [targetStr];
  }
  formatTargets(data: IMonitoringExportConfig[]) {
    if (data == null) {
      return '';
    }
    const retArr = [];
    data.forEach((req) => {
      let targetStr: string = '';
      for (const k in req) {
        if (req.hasOwnProperty(k) && k !== '_ui' && k !== 'credentials') {
          if (req[k]) {
            targetStr += k.charAt(0).toUpperCase() + k.slice(1) + ':' +  req[k] + ', ';
          }
        }
      }
      if (targetStr.length === 0) {
        targetStr += '*';
      } else {
        targetStr = targetStr.slice(0, -2);
      }
      retArr.push(targetStr);
    });
    return retArr;
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

  showUpdateButtons(rowData: MonitoringAlertDestination): boolean {
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
  computeTargets(): number {
    let totaltargets: number = 0;
    for (const policy of this.dataObjects) {
      if (policy.spec['syslog-export'].targets !== null) {
        totaltargets += policy.spec['syslog-export'].targets.length;
      }
    }
    const remainder = DestinationpolicyComponent.MAX_TOTAL_TARGETS - totaltargets;
    return Math.min(remainder, DestinationpolicyComponent.MAX_TARGETS_PER_POLICY);
  }

}
