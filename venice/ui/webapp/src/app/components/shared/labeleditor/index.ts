import {ApiObjectMeta} from '@sdk/v1/models/generated/cluster';

export interface VeniceObject {
  'kind'?: string;
  'api-version'?: string;
  'meta'?: ApiObjectMeta;
}

export interface LabelEditorModel {
  key: string;
  value: any;
  keyEditable?: boolean;  // TODO: needs to be passed in somewhere in future
  valueEditable?: boolean;  // TODO: needs to be passed in somewhere in future
  class?: string;  // row class
  keyClass?: string;
  valueClass?: string;
  valueParentClass?: string;
  deletable?: boolean;
  element?: string;  // TODO: at this time we only accept input as the label.
  inputType?: string;
  min?: number;
  max?: number;
  step?: number;
  keyPlaceholder?: string;
  valuePlaceholder?: string;
}

export interface LabelEditorMetadataModel {
  title?: string;
  titleClass?: string;
  keysEditable?: boolean;  // TODO: needs to be passed in somewhere in future
  valuesEditable?: boolean;  // TODO: needs to be passed in somewhere in future
  propsDeletable?: boolean;  // TODO: needs to be passed in somewhere in future
  keysClasses?: string;
  PropsClasses?: string;
  valuesParentsClasses?: string;
  valuesClasses?: string;
  extendable?: boolean;  // allowing add more kv on labels.
  save: boolean;
  cancel: boolean;
  saveTxt?: string;
  cancelTxt?: string;
}
