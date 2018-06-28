import { Component, OnInit, OnDestroy, EventEmitter, ViewEncapsulation, ViewChild, Input, Output, OnChanges, SimpleChanges } from '@angular/core';
import { Utility } from '@app/common/Utility';

import { Table } from 'primeng/table';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { ControllerService } from '@app/services/controller.service';
import { Animations } from '@app/animations';
import { FormGroup, FormControl } from '@angular/forms';
import { TabcontentComponent } from 'web-app-framework';
import { MonitoringAlertPolicy, MonitoringRequirement_operator } from '@sdk/v1/models/generated/monitoring';

@Component({
  selector: 'app-eventalertpolicies',
  templateUrl: './eventalertpolicies.component.html',
  styleUrls: ['./eventalertpolicies.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class EventalertpolicyComponent extends TabcontentComponent implements OnInit, OnChanges, OnDestroy {
  @ViewChild('eventAlertPoliciesTable') eventAlertPoliciesTable: Table;

  headerIcon: Icon = {
    margin: {
      top: '0px',
      left: '0px',
    },
    matIcon: 'notifications'
  };

  eventAlertPolicies: any;
  selectedEventAlertPolicies: any;
  count: number;

  cols: any[] = [
    { field: 'meta.name', header: 'Policy Name', class: 'eventalertpolicies-column-name', sortable: true },
    { field: 'spec.destinations', header: 'Destinations', class: 'eventalertpolicies-column-destinations', sortable: true },
    { field: 'spec.requirements', header: 'Requirements', class: 'eventalertpolicies-column-requirements', sortable: false },
    { field: 'spec.severity', header: 'Severity', class: 'eventalertpolicies-column-severity', sortable: false },
    { field: 'status.total-hits', header: 'Total Hits', class: 'eventalertpolicies-column-totalhits', sortable: false },
    { field: 'status.open-alerts', header: 'Open', class: 'eventalertpolicies-column-openalerts', sortable: false },
    { field: 'status.acknowledged-alerts', header: 'Acknowledged', class: 'eventalertpolicies-column-acknowledgealerts', sortable: false }
  ];

  @Input() data;
  @Output() refreshRequest: EventEmitter<any> = new EventEmitter();

  constructor(protected _controllerService: ControllerService) {
    super();
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
        callback: () => { this.createNewPolicy(); }
      },
      {
        cssClass: 'global-button-primary eventalertpolicies-button',
        text: 'REFRESH',
        callback: () => { this.refreshRequest.emit(true); }
      }
    ];
    this._controllerService.setToolbarData(currToolbar);
  }

  createNewPolicy() {
  }


  setRowData() {
    /**
     * We copy the data so that the table doesn't
     * automatically update when the input binding is updated
     * This allows us to freeze the table when a user is doing inline
     * editing on a row entry
     */
    const _ = Utility.getLodash();
    const items = _.cloneDeep(this.data);
    this.eventAlertPolicies = items;
    if (items != null) {
      this.count = items.length;
    } else {
      this.count = 0;
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
    if (changes.data != null) {
      this.setRowData();
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
      ret += req['field-or-metric'] + ' ';
      switch (req.operator) {
        case MonitoringRequirement_operator.Equals:
          ret += '=';
          break;
        case MonitoringRequirement_operator.NotEquals:
          ret += '!=';
          break;
        case MonitoringRequirement_operator.Gt:
          ret += '>';
          break;
        case MonitoringRequirement_operator.Lt:
          ret += '<';
          break;
        case MonitoringRequirement_operator.NotIn:
          ret += 'Not In';
          break;
        case MonitoringRequirement_operator.In:
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
    console.log(this.getClassName() + '.onUpdateRecord()', eventalertpolicy);
  }

  onDeleteRecord(event, eventalertpolicy) {
    console.log(this.getClassName() + '.onDeleteRecord()', eventalertpolicy);
  }

}
