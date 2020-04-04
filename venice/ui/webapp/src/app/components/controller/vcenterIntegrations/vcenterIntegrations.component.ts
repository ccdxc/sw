import { ChangeDetectorRef, Component, OnInit, ViewEncapsulation } from '@angular/core';
import { Animations } from '@app/animations';
import { CustomExportMap, TableCol } from '@app/components/shared/tableviewedit';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { ControllerService } from '@app/services/controller.service';
import { OrchestrationService } from '@app/services/generated/orchestration.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { HttpEventUtility } from '@common/HttpEventUtility';
import { Utility } from '@common/Utility';
import { TablevieweditAbstract } from '@components/shared/tableviewedit/tableviewedit.component';
import { IApiStatus, OrchestrationOrchestrator, IOrchestrationOrchestrator } from '@sdk/v1/models/generated/orchestration';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';
import * as _ from 'lodash';
import { Observable, Subscription } from 'rxjs';
import { WorkloadWorkload } from '@sdk/v1/models/generated/workload';
import { WorkloadService } from '@app/services/generated/workload.service';
import { VcenterWorkloadsTuple, ObjectsRelationsUtility } from '@app/common/ObjectsRelationsUtility';

/**
 * vCenter Integration page.
 * UI fetches all vcenter objects.
 *
 */
@Component({
  selector: 'app-vcenter-integrations',
  encapsulation: ViewEncapsulation.None,
  templateUrl: './vcenterIntegrations.component.html',
  styleUrls: ['./vcenterIntegrations.component.scss'],
  animations: [Animations]
})

export class VcenterIntegrationsComponent extends TablevieweditAbstract<IOrchestrationOrchestrator, OrchestrationOrchestrator> implements OnInit {

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

  exportFilename: string = 'PSM-vcenter-integrations';

  exportMap: CustomExportMap = {
    'workloads': (opts): string => {
      return opts.data.associatedWorkloads.map(wkld => wkld.meta.name).join(', ');
    }
  };

  subscriptions: Subscription[] = [];
  dataObjects: ReadonlyArray<OrchestrationOrchestrator>;
  vcenterIntegrationEventUtility: HttpEventUtility<OrchestrationOrchestrator>;

  workloadList: WorkloadWorkload[] = [];
  vcenterWorkloadsTuple: VcenterWorkloadsTuple = {} as VcenterWorkloadsTuple;

  disableTableWhenRowExpanded: boolean = true;
  isTabComponent: boolean = false;

  // Used for the table - when true there is a loading icon displayed
  tableLoading: boolean = false;

  cols: TableCol[] = [
    { field: 'meta.name', header: 'Name', class: 'vcenter-integration-column-name', sortable: true, width: '150px' },
    { field: 'meta.creation-time', header: 'Creation Time', class: 'vcenter-integration-column-date', sortable: true, width: '180px' },
    { field: 'spec.uri', header: 'URI', class: 'vcenter-integration-column-url', sortable: true, width: '180px' },
    { field: 'associatedWorkloads', header: 'Workloads', class: '', sortable: false, localSearch: true, width: 100 },
    { field: 'status.connection-status', header: 'Connection Status', class: 'vcenter-integration-column-status', sortable: true, width: '150px' },
    { field: 'status.last-connected', header: 'Last Connected Time', class: 'vcenter-integration-column-lastconnected', sortable: true, width: '180px' },
  ];

  constructor(private orchestrationService: OrchestrationService,
    protected cdr: ChangeDetectorRef,
    protected uiconfigsService: UIConfigsService,
    protected workloadService: WorkloadService,
    protected controllerService: ControllerService) {
    super(controllerService, cdr, uiconfigsService);
  }

  getVcenterIntegrations() {
    this.vcenterIntegrationEventUtility = new HttpEventUtility<OrchestrationOrchestrator>(OrchestrationOrchestrator);
    this.dataObjects = this.vcenterIntegrationEventUtility.array;
    const sub = this.orchestrationService.WatchOrchestrator().subscribe(
      response => {
        this.vcenterIntegrationEventUtility.processEvents(response);
      },
      this.controllerService.webSocketErrorHandler('Failed to get vCenter Integations')
    );
    this.subscriptions.push(sub);
  }

  setDefaultToolbar() {
    let buttons = [];
    if (this.uiconfigsService.isAuthorized(UIRolePermissions.orchestrationorchestrator_create)) {
      buttons = [{
        cssClass: 'global-button-primary vcenter-integrations-button vcenter-integrations-button-ADD',
        text: 'ADD VCENTER',
        computeClass: () => this.shouldEnableButtons ? '' : 'global-button-disabled',
        callback: () => { this.createNewObject(); }
      }];
    }
    this.controllerService.setToolbarData({
      buttons: buttons,
      breadcrumb: [{ label: 'vCenter ', url: Utility.getBaseUIUrl() + 'controller/vcenterintegrations' }]
    });
  }

  displayColumn(rowData: OrchestrationOrchestrator, col: TableCol): any {
    const fields = col.field.split('.');
    const value = Utility.getObjectValueByPropertyPath(rowData, fields);
    const column = col.field;
    switch (column) {
      default:
        return Array.isArray(value) ? JSON.stringify(value, null, 2) : value;
    }
  }

  displayColumn_workloads(rowData: OrchestrationOrchestrator): any {
    const associatedWorkloads: WorkloadWorkload[] =
        this.vcenterWorkloadsTuple[rowData.meta.name];

    if (!associatedWorkloads || associatedWorkloads.length === 0) {
      return '';
    }

    const names: string[] = [];
    // vcenter may have lots of workloads, only display 10 workloads but
    // provide a link that can allow user to go to workload page to filer
    // right workloads related to the selected vcenter.
    for (let i = 0; i < Math.min(10, associatedWorkloads.length); i++) {
      names.push(associatedWorkloads[i].meta.name);
    }
    const namesInStr = names.join(', ');
    return associatedWorkloads.length <= 10 ? namesInStr : namesInStr + ' ...';
  }

  postNgInit() {
    this.getVcenterIntegrations();
    this.watchWorkloads();
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

  buildVCenterWorkloadsMap() {
    if (this.dataObjects && this.dataObjects.length > 0 &&
        this.workloadList && this.workloadList.length > 0) {
      this.vcenterWorkloadsTuple = ObjectsRelationsUtility.buildVcenterWorkloadsMap(this.workloadList, this.dataObjects);
      this.dataObjects.forEach(vcenter => {
        const associatedWorkloads: WorkloadWorkload[] =
          this.vcenterWorkloadsTuple[vcenter.meta.name] || [];
        vcenter._ui.associatedWorkloads = associatedWorkloads;
      });
    }
  }

  deleteRecord(object: OrchestrationOrchestrator): Observable<{ body: IOrchestrationOrchestrator | IApiStatus | Error; statusCode: number }> {
    return this.orchestrationService.DeleteOrchestrator(object.meta.name);
  }

  generateDeleteConfirmMsg(object: IOrchestrationOrchestrator): string {
    return 'Are you sure you want to delete vCenter Integration ' + object.meta.name;
  }

  generateDeleteSuccessMsg(object: IOrchestrationOrchestrator): string {
    return 'Deleted vCenter Integration ' + object.meta.name;
  }

  getClassName(): string {
    return this.constructor.name;
  }
}
