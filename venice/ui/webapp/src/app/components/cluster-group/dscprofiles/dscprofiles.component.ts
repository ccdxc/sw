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
import { ClusterDSCProfile, IApiStatus, IClusterDSCProfile, ClusterDistributedServiceCard } from '@sdk/v1/models/generated/cluster';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';
import { Observable } from 'rxjs';
import { SortEvent } from 'primeng/components/common/api';

interface DSCProfileUiModel {
  associatedDSCS: ClusterDistributedServiceCard[];
  associatedDSCSPercentile: Number;
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

  naplesList: ClusterDistributedServiceCard[] = [];

  isTabComponent: boolean;
  disableTableWhenRowExpanded: boolean;
  exportFilename: string = 'Venice-DSC-Profiles';
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
    { field: 'spec.dscs', header: 'Associated DSCs', class: 'dscprofiles-column-dscs', sortable: false, width: 25 },
    { field: 'utilization', header: 'Utilization', class: 'dscprofiles-column-utilization', sortable: false, width: 10 },
    { field: 'spec.fwd-mode', header: 'FWD Mode', class: 'dscprofiles-column-fwd-mode', sortable: true, width: 10 },
    { field: 'spec.policy-mode', header: 'Policy Mode', class: 'dscprofiles-column-policy-mode', sortable: true, width: 10 },
    { field: 'meta.mod-time', header: 'Modification Time', class: 'dscprofiles-column-date', sortable: true, width: '180px' },
    { field: 'meta.creation-time', header: 'Creation Time', class: 'dscprofiles-column-date', sortable: true, width: '180px' },
  ];

  constructor(protected _controllerService: ControllerService,
    protected uiconfigsService: UIConfigsService,
    protected cdr: ChangeDetectorRef,
    private clusterService: ClusterService,
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
    this.watchDSCProfiles();
    this.watchNaples();
  }

  /**
   * Fetch DSC Profiles records
   */
  watchDSCProfiles() {
    this.dscprofilesEventUtility = new HttpEventUtility<ClusterDSCProfile>(ClusterDSCProfile, false, null, true); // https://pensando.atlassian.net/browse/VS-93 we want to trim the object
    this.dataObjects = this.dscprofilesEventUtility.array;
    const subscription = this.clusterService.WatchDSCProfile().subscribe(
      response => {
        this.dscprofilesEventUtility.processEvents(response);
        this.handleDataReady();
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
        const dscsnames = [];
        for (let j = 0; j < this.naplesList.length; j++) {
          const dsc: ClusterDistributedServiceCard = this.naplesList[j];
          if (dscProfile.meta.name === dsc.spec.dscprofile) {
            dscsnames.push(dsc);
          }
        }
        // this if block is temporary, once DSCProfile.proto bug fix, we don't need it.
        if (!dscProfile._ui) {
          dscProfile._ui = {};
        }
        const dscProfileUiModel: DSCProfileUiModel = {
          associatedDSCS: dscsnames,
          associatedDSCSPercentile: (dscsnames.length / this.naplesList.length)
        };
        dscProfile._ui = dscProfileUiModel;
      }
    }
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
    return 'Deleted DSC Profile ' + object.meta.name;
  }

  areSelectedRowsDeletable(): boolean {
    if (!this.uiconfigsService.isAuthorized(UIRolePermissions.networknetworkinterface_update)) {
      return false;
    }
    const selectedRows = this.getSelectedDataObjects();
    if (selectedRows.length === 0) {
      return false;
    }
    return true;
  }

  /**
   * This API serves HTML template. When there are many DSCs in one profile, we don't list all DSCs. This API builds the tooltip text;
   * @param dsc
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

  /**
   * This API serves html template.
   * @param dscProfile:ClusterDSCProfile
   */
  showDeleteButton(dscProfile: ClusterDSCProfile): boolean {
    // If dscProfile has associated DSC, we can not delete this dscProfile
    const uiData: DSCProfileUiModel = dscProfile._ui as DSCProfileUiModel;
    return (uiData && uiData.associatedDSCS && uiData.associatedDSCS.length > 0) ? false : true;
  }

}
