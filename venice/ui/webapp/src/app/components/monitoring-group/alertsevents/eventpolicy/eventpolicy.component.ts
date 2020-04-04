import { ChangeDetectorRef, Component, OnInit, ViewEncapsulation } from '@angular/core';
import { Animations } from '@app/animations';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { Utility } from '@app/common/Utility';
import { TablevieweditAbstract } from '@app/components/shared/tableviewedit/tableviewedit.component';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { ControllerService } from '@app/services/controller.service';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { EventsEvent } from '@sdk/v1/models/generated/events';
import { FieldsRequirement, IApiStatus, IMonitoringEventPolicy, MonitoringEventPolicy } from '@sdk/v1/models/generated/monitoring';
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
export class EventpolicyComponent extends TablevieweditAbstract<IMonitoringEventPolicy, MonitoringEventPolicy> implements OnInit {
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
    { field: 'meta.name', header: 'Name', class: 'eventpolicy-column-name', sortable: true, width: 35 },
    // Commenting out as it is not currently supported by backend
    // { field: 'spec.selector', header: 'Filters', class: 'eventpolicy-column-name', sortable: false, width: 30 },
    { field: 'spec.targets', header: 'Targets', class: 'eventpolicy-column-targets', sortable: false, width: 65 },
  ];

  exportFilename: string = 'PSM-event-policies';
  exportMap: CustomExportMap = {};

  isTabComponent = false;
  disableTableWhenRowExpanded = true;

  constructor(protected controllerService: ControllerService,
    protected uiconfigsService: UIConfigsService,
    protected cdr: ChangeDetectorRef,
    protected monitoringService: MonitoringService) {
    super(controllerService, cdr, uiconfigsService);
  }

  postNgInit() {
    this.getEventPolicy();
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
        computeClass: () => this.shouldEnableButtons ? '' : 'global-button-disabled',
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
      case 'spec.targets':
        return value.map(item => item.destination).join(', ');
      case 'spec.email-list':
        return JSON.stringify(value, null, 2);
      case 'spec.snmp-trap-servers':
        return JSON.stringify(value, null, 2);
      default:
        return Array.isArray(value) ? JSON.stringify(value, null, 2) : value;
    }
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

}
