import { Component, forwardRef, Input, OnInit } from '@angular/core';
import { ChipsComponent } from './chips.component';
import { NG_VALUE_ACCESSOR } from '@angular/forms';
import { AbstractControl, ValidatorFn, ValidationErrors } from '@angular/forms';
import { Utility } from '@app/common/Utility';

export const CHIPS_VALUE_ACCESSOR: any = {
  provide: NG_VALUE_ACCESSOR,
  useExisting: forwardRef(() =>FormChipsComponent), // tslint:disable-line
  multi: true
};

/**
 * This componet is a wrap on app-chips
 * it adds form binding to the from control
 * it taks one validFn for each ip and validate all chips
 *
 */


@Component({
  selector: 'app-formchips',
  template: `
    <span>
      <app-chips [addOnBlur]="true" [addOnTab]="true" [styleClass]="boxStyleClass"
          [ngClass]="{
            boxStyleClass: true,
            'app-chips-focused': isFocused,
            'app-chips-noneEmpty': !isFieldEmpty()
          }"
          [appFloatLabel]="chipsLabel"
          (onFocus)="onChipsFocus()" (onBlur)="onChipsBlur()"
          [matTooltip]="toolTip" [matTooltipClass]="tooltipClass"
          [formControl]="formControl" [placeholder]="placeholder">
        <ng-template let-item let-cancelFunc="cancelFunc" pTemplate="item">
          <div fxLayout="row" fxLayoutAlign="start center" class="ui-chips-token-item"
              [ngClass]="{'error': !itemValidator(item)}">
          <div fxFlex="none" class="ui-chips-token-item-text">{{item}}</div>
          <mat-icon fxFlex="none" class="ui-chips-token-icon-cancel"
              (click)="cancelFunc($event)">close</mat-icon>
          </div>
        </ng-template>
      </app-chips>
    </span>
  `,
  providers: [CHIPS_VALUE_ACCESSOR]
})

export class FormChipsComponent extends ChipsComponent  implements OnInit {
  @Input() itemValidator: (item: string) => boolean;
  @Input() itemErrorMsg: string = '';
  @Input() boxStyleClass: string = '';
  @Input() chipsLabel: string = '';
  @Input() toolTip: string = '';
  @Input() placeholder: string = '';
  @Input() formControl: AbstractControl;

  isFocused: boolean = false;
  originalTooltip: string = '';
  tooltipClass: string = 'chips_tooltip';


  ngOnInit() {
    if (!this.formControl.validator) {
      this.formControl.setValidators([this.isFieldValid()]);
    }
    if (this.chipsLabel) {
      this.placeholder = '';
    }
    this.originalTooltip = this.toolTip;
  }

  isFieldValid(): ValidatorFn {
    return (control: AbstractControl): ValidationErrors | null => {
      const arr: string[] = control.value;
      if (!arr) {
        return null;
      }
      for (let i = 0; i < arr.length; i++) {
        if (arr[i] && !this.itemValidator(arr[i])) {
          this.tooltipClass = 'chips_error_tooltip';
          this.toolTip = this.itemErrorMsg;
          return {
            field: {
              required: false,
              message: this.itemErrorMsg
            }
          };
        }
      }
      this.tooltipClass = 'chips_tooltip';
      this.toolTip = this.originalTooltip;
      return null;
    };
  }

  onChipsFocus() {
    this.isFocused = true;
  }

  onChipsBlur() {
    this.isFocused = false;
  }

  isFieldEmpty(): boolean {
    return Utility.isEmpty(this.formControl.value);
  }
}
