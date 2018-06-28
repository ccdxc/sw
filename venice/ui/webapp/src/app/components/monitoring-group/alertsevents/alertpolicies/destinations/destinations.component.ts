import { Component, OnInit, OnDestroy, EventEmitter, ViewEncapsulation, ViewChild, Input, Output, OnChanges, SimpleChanges } from '@angular/core';
import { Utility } from '@app/common/Utility';

import { Table } from 'primeng/table';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { ControllerService } from '@app/services/controller.service';
import { Animations } from '@app/animations';
import { TabcontentComponent } from 'web-app-framework';
import { FormGroup, FormControl, FormArray } from '@angular/forms';

@Component({
  selector: 'app-destinations',
  templateUrl: './destinations.component.html',
  styleUrls: ['./destinations.component.scss'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None
})
export class DestinationpolicyComponent extends TabcontentComponent implements OnInit, OnChanges, OnDestroy {
  @ViewChild('destinationsTable') destinationsTurboTable: Table;

  headerIcon: Icon = {
    margin: {
      top: '0px',
      left: '0px'
    },
    matIcon: 'send'
  };

  destinations: any;
  selectedDestinationPolicy: any;
  count: number;

  cols: any[] = [
    { field: 'meta.name', header: 'Policy Name', class: 'destinationpolicy-column-name', sortable: true },
    { field: 'spec.email-list', header: 'Email List', class: 'destinationpolicy-column-email-list', sortable: true },
    { field: 'spec.snmp-trap-servers', header: 'SNMP TRAP Servers', class: 'destinationpolicy-column-snmp_trap_servers', sortable: false },
    { field: 'status.total-notifications-sent', header: 'Total Notication Sent', class: 'destinationpolicy-column-total-notifications-sent', sortable: false },
  ];

  @Input() data;
  @Output() refreshRequest: EventEmitter<any> = new EventEmitter();

  constructor(protected _controllerService: ControllerService) {
    super();
  }

  ngOnInit() {
    if (this.isActiveTab) {
      this.setDefaultToolbar();
    }
    this.setRowData();
  }

  getClassName(): string {
    return this.constructor.name;
  }

  setDefaultToolbar() {
    const currToolbar = this._controllerService.getToolbarData();
    currToolbar.buttons = [
      {
        cssClass: 'global-button-primary destinations-button',
        text: 'ADD DESTINATION',
        callback: () => { this.createNewDestination(); }
      },
      {
        cssClass: 'global-button-primary destinations-button',
        text: 'REFRESH',
        callback: () => { this.refreshRequest.emit(true); }
      }
    ];
    this._controllerService.setToolbarData(currToolbar);
  }

  createNewDestination() {
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
    this.destinations = items;
    if (items != null) {
      this.count = items.length;
    } else {
      this.count = 0;
    }
  }

  ngOnChanges(changes: SimpleChanges) {
    // We only set the toolbar if we are becoming the active tab,
    if (changes.isActiveTab != null && this.isActiveTab) {
      this.setDefaultToolbar();
    }
    if (changes.data != null) {
      this.setRowData();
    }
  }

  ngOnDestroy() {
  }

  displayColumn(alerteventpolicies, col): any {
    const fields = col.field.split('.');
    const value = Utility.getObjectValueByPropertyPath(alerteventpolicies, fields);
    const column = col.field;
    switch (column) {
      case 'spec.email-list':
        return JSON.stringify(value, null, 2);
      case 'spec.snmp-trap-servers':
        return JSON.stringify(value, null, 2);
      default:
        return Array.isArray(value) ? JSON.stringify(value, null, 2) : value;
    }
  }

  onUpdateRecord(event, destinationpolicy) {
  }

}
