import { Component, OnInit, ViewEncapsulation, TemplateRef, Input, EventEmitter, Output } from '@angular/core';
import { CdkDragDrop, moveItemInArray } from '@angular/cdk/drag-drop';
import { Animations } from '@app/animations';

export interface OrderedItem<T> {
  id: string;
  data: T;
  inEdit: boolean;
}

@Component({
  selector: 'app-orderedlist',
  templateUrl: './orderedlist.component.html',
  styleUrls: ['./orderedlist.component.scss'],
  encapsulation: ViewEncapsulation.None,
  animations: Animations
})
export class OrderedlistComponent implements OnInit {
  @Input() dataArray: OrderedItem<any>[] = [];
  // Variables available in template
  // data, index, first, last, inEdit
  @Input() template: TemplateRef<any>;
  @Input() enableDragDrop: boolean = true;
  @Input() enableOrdering: boolean = true;
  // Emits the index of the clicked element
  @Output() itemClick: EventEmitter<number> = new EventEmitter<number>();
  @Output() orderChange: EventEmitter<any> = new EventEmitter<any>();

  constructor() { }

  ngOnInit() {
  }

  moveItem(currIndex, newIndex) {
    moveItemInArray(this.dataArray, currIndex, newIndex);
    this.orderChange.emit(true);
  }

  drop(event: CdkDragDrop<string[]>) {
    moveItemInArray(this.dataArray, event.previousIndex, event.currentIndex);
    this.orderChange.emit(true);
  }

  trackByFunc(index, item) {
    return item.id;
  }

}
