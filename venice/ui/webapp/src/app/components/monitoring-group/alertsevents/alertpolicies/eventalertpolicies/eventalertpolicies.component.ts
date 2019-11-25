import { ChangeDetectorRef, Component, Input, ViewEncapsulation } from '@angular/core';
import { Animations } from '@app/animations';
import { Utility } from '@app/common/Utility';
import { TablevieweditAbstract } from '@app/components/shared/tableviewedit/tableviewedit.component';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { ControllerService } from '@app/services/controller.service';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { EventsEvent } from '@sdk/v1/models/generated/events';
import { IApiStatus, IMonitoringAlertPolicy, MonitoringAlertDestination, MonitoringAlertPolicy, MonitoringAlertPolicySpec_severity } from '@sdk/v1/models/generated/monitoring';
import { Observable } from 'rxjs';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';
import { TableCol, CustomExportMap } from '@app/components/shared/tableviewedit';


@Component({
  selector: 'app-eventalertpolicies',
  templateUrl: './eventalertpolicies.component.html',
  styleUrls: ['./eventalertpolicies.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class EventalertpolicyComponent extends TablevieweditAbstract<IMonitoringAlertPolicy, MonitoringAlertPolicy> {
  @Input() dataObjects: MonitoringAlertPolicy[] = [];

  headerIcon: Icon = {
    margin: {
      top: '0px',
      left: '10px',
    },
    matIcon: 'notifications'
  };
  globalFilterFields: string[] = ['meta.name', 'spec.destinations', 'spec.severity'];
  severityEnum = MonitoringAlertPolicySpec_severity;

  cols: TableCol[] = [
    { field: 'meta.name', header: 'Policy Name', class: 'eventalertpolicies-column-name', sortable: true, width: 20 },
    { field: 'spec.destinations', header: 'Destinations', class: 'eventalertpolicies-column-destinations', sortable: true, width: 10 },
    { field: 'spec.requirements', header: 'Requirements', class: 'eventalertpolicies-column-requirements', sortable: false, width: 20 },
    { field: 'spec.severity', header: 'Severity', class: 'eventalertpolicies-column-severity', sortable: false, width: 15 },
    { field: 'status.total-hits', header: 'Total Hits', class: 'eventalertpolicies-column-totalhits', sortable: false, width: 10 },
    { field: 'status.open-alerts', header: 'Open', class: 'eventalertpolicies-column-openalerts', sortable: false, width: 10 },
    { field: 'status.acknowledged-alerts', header: 'Acknowledged', class: 'eventalertpolicies-column-acknowledgedalerts', sortable: false, width: 15 }
  ];

  exportFilename: string = 'Venice-event-alert-policies';

  isTabComponent = true;
  exportMap: CustomExportMap = {};
  disableTableWhenRowExpanded = true;

  @Input() destinations: MonitoringAlertDestination[] = [];

  constructor(protected controllerService: ControllerService,
    protected cdr: ChangeDetectorRef,
    protected uiconfigsService: UIConfigsService,
    protected monitoringService: MonitoringService) {
    super(controllerService, cdr, uiconfigsService);
  }

  // Hook for overriding
  postNgInit() { }

  getClassName(): string {
    return this.constructor.name;
  }

  setDefaultToolbar() {
    const currToolbar = this.controllerService.getToolbarData();
    currToolbar.buttons = [];
    if (this.uiconfigsService.isAuthorized(UIRolePermissions.monitoringalertpolicy_create)) {
      currToolbar.buttons = [
        {
          cssClass: 'global-button-primary eventalertpolicies-button',
          text: 'ADD ALERT POLICY',
          computeClass: () => this.shouldEnableButtons ? '' : 'global-button-disabled',
          callback: () => { this.createNewObject(); }
        },
      ];
    }
    this.controllerService.setToolbarData(currToolbar);
  }

  /**
   * This api serves html template
   */
  getAlertItemIconClass(record) {
    return 'global-alert-' + record.severity;
  }

  formatRequirements(data: MonitoringAlertPolicy) {
    const value = data.spec.requirements;
    if (value == null) {
      return '';
    }
    const retArr = [];
    value.forEach((req) => {
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

  displayColumn(eventalertpolicies, col): any {
    const fields = col.field.split('.');
    const value = Utility.getObjectValueByPropertyPath(eventalertpolicies, fields);
    const column = col.field;
    switch (column) {
      case 'spec.destinations':
        if (value != null) {
          return value.join(' ');
        }
        return '';
      default:
        return Array.isArray(value) ? JSON.stringify(value, null, 2) : value;
    }
  }

  deleteRecord(object: MonitoringAlertPolicy): Observable<{ body: IMonitoringAlertPolicy | IApiStatus | Error, statusCode: number }> {
    return this.monitoringService.DeleteAlertPolicy(object.meta.name);
  }

  generateDeleteConfirmMsg(object: IMonitoringAlertPolicy) {
    return 'Are you sure you want to delete policy ' + object.meta.name;
  }

  generateDeleteSuccessMsg(object: IMonitoringAlertPolicy) {
    return 'Deleted policy ' + object.meta.name;
  }

}
