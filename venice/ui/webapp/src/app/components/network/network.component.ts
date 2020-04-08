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
import { WorkloadWorkload } from '@sdk/v1/models/generated/workload';
import { WorkloadService } from '@app/services/generated/workload.service';
import { NetworkWorkloadsTuple, ObjectsRelationsUtility } from '@app/common/ObjectsRelationsUtility';

interface NetworkUIModel {
  associatedWorkloads: WorkloadWorkload[];
}

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

  workloadList: WorkloadWorkload[] = [];

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
    { field: 'spec.vlan-id', header: 'VLAN', class: 'network-column-vlan', sortable: true, width: '80px'},
    { field: 'spec.orchestrators', header: 'Orchestrators', class: 'network-column-orchestrators', sortable: false, width: 35 },
    { field: 'associatedWorkloads', header: 'Workloads', class: '', sortable: false, width: 35 },
  ];

  constructor(private networkService: NetworkService,
    protected cdr: ChangeDetectorRef,
    protected uiconfigsService: UIConfigsService,
    protected orchestrationService: OrchestrationService,
    protected workloadService: WorkloadService,
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
        this.buildVCenterWorkloadsMap();
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

  /**
   * Fetch workloads.
   */
  watchWorkloads() {
    const workloadSubscription = this.workloadService.ListWorkloadCache().subscribe(
      (response) => {
        if (response.connIsErrorState) {
          return;
        }
        this.workloadList = response.data as WorkloadWorkload[];
        this.buildVCenterWorkloadsMap();
      }
    );
    this.subscriptions.push(workloadSubscription);
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

  displayColumn_workloads(rowData: NetworkNetwork): any {
    const associatedWorkloads: WorkloadWorkload[] = rowData._ui.associatedWorkloads;
    // network may have lots of workloads, only display 8 workloads but
    // provide a link that can allow user to go to workload page to filer
    // right workloads related to the selected vcenter.
    return Utility.getMaxiumNumberWorkloadNames(associatedWorkloads);
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
    this.watchWorkloads();
  }

  buildVCenterWorkloadsMap() {
    if (this.dataObjects && this.dataObjects.length > 0 &&
        this.workloadList && this.workloadList.length > 0) {
      const networkWorkloadsTuple: NetworkWorkloadsTuple =
        ObjectsRelationsUtility.buildNetworkWorkloadsMap(this.workloadList, this.dataObjects);
      this.dataObjects = this.dataObjects.map(network => {
        const associatedWorkloads: WorkloadWorkload[] =
          networkWorkloadsTuple[network.meta.name] || [];
        const uiModel: NetworkUIModel = { associatedWorkloads };
        network._ui = uiModel;
        return network;
      });
    }
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

