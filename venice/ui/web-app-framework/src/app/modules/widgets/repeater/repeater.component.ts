import { Component, EventEmitter, Input, OnInit, Output, SimpleChanges, OnChanges, ViewEncapsulation, ViewChild } from '@angular/core';
import { AbstractControl, FormArray, FormControl, FormGroup } from '@angular/forms';
import { SelectItem, MultiSelect } from 'primeng/primeng';
import { RepeaterData, RepeaterItem, ValueType } from './index';
import { Animations } from '../../../animations';
import * as _ from 'lodash';


/**
 * Creates field selectors that are related, [key, operator, value]
 * Once key is chosen, it fills operator and value with the possible options
 * The value can either be a single select, multi select, or input field
 *
 * Existing data must be passed in as an Angular FormArray.
 */

/**
 * 2018-07-26 update (Jeff)
 * Repeater is updated to support allowing user to enter text in "key/field".  User does not have to pick a field but enter text
 * See test-repeater.component.ts
     this.data2 = [
       // must have only one object.
       {
         key: { label: 'text', value: 'text' },
         operators: [
           { label: 'is', value: 'is' },
           { label: 'is not', value: 'is not' },
         ],
         fieldType: ValueType.inputField,
         valueType: ValueType.inputField
       }
     ];
 *
 * out put looks like
 * [{"keyFormControl":"text","operatorFormControl":"is","valueFormControl":"kv1-v","keytextFormName":"k1"},{"keyFormControl":"text","operatorFormControl":"is not","valueFormControl":"kv2-v","keytextFormName":"k2"}]
 */
@Component({
  selector: 'app-repeater',
  templateUrl: './repeater.component.html',
  styleUrls: ['./repeater.component.css'],
  animations: [Animations],
  encapsulation: ViewEncapsulation.None,
})
export class RepeaterComponent implements OnInit, OnChanges {
  // The data for filling the reactive dropdown options

  public static ALL_OPTIONS: SelectItem = {label : 'All', value : 'All'};
  @Input() data: RepeaterData[] = [];

  // names to use when looking up and setting values in the repeaters
  @Input() keyFormName: string = 'keyFormControl';
  @Input() operatorFormName: string = 'operatorFormControl';
  @Input() valueFormName: string = 'valueFormControl';
  @Input() keytextFormName: string = 'keytextFormName';

  // This indicator whether to show filter box in key combo. It is very helpful if the number of selections is large
  @Input() keyDropdownFilter = false;

  // Passed in data to load into the repeaters
  @Input() formArray: FormArray;

  // The follows are expected to be function objects. buildKeyPlaceholder(repeater: RepeaterItem, keyFormName: string): string
  @Input() buildKeyPlaceholder: (repeater: RepeaterItem, keyFormName: string) => string = null;
  @Input() buildValuePlaceholder: (repeater: RepeaterItem, keyFormName: string) => string = null;

  @Input() styleClass: string;
  @Input() panelStyleClass: string;

  @Input() customValueOnBlur: (any, RepeaterItem?) => void;
  @Input() customValueOnKeydown: (any, RepeaterItem?) => void;
  @Input() customKeyOnBlur: (any, RepeaterItem?) => void;
  @Input() customKeyOnKeydown: (any, RepeaterItem?) => void;
  @Input() enableMultiSelectAll: boolean = false;
  @Input() enableMultiSelectFilter: boolean = false;
  // Determine if we want to generate valueLabelToValueMap in buildValuePlaceholder
  @Input() valueLabelToValueMap: {};

  // Emits all the repeater values whenever there is a change
  @Output() repeaterValues: EventEmitter<any> = new EventEmitter();

  idCount = 0; // Counter so that newly created repeaters never have conflicting ids
  repeaterList: RepeaterItem[] = [];
  keyOptions: SelectItem[]; // Holds all key options

  keyToOperator: { [key: string]: SelectItem[]};
  keyToValues: { [key: string]: SelectItem[] };
  keyToValueType: { [key: string]: ValueType };
  keyToValueHintText: { [key: string]: string};  


  valueOnBlur($event: any, repeaterItem: RepeaterItem) {
    if (this.customValueOnBlur) {
      this.customValueOnBlur($event, repeaterItem);
    }
  }

  valueOnKeydown($event, repeaterItem: RepeaterItem) {
    if(this.customValueOnKeydown) {
      this.customValueOnKeydown($event, repeaterItem);
    }
  }

  keyOnBlur($event: any, repeaterItem: RepeaterItem) {
    if (this.customKeyOnBlur) {
      this.customKeyOnBlur($event, repeaterItem);
    }
  }

  keyOnKeydown($event, repeaterItem: RepeaterItem){
    if(this.customKeyOnKeydown) {
      this.customKeyOnKeydown($event, repeaterItem);
    }
  }

  constructor() { }

  getClassName(): string {
    return this.constructor.name;
  }

  ngOnInit() {
    // Unpacking data and setting maps for filling in options
    this.initData();
  }

  initData() {
    this.idCount = 0;
    this.repeaterList = [];
    this.keyOptions = [];

    this.keyToOperator = {};
    this.keyToValues = {};
    this.keyToValueType = {};
    this.keyToValueHintText = {};

    if (this.data == null || this.data.length === 0) {
      console.error('No repeater data was given');
      return;
    }

    // Unpacking data and setting maps for filling in options
    this.data.forEach((d) => {
      this.keyOptions.push(d.key);
      this.keyToOperator[d.key.value] = d.operators;
      this.keyToValueType[d.key.value] = d.valueType;
      // if value is a single select or multi select, we add to the keyToValues map
      if (d.valueType !== ValueType.inputField && d.values != null) {
        this.keyToValues[d.key.value] = d.values;
      } else {
        if (d.valueHintText != null) {
          this.keyToValueHintText[d.key.value] = d.valueHintText;
        } else {
          this.keyToValueHintText[d.key.value] = this.generatePlaceHolder(d);
        }
      }
    });

    // Always start with one repeater if given no existing data
    if (this.formArray == null || this.formArray.length === 0) {
      this.createRepeater();
    } else {
      try {
        this.loadRepeaters();
      } catch (error) {
        throw error;
      }
    }
  }

  ngOnChanges(changes: SimpleChanges) {
    if (changes.data) {
      // Since the options have all changed, we reset
      // the entire component
      this.initData();
    }
  }

  generatePlaceHolder(item): string {
    return 'Enter ' + item.key.value;
  }

  private getRepeaterItemData(repeater: RepeaterItem): any {
    const key = repeater.formGroup.value[this.keyFormName];
    return this.getSourceDataItem(key);
  }

  private getSourceDataItem(key: string): any {
    for (let i = 0; i < this.data.length; i++) {
      if (key === this.data[i].key.value) {
        return this.data[i];
      }
    }
    return null;
  }

  isFloatLabelSupported(repeater: RepeaterItem): boolean {
    const repeaterData = this.getRepeaterItemData(repeater);
    return repeaterData.keyLabel !== null && repeaterData.keyLabel !== '' &&
            repeaterData.valueLabel != null && repeaterData.valueLabel != '';
  }

  getKeyLabel(repeater: RepeaterItem): string {
    const repeaterData = this.getRepeaterItemData(repeater);
    return this.isFloatLabelSupported(repeater) ? repeaterData.keyLabel : '';
  }

  getValueLabel(repeater: RepeaterItem): string {
    const repeaterData = this.getRepeaterItemData(repeater);
    return this.isFloatLabelSupported(repeater) ? repeaterData.valueLabel : '';
  }

  isToUseInputField(repeater: RepeaterItem): boolean {
    const repeaterData = this.getRepeaterItemData(repeater);
    if (repeaterData) {
      if (repeaterData['fieldType'] === 'inputField') {
        return true;
      }
    }
    return false;
  }

  isTouseInputFieldForKey(key: string) {
    const repeaterData = this.getSourceDataItem(key);
    if (repeaterData) {
      if (repeaterData['fieldType'] === 'inputField') {
        return true;
      }
    }
    return false;
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
    };
    newRepeater.formGroup.addControl(this.keyFormName, new FormControl(key));
    newRepeater.formGroup.addControl(this.operatorFormName, new FormControl(operator));
    newRepeater.formGroup.addControl(this.valueFormName, new FormControl(value));
    if (this.isTouseInputFieldForKey(key)) {
      newRepeater.formGroup.addControl(this.keytextFormName, new FormControl(''));
    }

    this.repeaterList.push(newRepeater);
    if (shouldEmit) {
      this.emitValues();
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
    repeater.formGroup.patchValue(patchObj);

    this.emitValues();
  }


  addRepeater() {
    this.createRepeater();
    this.emitValues();
  }

  removeRepeater(id: number) {
    // must always have one repeater shown for the UI
    if (this.repeaterList.length > 1) {
      let index = -1;
      for (let i = 0; i < this.repeaterList.length; i++) {
        if (this.repeaterList[i].id === id) {
          index = i;
          break;
        }
      }

      if (index !== -1) {
        this.removeFormArray(this.repeaterList[index]);
        this.repeaterList.splice(index, 1);
        this.emitValues();
      }
    }
  }

  removeFormArray(repeater: RepeaterItem) {
    if (this.formArray && repeater.formArrayIndex >= 0) {
      this.formArray.controls.splice(repeater.formArrayIndex, 1);
    }
  }

  /**
   * Emits the current value, as well as writes to the form control
   */
  emitValues() {
    const formGroups = this.getValues();
    this.repeaterValues.emit(formGroups);
  }

  public getValues(): any[] {
    const formGroups = [];
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
    });
    return formGroups;
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
    const operator = control.value[this.operatorFormName];
    const value = control.value[this.valueFormName];
    if (key == null) {
      throw new Error('key cannot be blank');
    }
    let keyContained = false;
    this.keyOptions.forEach((keyItem) => {
      if (keyItem.value === key) {
        keyContained = true;
      }
    });
    if (!keyContained) {
      throw new Error('key is not part of the known keys supplied');
    }

    if (operator != null) {
      const operators = this.keyToOperator[key];
      let operatorContained = false;
      operators.forEach((operatorItem: SelectItem) => {
        if (operatorItem.value === operator) {
          operatorContained = true;
        }
      });
      if (!operatorContained) {
        throw new Error('operator is not part of the known operators supplied');
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
      });
      if (!valueContained) {
        throw new Error('value is not part of the known values supplied');
      }
    } else {
      value.forEach(val => {
        valueContained = false;
        this.keyToValues[key].forEach((valueItem: SelectItem) => {
          if (valueItem.value === val) {
            valueContained = true;
          }
        });
        if (!valueContained) {
          throw new Error('value is not part of the known values supplied');
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
    // et err = null;
    if (this.formArray != null && this.formArray.length !== 0) {
      this.formArray.controls.forEach((control: AbstractControl, index: number) => {
        // Check that the control's key value exists in our set of keys
        this.validateControl(control);

        if (index >= this.repeaterList.length) {
          this.createRepeater(false);
        }
        this.repeaterList[index].formGroup.patchValue(control.value);

      });

      // Repatch given values, as the operator and value may have been changed
      this.formArray.controls.forEach((control: AbstractControl, index: number) => {
        this.repeaterList[index].formGroup.patchValue(control.value);
        this.repeaterList[index].formArrayIndex = index;
      });
      this.emitValues();
    }
  }

  placeholderForKey(repeater, keyFormName): string {
    if (this.isFloatLabelSupported) {
      return '';
    }
    if (this.buildKeyPlaceholder) {
      return this.buildKeyPlaceholder(repeater, keyFormName);
    }
    return this.keyToValueHintText[repeater.formGroup.value[keyFormName]];
  }

  placeholderForValue(repeater, keyFormName): string {
    if (this.isFloatLabelSupported) {
      return '';
    }
    if (this.buildValuePlaceholder) {
      if (this.valueLabelToValueMap) {
        const key = repeater.formGroup.value[keyFormName];
        if (this.valueLabelToValueMap[key]) {
          const repeaterCopy = _.cloneDeep(repeater);
          repeaterCopy.formGroup.value[keyFormName] = this.valueLabelToValueMap[key];
          return this.buildValuePlaceholder(repeaterCopy, keyFormName);
        }
        return this.buildValuePlaceholder(repeater, keyFormName);
      } else {
        return this.buildValuePlaceholder(repeater, keyFormName);
      }
    }
    return this.keyToValueHintText[repeater.formGroup.value[keyFormName]];
  }

  /**
   * This API serves html template
   * @param event
   *
   * If "enter" key is down, add a repeater
   */
  onAddRepeaterKeydown(event) {
    if (event && event.which === 13) {
      this.addRepeater();
    }
  }

  /**
   * This API serves html template
   * @param event
   *
   * If "enter" key is down, add a repeater
   */
  onRomveRepeaterKeydown(event, repeaterId: any) {
    if (event && event.which === 13) {
      this.removeRepeater(repeaterId);
    }
  }
}
