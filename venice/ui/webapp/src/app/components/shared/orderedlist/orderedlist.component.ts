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
  @Input() templateView: TemplateRef<any>;
  @Input() templateEdit: TemplateRef<any>;
  @Input() enableDragDrop: boolean = true;
  @Input() enableOrdering: boolean = true;
  @Input() skipRenderActionIcons: boolean = false;
  @Input() addItemText: string = 'ADD';

  // Emits the index of the clicked element
  @Output() itemClick: EventEmitter<number> = new EventEmitter<number>();
  @Output() addItem: EventEmitter<number> = new EventEmitter<number>();
  @Output() deleteItem: EventEmitter<number> = new EventEmitter<number>();
  @Output() orderChange: EventEmitter<any> = new EventEmitter<any>();

  supportItemClick: boolean = false;
  supportAddClick: boolean = false;

  constructor() { }

  ngOnInit() {
    this.supportItemClick = this.itemClick.observers.length > 0;
    this.supportAddClick = this.addItem.observers.length > 0;
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

  onItemClick(index) {
    if (!this.dataArray[index].inEdit) {
      this.editItem(index);
    }
  }

  editItem(index) {
    // Collapse any other open items, and make index item open
    this.dataArray.forEach((r, i) => {
      if (i === index) {
        r.inEdit = true;
      } else {
        r.inEdit = false;
      }
    });
  }

  onClickAdd() {
    if (this.supportAddClick) {
      this.addItem.emit();
      this.editItem(this.dataArray.length - 1);
    }
  }

  onClickEdit(index) {
    if (!this.supportItemClick) {
      this.editItem(index);
    } else {
      this.itemClick.emit(index);
    }
  }

  onClickDelete(index) {
    this.deleteItem.emit(index);
  }
}
