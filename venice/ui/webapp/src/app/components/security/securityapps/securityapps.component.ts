import { Component, ChangeDetectorRef, OnInit, ViewChild, ViewEncapsulation, OnDestroy } from '@angular/core';
import { Utility } from '@app/common/Utility';
import { BaseComponent } from '@app/components/base/base.component';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { ControllerService } from '@app/services/controller.service';
import { SecurityService } from '@app/services/generated/security.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { SecurityApp, ISecurityApp, IApiStatus } from '@sdk/v1/models/generated/security';
import { Table } from 'primeng/table';
import { Observable } from 'rxjs';
import { LazyrenderComponent } from '@app/components/shared/lazyrender/lazyrender.component';
import { TablevieweditAbstract } from '@app/components/shared/tableviewedit/tableviewedit.component';
import { TableCol, CustomExportMap } from '@app/components/shared/tableviewedit';
import { Animations } from '@app/animations';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';
import { PrettyDatePipe } from '@app/components/shared/Pipes/PrettyDate.pipe';


import { Subscription } from 'rxjs';
import { Icon } from '@app/models/frontend/shared/icon.interface';

@Component({
  selector: 'app-securityapps',
  templateUrl: './securityapps.component.html',
  styleUrls: ['./securityapps.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
/**
 * This component displays security-apps UI.
 * TODO: 2018-12-20, there is no UX design for this page yet. It simply show records to help QA viewing data.
 */
export class SecurityappsComponent extends TablevieweditAbstract<ISecurityApp, SecurityApp> implements OnInit, OnDestroy {
  @ViewChild('securityappsTable') securityappsTable: Table;
  @ViewChild(LazyrenderComponent) lazyRenderWrapper: LazyrenderComponent;
  dataObjects: ReadonlyArray<SecurityApp> = [];

  securityappsEventUtility: HttpEventUtility<SecurityApp>;
  disableTableWhenRowExpanded = true;
  subscriptions: Subscription[] = [];
  isTabComponent = false;
  exportFilename: string = 'Apps';
  exportMap: CustomExportMap = {};

  selectedSecurityApp: SecurityApp = null;

  bodyIcon: any = {
    margin: {
      top: '9px',
      left: '8px',
    },
    url: '/assets/images/icons/security/ico-app-black.svg',
  };

  headerIcon: Icon = {
    margin: {
      top: '0px',
      left: '10px',
    },
    matIcon: 'grid_on'
  };


  cols: TableCol[] = [
    { field: 'meta.name', header: 'Name', class: 'securityapps-column-metaname', sortable: true, width: 15 },
    { field: 'spec.alg.type', header: 'Configuration', class: 'securityapps-column-host-name', sortable: false, width: 35 },
    { field: 'meta.mod-time', header: 'Modification Time', class: 'securityapps-column-date', sortable: true, width: 25 },
    { field: 'meta.creation-time', header: 'Creation Time', class: 'securityapps-column-date', sortable: true, width: 25 },
  ];

  constructor(protected _controllerService: ControllerService,
    protected uiconfigsService: UIConfigsService,
    protected cdr: ChangeDetectorRef,
    protected securityService: SecurityService
  ) {
    super(_controllerService, cdr, uiconfigsService);
  }

  ngOnInit() {
    this._controllerService.publish(Eventtypes.COMPONENT_INIT, { 'component': 'SecurityappsComponent', 'state': Eventtypes.COMPONENT_INIT });
    this.getSecurityApps();
    this.setDefaultToolbar();
  }

  setDefaultToolbar() {
    let buttons = [];
    if (this.uiconfigsService.isAuthorized(UIRolePermissions.securityapp_create)) {
      buttons = [
        {
          cssClass: 'global-button-primary security-new-app',
          text: 'ADD APP',
          computeClass: () => this.shouldEnableButtons ? '' : 'global-button-disabled',
          callback: () => { this.createNewObject(); }
        }
      ];
    }
    this._controllerService.setToolbarData({
      buttons: buttons,
      breadcrumb: [{ label: 'Apps', url: Utility.getBaseUIUrl() + 'security/securityapps' }]
    });
  }

  postNgInit() {

  }

  getClassName(): string {
    return this.constructor.name;
  }

  ngOnDestroy() {
    this.subscriptions.forEach(
      subscription => {
        subscription.unsubscribe();
      }
    );
  }

  generateDeleteConfirmMsg(object: ISecurityApp) {
    return 'Are you sure you want to delete security app : ' + object.meta.name;
  }

  generateDeleteSuccessMsg(object: SecurityApp) {
    return 'Deleted security app ' + object.meta.name;
  }

  /**
   * Toolbar button call back function
   */
  refresh() {
    this.selectedSecurityApp = null;
    this.getSecurityApps();
  }

  /**
   * Fetch security apps records
   */
  getSecurityApps() {
    this.securityappsEventUtility = new HttpEventUtility<SecurityApp>(SecurityApp, false, null, true); // https://pensando.atlassian.net/browse/VS-93 we want to trim the object
    this.dataObjects = this.securityappsEventUtility.array;
    const subscription = this.securityService.WatchApp().subscribe(
      response => {
        this.securityappsEventUtility.processEvents(response);
        // As server  keeps pushing records to UI and UI has a selected securityApp, we have to update the selected one.
        if (this.selectedSecurityApp) {
          let matchedSecurityApp: SecurityApp = null;
          for (let i = 0; i < this.dataObjects.length; i++) {
            const secApp = this.dataObjects[i];
            if (secApp.meta.name === this.selectedSecurityApp.meta.name) {
              matchedSecurityApp = secApp;
            }
          }
          this.selectedSecurityApp = matchedSecurityApp; // matchedSecurityApp could be null. It means the UI selected one is deleted in server.
        }
      },
      this._controllerService.webSocketErrorHandler('Failed to get Apps')
    );
    this.subscriptions.push(subscription); // add subscription to list, so that it will be cleaned up when component is destroyed.
  }

  /**
   * This API serves html template
   */
  displaySecurityApp(): string {
    return JSON.stringify(this.selectedSecurityApp, null, 1);
  }

  deleteRecord(object: SecurityApp): Observable<{ body: SecurityApp | IApiStatus | Error, statusCode: number }> {
    return this.securityService.DeleteApp(object.meta.name);
  }

  /**
   * This API serves html template
   */
  onSecurityAppsTableRowClick(event, rowData: any) {
    this.selectedSecurityApp = rowData;
    this.securityappsTable.toggleRow(rowData, event);
    this.lazyRenderWrapper.resizeTable(); // This is necessary to properly show expanded row.
    return false;
  }

  displaySpecAlgType(rowData, col) {
    return this.displayColumn(rowData, col, false);
  }

  displayColumn(Data, col, hasUiHintMap: boolean = true): any {
    let fields = col.field.split('.');
    if (fields.includes('alg')) {
      if (Data.spec == null) {
        return '';
      }
      if (Data.spec.alg == null) {
        fields = ['spec', 'proto-ports'];
      }
    }
    let value = Utility.getObjectValueByPropertyPath(Data, fields);
    const column = col.field;
    if (fields.includes('alg')) {

      value = 'ALG Type:   ' + value;
    }
    if (fields.includes('proto-ports')) {
      const protoarray = [];
      for (const i of value) {
        protoarray.push('Protocol: ' + i.protocol + ' Ports: ' + i.ports);
      }
      value =  protoarray.join(', ');
    }
    if (fields.includes('mod-time') || fields.includes('creation-time')) {
      value = new PrettyDatePipe('en-US').transform(value);
    }
    switch (column) {
      default:
        return Array.isArray(value) ? value.join(', ') : value;
    }
  }

}
