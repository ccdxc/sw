import { ChangeDetectorRef, Component, OnDestroy, OnInit, ViewEncapsulation } from '@angular/core';
import { Animations } from '@app/animations';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { Utility } from '@app/common/Utility';
import { CustomExportMap, TableCol } from '@app/components/shared/tableviewedit';
import { TablevieweditAbstract } from '@app/components/shared/tableviewedit/tableviewedit.component';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { ControllerService } from '@app/services/controller.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { ClusterDSCProfile, IApiStatus, IClusterDSCProfile, ClusterDistributedServiceCard, ClusterDistributedServiceCardStatus_admission_phase } from '@sdk/v1/models/generated/cluster';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';
import { Observable } from 'rxjs';
import { ActivatedRoute } from '@angular/router';
import { ObjectsRelationsUtility } from '@app/common/ObjectsRelationsUtility';
import { SelectItem } from 'primeng/api';


interface DSCMacName {
  mac: string;
  name: string;
}
interface DSCProfileUiModel {
  associatedDSCS: ClusterDistributedServiceCard[];
  associatedDSCSPercentile: Number;
  pendingDSCNames?: DSCMacName[];
  featureset: string;
  description: string;
}

/**
 * This DscprofilesComponent is for DSC unified mode feature.
 * A DSC-profile contains many DSC features.
 * User can configure a profile by pick and choose features.  Features are orgainzied in groups.  UI will validate feature combination.
 *
 * User can assign DSC-Profiles to DSCs.
 * If a profile is associated with DSCs, UI will controls whether this profile can be updated or deleted.
 *
 *  TODO: 2020-02-28
 *  Link DSC to profiles. (once backend updates dscprofile.proto and dsc-profile populated with DSC data)
 */


@Component({
  selector: 'app-dscprofiles',
  templateUrl: './dscprofiles.component.html',
  styleUrls: ['./dscprofiles.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class DscprofilesComponent extends TablevieweditAbstract<IClusterDSCProfile, ClusterDSCProfile> implements OnInit, OnDestroy {
  dataObjects: ReadonlyArray<ClusterDSCProfile> = [];
  selectedDSCProfile: ClusterDSCProfile = null;

  naplesList: ClusterDistributedServiceCard[] = [];

  isTabComponent: boolean = false;
  disableTableWhenRowExpanded: boolean = true; // this make toolbar button disabled when record is in edit-mode.
  exportFilename: string = 'PSM-DSC-Profiles';
  exportMap: CustomExportMap = {};
  tableLoading: boolean = false;

  dscprofilesEventUtility: HttpEventUtility<ClusterDSCProfile>;

  maxDSCsPerRow: number = 8;

  bodyicon: any = {
    margin: {
      top: '9px',
      left: '8px',
    },
    url: '/assets/images/icons/cluster/profiles/dsc-profile-black.svg',
  };

  headerIcon: Icon = {
    margin: {
      top: '0px',
      left: '10px',
    },
    matIcon: 'grid_on'
  };

  cols: TableCol[] = [
    { field: 'meta.name', header: 'Name', class: 'dscprofiles-column-dscprofile-name', sortable: true, width: 15 },
    { field: 'DSCs', header: 'DSCs', class: 'dscprofiles-column-dscs', sortable: false, width: 30 },
    { field: 'utilization', header: 'Utilization', class: 'dscprofiles-column-utilization', sortable: false, width: 10 },
    // comment these two columns out for now 2020-05-01
    // { field: 'spec.feature-set', header: 'Feature Set', class: 'dscprofiles-column-feature-set', sortable: true, width: 20 },
    // { field: 'status.propagation-status.updated', header: 'Update DSC', class: 'dscprofiles-column-status-updated', sortable: true, width: 5 },
    { field: 'Feature', header: 'Feature Set', class: 'dscprofiles-column-feature-set', sortable: true, width: 20 },
    { field: 'Propagation', header: 'Propagation Status', class: 'dscprofiles-column-propagation-status', sortable: true, width: 20 },
    { field: 'status.propagation-status.pending-dscs', header: 'Pending DSC', class: 'dscprofiles-column-status-pendig', sortable: true, width: 20 },
    { field: 'meta.mod-time', header: 'Modification Time', class: 'dscprofiles-column-date', sortable: true, width: '180px' },
    { field: 'meta.creation-time', header: 'Creation Time', class: 'dscprofiles-column-date', sortable: true, width: '180px' },
  ];


  macToNameMap: { [key: string]: string } = {};
  viewPendingNaples: boolean;

  options: SelectItem[] = [
    { label: 'HPF', value: { InterVMServices: false,  FlowAware : false, Firewall: false, description: 'The High-Performance Forwarding (HPF) Feature Set enables a higher number of connections per second to be established through the card. The only feature supported, besides the ones of a typical NIC is interface-based ERSPAN.' } },
    { label: 'Flow Aware', value: { InterVMServices: false,  FlowAware : true, Firewall: false, description: 'The Flow Aware Feature Set includes features that require a DSC to keep track of individual flows. Examples include flow-based ERSPAN, flow statistics, NetFlow/IPFIX.' } },
    { label: 'Firewall', value: { InterVMServices: false,  FlowAware : true, Firewall: true, description: 'The Firewall Feature Set includes the capability of enforcing security policies, where a security policy specifies flows whose packets shall be forwarded or dropped by DSCs. The Firewall Feature Set includes also all the features of the Flow Aware feature set.' } },
    { label: 'Inter-VM Services', value: { InterVMServices: true,  FlowAware : true, Firewall: true, description: 'The Inter-VM Services Feature Set includes features of the Flow Aware and Firewall Features Set applied also to traffic among workloads (e.g., Virtual Machines) in execution on the same host. As an example, this Feature Set is required when implementing micro-segmentation.'}}
  ];

  constructor(protected _controllerService: ControllerService,
    protected uiconfigsService: UIConfigsService,
    protected cdr: ChangeDetectorRef,
    private clusterService: ClusterService,
    private _route: ActivatedRoute
  ) {
    super(_controllerService, cdr, uiconfigsService);
  }

  setDefaultToolbar() {
    let buttons = [];
    if (this.uiconfigsService.isAuthorized(UIRolePermissions.clusterdscprofile_create)) {
      buttons = [
        {
          cssClass: 'global-button-primary  dscprofiles-button ',
          text: 'ADD DSC PROFILE',
          computeClass: () => this.shouldEnableButtons ? '' : 'global-button-disabled',
          callback: () => { this.createNewObject(); }
        }
      ];
    }
    this._controllerService.setToolbarData({
      buttons: buttons,
      breadcrumb: [{ label: 'DSC Profiles', url: Utility.getBaseUIUrl() + 'cluster/dscprofiles' }]
    });
  }
  postNgInit(): void {
    this._route.queryParams.subscribe(params => {
      if (params.hasOwnProperty('dscprofile')) {
        // alerttab selected
        this.getSearchedNetworkInterface(params['dscprofile']);
      }
    });
    this.watchDSCProfiles();
    this.watchNaples();
  }

  getSearchedNetworkInterface(interfacename) {
    const subscription = this.clusterService.GetDSCProfile(interfacename).subscribe(
      response => {
        const networkinterface = response.body as ClusterDSCProfile;
        this.selectedDSCProfile = new ClusterDSCProfile(networkinterface);
        this.updateSelectedDSCProfile();
      },
      this._controllerService.webSocketErrorHandler('Failed to get Network interface ' + interfacename)
    );
    this.subscriptions.push(subscription); // add subscription to list, so that it will be cleaned up when component is destroyed.
  }

  updateSelectedDSCProfile() {
    this.processOneDSCProfile(this.selectedDSCProfile );
  }

  /**
   * Fetch DSC Profiles records
   */
  watchDSCProfiles() {
    const subscription = this.clusterService.ListDSCProfileCache().subscribe(
      response => {
        if (response.connIsErrorState) {
          return;
        }
        this.dataObjects  = response.data;
        this.handleDataReady(); // process DSCProfile. Note: At this this moment, this.selectedObj may not be available
        if (this.selectedDSCProfile) {
          this.selectedDSCProfile = null;  // clear out detail panel
        }
      },
      this._controllerService.webSocketErrorHandler('Failed to get DSC Profile')
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
        for (let j = 0; j < this.naplesList.length; j++) {
          const dsc: ClusterDistributedServiceCard = this.naplesList[j];
          this.macToNameMap[dsc.meta.name] = dsc.spec.id;
        }
        this.handleDataReady();
      }
    );
    this.subscriptions.push(dscSubscription);
  }

  handleDataReady() {
    // When naplesList and dataObjects (dscprofiles) are ready, build profile-dscs
    if (this.naplesList && this.dataObjects) {
      for (let i = 0; i < this.dataObjects.length; i++) {
        const dscProfile: ClusterDSCProfile = this.dataObjects[i];
        this.processOneDSCProfile(dscProfile);
      }
      this.updateSelectedDSCProfile();
      this.dataObjects = Utility.getLodash().cloneDeep(this.dataObjects);
    }
  }

  private processOneDSCProfile( dscProfile: ClusterDSCProfile ) {
    if (! dscProfile) {
      return;
    }
    const dscsnames = [];
    for (let j = 0; j < this.naplesList.length; j++) {
      const dsc: ClusterDistributedServiceCard = this.naplesList[j];
      if (dscProfile.meta.name === dsc.spec.dscprofile && dsc.status['admission-phase'] === ClusterDistributedServiceCardStatus_admission_phase.admitted) {
        dscsnames.push(dsc);
      }
    }
    const pendingDSCNames: DSCMacName[] = [];
    for (let k = 0; dscProfile.status['propagation-status']['pending-dscs'] && k < dscProfile.status['propagation-status']['pending-dscs'].length; k++) {
      const mac = dscProfile.status['propagation-status']['pending-dscs'][k];
      const name = this.macToNameMap[mac];
      pendingDSCNames.push({ mac: mac, name: name });
    }
    // this if block is temporary, once DSCProfile.proto bug fix, we don't need it.
    if (!dscProfile._ui) {
      dscProfile._ui = {};
    }
    const dscProfileUiModel: DSCProfileUiModel = {
      associatedDSCS: dscsnames,
      associatedDSCSPercentile: (dscsnames.length / this.naplesList.length),
      pendingDSCNames: pendingDSCNames,
      featureset: this.getFeatureName(dscProfile),
      description: this.getFeatureDescription(dscProfile)
    };
    dscProfile._ui = dscProfileUiModel;
  }
  getFeatureDescription(dscProfile: ClusterDSCProfile): string {
    const item = this.getFeaturesetHelper(dscProfile);
    return (item) ? item.value.description : '';
  }

  getFeatureName(dscProfile: ClusterDSCProfile): string {
    const item = this.getFeaturesetHelper(dscProfile);
    return (item) ? item.label : '';
  }

  getFeaturesetHelper(dscProfile: ClusterDSCProfile): SelectItem {
    const theOne = this.options.find((item: SelectItem) => {
      const keys  = this.getObjectKeys(dscProfile.spec['feature-set']);
      let matched = true;
      for (let i = 0; i < keys.length; i ++) {
        const key  = keys[i];
        const modelValue = (dscProfile.spec['feature-set'][key]) ? dscProfile.spec['feature-set'][key] : false;
        if (item.value[key] !== modelValue) {
          matched = false;
          break;
        }
      }
      return matched;
    });
    return theOne;
  }

  getNaplesName(mac: string): string {
    return this.macToNameMap[mac];
  }

  getClassName(): string {
    return this.constructor.name;
  }

  deleteRecord(object: ClusterDSCProfile): Observable<{ body: IClusterDSCProfile | IApiStatus | Error, statusCode: number; }> {
    return this.clusterService.DeleteDSCProfile(object.meta.name);
  }

  generateDeleteConfirmMsg(object: ClusterDSCProfile): string {
    return 'Are you sure you want to delete DSC Profile ' + object.meta.name;
  }

  generateDeleteSuccessMsg(object: ClusterDSCProfile): string {
    if ( this.selectedDSCProfile  &&  this.selectedDSCProfile.meta.name === object.meta.name ) {
      this.selectedDSCProfile = null;
    }
    return 'Deleted DSC Profile ' + object.meta.name;
  }

  areSelectedRowsDeletable(): boolean {
    if (!this.uiconfigsService.isAuthorized(UIRolePermissions.networknetworkinterface_delete)) {
      return false;
    }
    const selectedRows = this.getSelectedDataObjects();
    if (selectedRows.length === 0) {
      return false;
    }
    const list = selectedRows.filter((rowData: ClusterDSCProfile) => {
      const uiData: DSCProfileUiModel = rowData._ui as DSCProfileUiModel;
      return uiData.associatedDSCS && uiData.associatedDSCS.length > 0;
    }
    );
    return (list.length === 0);
  }

  /**
   * This API serves HTML template. When there are many DSCs in one profile, we don't list all DSCs. This API builds the tooltip text;
   * @param dscprofile
   */
  buildMoreDSCsTooltip(dscprofile: ClusterDSCProfile): string {
    const dscTips = [];
    const uiData: DSCProfileUiModel = dscprofile._ui as DSCProfileUiModel;
    const dscs = uiData.associatedDSCS;
    for (let i = 0; i < dscs.length; i++) {
      if (i >= this.maxDSCsPerRow) {
        const dsc = dscs[i];
        if (i <= 2 * this.maxDSCsPerRow) {
          // We don't want to lood too much records to tooltip. Just load another maxDSCsPerRow. Say maxDSCsPerRow=10, we list first 10 records in table. Tooltip text contains 11-20th records
          dscTips.push(dsc.meta.name);
        }
      }
    }
    return dscTips.join(' , ') + (dscs.length > 2 * this.maxDSCsPerRow) + ' ... more';
  }

  buildMorePendingDSCsTooltip(dscprofile: ClusterDSCProfile): string {
    const dscTips = [];
    const uiData: DSCProfileUiModel = dscprofile._ui as DSCProfileUiModel;
    const macname = uiData.pendingDSCNames;
    for (let i = 0; i < macname.length; i++) {
      if (i >= this.maxDSCsPerRow) {
        const dsc = macname[i];
        if (i <= 2 * this.maxDSCsPerRow) {
          // We don't want to lood too much records to tooltip. Just load another maxDSCsPerRow. Say maxDSCsPerRow=10, we list first 10 records in table. Tooltip text contains 11-20th records
          dscTips.push(dsc.name);
        }
      }
    }
    return dscTips.join(' , ') + (macname.length > 2 * this.maxDSCsPerRow) + ' ... more';
  }

  /**
   * This API serves html template.
   * @param dscProfile:ClusterDSCProfile
   */
  showDeleteButton(dscProfile: ClusterDSCProfile): boolean {
    // If dscProfile has associated DSC, we can not delete this dscProfile
    const uiData: DSCProfileUiModel = dscProfile._ui as DSCProfileUiModel;
    return (uiData && uiData.associatedDSCS && uiData.associatedDSCS.length > 0) ? false : true;
  }

  selectDSCProfile(event) {
    const $ = Utility.getJQuery();
    if (this.selectedDSCProfile && event.rowData === this.selectedDSCProfile) {
      this.selectedDSCProfile = null;
      if (event.event && event.event.currentTarget) {
        $(event.event.currentTarget).removeClass('dscprofiles-selected-row ');
        }
    } else {
      this.selectedDSCProfile = event.rowData;
      if (event.event && event.event.currentTarget) {
      if ($('.dscprofiles-selected-row')) {
        $('.dscprofiles-selected-row').removeClass('dscprofiles-selected-row ');
      }
      $(event.event.currentTarget).addClass('dscprofiles-selected-row');
      }
    }
  }

  closeDetails() {
    this.selectedDSCProfile = null;
  }

  viewPendingNaplesList() {
    this.viewPendingNaples = !this.viewPendingNaples;
  }

}
