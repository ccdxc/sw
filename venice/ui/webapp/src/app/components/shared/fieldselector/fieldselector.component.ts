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

  @Output() repeaterValues: EventEmitter<any> = new EventEmitter();
  fieldData: RepeaterData[] = [];

  constructor() { }

  ngOnInit() {
    if (!this.kind) {
      this.setDefaultData();
    } else {
      this.genFieldData();
    }
  }

  ngOnChanges(changes: SimpleChanges) {
  }

  getValues() {
    return this.fieldRepeater ? this.formatRepeaterData(this.fieldRepeater.getValues()) : [];
  }

  /**
   * If the user hasn't specified a value, we don't add
   * it into the values we return to the consumer.
   * If a field is free form text, we split by comma to turn
   * it into an array of values
   */
  formatRepeaterData(data) {
    if (data == null) {
      return null;
    }
    let retData = data.filter((item) => {
      return item[this.valueFormName] != null && item[this.valueFormName].length !== 0;
    });
    // make sure the value field is an array
    retData = retData.map((item) => {
      if (item[this.valueFormName] instanceof Array) {
        return item;
      } else {
        const arrVal = item[this.valueFormName].split(',');
        item[this.valueFormName] = arrVal.map(val => {
          return val.trim();
        });
      }
    });
    return retData;
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
      if (modelData[key] instanceof Object) {
        // Has nested object
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
    if (key === SearchUtil.SEARCHFIELD_META + '.labels' ||
      key === SearchUtil.SEARCHFIELD_META + '.self-link') {
      return false;
    }
    return true;
  }

  getFieldOperators(kind: string, keys: string[]): any[] {
    return SearchUtil.getOperators(kind, keys);
  }

  /**
   * Buidl place-holder text for repeater-item
   *
   * @param repeater
   * @param keyFormName
   */
  buildFieldValuePlaceholder(repeater: RepeaterItem, keyFormName: string) {
    // TODO: may change this once we have enhanced category-mapping.ts
    const key = repeater.formGroup.value[keyFormName];
    if (key.startsWith(SearchUtil.SEARCHFIELD_META)) {
      if (key.indexOf('time') > -1) {
        return 'YYYY-MM-DDTHH:mm:ss.sssZ';
      }
    }
    if (key.startsWith(SearchUtil.SEARCHFIELD_SPEC)) {
      if (key.indexOf('-ip') > -1) {
        return 'xxx.xxx.xxx.xxx';
      }
    }
    if (key.startsWith(SearchUtil.SEARCHFIELD_STATUS)) {
      if (key.indexOf('time') > -1) {
        return 'YYYY-MM-DDTHH:mm:ss.sssZ';
      }
      if (key.indexOf('date') > -1) {
        return 'YYYY-MM-DD';
      }
    }
    return key;
  }


}
