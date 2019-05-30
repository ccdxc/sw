
export interface TableCol {
  field: string;
  header: string;
  sortable?: boolean;
  class?: string;
  width: number;
  exportable?: boolean;
  disableSearch?: boolean;
  kind?: string;
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
