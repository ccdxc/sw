import { Component, EventEmitter, Input, OnChanges, OnDestroy, OnInit, Output, ViewEncapsulation } from '@angular/core';
import { Utility } from '@app/common/Utility';
import { Icon } from '@app/models/frontend/shared/icon.interface';

/**
 * Component for creating the heading of dashboard widgets
 * Should be used by all dashboard widgets for a standard look.
 */
@Component({
  selector: 'app-dsbdwidgetheader',
  templateUrl: './dsbdwidgetheader.component.html',
  styleUrls: ['./dsbdwidgetheader.component.scss'],
  encapsulation: ViewEncapsulation.None
})
export class DsbdwidgetheaderComponent implements OnInit, OnDestroy, OnChanges {
  has_update_string: boolean;
  _iconStyles: any;

  @Input() icon: Icon;
  @Input() title: String;
  @Input() last_update: String;
  @Input() id: String;
  @Input() menuItems: [any];

  @Output() widgetHeaderClick: EventEmitter<any> = new EventEmitter();
  @Output() widgetMenuSelect: EventEmitter<any> = new EventEmitter();

  items = [
    { text: 'Refresh' },
    { text: 'Export data' },
    { text: 'Go to detail page' }
  ];

  constructor() {
  }

  ngOnInit() {
    if (!this.id) {
      this.id = 'dashboard-header-' + Utility.s4();
    }
    if (this.icon == null) {
      this.icon = { margin: {} };
    }
    this._iconStyles = this._setIconStyles();
    this.has_update_string = this.last_update != null &&
      this.last_update !== '';
  }

  _setIconStyles() {
    const styles = {
      'margin-top': this.icon.margin.top,
      'margin-left': this.icon.margin.left,
    };
    return styles;
  }

  ngOnDestroy() {
  }

  ngOnChanges() {
    if (this.menuItems) {
      this.items = this.menuItems;
    }
  }

  itemClick($event) {
    this.widgetHeaderClick.emit(this.id);
  }

  menuSelect(item) {
    const obj = {
      id: this.id,
      menu: item
    };
    this.widgetMenuSelect.emit(obj);
  }
}
