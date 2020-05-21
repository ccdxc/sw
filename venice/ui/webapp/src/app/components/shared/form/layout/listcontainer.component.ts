import { Component, Input, TemplateRef, EventEmitter, Output, ViewEncapsulation, ChangeDetectionStrategy, OnInit, OnChanges } from '@angular/core';
import { FormGroup, FormArray } from '@angular/forms';
import { Animations } from '@app/animations';

@Component({
  selector: 'app-listcontainer',
  templateUrl: './listcontainer.component.html',
  styleUrls: ['./listcontainer.component.scss'],
  encapsulation: ViewEncapsulation.None,
  animations: Animations,
})

export class ListContainerComponent {
  @Input() addText: string = 'ADD';
  @Input() formGroup: FormGroup = null;
  @Input() maxiumCount: number = -1;
  @Input() rowGap: string = '15px';
  @Input() itemTemplate: TemplateRef<any>;
  @Input() showAddButton: () => boolean = null;
  @Input() showDeleteButton: (index: number) => boolean = null;

  @Output() addItem: EventEmitter<number> = new EventEmitter<number>();
  @Output() deleteItem: EventEmitter<number> = new EventEmitter<number>();

  showAdd(): boolean {
    if (this.showAddButton) {
      return this.showAddButton();
    }
    if (this.maxiumCount === -1) {
      return true;
    }
    const controls = this.formGroup.controls as any;
    return controls.length < this.maxiumCount;
  }

  showDelete(index): boolean {
    if (this.showDeleteButton) {
      return this.showDeleteButton(index);
    }
    const controls = this.formGroup.controls as any;
    return controls.length > 1;
  }

  onClickAdd() {
    this.addItem.emit();
  }

  onClickDelete(index: number) {
    this.deleteItem.emit(index);
  }

}
