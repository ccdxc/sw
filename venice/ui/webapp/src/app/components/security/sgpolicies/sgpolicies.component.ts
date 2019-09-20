import { Component, OnInit, ViewEncapsulation, OnDestroy, ChangeDetectorRef } from '@angular/core';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { Utility } from '@app/common/Utility';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { ControllerService } from '@app/services/controller.service';
import { SecurityService } from '@app/services/generated/security.service';
import { UIConfigsService, Features } from '@app/services/uiconfigs.service';
import { SecurityNetworkSecurityPolicy, ISecurityNetworkSecurityPolicy, IApiStatus } from '@sdk/v1/models/generated/security';
import { Observable } from 'rxjs';
import { TablevieweditAbstract } from '@app/components/shared/tableviewedit/tableviewedit.component';
import { TableCol } from '@app/components/shared/tableviewedit';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';

@Component({
  selector: 'app-sgpolicies',
  templateUrl: './sgpolicies.component.html',
  styleUrls: ['./sgpolicies.component.scss'],
  encapsulation: ViewEncapsulation.None
})
export class SgpoliciesComponent extends TablevieweditAbstract<ISecurityNetworkSecurityPolicy, SecurityNetworkSecurityPolicy> implements OnInit, OnDestroy {
  isTabComponent: boolean = false;
  disableTableWhenRowExpanded: boolean  = true;
  dataObjects: ReadonlyArray<SecurityNetworkSecurityPolicy> = [];
  exportFilename: string = 'Venice-sgpolicies';


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
    protected cdr: ChangeDetectorRef,
  ) {
    super(_controllerService, cdr, uiconfigsService);
  }

  postNgInit() {
    this.getSGPolicies();
  }

  setDefaultToolbar() {
    let buttons = [];
    if (this.uiconfigsService.isAuthorized(UIRolePermissions.securitynetworksecuritypolicy_create) && this.uiconfigsService.isFeatureEnabled(Features.createNetworkSecurityPolicy)) {
      buttons = [{
        cssClass: 'global-button-primary global-button-padding',
        text: 'ADD POLICY',
        computeClass: () => this.shouldEnableButtons ? '' : 'global-button-disabled',
        callback: () => { this.createNewObject(); }
      }];
    }
    this._controllerService.setToolbarData({
      buttons: buttons,
      breadcrumb: [{ label: 'Network Security Policies', url: Utility.getBaseUIUrl() + 'security/sgpolicies' }]
    });
  }

  getSGPolicies() {
    this.sgPoliciesEventUtility = new HttpEventUtility<SecurityNetworkSecurityPolicy>(SecurityNetworkSecurityPolicy);
    this.dataObjects = this.sgPoliciesEventUtility.array;
    const subscription = this.securityService.WatchNetworkSecurityPolicy().subscribe(
      response => {
        this.sgPoliciesEventUtility.processEvents(response);
      },
      this._controllerService.webSocketErrorHandler('Failed to get SG Policies')
    );
    this.subscriptions.push(subscription);
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
}
