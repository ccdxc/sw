import { Component, EventEmitter, Input, Output, OnInit, ViewEncapsulation, ViewChild, ElementRef, OnChanges } from '@angular/core';
import { ModuleUtility } from '../../ModuleUtility';

@Component({
  selector: 'pw-recursivelist',
  templateUrl: './recursivelist.component.html',
  styleUrls: ['./recursivelist.component.css']
})
export class RecursivelistComponent implements OnInit {

  @Input() data: any;
  @Input() id: string = null;
  @Input() styleClass: string = null;

  @Output() itemClick: EventEmitter<any> = new EventEmitter();

  protected list: any;
  constructor() { }

  ngOnInit() {
    if (!this.id) {
      this.id = 'pw-RecursiveList_' + ModuleUtility.s4();
    }
    if (this.data) {
      this.list = this.data;
    }
  }

  /**
   * This API serves HTML template.
   * It adds class to chart HTML-DOM element
   */
  getStyleClass(): string {
    return (this.styleClass) ? this.styleClass : 'pw-recursive-list';
  }

  /**
   * This API serves HTML template.
   * It adds class to chart HTML-DOM element
   */
  getListLabel(item): any {
    return item.title;
  }

  /**
   * This API emits event
   */
  selectItem(item): any {
    this.itemClick.emit(item);
  }

}
