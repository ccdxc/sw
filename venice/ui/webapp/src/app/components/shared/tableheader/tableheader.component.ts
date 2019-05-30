import { Component, OnInit, Input, EventEmitter, Output } from '@angular/core';
import { Icon } from '@app/models/frontend/shared/icon.interface';

export interface TableMenuItem {
  text: string;
  onClick: () => void;
  disabled?: boolean;
}

@Component({
  selector: 'app-tableheader',
  templateUrl: './tableheader.component.html',
  styleUrls: ['./tableheader.component.scss']
})
export class TableheaderComponent implements OnInit {
  @Input() title;
  @Input() icon: Icon;
  @Input() count: number;
  @Input() total: number;
  @Input() max: number;
  @Input() width: string;

  // Right hand side options
  @Input() lastUpdateTime: string;
  @Input() showRefreshIcon: boolean = false;
  @Input() tableMenuItems: TableMenuItem[] = [];

  @Output() refreshIconClick: EventEmitter<any> = new EventEmitter<any>();

  _iconStyles: any;

  constructor() { }

  ngOnInit() {
    if (this.icon == null) {
      this.icon = { margin: {} };
    }
    this._iconStyles = this._setIconStyles();
  }

  _setIconStyles() {
    const styles = {
      'margin-top': this.icon.margin.top,
      'margin-left': this.icon.margin.left,
    };
    return styles;
  }

}
