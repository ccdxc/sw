import { TableCol, CustomExportMap } from '.';
import { Utility } from '@app/common/Utility';
import { PrettyDatePipe } from '../Pipes/PrettyDate.pipe';

export class TableUtility {

  public static displayColumn(data, col, hasUiHintMap: boolean = true): any {
    const fields = col.field.split('.');
    const value = Utility.getObjectValueByPropertyPath(data, fields, hasUiHintMap);
    const column = col.field;
    switch (column) {
      case 'modification-time':
      case 'creation-time':
        return new PrettyDatePipe('en-US').transform(value);
      default:
        return Array.isArray(value) ? JSON.stringify(value) : value;
    }
  }

  public static exportTableCSV(columns: TableCol[], data, filename: string, customExportMap: CustomExportMap = {}, csvSeparator = ',', hasUiHintMap: boolean = true) {
    const csv = this.buildCSV(columns, csvSeparator, data, customExportMap, hasUiHintMap);
    this.invokeDownload(csv.toString(), filename, 'csv');
  }
  public static exportTableJSON(columns: TableCol[], data, filename: string, customExportMap: CustomExportMap = {}, csvSeparator = ',', hasUiHintMap: boolean = true) {
    const json = this.buildJSON(columns, data, customExportMap, hasUiHintMap);
    this.invokeDownload(JSON.stringify(json, null, 5), filename, 'json');
    }
  public static buildCSV(columns: TableCol[], csvSeparator: string, data: any, customExportMap: CustomExportMap, hasUiHintMap: boolean) {
    let csv = '\ufeff';
    // headers
    for (let i = 0; i < columns.length; i++) {
      const column = columns[i];
      if (column.exportable !== false && column.field) {
        csv += '"' + (column.header || column.field) + '"';
        if (i < (columns.length - 1)) {
          csv += csvSeparator;
        }
      }
    }

    // body
    data.forEach((record, ii) => {
      csv += '\n';
      for (let i = 0; i < columns.length; i++) {
        const column = columns[i];
        if (column.exportable !== false && column.field) {
          let cellData = Utility.getLodash().get(record, column.field);
          // Priority 1) check for custom export settings
          const customExportFunc = customExportMap[column.field];
          if (customExportFunc) {
            cellData = customExportFunc({
              data: record,
              field: column.field
            });
          } else {
            //  Priority 2) keep value from table data
            if (!!cellData) {
              // Priority 3) fetch value from ui hint
              cellData = TableUtility.displayColumn(record, column, hasUiHintMap);
            }
          }
          if (cellData == null) {
            // Priority 4) if nothing else, set value to empty string
            cellData = '';
          }
          if (typeof cellData === 'object') {
            cellData = JSON.stringify(cellData).replace(/"/g, '""');
          } else {
            cellData = String(cellData).replace(/"/g, '""');
          }
          csv += '"' + cellData + '"';
          if (i < (columns.length - 1)) {
            csv += csvSeparator;
          }
        }
      }
    });
    return csv;
  }
  public static buildJSON(columns: TableCol[], data: any[], customExportMap: CustomExportMap, hasUiHintMap: boolean) {
    let csv = '\ufeff';
    const output = [];
    const headers: string[] = [];
    // headers
    for (let i = 0; i < columns.length; i++) {
      const column = columns[i];
      if (column.exportable !== false && column.field) {
        csv += '"' + (column.header || column.field) + '"';
        headers.push((column.header || column.field));
      }
    }
    // body
    data.forEach((record, ii) => {
      const rec = {};
      csv += '\n';
      for (let i = 0; i < columns.length; i++) {
        const column = columns[i];
        if (column.exportable !== false && column.field) {
          let cellData = Utility.getLodash().get(record, column.field);
          // Priority 1) check for custom export settings
          const customExportFunc = customExportMap[column.field];
          if (customExportFunc) {
            cellData = customExportFunc({
              data: record,
              field: column.field
            });
          } else {
            //  Priority 2) keep value from table data
            if (!!cellData) {
              // Priority 3) fetch value from ui hint
              cellData = TableUtility.displayColumn(record, column, hasUiHintMap);
            }
          }
          if (cellData == null) {
            // Priority 4) if nothing else, set value to empty string
            cellData = '';
          }
          if (typeof cellData === 'object') {
            cellData = JSON.stringify(cellData).replace(/"/g, '""');
          } else {
            cellData = String(cellData).replace(/"/g, '""');
          }
            csv += '"' + cellData + '"';
          if (!Utility.isEmpty(headers[i])) {
            rec[headers[i]] = cellData;
          }
        }
      }
      output.push(rec);
    });
    return output;
  }
  public static invokeDownload(content: string, filename: any, filetype: string = 'csv') {
    const blob = new Blob([content], {
      type: 'text/' + filetype + ';charset=utf-8;' // it looks like 'text/csv;charset=utf-8;'
    });
    if (window.navigator.msSaveOrOpenBlob) {
      navigator.msSaveOrOpenBlob(blob, filename + '.csv');
    } else {
      const link = document.createElement('a');
      link.style.display = 'none';
      document.body.appendChild(link);
      if (link.download !== undefined) {
        link.setAttribute('href', URL.createObjectURL(blob));
        //  link.setAttribute('download', filename + '.csv');
        link.setAttribute('download', filename + '.' + filetype); // xxx.csv
        link.click();
      } else {
        content = 'data:text/' + filetype + ';charset=utf-8,' + content; // looks like 'data:text/csv;charset=utf-8,' + csv;
        window.open(encodeURI(content));
      }
      document.body.removeChild(link);
    }
    return content;
  }
}
