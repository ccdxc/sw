import { ChangeDetectorRef, Component, OnDestroy, OnInit, ViewChild, ViewEncapsulation } from '@angular/core';
import { ActivatedRoute } from '@angular/router';
import { Animations } from '@app/animations';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { Utility } from '@app/common/Utility';
import { LazyrenderComponent } from '@app/components/shared/lazyrender/lazyrender.component';
import { PrettyDatePipe } from '@app/components/shared/Pipes/PrettyDate.pipe';
import { CustomExportMap, TableCol } from '@app/components/shared/tableviewedit';
import { TablevieweditAbstract } from '@app/components/shared/tableviewedit/tableviewedit.component';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { ControllerService } from '@app/services/controller.service';
import { SecurityService } from '@app/services/generated/security.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { IApiStatus, ISecurityApp, SecurityApp } from '@sdk/v1/models/generated/security';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';
import { Table } from 'primeng/table';
import { Observable, Subscription } from 'rxjs';

interface PartialTableCol {
  field: string;
}


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
  exportMap: CustomExportMap = {
    'spec.alg.type': (opts): string => {
      return opts.data.spec;
    }
  };

  selectedApp: SecurityApp = null;

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
    { field: 'meta.mod-time', header: 'Modification Time', class: 'securityapps-column-date', sortable: true, width: '175px' },
    { field: 'meta.creation-time', header: 'Creation Time', class: 'securityapps-column-date', sortable: true, width: '175px' },
    { field: 'spec.alg.type', header: 'Configuration', class: 'securityapps-column-host-name', sortable: false, width: 35 },
  ];

  constructor(protected _controllerService: ControllerService,
    protected uiconfigsService: UIConfigsService,
    protected cdr: ChangeDetectorRef,
    protected securityService: SecurityService,
    private _route: ActivatedRoute
  ) {
    super(_controllerService, cdr, uiconfigsService);
  }

  postNgInit() {
    this._route.queryParams.subscribe(params => {
      if (params.hasOwnProperty('app')) {
        // alerttab selected
        this.getSearchedSecurityApp(params['app']);
      }
    });

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

  generateDeleteSuccessMsg(object: ISecurityApp) {
    return 'Deleted security app ' + object.meta.name;
  }

  /**
   * Toolbar button call back function
   */
  refresh() {
    this.selectedSecurityApp = null;
    this.getSecurityApps();
  }

  getSearchedSecurityApp(appname) {
    const subscription = this.securityService.GetApp(appname).subscribe(
      response => {
        this.selectedApp = response.body as SecurityApp;
      },
      this._controllerService.webSocketErrorHandler('Failed to get Apps')
    );
    this.subscriptions.push(subscription); // add subscription to list, so that it will be cleaned up when component is destroyed.
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

  displaySpecAlgType(rowData: SecurityApp, col: TableCol) {
    return this.displayColumn(rowData, col, false);
  }

  /**
   * col: TableCol|PartialTableCol make html (line 79) {{displayColumn(this.selectedApp, {'field': 'spec.alg.type'})}} pas compilation process.
   * @param rowData
   * @param col
   * @param hasUiHintMap
   */
  displayColumn(rowData: SecurityApp, col: TableCol|PartialTableCol, hasUiHintMap: boolean = true): any {
    const fields = col.field.split('.');
    if (fields.includes('alg')) {
      if (rowData.spec == null) {
        return '';
      }
    }
    let value = null;
    if (!fields.includes('alg') || rowData.spec.alg) {
      value = Utility.getObjectValueByPropertyPath(rowData, fields);
    }
    const column = col.field;
    if (fields.includes('alg')) {
      if (rowData.spec.alg == null) {
        value = '';
      } else {
        value = 'ALG Type: ' + value;
      }
      const protocolValues = Utility.getObjectValueByPropertyPath(rowData, ['spec', 'proto-ports']);
      if (protocolValues) {
        const protoarray = [];
        for (const i of protocolValues) {
          protoarray.push('Protocol: ' + i.protocol + ' Ports: ' + i.ports);
        }
        if (value) {
          value += '; ';
        }
        value +=  protoarray.join(', ');
      }
    }
    if (fields.includes('mod-time') || fields.includes('creation-time')) {
      value = new PrettyDatePipe('en-US').transform(value);
    }
    switch (column) {
      default:
        return Array.isArray(value) ? value.join(', ') : value;
    }
  }

  selectApp(event) {
    if ( this.selectedApp && event.rowData === this.selectedApp ) {
      this.selectedApp = null;
    } else {
      this.selectedApp = event.rowData;
    }
  }

  closeDetails() {
    this.selectedApp = null;
  }

  dateToString(date) {
    const prettyDate = new PrettyDatePipe('en-US');
    return prettyDate.transform(date);
  }

}
