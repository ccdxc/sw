import { Component, ViewEncapsulation, Input, forwardRef, ViewChild, TemplateRef, OnInit, AfterViewInit, AfterContentInit } from '@angular/core';
import { Chips, AutoComplete } from 'primeng/primeng';
import { NG_VALUE_ACCESSOR } from '@angular/forms';
import { Utility } from '@app/common/Utility';
import { ENTER, SPACE, BACKSPACE } from '@angular/cdk/keycodes';
import { Subject } from 'rxjs';
import 'rxjs/add/operator/map';
import 'rxjs/add/operator/debounceTime';
import 'rxjs/operators';

export const CHIPS_VALUE_ACCESSOR: any = {
  provide: NG_VALUE_ACCESSOR,
  useExisting: forwardRef(() => ChipsComponent), // tslint:disable-line
  multi: true
};

/**
 * In order to add suggestion, we embed an autoComplete widget
 * and hijack some of the actions.
 *
 * A cleaner solution is to modify the multiple option that
 * comes as default with the p-autoComplete widget so that it can
 * also function as a chip without any suggestions. This requires
 * updating our primeng version. Pushing off till after FCS
 *
 * TODO: Update to use p-autoComplete as the component we are extending from
 * instead of pChips.
 */

@Component({
  selector: 'app-chips',
  templateUrl: './chips.component.html',
  styleUrls: ['./chips.component.scss'],
  encapsulation: ViewEncapsulation.None,
  providers: [CHIPS_VALUE_ACCESSOR]
})
export class ChipsComponent extends Chips implements OnInit, AfterViewInit, AfterContentInit {
  @ViewChild('autoCompleteWidget') autoComplete: AutoComplete;


  @Input() separatorKeyCodes: number[] = [ENTER, SPACE];
  // 64 is the max length of object names
  @Input() maxChipLength: number = Utility.MAX_OBJECT_NAME_LENGTH;
  @Input() useAutoComplete: boolean = false;
  @Input() autoCompleteField: string = '';
  @Input() autoCompleteOptions: any[] = [];

  mouseleavesTerms = new Subject<Event>();

  itemAutoCompleteTemplate: TemplateRef<any>;

  filteredOptions: any[] = [];

  filterOptions(event) {
    this.filteredOptions = [];
    this.autoCompleteOptions.forEach( (option) => {
      const val: string = Utility.getLodash().get(option, this.autoCompleteField);
      if (val.toLowerCase().startsWith(event.query.toLowerCase())) {
        this.filteredOptions.push(option);
      }
    });
  }

  ngOnInit(): void {
    this.mouseleavesTerms.debounceTime(500)
      .subscribe(() => {
        this.handleMouseLeaveEvent();
      });
  }

  ngAfterContentInit() {
    this.templates.forEach((item) => {
        switch (item.getType()) {
            case 'itemAutoComplete':
                this.itemAutoCompleteTemplate = item.template;
            break;
            case 'item':
                this.itemTemplate = item.template;
            break;

            default:
                this.itemTemplate = item.template;
            break;
        }
    });
  }

  ngAfterViewInit() {
    if (this.useAutoComplete) {

    this.inputViewChild = this.autoComplete.inputEL;
    const originalKeyDown = this.autoComplete.onKeydown;
    this.autoComplete.onKeydown = (event) => {
      // we reset this assignment on each keydown as the input El might have changed
      this.inputViewChild = this.autoComplete.inputEL;
      // If auto complete box is open and user navigation is inside
      // suggestions
      if (this.autoComplete.overlayVisible && this.autoComplete.highlightOption) {
        originalKeyDown.call(this.autoComplete, event);
      } else if (this.separatorKeyCodes.includes(event.which)) {
        this.onKeydown(event);
      } else if (BACKSPACE === event.which && this.inputViewChild.nativeElement.value.length === 0) {
        this.onKeydown(event);
      } else {
        originalKeyDown.call(this.autoComplete, event);
      }
      };
    }
  }

  itemSelect(value) {
    if (this.useAutoComplete) {
      this.inputViewChild = this.autoComplete.inputEL;
      if (this.inputViewChild.nativeElement.value.length !== 0) {
        this.addItem(event, this.inputViewChild.nativeElement.value);
        this.inputViewChild.nativeElement.value = '';
      }
    }
  }

  onClick(event) {
    if (this.useAutoComplete) {
      this.inputViewChild = this.autoComplete.inputEL;
    }
    this.inputViewChild.nativeElement.focus();
  }

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
    } else if (BACKSPACE === event.which) {
        if (this.inputViewChild.nativeElement.value.length === 0 && this.value && this.value.length > 0) {
            this.value = [...this.value];
            const removedItem = this.value.pop();
            this.onModelChange(this.value);
            this.onRemove.emit({
                originalEvent: event,
                value: removedItem
            });
        }
    } else {
      super.onKeydown(event);
    }
  }

  onInputMouseout(): void {
    this.mouseleavesTerms.next();
  }

  handleMouseLeaveEvent() {
    if (this.addOnBlur && this.inputViewChild.nativeElement.value) {
      this.addItem(event, this.inputViewChild.nativeElement.value);
      this.inputViewChild.nativeElement.value = '';
    }
    this.onModelTouched();
  }
}
