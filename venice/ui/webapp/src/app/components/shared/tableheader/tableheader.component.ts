import { Component, OnInit, Input, EventEmitter, Output, TemplateRef } from '@angular/core';
import { Icon } from '@app/models/frontend/shared/icon.interface';
import { TableCol } from '../tableviewedit';

export interface TableMenuItem {
  text: string;
  onClick: () => void;
  disabled?: boolean;
}

/**
 * This TableheaderComponent is used in TableViewEdit.
 * See hosts.component.ts/html". We inject actionButtonsTemplate" to
 *  <app-tableheader  [enableColumnSelect]="true"
 *     [cols]="cols" (columnSelectChange)="onColumnSelectChange($event)"
 */

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
  @Input() showTotalHits: boolean = false;  // e.g In auditEvent table, there are 2m hits, but each query fetch 4K records. We want to show total hits. Thus, we want sh

  // Right hand side options
  @Input() lastUpdateTime: string;
  @Input() showRefreshIcon: boolean = false;
  @Input() tableMenuItems: TableMenuItem[] = [];

  @Input() actionButtonsTemplate: TemplateRef<any>;
  @Input() enableColumnSelect: boolean = false;
  @Input() cols: TableCol[] = [];

  @Output() refreshIconClick: EventEmitter<any> = new EventEmitter<any>();
  @Output() columnSelectChange: EventEmitter<any> = new EventEmitter<any>();

  _iconStyles: any;

  selectedcolumns: TableCol[];

  constructor() { }

  ngOnInit() {
    if (this.icon == null) {
      this.icon = { margin: {} };
    }
    this._iconStyles = this._setIconStyles();
    this.selectedcolumns = this.cols;
  }

  onColumnSelectChange($event) {
    this.columnSelectChange.emit($event);
  }

  _setIconStyles() {
    const styles = {
      'margin-top': this.icon.margin.top,
      'margin-left': this.icon.margin.left,
    };
    return styles;
  }

}
