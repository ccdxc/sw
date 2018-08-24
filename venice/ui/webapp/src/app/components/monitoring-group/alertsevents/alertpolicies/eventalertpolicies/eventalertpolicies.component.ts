import { Component, Input, IterableDiffer, IterableDiffers, OnChanges, OnDestroy, OnInit, SimpleChanges, ViewChild, ViewEncapsulation, DoCheck, EventEmitter, Output, ChangeDetectorRef } from '@angular/core';
import { Animations } from '@app/animations';
import { HttpEventUtility } from '@app/common/HttpEventUtility';
import { Utility } from '@app/common/Utility';
import { Table } from 'primeng/table';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { ControllerService } from '@app/services/controller.service';
import { TabcontentComponent } from 'web-app-framework';
import { MonitoringAlertPolicy, FieldsRequirement_operator, MonitoringAlertDestination, IMonitoringAlertPolicy } from '@sdk/v1/models/generated/monitoring';
import { MonitoringService } from '@app/services/generated/monitoring.service';


@Component({
  selector: 'app-eventalertpolicies',
  templateUrl: './eventalertpolicies.component.html',
  styleUrls: ['./eventalertpolicies.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class EventalertpolicyComponent extends TabcontentComponent implements OnInit, OnChanges, OnDestroy, DoCheck {
  @ViewChild('eventAlertPoliciesTable') eventAlertPoliciesTable: Table;

  headerIcon: Icon = {
    margin: {
      top: '0px',
      left: '0px',
    },
    matIcon: 'notifications'
  };
  globalFilterFields: string[] = ['meta.name', 'spec.destinations', 'spec.severity'];

  eventAlertPolicies: any;
  selectedEventAlertPolicies: any;
  count: number;
  expandedRowData: any;
  arrayDiffers: IterableDiffer<IMonitoringAlertPolicy>;

  cols: any[] = [
    { field: 'meta.name', header: 'Policy Name', class: 'eventalertpolicies-column-name', sortable: true },
    { field: 'spec.destinations', header: 'Destinations', class: 'eventalertpolicies-column-destinations', sortable: true },
    { field: 'spec.requirements', header: 'Requirements', class: 'eventalertpolicies-column-requirements', sortable: false },
    { field: 'spec.severity', header: 'Severity', class: 'eventalertpolicies-column-severity', sortable: false },
    { field: 'status.total-hits', header: 'Total Hits', class: 'eventalertpolicies-column-totalhits', sortable: false },
    { field: 'status.open-alerts', header: 'Open', class: 'eventalertpolicies-column-openalerts', sortable: false },
    { field: 'status.acknowledged-alerts', header: 'Acknowledged', class: 'eventalertpolicies-column-acknowledgealerts', sortable: false }
  ];

  creatingMode: boolean = false;
  editingMode: boolean = false;

  // If we receive new data, but the display is frozen (user editing),
  // this should be set to true so that when user exits editing, we can update the display
  hasNewData: boolean = true;

  // Whether the toolbar buttons should be enabled
  shouldEnableButtons: boolean = true;

  @Input() policies: MonitoringAlertPolicy[] = [];
  @Input() destinations: MonitoringAlertDestination[] = [];
  @Output() tableRowExpandClick: EventEmitter<any> = new EventEmitter();

  constructor(protected _controllerService: ControllerService,
    protected _iterableDiffers: IterableDiffers,
    private cdr: ChangeDetectorRef,
    protected _monitoringService: MonitoringService) {
    super();
    this.arrayDiffers = _iterableDiffers.find([]).create(HttpEventUtility.trackBy);
  }

  ngOnInit() {
    if (this.isActiveTab) {
      this.setDefautlToolbar();
    }
    this.setRowData();
  }

  getClassName(): string {
    return this.constructor.name;
  }

  setDefautlToolbar() {
    const currToolbar = this._controllerService.getToolbarData();
    currToolbar.buttons = [
      {
        cssClass: 'global-button-primary eventalertpolicies-button',
        text: 'ADD EVENT POLICY',
        computeClass: () => this.shouldEnableButtons ? '' : 'global-button-disabled',
        callback: () => { this.createNewPolicy(); }
      },
    ];
    this._controllerService.setToolbarData(currToolbar);
  }

  createNewPolicy() {
    // If a row is expanded, we shouldnt be able to open a create new policy form
    if (!this.editingMode) {
      this.creatingMode = true;
      this.editMode.emit(true);
    }
  }

  creationFormClose() {
    this.creatingMode = false;
    this.editMode.emit(false);
  }

  setRowData() {
    /**
     * We copy the data so that the table doesn't
     * automatically update when the input binding is updated
     * This allows us to freeze the table when a user is doing inline
     * editing on a row entry
     */
    const _ = Utility.getLodash();
    const items = _.cloneDeep(this.policies);
    this.eventAlertPolicies = items;
    if (items != null) {
      this.count = items.length;
    } else {
      this.count = 0;
    }
  }

  /**
   * We check if any of the objects in the array have changed
   * This kind of detection is not automatically done by angular
   * To improve efficiency, we check only the name and last mod time
   * (see trackBy function) instead of checking every object field.
   */
  ngDoCheck() {
    const changes = this.arrayDiffers.diff(this.policies);
    if (changes) {
      if (this.editingMode) {
        this.hasNewData = true;
      } else {
        this.setRowData();
      }
    }
  }

  /**
   * This api serves html template
   */
  getAlertItemIconClass(record) {
    return 'global-alert-' + record.severity;
  }

  ngOnChanges(changes: SimpleChanges) {
    // We only set the toolbar if we are becoming the active tab,
    if (changes.isActiveTab != null && this.isActiveTab) {
      this.setDefautlToolbar();
    }
  }

  ngOnDestroy() {
  }

  formatRequirements(data: MonitoringAlertPolicy) {
    // TODO: Use ui_hints from the swagger as the display values
    const value = data.spec.requirements;
    if (value == null) {
      return '';
    }
    const retArr = [];
    value.forEach((req) => {
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
        case FieldsRequirement_operator.lt:
          ret += '<';
          break;
        case FieldsRequirement_operator.notIn:
          ret += 'Not In';
          break;
        case FieldsRequirement_operator.in:
          ret += 'In';
          break;
        default:
          ret += req.operator;
      }

      ret += ' ';

      if (req.values != null) {
        // Using v instead of value to not shadow the earlier value
        req.values.forEach((v) => {
          ret += v + ', ';
        });
        // Subtract last comma and space
        ret = ret.slice(0, ret.length - 2);
      }
      retArr.push(ret);
    });
    return retArr;
  }

  displayColumn(eventalertpolicies, col): any {
    const fields = col.field.split('.');
    const value = Utility.getObjectValueByPropertyPath(eventalertpolicies, fields);
    const column = col.field;
    switch (column) {
      case 'spec.destinations':
        return JSON.stringify(value, null, 2);
      default:
        return Array.isArray(value) ? JSON.stringify(value, null, 2) : value;
    }
  }

  onUpdateRecord(event, eventalertpolicy) {
    // If in creation mode, don't allow row expansion
    if (this.creatingMode) {
      return;
    }
    if (!this.editingMode) {
      this.eventAlertPoliciesTable.toggleRow(eventalertpolicy, event);
      this.expandedRowData = eventalertpolicy;
      this.editMode.emit(true);
      this.editingMode = true;
      this.shouldEnableButtons = false;
    } else {
      this.editingMode = false;
      this.editMode.emit(false);
      this.shouldEnableButtons = true;
      // We don't untoggle the row here, it will happen when rowExpandAnimationComplete
      // is called.
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
    if (!this.editingMode) {
      // we are exiting the row expand
      this.eventAlertPoliciesTable.toggleRow(this.expandedRowData, event);
      this.expandedRowData = null;
      if (this.hasNewData) {
        this.setRowData();
      }
      // Needed to prevent "ExpressionChangedAfterItHasBeenCheckedError"
      // We force an additional change detection cycle
      this.cdr.detectChanges();
    }
  }

  onDeleteRecord(event, eventalertpolicy: IMonitoringAlertPolicy) {
    // Should not be able to delete any record while we are editing
    if (this.editingMode) {
      return;
    }
    this._monitoringService.DeleteAlertPolicy(eventalertpolicy.meta.name);
  }

}
