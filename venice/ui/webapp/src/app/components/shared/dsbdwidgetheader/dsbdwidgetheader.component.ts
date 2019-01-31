import { Component, EventEmitter, Input, OnChanges, OnDestroy, OnInit, Output, ViewEncapsulation } from '@angular/core';
import { Utility } from '@app/common/Utility';
import { Icon } from '@app/models/frontend/shared/icon.interface';

export interface MenuItems {
  text: string;
  onClick: () => void;
}

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
  hasUpdateString: boolean;
  _iconStyles: any;

  @Input() icon: Icon;
  @Input() iconColor: string = '#FFF';
  @Input() iconFillColor: string = '#FFF';
  @Input() title: String;
  // lastUpdateTime will be displayed if its given, otherwise it will
  // try to display timeRange if given. If both are given, it will
  // display lastUpdateTime
  @Input() lastUpdateTime: string = '2018-08-23T17:35:08.534909931Z';
  @Input() timeRange: string;
  @Input() id: String;
  @Input() menuItems: MenuItems[] = [];


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
    this.hasUpdateString = this.lastUpdateTime != null &&
      this.lastUpdateTime !== '';
  }

  ngOnChanges() {
    this.hasUpdateString = this.lastUpdateTime != null &&
      this.lastUpdateTime !== '';
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
}
