import { Component, OnInit, ViewEncapsulation, Input, OnChanges, SimpleChanges, Output, EventEmitter, ViewChild } from '@angular/core';
import { SearchUtil } from '@app/components/search/SearchUtil';
import { ValueType, RepeaterData, RepeaterItem, RepeaterComponent } from 'web-app-framework';
import { Utility } from '@app/common/Utility';
import { FormArray, FormGroup, FormControl } from '@angular/forms';

/**
 * Given a kind, will build a field selector for the attributes of that kind.
 *
 * If the passed in data array has its value control as an array instead of a string,
 * we modify the control to be a string instead of an array.
 * When the consumer is ready to use the value of this field selector, they should call
 * the getValues function which will return the values with the value field as an array instead of a string.
 * See NeweventalertpolicyComponent for an exaple.
 */

@Component({
  selector: 'app-fieldselector',
  templateUrl: './fieldselector.component.html',
  styleUrls: ['./fieldselector.component.scss'],
  encapsulation: ViewEncapsulation.None
})
export class FieldselectorComponent implements OnInit, OnChanges {
  @ViewChild('fieldRepeater') fieldRepeater: RepeaterComponent;

  @Input() kind: string;
  @Input() formArray = new FormArray([]);
  @Input() keyFormName: string = 'keyFormControl';
  @Input() operatorFormName: string = 'operatorFormControl';
  @Input() valueFormName: string = 'valueFormControl';
  @Input() keytextFormName: string = 'keytextFormName';

  fieldData: RepeaterData[] = [];
  buildFieldValuePlaceholder = SearchUtil.buildFieldValuePlaceholder;

  // TODO: Default repopulated unacceptable events keys
  @Input() unacceptableKeys: string[] = ['meta.tenant', 'object-ref.tenant', 'meta.labels', 'meta.self-link'];

  @Output() repeaterValues: EventEmitter<any> = new EventEmitter();
  constructor() { }

  ngOnInit() {
    if (!this.kind) {
      this.setDefaultData();
    } else {
      this.genFieldData();
    }
  }

  ngOnChanges(changes: SimpleChanges) {
    if (changes.formArray) {
      if (this.formArray != null && this.formArray.length != null) {
        this.formArray.controls.forEach((control) => {
          const values = control.value;
          if (values[this.operatorFormName] === 'equals') {
            values[this.operatorFormName] = 'in';
            control.patchValue(values);
          } else if (control.value[this.operatorFormName] === 'notEquals') {
            values[this.operatorFormName] = 'notIn';
            control.patchValue(values);
          }
        });
      }

      if (!this.kind) {
        this.setDefaultData();
      } else {
        this.genFieldData();
      }
    }
  }

  getValues() {
    return this.fieldRepeater ? Utility.formatRepeaterData(this.fieldRepeater.getValues(), this.valueFormName) : [];
  }


  emitRepeaterValues(values) {
    this.repeaterValues.emit(this.getValues());
  }

  setDefaultData() {
    this.fieldData = [
      {
        key: { label: 'name', value: 'name' },
        operators: SearchUtil.stringOperators,
        valueType: ValueType.inputField
      },
      {
        key: { label: 'tenant', value: 'tenant' },
        operators: SearchUtil.stringOperators,
        valueType: ValueType.inputField
      },
      {
        key: { label: 'namespace', value: 'namespace' },
        operators: SearchUtil.stringOperators,
        valueType: ValueType.inputField
      },
      {
        key: { label: 'creation-time', value: 'creation-time' },
        operators: SearchUtil.numberOperators,
        valueType: ValueType.inputField
      },
      {
        key: { label: 'modified-time', value: 'mod-time' },
        operators: SearchUtil.numberOperators,
        valueType: ValueType.inputField
      }
    ];
  }

  genFieldData() {
    const modelData = SearchUtil.getModelInfoByKind(this.kind);
    this.fieldData = this.getFieldDataHelper(this.kind, modelData);
  }

  /**
   * Recursively walks through and if there is a nested object
   * flattens it.
   * @param kind
   * @param modelData
   */
  getFieldDataHelper(kind: string, modelData, prefix: string[] = []): any[] {
    const _ = Utility.getLodash();
    let returnData = [];
    Object.keys(modelData).forEach((key) => {
      const newPrefix = _.cloneDeep(prefix);
      newPrefix.push(key);
      const currentPath = newPrefix.join('.');
      if (modelData[key] instanceof Object && !Array.isArray(modelData[key])) {
        // Has nested object and it is not an array.  (FieldSelector on AuthUser, status.authenticators=['local']. We don't want to traverse to "local" layer)
        const nestedFieldData = this.getFieldDataHelper(kind, modelData[key], newPrefix);
        returnData = returnData.concat(nestedFieldData);
      } else {
        if (this.acceptField(currentPath)) {
          const config = {
            key: {
              label: currentPath,
              value: currentPath,
            },
            operators: this.getFieldOperators(kind, newPrefix),
            values: null,
            valueType: null
          };
          const propInfo = modelData.getPropInfo(key);
          if (propInfo.enum != null) {
            config.values = Utility.convertEnumToSelectItem(propInfo.enum);
            config.valueType = ValueType.multiSelect;
          } else {
            config.valueType = ValueType.inputField;
          }
          returnData.push(config);
        }
      }
    });
    return returnData;
  }

  acceptField(key): boolean {
    return (this.unacceptableKeys.includes(key)) ? false : true;
  }

  getFieldOperators(kind: string, keys: string[]): any[] {
    return SearchUtil.getOperators(kind, keys);
  }
}
