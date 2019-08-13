import { Component, OnInit, ViewEncapsulation, ElementRef, Input, forwardRef } from '@angular/core';
import { Chips } from 'primeng/primeng';
import { NG_VALUE_ACCESSOR } from '@angular/forms';
import { Utility } from '@app/common/Utility';
import { ENTER, SPACE } from '@angular/cdk/keycodes';

export const CHIPS_VALUE_ACCESSOR: any = {
  provide: NG_VALUE_ACCESSOR,
  useExisting: forwardRef(() => ChipsComponent), // tslint:disable-line
  multi: true
};

@Component({
  selector: 'app-chips',
  templateUrl: './chips.component.html',
  styleUrls: ['./chips.component.scss'],
  encapsulation: ViewEncapsulation.None,
  providers: [CHIPS_VALUE_ACCESSOR]
})
export class ChipsComponent extends Chips {
  @Input() separatorKeyCodes: number[] = [ENTER, SPACE];
  // 64 is the max length of object names
  @Input() maxChipLength: number = Utility.MAX_OBJECT_NAME_LENGTH;

  generateCancelClickFunction(index) {
    return (event) => {
      this.removeItem(event, index);
    };
  }

  onKeydown(event: KeyboardEvent): void {
    if (this.separatorKeyCodes.includes(event.which)) {
      // simulate enter key
      this.addItem(event, this.inputViewChild.nativeElement.value);
      this.inputViewChild.nativeElement.value = '';

      event.preventDefault();
    } else {
      super.onKeydown(event);
    }
  }
}
