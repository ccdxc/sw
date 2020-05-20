import { ChangeDetectorRef, Component, OnInit, ViewEncapsulation, DoCheck, IterableDiffer, IterableDiffers } from '@angular/core';
import { Animations } from '@app/animations';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { Utility } from '@app/common/Utility';
import { TablevieweditAbstract } from '@app/components/shared/tableviewedit/tableviewedit.component';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { ControllerService } from '@app/services/controller.service';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { EventsEvent } from '@sdk/v1/models/generated/events';
import { FieldsRequirement, IApiStatus, IMonitoringEventPolicy, MonitoringEventPolicy, IMonitoringExportConfig } from '@sdk/v1/models/generated/monitoring';
import { Observable } from 'rxjs';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';
import { TableCol, CustomExportMap } from '@app/components/shared/tableviewedit';

@Component({
  selector: 'app-eventpolicy',
  templateUrl: './eventpolicy.component.html',
  styleUrls: ['./eventpolicy.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class EventpolicyComponent extends TablevieweditAbstract<IMonitoringEventPolicy, MonitoringEventPolicy> implements OnInit, DoCheck {
  public static MAX_TARGETS_PER_POLICY = 2;
  public static MAX_TOTAL_TARGETS = 8;
  dataObjects: ReadonlyArray<MonitoringEventPolicy> = [];

  bodyIcon: Icon = {
    margin: {
      top: '8px',
      left: '10px',
    },
    matIcon: 'send'
  };

  headerIcon: Icon = {
    margin: {
      top: '0px',
      left: '10px',
    },
    matIcon: 'grid_on'
  };

  // Used for processing watch events
  policyEventUtility: HttpEventUtility<MonitoringEventPolicy>;

  cols: TableCol[] = [
    { field: 'meta.name', header: 'Name', class: 'eventpolicy-column-name', sortable: true, width: 25 },
    { field: 'spec', header: 'Syslog Exports', class: 'eventpolicy-column-syslog', sortable: false, width: 25 },
    // Commenting out as it is not currently supported by backend
    // { field: 'spec.selector', header: 'Filters', class: 'eventpolicy-column-name', sortable: false, width: 30 },
    { field: 'spec.targets', header: 'Targets', class: 'eventpolicy-column-targets-destination', sortable: false, width: 25 },
    // { field: 'spec.targets', header: 'Gateway', class: 'eventpolicy-column-target-gateway', sortable: false, width: 25 },
    // { field: 'spec.targets.transport', header: 'Transport', class: 'eventpolicy-column-transport', sortable: false, width: 25 }
  ];

  exportFilename: string = 'PSM-event-policies';
  exportMap: CustomExportMap = {};

  isTabComponent = false;
  disableTableWhenRowExpanded = true;

  maxNewTargets: number = EventpolicyComponent.MAX_TARGETS_PER_POLICY;
  arrayDiffers: IterableDiffer<any>;

  constructor(protected controllerService: ControllerService,
    protected uiconfigsService: UIConfigsService,
    protected cdr: ChangeDetectorRef,
    protected monitoringService: MonitoringService,
    protected _iterableDiffers: IterableDiffers) {
    super(controllerService, cdr, uiconfigsService);
    this.arrayDiffers = _iterableDiffers.find([]).create(HttpEventUtility.trackBy);
  }

  postNgInit() {
    this.getEventPolicy();
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
    let buttons = [];
    if (this.uiconfigsService.isAuthorized(UIRolePermissions.monitoringeventpolicy_create)) {
      buttons = [{
        cssClass: 'global-button-primary eventpolicy-button',
        text: 'ADD EVENT POLICY',
        genTooltip: () => this.getTooltip(),
        computeClass: () => this.shouldEnableButtons && this.maxNewTargets > 0 ? '' : 'global-button-disabled',
        callback: () => { this.createNewObject(); }
      }];
    }
    this.controllerService.setToolbarData({
      buttons: buttons,
      breadcrumb: [{ label: 'Alerts & Events', url: Utility.getBaseUIUrl() + 'monitoring/alertsevents' },
      { label: 'Event Policy', url: Utility.getBaseUIUrl() + 'monitoring/alertsevents/eventpolicy' }
      ]
    });
  }
  getTooltip(): string {
    return this.maxNewTargets === 0 ? 'Cannot exceed 8 total targets across event policies' : '';
  }
  getEventPolicy() {
    this.policyEventUtility = new HttpEventUtility<MonitoringEventPolicy>(MonitoringEventPolicy);
    this.dataObjects = this.policyEventUtility.array;
    const sub = this.monitoringService.WatchEventPolicy().subscribe(
      (response) => {
        this.policyEventUtility.processEvents(response);
      },
      this.controllerService.webSocketErrorHandler('Failed to get Event Policies')
    );
    this.subscriptions.push(sub);
  }

  displayColumn(exportData, col): any {
    const fields = col.field.split('.');
    const value = Utility.getObjectValueByPropertyPath(exportData, fields);
    const column = col.field;
    switch (column) {
      case 'spec':
        return this.formatSyslogExports(value);
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

  formatRequirements(data: FieldsRequirement[]) {
    if (data == null) {
      return '';
    }
    const retArr = [];
    data.forEach((req) => {
      let ret = '';
      ret += req['key'] + ' ';
      ret += Utility.getFieldOperatorSymbol(req.operator);

      ret += ' ';

      if (req.values != null) {
        let values = [];
        const enumInfo = Utility.getNestedPropInfo(new EventsEvent(), req.key).enum;
        values = req.values.map((item) => {
          if (enumInfo != null && [item] != null) {
            return enumInfo[item];
          }
          return item;
        });
        ret += values.join(' or ');
      }
      retArr.push(ret);
    });
    return retArr;
  }

  deleteRecord(object: MonitoringEventPolicy): Observable<{ body: IMonitoringEventPolicy | IApiStatus | Error, statusCode: number }> {
    return this.monitoringService.DeleteEventPolicy(object.meta.name);
  }

  generateDeleteConfirmMsg(object: IMonitoringEventPolicy) {
    return 'Are you sure you want to delete policy ' + object.meta.name;
  }

  generateDeleteSuccessMsg(object: IMonitoringEventPolicy) {
    return 'Deleted policy ' + object.meta.name;
  }
  computeTargets(): number {
    let totaltargets: number = 0;
    for (const policy of this.dataObjects) {
      if (policy.spec.targets !== null) {
        totaltargets += policy.spec.targets.length;
      }
    }
    const remainder = EventpolicyComponent.MAX_TOTAL_TARGETS - totaltargets;
    return Math.min(remainder, EventpolicyComponent.MAX_TARGETS_PER_POLICY);
  }

}
