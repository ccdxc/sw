
import { AfterViewInit, Component, OnDestroy, OnInit, ViewEncapsulation, EventEmitter, Output } from '@angular/core';
import { ActivatedRoute } from '@angular/router';
import { Animations } from '@app/animations';

import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { BaseComponent } from '@components/base/base.component';
import { ControllerService } from '@app/services/controller.service';
import { RolloutService } from '@app/services/generated/rollout.service';
import { Utility } from '@common/Utility';
import { UIConfigsService } from '@app/services/uiconfigs.service';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { IApiStatus, IRolloutRollout, RolloutRollout, RolloutRolloutSpec } from '@sdk/v1/models/generated/rollout';
import { TableCol } from '@app/components/shared/tableviewedit/tableviewedit.component';

/**
 * This component let user monitor a rollout status.
 * User can suspend an in-progress Rollout
 */
@Component({
  selector: 'app-rolloutstatus',
  templateUrl: './rolloutstatus.component.html',
  styleUrls: ['./rolloutstatus.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None,
})
export class RolloutstatusComponent extends BaseComponent implements OnInit, OnDestroy {
  subscriptions = [];
  selectedRolloutId: string;
  selectedRollout: RolloutRollout;

  // Holds all policy objects
  rollouts: ReadonlyArray<RolloutRollout>;
  rolloutsEventUtility: HttpEventUtility<RolloutRollout>;

  // Whether we show a deletion overlay
  showDeletionScreen: boolean;

  // Whether we show a missing overlay
  showMissingScreen: boolean;

  loading: boolean = false;

  bodyicon: any = {
    margin: {
      top: '9px',
      left: '8px'
    },
    matIcon: 'update'
  };

  statusCols: TableCol[] = [
    { field: 'name', header: 'Name', class: 'rolloutstatus-column rolloutstatus-column-name', sortable: true, width: 10 },
    { field: 'phase', header: 'Phase', class: 'rolloutstatus-column rolloutstatus-column-phase', sortable: true, width: 10 },
    { field: 'start-time', header: 'Start Time', class: 'rolloutstatus-column rolloutstatus-column-sdate', sortable: true, width: 10 },
    { field: 'end-time', header: 'End Time', class: 'rolloutstatus-column rolloutstatus-column-edate', sortable: true, width: 10 },
    { field: 'reason', header: 'Reason', class: 'rolloutstatus-column rolloutstatus-column-reason', sortable: true, width: 20 },
    { field: 'message', header: 'Message', class: 'rolloutstatus-column rolloutstatus-column-message', sortable: false, width: 40 }
  ];

  constructor(protected _controllerService: ControllerService,
    private _route: ActivatedRoute, protected UIConfigService: UIConfigsService,
    protected rolloutService: RolloutService) {
    super(_controllerService, UIConfigService);
  }
  /**
   * Component is about to instantiate
   */
  ngOnInit() {
    this._controllerService.publish(Eventtypes.COMPONENT_INIT, { 'component': 'RolloutstatusComponent', 'state': Eventtypes.COMPONENT_INIT });
    this._route.paramMap.subscribe(params => {
      const id = params.get('id');
      this.selectedRolloutId = id;
      this.showDeletionScreen = false;
      this.showMissingScreen = false;

      this.getRolloutDetail();
      this._controllerService.setToolbarData({
        buttons: [
          {
            cssClass: 'global-button-primary rolloutstatus-toolbar-button',
            text: 'SUSPEND ROLLOUT',
            callback: () => {
              this.onSuspendRollout();
            }
          },
        ],
        breadcrumb: [
          { label: 'System Upgrade', url: Utility.getBaseUIUrl() + 'settings/upgrade/rollouts' },
          { label: id, url: Utility.getBaseUIUrl() + 'settings/upgrade/rollouts/' + id }]
      });
    });
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


  suspendRollout() {
    this.selectedRollout.spec.suspend = true;
    const rollout = this.selectedRollout.getModelValues();
    const sub = this.rolloutService.DoRollout(rollout).subscribe(
      (response) => {
        this._controllerService.invokeSuccessToaster('Success', 'Rollout ' + this.selectedRollout.meta.name + ' was suspended!');
        // TODO:  What should UI do when rollout is suspeced.
      },
      this._controllerService.restErrorHandler('')
    );
    this.subscriptions.push(sub);
  }

  onSuspendRollout() {
    this._controllerService.invokeConfirm({
      header: 'Are you sure to suspend current rollout?',
      message: 'This action cannot be reversed',
      acceptLabel: 'Suspend',
      accept: () => {
        this.suspendRollout();
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
        } else {
          // Must have received a delete event.
          this.showDeletionScreen = true;
          // Putting focus onto the overlay to prevent user
          // being able to interact with the page underneath

          this.selectedRollout = null;
        }
      },
    );
    this.subscriptions.push(subscription);
  }

  selectedIndexChangeEvent(event) {
    // TODO: add code
  }

  displayColumn(data, col): any {
    const fields = col.field.split('.');
    const value = Utility.getObjectValueByPropertyPath(data, fields);
    const column = col.field;
    switch (column) {
      default:
        return Array.isArray(value) ? JSON.stringify(value, null, 2) : value;
    }
  }

}
