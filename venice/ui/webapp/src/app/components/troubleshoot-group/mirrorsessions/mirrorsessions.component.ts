import { ChangeDetectorRef, Component, ViewEncapsulation, OnInit } from '@angular/core';
import { Animations } from '@app/animations';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { Utility } from '@app/common/Utility';
import { TablevieweditAbstract } from '@app/components/shared/tableviewedit/tableviewedit.component';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { ControllerService } from '@app/services/controller.service';
import { SecurityService } from '@app/services/generated/security.service';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import {
  IApiStatus, IMonitoringMirrorSession, MonitoringMirrorSession, MonitoringMatchRule,
  MonitoringMirrorCollector, MonitoringMatchSelector, MonitoringAppProtoSelector
} from '@sdk/v1/models/generated/monitoring';
import { ILabelsSelector } from '@sdk/v1/models/generated/monitoring/labels-selector.model';
import { SecurityApp } from '@sdk/v1/models/generated/security';
import { Observable } from 'rxjs';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';
import { TableCol, CustomExportMap } from '@app/components/shared/tableviewedit';
import { SelectItem } from 'primeng/api';

@Component({
  selector: 'app-mirrorsessions',
  templateUrl: './mirrorsessions.component.html',
  styleUrls: ['./mirrorsessions.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class MirrorsessionsComponent extends TablevieweditAbstract<IMonitoringMirrorSession, MonitoringMirrorSession> implements OnInit {
  exportMap: CustomExportMap = {};
  dataObjects: ReadonlyArray<MonitoringMirrorSession> = [];

  mirrorsessonsEventUtility: HttpEventUtility<MonitoringMirrorSession>;

  bodyicon: any = {
    margin: {
      top: '9px',
      left: '8px',
    },
    url: '/assets/images/icons/monitoring/ico-mirror-session-black.svg',
  };
  headerIcon: Icon = {
    margin: {
      top: '0px',
      left: '10px',
    },
    matIcon: 'grid_on'
  };

  cols: TableCol[] = [
    { field: 'meta.name', header: 'Name', sortable: true, width: '125px', notReorderable: true },
    { field: 'meta.mod-time', header: 'Update Time', sortable: true, width: '180px', notReorderable: true },
    { field: 'meta.creation-time', header: 'Creation Time', sortable: true, width: '180px', notReorderable: true },
    { field: 'spec.packet-size', header: 'Packet Size', sortable: false, width: '100px', },
    // { field: 'spec.packet-filters', header: 'Packet Filters', sortable: false, width: 10 },
    { field: 'spec.collectors', header: 'collectors', sortable: false, width: 10 },
    { field: 'spec.interfaces.selectors', header: 'Interface Selectors', sortable: false, width: 20 },
    { field: 'spec.match-rules', header: 'Match Rules', sortable: false, width: 30 },
    // { field: 'status.oper-state', header: 'OP Status', sortable: true, width: '175px' }
  ];

  exportFilename: string = 'Venice-mirrorsessons';

  isTabComponent = false;
  disableTableWhenRowExpanded = true;

  securityAppsEventUtility: HttpEventUtility<SecurityApp>;
  securityApps: ReadonlyArray<SecurityApp> = [];
  securityAppOptions: SelectItem[] = [];

  constructor(protected controllerService: ControllerService,
    protected uiconfigsService: UIConfigsService,
    protected cdr: ChangeDetectorRef,
    protected securityService: SecurityService,
    protected monitoringService: MonitoringService) {
    super(controllerService, cdr, uiconfigsService);
  }

  /**
  * Overide super's API
  * It will return this Component name
  */
  getClassName(): string {
    return this.constructor.name;
  }

  postNgInit() {
    this.getMirrorSessions();
    this.getSecurityApps();
  }

  getMirrorSessions() {
    this.mirrorsessonsEventUtility = new HttpEventUtility<MonitoringMirrorSession>(MonitoringMirrorSession);
    this.dataObjects = this.mirrorsessonsEventUtility.array;
    const sub = this.monitoringService.WatchMirrorSession().subscribe(
      response => {
        this.mirrorsessonsEventUtility.processEvents(response);
      },
      this.controllerService.webSocketErrorHandler('Failed to get Mirror Sessions')
    );
    this.subscriptions.push(sub);
  }

  getSecurityApps() {
    this.securityAppsEventUtility = new HttpEventUtility<SecurityApp>(SecurityApp, false, null, true); // https://pensando.atlassian.net/browse/VS-93 we want to trim the object
    this.securityApps = this.securityAppsEventUtility.array as ReadonlyArray<SecurityApp>;
    const subscription = this.securityService.WatchApp().subscribe(
      response => {
        this.securityAppsEventUtility.processEvents(response);
        this.securityAppOptions = this.securityApps.map(app => {
          return {
            label: app.meta.name,
            value: app.meta.uuid,
          };
        });
      },
      this._controllerService.webSocketErrorHandler('Failed to get Apps')
    );
    this.subscriptions.push(subscription); // add subscription to list, so that it will be cleaned up when component is destroyed.
  }

  setDefaultToolbar(): void {
    let buttons = [];
    if (this.uiconfigsService.isAuthorized(UIRolePermissions.monitoringmirrorsession_create)) {
      buttons = [{
        cssClass: 'global-button-primary mirrorsessions-toolbar-button mirrorsessions-toolbar-button-ADD',
        text: 'ADD MIRROR SESSION',
        computeClass: () => (this.shouldEnableButtons && this.dataObjects.length < 8) ? '' : 'global-button-disabled',
        callback: () => { this.createNewObject(); }
      }];
    }
    this.controllerService.setToolbarData({
      buttons: buttons,
      breadcrumb: [{ label: 'Mirror Sessions', url: Utility.getBaseUIUrl() + 'troubleshoot/mirrorsessions' }]
    });
  }

  displayColumn(data, col): any {
    const fields = col.field.split('.');
    const value = Utility.getObjectValueByPropertyPath(data, fields);
    const column = col.field;
    switch (column) {
      case 'spec.interfaces.selectors':
        // currently we pick the 1st selector from array
        if (Array.isArray(value) && value.length) {
          return this.displayColumn_interfaceselectors(value[0]);
        }
        return '';
      case 'spec.collectors':
        return this.displayColumn_collectors(value);
      case 'spec.match-rules':
        return this.displayColumn_matchRules(value);
      default:
         return Array.isArray(value) ? value.join(', ') : value;
    }
  }

  displayMatchRules(rule: MonitoringMatchRule): string {
    const arr: string[] = [];
    if (rule) {
      const source: MonitoringMatchSelector = rule.source;
      const destination: MonitoringMatchSelector = rule.destination;
      const appProt: MonitoringAppProtoSelector = rule['app-protocol-selectors'];
      if (source['ip-addresses'] && source['ip-addresses'].length > 0) {
        arr.push('Source IPs: ' + source['ip-addresses'].join(', '));
      }
      if (source['mac-addresses'] && source['mac-addresses'].length > 0) {
        arr.push('Source MACs: ' + source['mac-addresses'].join(', '));
      }
      if (destination['ip-addresses'] && destination['ip-addresses'].length > 0) {
        arr.push('Destination IPs: ' + destination['ip-addresses'].join(', '));
      }
      if (destination['mac-addresses'] && destination['mac-addresses'].length > 0) {
        arr.push('Destination MACs: ' + destination['mac-addresses'].join(', '));
      }
      if (appProt['proto-ports'] && appProt['proto-ports'].length > 0) {
        arr.push('Protocols: ' + appProt['proto-ports'].join(', '));
      } else if (appProt.applications && appProt.applications.length > 0 &&
          this.securityAppOptions.length > 0) {
        const appNames: string[] = [];
        appProt.applications.forEach(appId => {
          const appObj: SelectItem = this.securityAppOptions.find(item => item.value === appId);
          if (appObj) {
            appNames.push(appObj.label);
          }
        });
        if (appNames.length > 0) {
          arr.push('Applications: ' + appNames.join(', '));
        }
      }
    }
    return arr.join('; ');
  }

  displayColumn_matchRules(value: MonitoringMatchRule[]): string {
    const list: string[] = value.map(item => this.displayMatchRules(item));
    return this.displayListInColumn(list);
  }

  displayColumn_collectors(value: MonitoringMirrorCollector[]): string {
    const list: string[] = value.map(item => item.type + ': ' + item['export-config'].destination);
    return this.displayListInColumn(list);
  }

  displayColumn_interfaceselectors(value: ILabelsSelector): string {
    const list: string[] = Utility.convertOneLabelSelectorToStringList(value);
    return this.displayListInColumn(list);
  }

  displayListInColumn(list: string[]): string {
    return list.reduce((accum: string, item: string) =>
      accum + '<div class="ellipsisText" title="' + item + '">' + item + '</div>', '');
  }

  deleteRecord(object: MonitoringMirrorSession): Observable<{ body: IMonitoringMirrorSession | IApiStatus | Error; statusCode: number; }> {
    return this.monitoringService.DeleteMirrorSession(object.meta.name);
  }
  generateDeleteConfirmMsg(object: MonitoringMirrorSession): string {
    return 'Are you sure that you want to delete mirror session: ' + object.meta.name + '?';
  }
  generateDeleteSuccessMsg(object: MonitoringMirrorSession): string {
    return 'Deleted delete mirror session ' + object.meta.name;
  }
}
