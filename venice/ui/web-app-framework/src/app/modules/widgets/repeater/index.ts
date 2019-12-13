import { FormGroup } from '@angular/forms';
import { SelectItem } from 'primeng/primeng';

export enum ValueType {
  singleSelect = 'singleSelect',
  multiSelect = 'multiSelect',
  inputField = 'inputField'
}

export interface RepeaterData {
  key: SelectItem;
  operators: SelectItem[];
  values?: SelectItem[];
  valueType: ValueType;
  // If valueType is inputField
  fieldType?: ValueType;
  // If fieldType is inputField
  valueHintText?: string;
  // added here to supoort float label
  keyLabel?: string;
  valueLabel?: string;
}

export interface RepeaterItem {
  id: number;
  formGroup: FormGroup;
  // we build repeater from formArray. The generated repeater can link back to formArray
  formArrayIndex?: number;
}


export interface RepeaterItemUnique {
  id: number;
  formGroup: FormGroup;
  keyOptions: any[];
}
