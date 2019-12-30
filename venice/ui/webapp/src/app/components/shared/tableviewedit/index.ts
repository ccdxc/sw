import { FieldsRequirement } from '@sdk/v1/models/generated/search';

export interface TableCol {
  field: string;
  header: string;
  sortable?: boolean;
  class?: string;
  width?: number | string;
  minColumnWidth?: number;
  exportable?: boolean;
  disableSearch?: boolean;
  localSearch?: boolean;
  kind?: string;
  roleGuard?: string;
  notReorderable?: boolean ;
  filterfunction?: (reqLocal: FieldsRequirement, data: any) => any[] ;
  advancedSearchOperator?: any[];
}

export interface CustomExportFunctionOpts {
  data: any;
  field: string;
}

export type CustomExportFunction = (opts: CustomExportFunctionOpts) => string;

export interface CustomExportMap { [fieldname: string]: CustomExportFunction; }

export interface RowClickEvent {
  event: any;
  rowData: any;
}
