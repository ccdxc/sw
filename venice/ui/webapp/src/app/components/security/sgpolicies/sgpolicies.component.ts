import { Component, OnInit, ViewEncapsulation, OnDestroy, ChangeDetectorRef } from '@angular/core';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { Utility } from '@app/common/Utility';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { ControllerService } from '@app/services/controller.service';
import { SecurityService } from '@app/services/generated/security.service';
import { UIConfigsService, Features } from '@app/services/uiconfigs.service';
import { SecurityNetworkSecurityPolicy, ISecurityNetworkSecurityPolicy, IApiStatus, ISecurityNetworkSecurityPolicyList, SecurityApp, SecuritySecurityGroup } from '@sdk/v1/models/generated/security';
import { Observable } from 'rxjs';
import { TablevieweditAbstract } from '@app/components/shared/tableviewedit/tableviewedit.component';
import { TableCol, CustomExportMap } from '@app/components/shared/tableviewedit';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';
import { SelectItem } from 'primeng/api';
import { WorkloadWorkload } from '@sdk/v1/models/generated/workload';
import { WorkloadService } from '@app/services/generated/workload.service';

@Component({
  selector: 'app-sgpolicies',
  templateUrl: './sgpolicies.component.html',
  styleUrls: ['./sgpolicies.component.scss'],
  encapsulation: ViewEncapsulation.None
})
export class SgpoliciesComponent extends TablevieweditAbstract<ISecurityNetworkSecurityPolicy, SecurityNetworkSecurityPolicy> implements OnInit, OnDestroy {
  isTabComponent: boolean = false;
  disableTableWhenRowExpanded: boolean = true;
  dataObjects: ReadonlyArray<SecurityNetworkSecurityPolicy> = [];
  exportFilename: string = 'PSM-sgpolicies';
  exportMap: CustomExportMap = {};

  securityApps: ReadonlyArray<SecurityApp> = [];
  securityAppOptions: SelectItem[] = [];

  securityGroups: ReadonlyArray<SecuritySecurityGroup> = [];
  securityGroupOptions: SelectItem[] = [];

  workloads: ReadonlyArray<WorkloadWorkload> = [];
  // Map from IP to workload name
  ipOptions: any[] = [];

  // Currently venice supports only one security policy.
  MAX_POLICY_NUM: number = 1;
  EDIT_INLINE_MAX_RULES_LIMIT: number = 0;

  // Holds all policy objects
  sgPoliciesEventUtility: HttpEventUtility<SecurityNetworkSecurityPolicy>;

  // All columns are set as not sortable as it isn't currently supported
  cols: TableCol[] = [
    { field: 'meta.name', header: 'Policy Name', class: 'sgpolicies-column-name', sortable: true, width: 'auto' },
    { field: 'meta.mod-time', header: 'Modification Time', class: 'sgpolicies-column-date', sortable: true, width: '180px' },
    { field: 'meta.creation-time', header: 'Creation Time', class: 'sgpolicies-column-date', sortable: true, width: '25' },
  ];

  bodyIcon = {
    margin: {
      top: '9px',
      left: '8px'
    },
    url: '/assets/images/icons/security/icon-security-policy-black.svg'
  };

  tableIcon: Icon = {
    margin: {
      top: '0px',
      left: '0px',
    },
    matIcon: 'grid_on'
  };

  constructor(protected _controllerService: ControllerService,
    protected uiconfigsService: UIConfigsService,
    protected securityService: SecurityService,
    protected workloadService: WorkloadService,
    protected cdr: ChangeDetectorRef,
  ) {
    super(_controllerService, cdr, uiconfigsService);
    this.shouldEnableButtons = false;
  }

  postNgInit() {
    this.getSecurityPolicies();
    this.getSecurityApps();
    // this.getWorkloads();
    // this.getSecuritygroups();
  }

  setDefaultToolbar() {
    let buttons = [];
    if (this.uiconfigsService.isAuthorized(UIRolePermissions.securitynetworksecuritypolicy_create)) {
      buttons = [{
        cssClass: 'global-button-primary global-button-padding',
        text: 'ADD POLICY',
        computeClass: () => this.shouldEnableButtons ? '' : 'global-button-disabled',
        callback: () => { this.createNewObject(); },
        genTooltip: () => this.getTooltip(),
      }];
    }
    this._controllerService.setToolbarData({
      buttons: buttons,
      breadcrumb: [{ label: 'Network Security Policies', url: Utility.getBaseUIUrl() + 'security/sgpolicies' }]
    });
  }

  getTooltip(): string {
    return this.dataObjects.length > 0 ? 'System allows one network security policy' : 'Add network security policy';
  }

  getSecurityPolicies() {
    this.securityService.ListNetworkSecurityPolicy().subscribe(
      (response) => {
        if (response && response.body) {
          const body: ISecurityNetworkSecurityPolicyList = response.body as ISecurityNetworkSecurityPolicyList;
          if (!body.items || (body.items && body.items.length < this.MAX_POLICY_NUM)) {
            this.shouldEnableButtons = true;
          }
        }
      },
      (error) => {
        this.controllerService.invokeRESTErrorToaster('Failed to get network security policy', error);
      }
    );
    this.sgPoliciesEventUtility = new HttpEventUtility<SecurityNetworkSecurityPolicy>(SecurityNetworkSecurityPolicy);
    this.dataObjects = this.sgPoliciesEventUtility.array;
    const subscription = this.securityService.WatchNetworkSecurityPolicy().subscribe(
      response => {
        this.sgPoliciesEventUtility.processEvents(response);
        // user may delete the only one sg-policy. If so, UI should enable the [Add POLICY] toolbar button
        this.shouldEnableButtons = (this.dataObjects.length === 0);

        this.setDefaultToolbar();
      },
      this._controllerService.webSocketErrorHandler('Failed to get security policies')
    );
    this.subscriptions.push(subscription);
  }

  getSecurityApps() {
    const sub = this.securityService.ListAppCache().subscribe(
      response => {
        if (response.connIsErrorState) {
          return;
        }
        this.securityApps = response.data;
        this.securityAppOptions = this.securityApps.map(item => {
          return {
            label: item.meta.name,
            value: item.meta.name
          };
        });
      },
      this.controllerService.webSocketErrorHandler('Failed to get apps')
    );
    this.subscriptions.push(sub);
  }

  getSecuritygroups() {
    const sub = this.securityService.ListSecurityGroupCache().subscribe(
      response => {
        if (response.connIsErrorState) {
          return;
        }
        this.securityGroups = response.data;
        this.securityAppOptions = this.securityGroups.map(item => {
          return {
            label: item.meta.name,
            value: item.meta.name
          };
        });
      },
      this.controllerService.webSocketErrorHandler('Failed to get security policy groups')
    );
    this.subscriptions.push(sub);
  }


  getWorkloads() {
    const workloadSubscription = this.workloadService.ListWorkloadCache().subscribe(
      (response) => {
        if (response.connIsErrorState) {
          return;
        }
        this.workloads = response.data as WorkloadWorkload[];
        this.buildIPMap();
      },
      this.controllerService.webSocketErrorHandler('Failed to get workloads')
    );
    this.subscriptions.push(workloadSubscription);
  }

  buildIPMap() {
    const ipMap = {};
    this.ipOptions = [];
    // Taking IPs from spec, since status isn't always filled out currently
    // TODO: Take IPs from status
    this.workloads.forEach((w) => {
      w.spec.interfaces.forEach((intf) => {
        intf['ip-addresses'].forEach((ip) => {
          ipMap[ip] = w.meta.name;
        });
      });
    });
    Object.keys(ipMap).forEach(ip => {
      this.ipOptions.push({ ip: ip, workload: ipMap[ip] });
    });
  }

  displayColumn(data, col): any {
    const fields = col.field.split('.');
    const value = Utility.getObjectValueByPropertyPath(data, fields);
    const column = col.field;
    switch (column) {
      default:
        return Array.isArray(value) ? JSON.stringify(value, null, 2) : value;
    }
  }

  deleteRecord(object: SecurityNetworkSecurityPolicy): Observable<{ body: ISecurityNetworkSecurityPolicy | IApiStatus | Error, statusCode: number }> {
    return this.securityService.DeleteNetworkSecurityPolicy(object.meta.name);
  }

  generateDeleteConfirmMsg(object: ISecurityNetworkSecurityPolicy) {
    return 'Are you sure you want to delete SG Policy ' + object.meta.name;
  }

  generateDeleteSuccessMsg(object: ISecurityNetworkSecurityPolicy) {
    return 'Deleted SG Policy ' + object.meta.name;
  }

  editTooltip(rowData) {
    if (rowData.spec.rules.length > this.EDIT_INLINE_MAX_RULES_LIMIT) {
      return 'Edit Policy on Details Page';
    } else {
      return 'Edit Policy Inline';
    }
  }

  attemptInlineEdit(rowData): boolean {
    if (rowData.spec.rules.length > this.EDIT_INLINE_MAX_RULES_LIMIT) {
      this._controllerService.navigate(['/security', 'sgpolicies', rowData.meta.name]);
    }
    return true;
  }
}
