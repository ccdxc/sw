import { Component, OnInit, ViewEncapsulation, ChangeDetectorRef } from '@angular/core';
import { Animations } from '@app/animations';
import { TablevieweditAbstract } from '@app/components/shared/tableviewedit/tableviewedit.component';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { IApiStatus, NetworkNetwork, INetworkNetwork, NetworkOrchestratorInfo } from '@sdk/v1/models/generated/network';
import { CustomExportMap, TableCol } from '@app/components/shared/tableviewedit';
import { Subscription, Observable } from 'rxjs';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { NetworkService } from '@app/services/generated/network.service';
import { OrchestrationService } from '@app/services/generated/orchestration.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { ControllerService } from '@app/services/controller.service';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';
import { Utility } from '@app/common/Utility';
import { OrchestrationOrchestrator } from '@sdk/v1/models/generated/orchestration';
import { SelectItem } from 'primeng/api';

@Component({
  selector: 'app-network',
  encapsulation: ViewEncapsulation.None,
  templateUrl: './network.component.html',
  styleUrls: ['./network.component.scss'],
  animations: [Animations]
})

export class NetworkComponent extends TablevieweditAbstract<INetworkNetwork, NetworkNetwork> implements OnInit {

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

  exportFilename: string = 'PSM-networks';

  exportMap: CustomExportMap = {};

  vcenters: ReadonlyArray<OrchestrationOrchestrator> = [];
  vcenterOptions: SelectItem[] = [];

  subscriptions: Subscription[] = [];
  dataObjects: ReadonlyArray<NetworkNetwork>;
  networkEventUtility: HttpEventUtility<NetworkNetwork>;

  disableTableWhenRowExpanded: boolean = true;
  isTabComponent: boolean = false;

  // Used for the table - when true there is a loading icon displayed
  tableLoading: boolean = false;

  cols: TableCol[] = [
    { field: 'meta.name', header: 'Name', class: 'network-column-name', sortable: true, width: 20 },
    { field: 'meta.creation-time', header: 'Creation Time', class: 'vcenter-integration-column-date', sortable: true, width: '180px' },
    { field: 'spec.vlan-id', header: 'VLAN', class: 'network-column-vlan', sortable: true, width: 10 },
    { field: 'spec.orchestrators', header: 'Orchestrators', class: 'network-column-orchestrators', sortable: false, width: 70 },
    // { field: 'status.workloads', header: 'Workloads', class: 'network-column-workloads', sortable: false, width: 40 },
  ];

  constructor(private networkService: NetworkService,
    protected cdr: ChangeDetectorRef,
    protected uiconfigsService: UIConfigsService,
    protected orchestrationService: OrchestrationService,
    protected controllerService: ControllerService) {
    super(controllerService, cdr, uiconfigsService);
  }

  getNetworks() {
    const hostSubscription = this.networkService.ListNetworkWithWebsocketUpdate().subscribe(
      (response) => {
        if (response.connIsErrorState) {
          return;
        }
        this.dataObjects = response.data;
      },
      this.controllerService.webSocketErrorHandler('Failed to get networks')
    );
    this.subscriptions.push(hostSubscription);
  }

  getVcenterIntegrations() {
    const sub = this.orchestrationService.ListOrchestratorsWithWebsocketUpdate().subscribe(
      response => {
        if (response.connIsErrorState) {
          return;
        }
        this.vcenters = response.data;
        this.vcenterOptions = this.vcenters.map(vcenter => {
          return {
            label: vcenter.meta.name,
            value: vcenter.meta.name
          };
        });
        this.vcenterOptions.push({label: '', value: null});
      },
      this.controllerService.webSocketErrorHandler('Failed to get vCenters')
    );
    this.subscriptions.push(sub);
  }

  setDefaultToolbar() {
    let buttons = [];
    if (this.uiconfigsService.isAuthorized(UIRolePermissions.networknetwork_create)) {
      buttons = [{
        cssClass: 'global-button-primary networks-button networks-button-ADD',
        text: 'ADD NETWORK',
        computeClass: () => this.shouldEnableButtons ? '' : 'global-button-disabled',
        callback: () => { this.createNewObject(); }
      }];
    }
    this.controllerService.setToolbarData({
      buttons: buttons,
      breadcrumb: [{ label: 'Network ', url: Utility.getBaseUIUrl() + 'networks' }]
    });
  }

  displayColumn(rowData: NetworkNetwork, col: TableCol): any {
    const fields = col.field.split('.');
    const value = Utility.getObjectValueByPropertyPath(rowData, fields);
    const column = col.field;
    switch (column) {
      case 'status.workloads':
        return this.displayColumn_workloads(value);
      case 'spec.orchestrators':
        return this.displayColumn_orchestrators(value);
      case 'spec.vlan-id':
        return value ? value : 0;
      default:
        return Array.isArray(value) ? JSON.stringify(value, null, 2) : value;
    }
  }

  displayColumn_workloads(values: string[]): any {
    return '';
  }

  displayColumn_orchestrators(values: NetworkOrchestratorInfo[]): any {
    const map: {'vCenter': string, 'dataCenters': string[]} = {} as any;
    values.forEach((value: NetworkOrchestratorInfo) => {
      if (!map[value['orchestrator-name']]) {
        map[value['orchestrator-name']] = [value.namespace];
      } else {
        map[value['orchestrator-name']].push(value.namespace);
      }
    });
    let result: string = '';
    for (const key of Object.keys(map)) {
      const eachRow: string = 'vCenter: ' + key + ', Datacenter: ' + map[key].join(', ');
      result += '<div class="ellipsisText" title="' + eachRow + '">' + eachRow + '</div>';
    }
    return result;
  }

  postNgInit() {
    this.getNetworks();
    this.getVcenterIntegrations();
  }

  deleteRecord(object: NetworkNetwork): Observable<{ body: INetworkNetwork | IApiStatus | Error; statusCode: number }> {
    return this.networkService.DeleteNetwork(object.meta.name);
  }

  generateDeleteConfirmMsg(object: INetworkNetwork): string {
    return 'Are you sure you want to delete network ' + object.meta.name;
  }

  generateDeleteSuccessMsg(object: INetworkNetwork): string {
    return 'Deleted network ' + object.meta.name;
  }

  getClassName(): string {
    return this.constructor.name;
  }
}

