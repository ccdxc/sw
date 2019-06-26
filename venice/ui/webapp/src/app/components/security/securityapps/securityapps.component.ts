import { Component, OnInit, ViewChild, ViewEncapsulation, OnDestroy } from '@angular/core';
import { Utility } from '@app/common/Utility';
import { BaseComponent } from '@app/components/base/base.component';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { ControllerService } from '@app/services/controller.service';
import { SecurityService } from '@app/services/generated/security.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { SecurityApp } from '@sdk/v1/models/generated/security';
import { Table } from 'primeng/table';
import { LazyrenderComponent } from '@app/components/shared/lazyrender/lazyrender.component';


import { Subscription } from 'rxjs';
import { Icon } from '@app/models/frontend/shared/icon.interface';

@Component({
  selector: 'app-securityapps',
  templateUrl: './securityapps.component.html',
  styleUrls: ['./securityapps.component.scss'],
  encapsulation: ViewEncapsulation.None
})
/**
 * This component displays security-apps UI.
 * TODO: 2018-12-20, there is no UX design for this page yet. It simply show records to help QA viewing data.
 */
export class SecurityappsComponent extends BaseComponent implements OnInit, OnDestroy {
  @ViewChild('securityappsTable') securityappsTable: Table;
  @ViewChild(LazyrenderComponent) lazyRenderWrapper: LazyrenderComponent;

  securityappsEventUtility: HttpEventUtility<SecurityApp>;

  subscriptions: Subscription[] = [];

  securityApps: ReadonlyArray<SecurityApp> = [];
  selectedSecurityApp: SecurityApp = null;

  bodyicon: any = {
    margin: {
      top: '9px',
      left: '8px',
    },
    url: '/assets/images/icons/security/ico-app-black.svg',
  };

  tableIcon: Icon = {
    margin: {
      top: '0px',
      left: '10px',
    },
    matIcon: 'grid_on'
  };


  cols: any[] = [
    { field: 'meta.name', header: 'Name', class: 'securityapps-column-metaname', sortable: true },
    { field: 'spec.alg.type', header: 'ALG Type', class: 'securityapps-column-host-name', sortable: false },
    { field: 'meta.mod-time', header: 'Modification Time', class: 'securityapps-column-date', sortable: true },
    { field: 'meta.creation-time', header: 'Creation Time', class: 'securityapps-column-date', sortable: true },
  ];

  constructor(protected _controllerService: ControllerService,
    protected uiconfigsService: UIConfigsService,
    protected securityService: SecurityService,
  ) {
    super(_controllerService);
  }

  ngOnInit() {
    this._controllerService.publish(Eventtypes.COMPONENT_INIT, { 'component': 'SecurityappsComponent', 'state': Eventtypes.COMPONENT_INIT });
    this.getSecurityApps();
    this._controllerService.setToolbarData({
      buttons: [],
      breadcrumb: [{ label: 'Security Apps', url: Utility.getBaseUIUrl() + 'security/securityapps' }]
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
    this.securityApps = this.securityappsEventUtility.array as ReadonlyArray<SecurityApp>;
    const subscription = this.securityService.WatchApp().subscribe(
      response => {
        this.securityappsEventUtility.processEvents(response);
        // As server  keeps pushing records to UI and UI has a selected securityApp, we have to update the selected one.
        if (this.selectedSecurityApp) {
          let matchedSecurityApp: SecurityApp = null;
          for (let i = 0; i < this.securityApps.length; i++) {
            const secApp = this.securityApps[i];
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

}
