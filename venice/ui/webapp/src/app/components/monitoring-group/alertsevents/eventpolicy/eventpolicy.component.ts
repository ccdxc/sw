import { Component, OnInit, ViewEncapsulation, ViewChild, ChangeDetectorRef, OnDestroy } from '@angular/core';
import { Animations } from '@app/animations';
import { BaseComponent } from '@app/components/base/base.component';
import { ControllerService } from '@app/services/controller.service';
import { MonitoringService } from '@app/services/generated/monitoring.service';
import { MessageService } from 'primeng/primeng';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { Utility } from '@app/common/Utility';
import { MonitoringEventPolicy, FieldsRequirement, FieldsRequirement_operator } from '@sdk/v1/models/generated/monitoring';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { Eventtypes } from '@app/enum/eventtypes.enum';
import { Table } from 'primeng/table';
import { EventsEvent } from '@sdk/v1/models/generated/events';

@Component({
  selector: 'app-eventpolicy',
  templateUrl: './eventpolicy.component.html',
  styleUrls: ['./eventpolicy.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class EventpolicyComponent extends BaseComponent implements OnInit, OnDestroy {
  @ViewChild('eventPoliciesTable') policytable: Table;
  subscriptions = [];

  bodyIcon: Icon = {
    margin: {
      top: '8px',
      left: '10px',
    },
    matIcon: 'send'
  };

  headerIcon: Icon = {
    margin: {
      top: '0px',
      left: '10px',
    },
    matIcon: 'grid_on'
  };

  // Used for processing watch events
  policyEventUtility: HttpEventUtility<MonitoringEventPolicy>;
  policies: ReadonlyArray<MonitoringEventPolicy> = [];

  displayedPolicies: ReadonlyArray<MonitoringEventPolicy> = [];

  expandedRowData: any;
  creatingMode: boolean = false;
  showEditingForm: boolean = false;

  // If we receive new data, but the display is frozen (user editing),
  // this should be set to true so that when user exits editing, we can update the display
  hasNewData: boolean = true;

  // Whether the toolbar buttons should be enabled
  shouldEnableButtons: boolean = true;

  cols: any[] = [
    { field: 'meta.name', header: 'Name', class: 'eventpolicy-column-name', sortable: false },
    { field: 'spec.selector', header: 'Filters', class: 'destinations-column-name', sortable: false },
    { field: 'spec.targets', header: 'Targets', class: 'eventpolicy-column-targets', sortable: false, isLast: true },
  ];

  constructor(protected _controllerService: ControllerService,
    private cdr: ChangeDetectorRef,
    protected _monitoringService: MonitoringService,
    protected messageService: MessageService) {
    super(_controllerService, messageService);
  }

  ngOnInit() {
    this._controllerService.setToolbarData({
      buttons: [{
        cssClass: 'global-button-primary eventpolicy-button',
        text: 'ADD EVENT POLICY',
        computeClass: () => this.shouldEnableButtons ? '' : 'global-button-disabled',
        callback: () => { this.createNewPolicy(); }
      },
      ],
      breadcrumb: [{ label: 'Alerts & Events', url: Utility.getBaseUIUrl() + 'monitoring/alertsevents' },
      { label: 'Event Policy', url: Utility.getBaseUIUrl() + 'monitoring/alertsevents/eventpolicy' }
      ]
    });
    this.getEventPolicy();
  }

  getEventPolicy() {
    this.policyEventUtility = new HttpEventUtility<MonitoringEventPolicy>(MonitoringEventPolicy);
    const sub = this._monitoringService.WatchEventPolicy().subscribe(
      (response) => {
        this.policies = this.policyEventUtility.processEvents(response);
        if (this.isInEditMode()) {
          this.hasNewData = true;
        } else {
          this.setTableData();
        }
      },
      this._controllerService.restErrorHandler('Failed to get event policies')
    );
    this.subscriptions.push(sub);
  }

  setTableData() {
    /**
     * We copy the data so that the table doesn't
     * automatically update when the input binding is updated
     * This allows us to freeze the table when a user is doing inline
     * editing on a row entry
     */
    if (this.policies == null) {
      return;
    }
    const _ = Utility.getLodash();
    const policies = _.cloneDeep(this.policies);
    this.displayedPolicies = policies;
  }

  createNewPolicy() {
    // If a row is expanded, we shouldnt be able to open a create new policy form
    if (!this.isInEditMode()) {
      this.creatingMode = true;
    }
  }

  /**
   * Called when a row expand animation finishes
   * The animation happens when the row expands, and when it collapses
   * If it is expanding, then we are in ediitng mode (set in onUpdateRecord).
   * If it is collapsing, then editingMode should be false, (set in onUpdateRecord).
   * When it is collapsing, we toggle the row on the turbo table
   *
   * This is because we must wait for the animation to complete before toggling
   * the row on the turbo table for a smooth animation.
   * @param  $event Angular animation end event
   */
  rowExpandAnimationComplete($event) {
    if (!this.showEditingForm) {
      // we are exiting the row expand
      this.policytable.toggleRow(this.expandedRowData, event);
      this.expandedRowData = null;
      if (this.hasNewData) {
        this.setTableData();
      }
      // Needed to prevent "ExpressionChangedAfterItHasBeenCheckedError"
      // We force an additional change detection cycle
      this.cdr.detectChanges();
    }
  }

  onUpdateRecord(event, policy) {
    // If in creation mode, don't allow row expansion
    if (this.creatingMode) {
      return;
    }
    if (!this.isInEditMode()) {
      // Entering edit mode
      this.policytable.toggleRow(policy, event);
      this.expandedRowData = policy;
      this.showEditingForm = true;
      this.shouldEnableButtons = false;
    } else {
      this.showEditingForm = false;
      this.shouldEnableButtons = true;
      // We don't untoggle the row here, it will happen when rowExpandAnimationComplete
      // is called.
    }
  }

  onDeleteRecord(event, policy: MonitoringEventPolicy) {
    // Should not be able to delete any record while we are editing
    if (this.isInEditMode()) {
      return;
    }

    const msg = 'Deleted policy ' + policy.meta.name;
    const sub = this._monitoringService.DeleteEventPolicy(policy.meta.name).subscribe(
      response => {
        this.invokeSuccessToaster('Delete Successful', msg);
      },
      this.restErrorHandler('Delete Failed')
    );
    this.subscriptions.push(sub);
  }

  creationFormClose() {
    this.creatingMode = false;
  }

  isInEditMode() {
    return this.expandedRowData != null;
  }

  displayColumn(exportData, col): any {
    const fields = col.field.split('.');
    const value = Utility.getObjectValueByPropertyPath(exportData, fields);
    const column = col.field;
    switch (column) {
      case 'spec.targets':
        return value.map(item => item.destination).join(', ');
      case 'spec.email-list':
        return JSON.stringify(value, null, 2);
      case 'spec.snmp-trap-servers':
        return JSON.stringify(value, null, 2);
      default:
        return Array.isArray(value) ? JSON.stringify(value, null, 2) : value;
    }
  }

  formatRequirements(data: FieldsRequirement[]) {
    if (data == null) {
      return '';
    }
    const retArr = [];
    data.forEach((req) => {
      let ret = '';
      ret += req['key'] + ' ';
      switch (req.operator) {
        case FieldsRequirement_operator.equals:
          ret += '=';
          break;
        case FieldsRequirement_operator.notEquals:
          ret += '!=';
          break;
        case FieldsRequirement_operator.gt:
          ret += '>';
          break;
        case FieldsRequirement_operator.gte:
          ret += '>=';
          break;
        case FieldsRequirement_operator.lte:
          ret += '<=';
          break;
        case FieldsRequirement_operator.lt:
          ret += '<';
          break;
        case FieldsRequirement_operator.in:
          ret += '=';
          break;
        case FieldsRequirement_operator.notIn:
          ret += '!=';
          break;
        default:
          ret += req.operator;
      }

      ret += ' ';

      if (req.values != null) {
        let values = [];
        const enumInfo = Utility.getNestedPropInfo(new EventsEvent(), req.key).enum;
        values = req.values.map((item) => {
          if (enumInfo != null && [item] != null) {
            return enumInfo[item];
          }
          return item;
        });
        ret += values.join(' or ');
      }
      retArr.push(ret);
    });
    return retArr;
  }

  /**
   * Component is about to exit
   */
  ngOnDestroy() {
    // publish event that AppComponent is about to be destroyed
    this._controllerService.publish(Eventtypes.COMPONENT_DESTROY, { 'component': 'Eventpolicy component', 'state': Eventtypes.COMPONENT_DESTROY });
    this.subscriptions.forEach(subscription => {
      subscription.unsubscribe();
    });
  }

}
