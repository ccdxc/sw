import { FormGroup } from "@angular/forms";
import { SelectItem } from "primeng/primeng";

export enum ValueType {
  singleSelect = 'singleSelect',
  multiSelect = 'multiSelect',
  inputField = 'inputField'
}

export interface RepeaterData {
  key: SelectItem,
  operators: SelectItem[],
  values?: SelectItem[],
  valueType: ValueType,
  // If valueType is inputField
  valueHintText?: string,
}

export interface RepeaterItem {
  id: number;
  formGroup: FormGroup,
}

export interface RepeaterItemUnique {
  id: number;
  formGroup: FormGroup,
  keyOptions: any[];
}
