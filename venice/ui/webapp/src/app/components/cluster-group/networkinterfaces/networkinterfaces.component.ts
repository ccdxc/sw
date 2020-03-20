import { ChangeDetectorRef, Component, ViewEncapsulation, OnInit, OnDestroy, ViewChild } from '@angular/core';
import { Animations } from '@app/animations';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { Utility } from '@app/common/Utility';
import { TablevieweditAbstract } from '@app/components/shared/tableviewedit/tableviewedit.component';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { ControllerService } from '@app/services/controller.service';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { NetworkNetworkInterface, INetworkNetworkInterface, INetworkNetworkInterfaceList, IApiStatus } from '@sdk/v1/models/generated/network';
import { ClusterDistributedServiceCard, ClusterDistributedServiceCardStatus_admission_phase_uihint, IClusterDistributedServiceCard } from '@sdk/v1/models/generated/cluster';
import { Observable, forkJoin } from 'rxjs';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { ActivatedRoute } from '@angular/router';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';
import { TableCol, RowClickEvent, CustomExportMap } from '@app/components/shared/tableviewedit';
import { TableUtility } from '@app/components/shared/tableviewedit/tableutility';
import { ClusterService } from '@app/services/generated/cluster.service';
import { NetworkService } from '@app/services/generated/network.service';
import { BrowserService } from '@app/services/generated/browser.service';
import { LabelEditorMetadataModel } from '@app/components/shared/labeleditor';
import { ObjectsRelationsUtility, DSCsNameMacMap } from '@app/common/ObjectsRelationsUtility';
import { PrettyDatePipe } from '@app/components/shared/Pipes/PrettyDate.pipe';
import { FormArray } from '@angular/forms';
import { AdvancedSearchComponent } from '@app/components/shared/advanced-search/advanced-search.component';
import { SearchUtil } from '@app/components/search/SearchUtil';
import { FieldsRequirement } from '@sdk/v1/models/generated/search';
import * as _ from 'lodash';

/**
 * NetworkinterfacesComponent is linked to DSC object.
 * Each DSC has logical interfaces (Lif) and physical interfaces (up-links)
 *
 * Network interface objects are created by backend. User can manipulate labels (CRUD)
 *
 * Metrics should be on DSC.networkinterface.
 *
 * ERSPAN (Mirror Session) will use DSC.networkinterface labels
 *
 * TODO: 2020-02-27
 *  1. Scale test,  1000 DSCs will generate 16000 network interfaces.
 *  2. Add advance search
 *
 */

@Component({
  selector: 'app-networkinterfaces',
  templateUrl: './networkinterfaces.component.html',
  styleUrls: ['./networkinterfaces.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class NetworkinterfacesComponent extends TablevieweditAbstract<INetworkNetworkInterface, NetworkNetworkInterface> implements OnInit, OnDestroy {

  public static NETWORKINTERFACE_FIELD_DSC: string = 'associatedDSC';

  @ViewChild('advancedSearchComponent') advancedSearchComponent: AdvancedSearchComponent;

  maxSearchRecords: number = 8000;

  dataObjects: ReadonlyArray<NetworkNetworkInterface>;
  dataObjectsBackUp: ReadonlyArray<NetworkNetworkInterface> = null;
  naplesList: ClusterDistributedServiceCard[] = [];

  techsupportrequestsEventUtility: HttpEventUtility<NetworkNetworkInterface>;

  labelEditorMetaData: LabelEditorMetadataModel;

  bodyicon: any = {
    margin: {
      top: '9px',
      left: '8px',
    },
    url: '/assets/images/icons/cluster/networkinterfaces/dsc-interface-black.svg',
  };
  headerIcon: Icon = {
    margin: {
      top: '0px',
      left: '10px',
    },
    matIcon: 'grid_on'
  };

  cols: TableCol[] = [
    { field: 'meta.name', header: 'Name', class: 'networkinterfaces-column-name', sortable: true, width: 15, notReorderable: true },
    { field: 'status.dsc', header: 'DSC', class: ' networkinterfaces-column-dsc', sortable: true, width: 15 },
    { field: 'meta.creation-time', header: 'Creation Time', class: 'networkinterfaces-column-date', sortable: true, width: '180px', notReorderable: true },
    { field: 'spec.admin-status', header: 'Admin Status', class: ' networkinterfaces-column-admintype', sortable: false, width: 45 },
    { field: 'spec.type', header: 'Type', class: ' networkinterfaces-column-type', sortable: true, width: 15 },
    { field: 'meta.labels', header: 'Labels', class: '', sortable: true, width: 7 },
    { field: 'status.oper-status', header: 'OP Status', class: ' networkinterfaces-column-opstatus', sortable: true, width: 15 }
  ];

  exportFilename: string = 'Venice-networkinterfaces';
  exportMap: CustomExportMap = {};

  isTabComponent = false;
  disableTableWhenRowExpanded = true;
  tableLoading: boolean = false;
  inLabelEditMode: boolean = false;

  selectedNetworkInterface: NetworkNetworkInterface = null;

  _myDSCnameToMacMap: DSCsNameMacMap;

  // advance search variables
  advSearchCols: TableCol[] = [];
  fieldFormArray = new FormArray([]);


  constructor(protected controllerService: ControllerService,
    protected clusterService: ClusterService,
    protected uiConfigsService: UIConfigsService,
    protected networkService: NetworkService,
    protected cdr: ChangeDetectorRef,
    protected browserService: BrowserService,
    private _route: ActivatedRoute
  ) {
    super(controllerService, cdr, uiConfigsService);
  }

  /**
  * Overide super's API
  * It will return this Component name
  */
  getClassName(): string {
    return this.constructor.name;
  }

  postNgInit() {
    this._route.queryParams.subscribe(params => {
      if (params.hasOwnProperty('interface')) {
        // alerttab selected
        this.getSearchedNetworkInterface(params['interface']);
      }
    });
    this.watchNetworkInterfaces();
    this.watchNaples();
    this.buildAdvSearchCols();
  }

  setDefaultToolbar() {
    const buttons = [];
    this.controllerService.setToolbarData({
      buttons: buttons,
      breadcrumb: [{ label: 'Network Interfaces', url: Utility.getBaseUIUrl() + '/cluster/networkinterfaces' }]
    });
  }

  getSearchedNetworkInterface(interfacename) {
    const subscription = this.networkService.GetNetworkInterface(interfacename).subscribe(
      response => {
        const networkinterface = response.body as NetworkNetworkInterface;
        this.selectedNetworkInterface = new NetworkNetworkInterface(networkinterface);
        this.updateSelectedNetworkInterface();
      },
      this._controllerService.webSocketErrorHandler('Failed to get Network interface ' + interfacename)
    );
    this.subscriptions.push(subscription); // add subscription to list, so that it will be cleaned up when component is destroyed.
  }

  watchNaples() {
    const dscSubscription = this.clusterService.ListDistributedServiceCardCache().subscribe(
      (response) => {
        if (response.connIsErrorState) {
          return;
        }
        this.naplesList = response.data as ClusterDistributedServiceCard[];
        this.handleDataReady();
      }
    );
    this.subscriptions.push(dscSubscription);
  }

  watchNetworkInterfaces() {
    const dscSubscription = this.networkService.ListNetworkInterfacesCache().subscribe(
      (response) => {
        if (response.connIsErrorState) {
          return;
        }
        this.dataObjects = response.data;
        this.handleDataReady();
        this.dataObjectsBackUp = Utility.getLodash().cloneDeepWith(this.dataObjects); // make a copy of server provided data
      }
    );
    this.subscriptions.push(dscSubscription);
  }

  handleDataReady() {
    // When naplesList and networkinterfaces list are ready, build networkinterface-dsc map.
    if (this.naplesList && this.dataObjects) {
      this._myDSCnameToMacMap = ObjectsRelationsUtility.buildDSCsNameMacMap(this.naplesList);
      this.dataObjects.forEach((networkNetworkInterface: NetworkNetworkInterface) => {
        const dscname = this._myDSCnameToMacMap.macToNameMap[networkNetworkInterface.status.dsc];
        networkNetworkInterface._ui[NetworkinterfacesComponent.NETWORKINTERFACE_FIELD_DSC] = (dscname) ? dscname : networkNetworkInterface.status.dsc;
      });
      this.updateSelectedNetworkInterface();
    }
  }

  updateSelectedNetworkInterface() {
    if (this.selectedNetworkInterface && this._myDSCnameToMacMap) {
      const dscname = this._myDSCnameToMacMap.macToNameMap[this.selectedNetworkInterface.status.dsc];
      this.selectedNetworkInterface._ui[NetworkinterfacesComponent.NETWORKINTERFACE_FIELD_DSC] = (dscname) ? dscname : this.selectedNetworkInterface.status.dsc;
    }
  }

  showDeleteIcon(): boolean {
    return true;
  }

  generateDeleteConfirmMsg(object: INetworkNetworkInterface) {
    return 'Are you sure to delete network interface: ' + object.meta.name;
  }

  generateDeleteSuccessMsg(object: INetworkNetworkInterface) {
    return 'Deleted network interface ' + object.meta.name;
  }

  deleteRecord(object: NetworkNetworkInterface): Observable<{ body: INetworkNetworkInterface | IApiStatus | Error; statusCode: number; }> {
    throw new Error('Method not supported.');
  }

  editLabels() {
    this.labelEditorMetaData = {
      title: 'Editing network interface objects',
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

  handleEditSave(networkinterfaces: NetworkNetworkInterface[]) {
    this.updateWithForkjoin(networkinterfaces);
  }

  handleEditCancel($event) {
    this.inLabelEditMode = false;
  }

  updateWithForkjoin(networkinterfaces: NetworkNetworkInterface[]) {
    const observables: Observable<any>[] = [];
    for (const networkinterfaceObj of networkinterfaces) {
      const name = networkinterfaceObj.meta.name;
      const sub = this.networkService.UpdateNetworkInterface(name, networkinterfaceObj);
      observables.push(sub);
    }

    const summary = 'Network Interface update';
    const objectType = 'Network Interface';
    this.handleForkJoin(observables, summary, objectType);
  }

  private handleForkJoin(observables: Observable<any>[], summary: string, objectType: string) {
    forkJoin(observables).subscribe((results: any[]) => {
      let successCount: number = 0;
      let failCount: number = 0;
      const errors: string[] = [];
      for (let i = 0; i < results.length; i++) {
        if (results[i]['statusCode'] === 200) {
          successCount += 1;
        } else {
          failCount += 1;
          errors.push(results[i].body.message);
        }
      }
      if (successCount > 0) {
        const msg = 'Successfully updated ' + successCount.toString() + ' ' + objectType + '.';
        this._controllerService.invokeSuccessToaster(summary, msg);
        this.inLabelEditMode = false;
      }
      if (failCount > 0) {
        this._controllerService.invokeRESTErrorToaster(summary, errors.join('\n'));
      }
    },
      this._controllerService.restErrorHandler(summary + ' Failed'));
  }

  selectNetworkInterface(event) {
    if (this.selectedNetworkInterface && event.rowData === this.selectedNetworkInterface) {
      this.selectedNetworkInterface = null;
    } else {
      this.selectedNetworkInterface = event.rowData;
    }
  }

  closeDetails() {
    this.selectedNetworkInterface = null;
  }

  // advance search
  buildAdvSearchCols() {
    this.advSearchCols = this.cols.filter((col: TableCol) => {
      return (col.field !== 'meta.creation-time' && col.field !== 'status.dsc');
    });
    this.advSearchCols.push(
      {
        field: 'DSC', header: 'DSC Name', localSearch: true, kind: 'DistributedServiceCard',
        filterfunction: this.searchDSC,
        advancedSearchOperator: SearchUtil.stringOperators
      }
    );
  }

  // advance search APIs
  onCancelSearch($event) {
    this.controllerService.invokeInfoToaster('Information', 'Cleared search criteria, Table refreshed.');
    this.dataObjects = this.dataObjectsBackUp;
  }

   /**
   * Execute table search
   * @param field
   * @param order
   */
  onSearchWorkloads(field = this.tableContainer.sortField, order = this.tableContainer.sortOrder) {
    const searchResults = this.onSearchDataObjects(field, order, 'NetworkInterface', this.maxSearchRecords, this.advSearchCols, this.dataObjectsBackUp, this.advancedSearchComponent);
    if (searchResults && searchResults.length > 0) {
      this.dataObjects = [];
      this.dataObjects = searchResults;
    }
  }

  searchDSC(requirement: FieldsRequirement, data = this.dataObjects): any[] {
    const outputs: any[] = [];
    for (let i = 0; data && i < data.length; i++) {
      const recordValue  = data[i]._ui.associatedDSC; //  data[i]._ui.associatedDSC is the dsc name
        const searchValues = requirement.values;
        let operator = String(requirement.operator);
        operator = TableUtility.convertOperator(operator);
        for (let j = 0; j < searchValues.length; j++) {
          const activateFunc = TableUtility.filterConstraints[operator];
          if (activateFunc && activateFunc(recordValue, searchValues[j])) {
            outputs.push(data[i]);
          }
        }
    }
    return outputs;
  }

}
