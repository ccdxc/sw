import { Component, EventEmitter, Input, OnInit, Output, ViewEncapsulation } from '@angular/core';
import { AbstractControl, FormArray, FormControl, FormGroup } from '@angular/forms';
import { SelectItem } from 'primeng/primeng';
import { RepeaterData, RepeaterItem, ValueType } from './index';


/**
 * Creates field selectors that are related, [key, operator, value]
 * Once key is chosen, it fills operator and value with the possible options
 * The value can either be a single select, multi select, or input field
 * 
 * Existing data must be passed in as an Angular FormArray.
 */
@Component({
  selector: 'app-repeater',
  templateUrl: './repeater.component.html',
  styleUrls: ['./repeater.component.css'],
  encapsulation: ViewEncapsulation.None,
})
export class RepeaterComponent implements OnInit {
  // The data for filling the reactive dropdown options 
  @Input() data: RepeaterData[] = [];

  // names to use when looking up and setting values in the repeaters
  @Input() keyFormName: string = 'keyFormControl';
  @Input() operatorFormName: string = 'operatorFormControl';
  @Input() valueFormName: string = 'valueFormControl';

  // Passed in data to load into the repeaters
  @Input() formArray: FormArray;

  // Emits all the repeater values whenever there is a change
  @Output() repeaterValues: EventEmitter<any> = new EventEmitter();

  idCount = 0; // Counter so that newly created repeaters never have conflicting ids
  repeaterList: RepeaterItem[] = [];
  keyOptions: SelectItem[] = []; // Holds all key options

  keyToOperator: { [key: string]: SelectItem[] } = {};
  keyToValues: { [key: string]: SelectItem[] } = {};
  keyToValueType: { [key: string]: ValueType } = {};
  keyToValueHintText: { [key: string]: string } = {};

  constructor() { }

  ngOnInit() {
    // Unpacking data and setting maps for filling in options
    this.data.forEach((d) => {
      this.keyOptions.push(d.key);
      this.keyToOperator[d.key.value] = d.operators;
      this.keyToValueType[d.key.value] = d.valueType;
      // if value is a single select or multi select, we add to the keyToValues map
      if (d.valueType != ValueType.inputField && d.values != null) {
        this.keyToValues[d.key.value] = d.values;
      } else {
        if (d.valueHintText != null) {
          this.keyToValueHintText[d.key.value] = d.valueHintText;
        } else {
          this.keyToValueHintText[d.key.value] = 'Enter ' + d.key.value;
        }
      }
    })

    // Always start with one repeater if given no existing data
    if (this.formArray == null || this.formArray.length == 0) {
      this.createRepeater();
    } else {
      this.loadRepeaters();
    }
  }

  /**
   * Creates a new repeater with the first available key selected, its first operator
   * selected, and the first value selected if it is a single select field
   * 
   * @param shouldEmit Whether to emit a value change and write to the form array
   */
  private createRepeater(shouldEmit = true) {
    // getting key options
    const key = this.keyOptions[0].value;
    // set initial operator value
    const operator = this.keyToOperator[key][0].value;
    let value = null;
    // only set initial value for the value field if its a single select
    if (this.keyToValueType[key] === ValueType.singleSelect) {
      value = this.keyToValues[key][0].value;
    }

    const newRepeater: RepeaterItem = {
      id: this.idCount++,
      formGroup: new FormGroup({
      }),
    }
    newRepeater.formGroup.addControl(this.keyFormName, new FormControl(key));
    newRepeater.formGroup.addControl(this.operatorFormName, new FormControl(operator));
    newRepeater.formGroup.addControl(this.valueFormName, new FormControl(value));

    this.repeaterList.push(newRepeater);
    if (shouldEmit) {
      this.emitValues()
    }
  }

  /**
   * Updates options for all repeaters when a key dropdown changes value
   */
  keyDropdownChange(repeater: RepeaterItem) {
    // Set default values of the repeater's operator and valueForm
    const key = repeater.formGroup.value[this.keyFormName];
    const operator = this.keyToOperator[key][0].value;
    let valueOption = null;
    // only set initial value for the value field if its a single select
    if (this.keyToValueType[key] === ValueType.singleSelect) {
      valueOption = this.keyToValues[key][0].value;
    }
    const patchObj = {};
    patchObj[this.operatorFormName] = operator;
    patchObj[this.valueFormName] = valueOption;
    repeater.formGroup.patchValue(patchObj)

    this.emitValues();
  }


  addRepeater() {
    this.createRepeater();
    this.emitValues()
  }

  removeRepeater(id: number) {
    // must always have one repeater shown for the UI
    if (this.repeaterList.length > 1) {
      let index = -1;
      for (let i = 0; i < this.repeaterList.length; i++) {
        if (this.repeaterList[i].id == id) {
          index = i;
          break;
        }
      }

      if (index != -1) {
        this.repeaterList.splice(index, 1)[0];
        this.emitValues()
      }
    }
  }

  /**
   * Emits the current value, as well as writes to the form control
   */
  emitValues() {
    let formGroups = [];
    this.repeaterList.forEach((repeater, index) => {
      formGroups.push(repeater.formGroup.value);

      // Setting values of the formArray
      if (this.formArray != null) {
        // Can only be one off, as we are counting up from 0
        if (index >= this.formArray.length) {
          this.formArray.insert(index, new FormControl(repeater.formGroup.value));
        } else {
          // We remove the control instead of patching, as if it has a nested
          // formArray, Angular will not reset the number of controls associated with 
          // the array, only the value. The suggested solution from the Angular team is to 
          // replace it with a new FormArray.
          this.formArray.removeAt(index);
          this.formArray.insert(index, new FormControl(repeater.formGroup.value));
        }
      }
    })
    this.repeaterValues.emit(formGroups);
  }

  /**
   * Validates the data of the passed in control
   * The Key, operator, and possibly value should be part of the 
   * repeater data
   * .
   * @param control FormControl or FormGroup to be loaded
   */
  validateControl(control: AbstractControl) {
    const key = control.value[this.keyFormName];
    const operator = control.value[this.operatorFormName]
    const value = control.value[this.valueFormName]
    if (key == null) {
      throw new Error("key cannot be blank");
    }
    let keyContained = false;
    this.keyOptions.forEach((keyItem) => {
      if (keyItem.value == key) {
        keyContained = true;
      }
    });
    if (!keyContained) {
      throw new Error("key is not part of the known keys supplied");
    }

    if (operator != null) {
      const operators = this.keyToOperator[key];
      let operatorContained = false;
      operators.forEach((operatorItem: SelectItem) => {
        if (operatorItem.value === operator) {
          operatorContained = true;
        }
      })
      if (!operatorContained) {
        throw new Error("operator is not part of the known operators supplied");
      }
    }

    if (this.keyToValueType[key] === ValueType.inputField || value == null) {
      return true;
    }

    let valueContained;
    if (this.keyToValueType[key] === ValueType.singleSelect) {
      valueContained = false;
      this.keyToValues[key].forEach((valueItem: SelectItem) => {
        if (valueItem.value === value) {
          valueContained = true;
        }
      })
      if (!valueContained) {
        throw new Error("value is not part of the known values supplied");
      }
    } else {
      value.forEach(value => {
        valueContained = false;
        this.keyToValues[key].forEach((valueItem: SelectItem) => {
          if (valueItem.value === value) {
            valueContained = true;
          }
        })
        if (!valueContained) {
          throw new Error("value is not part of the known values supplied");
        }
        valueContained = false;
      });
    }

    return true;
  }

  /**
   * load data from the given form array into the repeaters
   */
  loadRepeaters() {
    if (this.formArray != null && this.formArray.length != 0) {
      this.formArray.controls.forEach((control: AbstractControl, index: number) => {
        // Check that the control's key value exists in our set of keys
        this.validateControl(control);

        // Index should only be one off from actual length, 
        // we create a new repeater for it if needed
        if (index >= this.repeaterList.length) {
          this.createRepeater(false);
        }
        this.repeaterList[index].formGroup.patchValue(control.value);
      });

      // Repatch given values, as the operator and value may have been changed
      this.formArray.controls.forEach((control: AbstractControl, index: number) => {
        this.repeaterList[index].formGroup.patchValue(control.value);
      });
      this.emitValues();
    }
  }
}
