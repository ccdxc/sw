import { ChangeDetectorRef, Component, DoCheck, Input, IterableDiffer, IterableDiffers, OnDestroy, OnInit, ViewChild, ViewEncapsulation } from '@angular/core';
import { Animations } from '@app/animations';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { Utility } from '@app/common/Utility';
import { TableCol, TablevieweditAbstract } from '@app/components/shared/tableviewedit/tableviewedit.component';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { ControllerService } from '@app/services/controller.service';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { MonitoringFwlogPolicy, IMonitoringFwlogPolicy, IApiStatus } from '@sdk/v1/models/generated/monitoring';
import { Table } from 'primeng/table';
import { Observable } from 'rxjs';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';

@Component({
  selector: 'app-fwlogpolicies',
  templateUrl: './fwlogpolicies.component.html',
  styleUrls: ['./fwlogpolicies.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class FwlogpoliciesComponent extends TablevieweditAbstract<IMonitoringFwlogPolicy, MonitoringFwlogPolicy> implements OnInit {
  @ViewChild('policiesTable') policytable: Table;
  subscriptions = [];

  isTabComponent = false;
  disableTableWhenRowExpanded = true;

  dataObjects: ReadonlyArray<MonitoringFwlogPolicy> = [];

  fwlogPoliciesEventUtility: HttpEventUtility<MonitoringFwlogPolicy>;

  bodyIcon: any = {
    margin: {
      top: '9px',
      left: '8px'
    },
    url: '/assets/images/icons/monitoring/icon-firewall-policy-black.svg'
  };

  headerIcon: Icon = {
    margin: {
      top: '0px',
      left: '10px',
    },
    matIcon: 'grid_on'
  };

  cols: any[] = [
    { field: 'meta.name', header: 'Targets', class: 'fwlogpolicies-column-name', sortable: false, width: 30 },
    { field: 'spec.filter', header: 'Exports', class: 'fwlogpolicies-column-filter', sortable: false, width: 35 },
    { field: 'spec.targets', header: 'Targets', class: 'fwlogpolicies-column-targets', sortable: false, isLast: false, width: 35 },
  ];

  constructor(protected controllerService: ControllerService,
    protected uiconfigsService: UIConfigsService,
    protected cdr: ChangeDetectorRef,
    protected monitoringService: MonitoringService,
  ) {
    super(controllerService, cdr);
  }

  postNgInit() {
    this.getFwlogPolicies();
  }

  getClassName(): string {
    return this.constructor.name;
  }

  setDefaultToolbar() {
    let buttons = [];
    if (this.uiconfigsService.isAuthorized(UIRolePermissions.monitoringfwlogpolicy_create)) {
      buttons = [{
        cssClass: 'global-button-primary fwlogpolicies-button fwlogpolicies-button-ADD',
        text: 'ADD FIREWALL LOG POLICY',
        computeClass: () => this.shouldEnableButtons ? '' : 'global-button-disabled',
        callback: () => { this.createNewObject(); }
      }]
    }
    this.controllerService.setToolbarData({
      buttons: buttons,
      breadcrumb: [{ label: 'Firewall Logs', url: Utility.getBaseUIUrl() + 'monitoring/fwlogs' },
      { label: 'Firewall Log Policies', url: Utility.getBaseUIUrl() + 'monitoring/fwlogs/fwlogpolicies' }
      ]
    });
  }

  getFwlogPolicies() {
    this.fwlogPoliciesEventUtility = new HttpEventUtility<MonitoringFwlogPolicy>(MonitoringFwlogPolicy);
    this.dataObjects = this.fwlogPoliciesEventUtility.array;
    const subscription = this.monitoringService.WatchFwlogPolicy().subscribe(
      (response) => {
        this.fwlogPoliciesEventUtility.processEvents(response);
      },
    );
    this.subscriptions.push(subscription);
  }


  displayColumn(exportData, col): any {
    const fields = col.field.split('.');
    const value = Utility.getObjectValueByPropertyPath(exportData, fields);
    const column = col.field;
    switch (column) {
      case 'spec.targets':
        return value.map(item => item.destination).join(', ');
      default:
        return Array.isArray(value) ? value.join(', ') : value;
    }
  }

  deleteRecord(object: MonitoringFwlogPolicy): Observable<{ body: MonitoringFwlogPolicy | IApiStatus | Error, statusCode: number }> {
    return this.monitoringService.DeleteFwlogPolicy(object.meta.name);
  }

  generateDeleteConfirmMsg(object: IMonitoringFwlogPolicy) {
    return 'Are you sure to delete firewall log policy : ' + object.meta.name;
  }

  generateDeleteSuccessMsg(object: MonitoringFwlogPolicy) {
    return 'Deleted firewall log policy ' + object.meta.name;
  }

}
