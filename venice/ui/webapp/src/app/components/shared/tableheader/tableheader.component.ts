import { Component, OnInit, Input } from '@angular/core';
import { Icon } from '@app/models/frontend/shared/icon.interface';

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
  @Input() width: string;

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
