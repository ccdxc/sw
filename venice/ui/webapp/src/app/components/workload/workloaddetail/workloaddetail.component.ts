import { Component, OnDestroy, OnInit, ViewEncapsulation } from '@angular/core';
import { ActivatedRoute, Router } from '@angular/router';
import { Animations } from '@app/animations';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { ObjectsRelationsUtility } from '@app/common/ObjectsRelationsUtility';
import { Utility } from '@app/common/Utility';
import { DataComponent } from '@app/components/shared/datacomponent/datacomponent.component';
import { LabelEditorMetadataModel } from '@app/components/shared/labeleditor';
import { TableCol } from '@app/components/shared/tableviewedit';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { ControllerService } from '@app/services/controller.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { WorkloadService } from '@app/services/generated/workload.service';
import { SearchService } from '@app/services/generated/search.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { ClusterDistributedServiceCard, ClusterHost } from '@sdk/v1/models/generated/cluster';
import { SearchSearchRequest } from '@sdk/v1/models/generated/search';
import { IStagingBulkEditAction } from '@sdk/v1/models/generated/staging';
import { WorkloadWorkload } from '@sdk/v1/models/generated/workload';

@Component({
  selector: 'app-workloaddetail',
  templateUrl: './workloaddetail.component.html',
  styleUrls: ['./workloaddetail.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class WorkloaddetailComponent extends DataComponent implements OnInit, OnDestroy {
  allHosts: ReadonlyArray<ClusterHost> = [];
  allNaples: ReadonlyArray<ClusterDistributedServiceCard> = [];
  initHosts: boolean;
  initNaples: boolean;
  initWorkload: boolean;
  inLabelEditMode: boolean;
  interfaceColumns: TableCol[] = [];
  interfaceColumnsAll: ReadonlyArray<TableCol> = [
    { field: 'mac-address', header: 'MAC Address', sortable: true },
    { field: 'ip-addresses', header: 'IP Addresses' },
    { field: 'external-vlan', header: 'External VLAN', sortable: true },
    { field: 'network', header: 'Network', sortable: true },
    { field: 'micro-seg-vlan', header: 'Micro Seg VLAN', sortable: true },
  ];
  labelColumns: TableCol[] = [
    { field: 'key', header: 'Key', sortable: true },
    { field: 'value', header: 'Value', sortable: true },
  ];
  labelData: Array<any> = [];
  labelEditorMetaData: LabelEditorMetadataModel;
  loading: boolean;
  naples: ClusterDistributedServiceCard[] = [];
  selectedId: string;
  selectedObj: WorkloadWorkload;
  showDeletionScreen: boolean;
  showMissingScreen: boolean;
  subscriptions = [];

  bodyicon: any = {
    margin: {
      top: '9px',
      left: '8px'
    },
    url: '/assets/images/icons/workload/icon-workloads.svg'
  };

  constructor(protected _controllerService: ControllerService,
    private _route: ActivatedRoute,
    private _router: Router,
    private clusterService: ClusterService,
    protected workloadService: WorkloadService,
    protected searchService: SearchService,
    protected uiconfigsService: UIConfigsService,
  ) {
    super(_controllerService, uiconfigsService);
  }

  ngOnInit() {
    if (this.uiconfigsService.isFeatureEnabled('cloud')) {
      this.uiconfigsService.navigateToHomepage();
      return;
    }
    this.loading = true;
    this._controllerService.publish(Eventtypes.COMPONENT_INIT, { 'component': 'WorkloaddetailComponent', 'state': Eventtypes.COMPONENT_INIT });
    this._route.paramMap.subscribe(params => {
      this.selectedId = params.get('id');
      this.subscriptions.forEach(subscription => {
        subscription.unsubscribe();
      });
      const queryAction = this._route.snapshot.queryParamMap.get('action');
      if (queryAction === 'lookup') {
        this.searchWorkload();
      } else {
        this.init();
      }
    });
  }

  searchWorkload() {
    const query: SearchSearchRequest = new SearchSearchRequest({
      aggregate: false,
      query: {
        kinds: ['Workload'],
        fields: {
          requirements: [
            {
              key: 'spec.id.keyword',
              operator: 'equals',
              values: [this.selectedId]
            }
          ]
        }
      }
    });
    this.searchService.PostQuery(query).subscribe(
      resp => {
        const body = resp.body as any;
        const entries = body.entries || [];
        const entry = entries[0];

        if (!entry) {
          this.init();
        } else {
          const name = entry.object.meta.name;
          this._router.navigateByUrl(`workload/${name}`);
        }
      },
      (error) => {
        this._controllerService.invokeRESTErrorToaster('Failed to search workload ' + this.selectedId, error);
      },
      () => this.init()
    );
  }

  init() {
    this.initHosts = true;
    this.initNaples = true;
    this.initWorkload = true;
    this.initializeData();
    this.setWorkloadDetailToolbar(this.selectedId);
    const workloadSub = this.getWorkloadDetails();
    const hostsSub = this.getHosts();
    const naplesSub = this.getNaples();
    this.subscriptions.push(workloadSub, hostsSub, naplesSub);
  }

  initializeData() {
    this.subscriptions.forEach(sub => {
      sub.unsubscribe();
    });
    this.subscriptions = [];
    this.selectedObj = null;
  }

  getHosts() {
    return this.clusterService.ListHostCache().subscribe(
      response => {
        if (response.connIsErrorState) {
          return;
        }
        this.allHosts = response.data as ClusterHost[];
        this.initHosts = false;
        this.mapData();
      },
      () => {
        this.initHosts = false;
        this._controllerService.webSocketErrorHandler('Failed to get hosts');
      }
    );
  }

  getNaples() {
    return this.clusterService.ListDistributedServiceCardCache().subscribe(
      response => {
        if (response.connIsErrorState) {
          return;
        }
        this.allNaples = response.data as ClusterDistributedServiceCard[];
        this.initNaples = false;
        this.mapData();
      },
      () => {
        this.initNaples = false;
        this._controllerService.webSocketErrorHandler('Failed to get DSCs');
      }
    );
  }

  getWorkloadDetails() {
    return this.workloadService.WatchWorkload({ 'field-selector': 'meta.name=' + this.selectedId }).subscribe(
      response => {
        const objEventUtility = new HttpEventUtility<WorkloadWorkload>(WorkloadWorkload);
        const objList = objEventUtility.array;
        objEventUtility.processEvents(response);
        if (objList.length > 1) {
          console.error('Received ' + objList.length + ' workload objects.');
        }
        if (this.selectedObj != null && objList.length > 0) {
          this.selectedObj = new WorkloadWorkload(objList[0]);
        } else if (this.selectedObj == null && objList.length > 0) {
          // In case object was deleted and then re-added while we are on the same screen
          this.showDeletionScreen = false;
          // In case object wasn't created yet and then was added while we are on the same screen
          this.showMissingScreen = false;
          this.selectedObj = new WorkloadWorkload(objList[0]);
          this.setWorkloadDetailToolbar(this.selectedId);
        } else if (objList.length === 0) {
          // Must have received a delete event.
          this.showDeletionScreen = true;
          this.selectedObj = null;
        }
        this.initWorkload = false;
        if (this.selectedObj) {
          this.setInterfaceColumns();
          this.setLabelData();
          this.mapData();
        }
        this.loading = false;
      },
      () => {
        this.initWorkload = false;
        this.loading = false;
        this._controllerService.webSocketErrorHandler('Failed to get workloads');
      }
    );
  }

  mapData() {
    if (!this.initHosts && !this.initNaples && !this.initWorkload) {
      const host: ClusterHost = ObjectsRelationsUtility.getHostByMetaName(this.allHosts, this.selectedObj.spec['host-name']);
      this.naples = ObjectsRelationsUtility.getDSCsByHost(this.allNaples, host);
    }
  }

  setInterfaceColumns() {
    this.interfaceColumns = [];
    this.interfaceColumns = this.interfaceColumnsAll.filter(col => {
      if ((Utility.isWorkloadSystemGenerated(this.selectedObj) && col.field === 'external-vlan') ||
          (!Utility.isWorkloadSystemGenerated(this.selectedObj) && col.field === 'network')) {
            return false;
      }
      return true;
    });
  }

  setLabelData() {
    const labels = this.selectedObj.meta.labels || {};
    this.labelData = Object.keys(labels).map(key => {
      return {
        key: key,
        value: labels[key],
      };
    });
  }

  setWorkloadDetailToolbar(name: string) {
    this._controllerService.setToolbarData({
      buttons: [],
      breadcrumb: [
        { label: 'Workloads Overview', url: Utility.getBaseUIUrl() + 'workload' },
        { label: name, url: Utility.getBaseUIUrl() + 'workload/' + name }
      ]
    });
  }

  editLabels() {
    this.labelEditorMetaData = {
      title: 'Edit Workload Labels',
      keysEditable: true,
      valuesEditable: true,
      propsDeletable: true,
      extendable: true,
      save: true,
      cancel: true,
    };

    if (!this.inLabelEditMode) {
      this.inLabelEditMode = true;
    }
  }

  handleEditSave(updatedWorkloads: WorkloadWorkload[]) {
    this.bulkeditLabels(updatedWorkloads);
  }

  handleEditCancel($event) {
    this.inLabelEditMode = false;
  }

  onBulkEditSuccess(veniceObjects: any[], stagingBulkEditAction: IStagingBulkEditAction, successMsg: string, failureMsg: string) {
    this.inLabelEditMode = false;
  }

  onBulkEditFailure(error: Error, veniceObjects: any[], stagingBulkEditAction: IStagingBulkEditAction, successMsg: string, failureMsg: string, ) {}

  /**
   * Invoke changing meta.lablels using bulkedit API
   * @param updatedWorkloads
   */
  bulkeditLabels(updatedWorkloads: WorkloadWorkload[]) {
    const successMsg: string = 'Updated workload labels';
    const failureMsg: string = 'Failed to update workload labels';
    const stagingBulkEditAction = this.buildBulkEditLabelsPayload(updatedWorkloads);
    this.bulkEditHelper(updatedWorkloads, stagingBulkEditAction, successMsg, failureMsg );
  }

  clearSelectedDataObjects() {}

  getSelectedDataObjects() {
    return this.selectedObj ? [this.selectedObj] : [];
  }

  ngOnDestroy() {
    this._controllerService.publish(Eventtypes.COMPONENT_DESTROY, { 'component': 'WorkloaddetailComponent', 'state': Eventtypes.COMPONENT_DESTROY });
    this.subscriptions.forEach(subscription => {
      subscription.unsubscribe();
    });
  }
}
