import { ChangeDetectorRef, Component, OnInit, ViewEncapsulation } from '@angular/core';
import { TableCol } from '@app/components/shared/tableviewedit';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { ControllerService } from '@app/services/controller.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { HttpEventUtility } from '@common/HttpEventUtility';
import { Utility } from '@common/Utility';
import { TablevieweditAbstract } from '@components/shared/tableviewedit/tableviewedit.component';
import { ClusterSmartNIC, IApiStatus } from '@sdk/v1/models/generated/cluster';
import { ClusterHost, IClusterHost } from '@sdk/v1/models/generated/cluster/cluster-host.model';
import { Observable, Subscription } from 'rxjs';
import { Animations } from '@app/animations';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';

@Component({
  selector: 'app-hosts',
  encapsulation: ViewEncapsulation.None,
  templateUrl: './hosts.component.html',
  styleUrls: ['./hosts.component.scss'],
  animations: [Animations]
})
export class HostsComponent extends TablevieweditAbstract<IClusterHost, ClusterHost> implements OnInit {

  bodyicon: Icon = {
    margin: {
      top: '9px',
      left: '8px'
    },
    svgIcon: 'host'
  };

  headerIcon: Icon = {
    margin: {
      top: '0px',
      left: '10px',
    },
    matIcon: 'computer'
  };
  nameToMacMap: { [key: string]: string; } = {};
  macToNameMap: { [key: string]: string; } = {};
  hostsEventUtility: HttpEventUtility<ClusterHost>;
  subscriptions: Subscription[] = [];
  dataObjects: ReadonlyArray<ClusterHost>;
  naples: ReadonlyArray<ClusterSmartNIC> = [];
  naplesEventUtility: HttpEventUtility<ClusterSmartNIC>;
  disableTableWhenRowExpanded: boolean = true;
  isTabComponent: boolean = false;

  cols: TableCol[] = [
    {field: 'meta.name', header: 'Name', class: 'hosts-column-host-name', sortable: true, width: 20},
    {field: 'meta.mod-time', header: 'Modification Time', class: 'hosts-column-date', sortable: true, width: '180px'},
    {field: 'meta.creation-time', header: 'Creation Time', class: 'hosts-column-date', sortable: true, width: '180px'},
    {field: 'spec.smart-nics', header: 'DSC', class: 'hosts-column-smart-nics', sortable: false},
  ];

  exportFilename: string = 'Venice-hosts';

  constructor(private clusterService: ClusterService,
              protected cdr: ChangeDetectorRef,
              protected uiconfigsService: UIConfigsService,
              protected controllerService: ControllerService) {
    super(controllerService, cdr);
  }

  getHosts() {
    this.hostsEventUtility = new HttpEventUtility<ClusterHost>(ClusterHost, true);
    this.dataObjects = this.hostsEventUtility.array as ReadonlyArray<ClusterHost>;
    const subscription = this.clusterService.WatchHost().subscribe(
      response => {
        this.hostsEventUtility.processEvents(response);
        this.dataObjects.map(rowData => {
          rowData['processedSmartNics'] = this.processSmartNics(rowData);
        });
      },
      this.controllerService.webSocketErrorHandler('Failed to get Hosts info')
    );
    this.subscriptions.push(subscription);
  }

  getNaples() {
    this.naplesEventUtility = new HttpEventUtility<ClusterSmartNIC>(ClusterSmartNIC);
    this.naples = this.naplesEventUtility.array as ReadonlyArray<ClusterSmartNIC>;
    const subscription = this.clusterService.WatchSmartNIC().subscribe(
      response => {
        this.naplesEventUtility.processEvents(response);
        // name to mac-address map
        this.nameToMacMap = {};
        this.macToNameMap = {};
        for (const smartnic of this.naples) {
          if (smartnic.spec.id != null && smartnic.spec.id !== '') {
            this.nameToMacMap[smartnic.spec.id] = smartnic.meta.name;
            this.macToNameMap[smartnic.meta.name] = smartnic.spec.id;
          }
        }
      },
      this.controllerService.webSocketErrorHandler('Failed to get NAPLES info')
    );
    this.subscriptions.push(subscription); // add subscription to list, so that it will be cleaned up when component is destroyed.
  }

  setDefaultToolbar() {

    let buttons = [];

    if (this.uiconfigsService.isAuthorized(UIRolePermissions.clusterhost_create)) {
      buttons = [{
        cssClass: 'global-button-primary host-button newhost-button-ADD',
        text: 'ADD HOST',
        callback: () => { this.createNewObject(); }
      }];
    }

    this.controllerService.setToolbarData({
      buttons: buttons,
      breadcrumb: [{label: 'Hosts', url: Utility.getBaseUIUrl() + 'cluster/hosts'}]
    });
  }

  // This func is only working for when
  // spec.smart-nics and status.admitted-smart-nics will only be of length one, and that if status has an entry it's referring to the one in spec.
  isAdmitted(specValue, statusValue): boolean {
    return specValue.length === 1 && statusValue.length === 1;
  }

  displayColumn(exportData, col): any {
    const fields = col.field.split('.');
    const value = Utility.getObjectValueByPropertyPath(exportData, fields);
    const column = col.field;
    switch (column) {
      default:
        return Array.isArray(value) ? JSON.stringify(value, null, 2) : value;
    }
  }

  processSmartNics(exportData) {
    const fields = 'spec.smart-nics'.split('.');
    const value = Utility.getObjectValueByPropertyPath(exportData, fields);
    const statusValue = Utility.getObjectValueByPropertyPath(exportData, 'status.admitted-smart-nics'.split('.'));

    // We only have one entry at this point
    return value.map(v => {
      if (v.hasOwnProperty('id') && v['id']) {
        return {
          text: v['id'],
          mac: this.nameToMacMap[v['id']] || '',
          admitted: this.isAdmitted(value, statusValue)
        };
      } else if (v.hasOwnProperty('mac-address') && v['mac-address']) {
        let text = this.macToNameMap[v['mac-address']];
        if (text == null) {
          text = v['mac-address'];
        }
        return {
          text: text,
          mac: v['mac-address'],
          admitted: this.isAdmitted(value, statusValue)
        };
      } else {
        return {
          text: 'N/A',
          mac: '',
          admitted: this.isAdmitted(value, statusValue)
        };
      }
    });
  }


  postNgInit() {
    this.getNaples();
    this.getHosts();
  }

  deleteRecord(object: ClusterHost): Observable<{ body: IClusterHost | IApiStatus | Error | IClusterHost; statusCode: number }> {
    return this.clusterService.DeleteHost(object.meta.name);
  }

  generateDeleteConfirmMsg(object: IClusterHost): string {
    return 'Are you sure you want to delete host ' + object.meta.name;
  }

  generateDeleteSuccessMsg(object: IClusterHost): string {
    return 'Deleted host ' + object.meta.name;
  }

  getClassName(): string {
    return this.constructor.name;
  }

}
