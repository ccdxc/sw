
import { Component, OnDestroy, OnInit, ViewEncapsulation } from '@angular/core';
import { ActivatedRoute, Router } from '@angular/router';
import { Animations } from '@app/animations';

import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { BaseComponent } from '@components/base/base.component';
import { ControllerService } from '@app/services/controller.service';
import { RolloutService } from '@app/services/generated/rollout.service';
import { ClusterService } from '@app/services/generated/cluster.service';
import { ClusterSmartNIC } from '@sdk/v1/models/generated/cluster';

import { Utility } from '@common/Utility';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { RolloutRollout, RolloutRolloutStatus_state } from '@sdk/v1/models/generated/rollout';
import { ToolbarData } from '@app/models/frontend/shared/toolbar.interface';
import { RolloutUtil } from '../RolloutUtil';
import { EnumRolloutOptions } from '../';
import { TableCol } from '@app/components/shared/tableviewedit';
import { UIRolePermissions } from '@sdk/v1/models/generated/UI-permissions-enum';

/**
 * This component let user monitor a rollout status.
 * User can STOP an in-progress Rollout
 */
@Component({
  selector: 'app-rolloutstatus',
  templateUrl: './rolloutstatus.component.html',
  styleUrls: ['./rolloutstatus.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None,
})
export class RolloutstatusComponent extends BaseComponent implements OnInit, OnDestroy {
  public static STOP_BUTTON_TEXT  = 'STOP ROLLOUT';

  subscriptions = [];
  selectedRolloutId: string;
  selectedRollout: RolloutRollout;
  selectedRolloutNicNodeTypes: string = RolloutUtil.ROLLOUTTYPE_BOTH_NAPLES_VENICE;

  // Holds all policy objects
  rollouts: ReadonlyArray<RolloutRollout>;
  rolloutsEventUtility: HttpEventUtility<RolloutRollout>;

  // Whether we show a deletion overlay
  showDeletionScreen: boolean;

  // Whether we show a missing overlay
  showMissingScreen: boolean;

  loading: boolean = false;
  tabIndex: number = 0;

  bodyicon: any = {
    margin: {
      top: '9px',
      left: '8px'
    },
    matIcon: 'update'
  };

  statusCols: TableCol[] = [
    { field: 'name', header: 'Name', class: 'rolloutstatus-column rolloutstatus-column-name', sortable: true, width: 15 },
    { field: 'phase', header: 'Phase', class: 'rolloutstatus-column rolloutstatus-column-phase', sortable: true, width: 15 },
    { field: 'start-time', header: 'Start Time', class: 'rolloutstatus-column rolloutstatus-column-sdate', sortable: true, width: 15 },
    { field: 'end-time', header: 'End Time', class: 'rolloutstatus-column rolloutstatus-column-edate', sortable: true, width: 15 },
  /*  VS-355 comment out reason field for now
   { field: 'reason', header: 'Previous Phase', class: 'rolloutstatus-column rolloutstatus-column-reason', sortable: true, width: 20 }, // VS-299
   */
  { field: 'message', header: 'Message', class: 'rolloutstatus-column rolloutstatus-column-message', sortable: false, width: 40 }
  ];

  naples: ReadonlyArray<ClusterSmartNIC> = [];
// During progressing rollout, WatchSmartNIC may fail resulting in this.naples to be set to empty.
// This affects the "name" column of the NICs table (see getNICID func).
// Hence we keep the last valid copy of naples as naplesCopy, which is used to render NIC names.
  naplesCopy: ReadonlyArray<ClusterSmartNIC> = [];

  // Used for processing the stream events
  naplesEventUtility: HttpEventUtility<ClusterSmartNIC>;

  labelSelectorsStrings: string[] = null;

  constructor(protected _controllerService: ControllerService,
    private _route: ActivatedRoute, protected UIConfigService: UIConfigsService,
    protected rolloutService: RolloutService, private clusterService: ClusterService,
    protected router: Router) {
    super(_controllerService, UIConfigService);
  }
  /**
   * Component is about to instantiate
   */
  ngOnInit() {
    this._controllerService.publish(Eventtypes.COMPONENT_INIT, { 'component': 'RolloutstatusComponent', 'state': Eventtypes.COMPONENT_INIT });
    this._route.paramMap.subscribe(params => {
      let id = '';
      if (this.router.url === '/maintenance') {
        id = Utility.getInstance().getCurrentRollout().meta.name;
        this.selectedRolloutId = id;
      } else {
        this.selectedRolloutId = params.get('id');
      }
      this.getRolloutDetail();
      this.showDeletionScreen = false;
      this.showMissingScreen = false;
      this.setDefaultToolbar(this.selectedRolloutId);
    });
    this.getNaples();
  }



  setDefaultToolbar(id: string) {
    const toolbarData: ToolbarData = {
      breadcrumb: [
        { label: 'System Upgrade', url: Utility.getBaseUIUrl() + 'admin/upgrade/rollouts' },
        { label: id, url: Utility.getBaseUIUrl() + 'admin/upgrade/rollouts/' + id }],
      buttons: []
    };
    this._controllerService.setToolbarData(toolbarData);
  }

  addToolbarButton() {
    const toolbarData: ToolbarData = this._controllerService.getToolbarData();
    if (this.selectedRollout && RolloutUtil.isRolloutPending(this.selectedRollout) ) {
      if (!this.hasStopButtonAlready(toolbarData) && this.uiconfigsService.isAuthorized(UIRolePermissions.rolloutrollout_delete)) {  // VS-328.  We just want to add stop-button once.
        toolbarData.buttons.push(
          {
            cssClass: 'global-button-primary rolloutstatus-toolbar-button',
            text: RolloutstatusComponent.STOP_BUTTON_TEXT,
            callback: () => {
              this.onStopRollout();
            }
          }
        );
      }
      this._controllerService.setToolbarData(toolbarData);
    }
  }

  /**
   * This function check whethere to add [STOP ROLLOUT] button in toolbar
   */
  private hasStopButtonAlready(toolbarData: ToolbarData ): boolean {
      const hasStop = toolbarData.buttons.some( (button) => {
        return (button.text === RolloutstatusComponent.STOP_BUTTON_TEXT);
      });
      return hasStop;
  }

  /**
   * Component is about to exit
   */
  ngOnDestroy() {
    // publish event that AppComponent is about to be destroyed
    this._controllerService.publish(Eventtypes.COMPONENT_DESTROY, { 'component': 'RolloutstatusComponent', 'state': Eventtypes.COMPONENT_DESTROY });
    this.subscriptions.forEach(subscription => {
      subscription.unsubscribe();
    });
  }

  getClassName(): string {
    return this.constructor.name;
  }

  getNaples() {
    this.naplesEventUtility = new HttpEventUtility<ClusterSmartNIC>(ClusterSmartNIC);
    this.naples = this.naplesEventUtility.array as ReadonlyArray<ClusterSmartNIC>;
    const subscription = this.clusterService.WatchSmartNIC().subscribe(
      response => {
        this.naplesEventUtility.processEvents(response);
        this.naplesCopy = this.naples;
      },
      (error) => {
        if (Utility.getInstance().getMaintenanceMode()) {
          setTimeout(() => {
            this.getNaples();
          }, 3000);
        } else {
          this._controllerService.webSocketErrorToaster('Failed to get Naples', error);
        }
      },
      () => {
        setTimeout(() => {
          this.getNaples();  // Watch ends during rollout, try again so new updates can be shown on UI
        }, 3000);
      }
    );
    this.subscriptions.push(subscription); // add subscription to list, so that it will be cleaned up when component is destroyed.
  }


  stopRollout() {
    this.selectedRollout.spec.suspend = true;
    const rollout = this.selectedRollout.getModelValues();
    const sub = this.rolloutService.StopRollout(rollout).subscribe(
      (response) => {
        this._controllerService.invokeSuccessToaster('Success', 'Rollout ' + this.selectedRollout.meta.name + ' was suspended!');
        // TODO:  What should UI do when rollout is suspeced.
      },
      this._controllerService.restErrorHandler('Failed to suspend rollout')
    );
    this.subscriptions.push(sub);
  }

  onStopRollout() {
    this._controllerService.invokeConfirm({
      header: 'Are you sure you want to stop current rollout?',
      message: 'This action cannot be reversed',
      acceptLabel: 'Stop',
      accept: () => {
        this.stopRollout();
      }
    });
  }

  getRolloutDetail() {
    // We perform a get then a watch.  so that we can know if the object the user is
    // looking for exists or not.
    const getSubscription = this.rolloutService.GetRollout(this.selectedRolloutId).subscribe(
      response => {
        this.watchRolloutDetail();
      },
      error => {
        // If we receive any error code we display object is missing
        this.showMissingScreen = true;
      }
    );
    this.subscriptions.push(getSubscription);
  }

  watchRolloutDetail() {
    this.rolloutsEventUtility = new HttpEventUtility<RolloutRollout>(RolloutRollout);
    this.rollouts = this.rolloutsEventUtility.array;
    const subscription = this.rolloutService.WatchRollout({ 'field-selector': 'meta.name=' + this.selectedRolloutId }).subscribe(
      response => {
        this.rolloutsEventUtility.processEvents(response);
        if (this.rollouts.length > 1) {
          // because of the name selector, we should
          // have only got one object
          console.error(
            this.getClassName() + '.watchRolloutDetail() ' + 'Received more than one rollout. Expected ' + this.selectedRolloutId + ', received ' +
            this.rollouts.map((rollout) => rollout.meta.name).join(', '));
        }
        if (this.rollouts.length > 0) {
          // In case object was deleted and then readded while we are on the same screen
          this.showDeletionScreen = false;
          // In case object wasn't created yet and then was added while we are on the same screen
          this.showMissingScreen = false;
          // Set sgpolicyrules
          this.selectedRollout = this.rollouts[0];
          this.addToolbarButton();
          this.selectedRolloutNicNodeTypes = RolloutUtil.getRolloutNaplesVeniceType(this.selectedRollout);
          this.labelSelectorsStrings = RolloutUtil.formatRolloutNaplesCriteria(this.selectedRollout);
        } else {
          // Must have received a delete event.
          this.showDeletionScreen = true;
          // Putting focus onto the overlay to prevent user
          // being able to interact with the page underneath

          this.selectedRollout = null;
        }
      },
      (error) => {
        if (Utility.getInstance().getMaintenanceMode()) {
          setTimeout(() => {
            this.watchRolloutDetail();
          }, 3000);
        } else {
        this._controllerService.webSocketErrorToaster('Failed to get Rollout', error);
        }
      },
      () => {
        setTimeout(() => {
          this.watchRolloutDetail();
        }, 3000);
      }
    );
    this.subscriptions.push(subscription);
  }

  selectedIndexChangeEvent(event) {
    // TODO: add code
  }

  /**
   * This API serves html template
   * @param data
   * @param col
   */
  displayColumn(data: any, col: TableCol): string {
    const fields = col.field.split('.');
    const value = Utility.getObjectValueByPropertyPath(data, fields);
    const column = col.field;
    switch (column) {
      default:
        return Array.isArray(value) ? JSON.stringify(value, null, 2) : value;
    }
  }

  /**
   * This API serves html template
   * @param data
   * @param col
   * @param tablename
   *
   * VS-312  requests to have rollout status page's NAPLE table should display NIC id (human readable text)
   * We check "tablename" to build name column display.
   */
  displayNameColumn(data: any, col: TableCol, tablename: string): string {
      if (tablename === 'NAPLES') {
        const fields = col.field.split('.');
        const value = Utility.getObjectValueByPropertyPath(data, fields);
        const column = col.field;
        const id  = this.getNICID(value);
        return id;
      } else {
        return this.displayColumn(data, col);
      }
  }

  getNICID(nicNameInMAC: string): string {
    const matchedNaple  = this.naplesCopy.find(naple => {
      return naple.meta.name === nicNameInMAC;
    });
    return matchedNaple ? matchedNaple.spec.id : nicNameInMAC;
  }

  isVeniceOnly(): boolean {
    return (this.selectedRolloutNicNodeTypes === RolloutUtil.ROLLOUTTYPE_VENICE_ONLY);
  }

  isNaplesOnly(): boolean {
    return (this.selectedRolloutNicNodeTypes === RolloutUtil.ROLLOUTTYPE_NAPLES_ONLY);
  }

  isVeniceNaples(): boolean {
    return (this.selectedRolloutNicNodeTypes === RolloutUtil.ROLLOUTTYPE_BOTH_NAPLES_VENICE);
  }

  getRolloutVeniceNaplesType(): string {
    return EnumRolloutOptions[this.selectedRolloutNicNodeTypes];
  }

}
