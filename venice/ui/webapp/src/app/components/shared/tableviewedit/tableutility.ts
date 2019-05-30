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
        return Array.isArray(value) ? JSON.stringify(value, null, 2) : value;
    }
  }

  public static exportTable(columns: TableCol[], data, filename, customExportMap: CustomExportMap = {}, csvSeparator = ',', hasUiHintMap: boolean = true) {
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

          if (cellData != null) {
            const customExportFunc = customExportMap[column.field];
            if (customExportFunc) {
              cellData = customExportFunc({
                data: record,
                field: column.field
              });
            } else {
              cellData = TableUtility.displayColumn(record, column, hasUiHintMap);
            }
          } else {
            cellData = '';
          }
          cellData = String(cellData).replace(/"/g, '""');


          csv += '"' + cellData + '"';

          if (i < (columns.length - 1)) {
            csv += csvSeparator;
          }
        }
      }
    });

    const blob = new Blob([csv], {
      type: 'text/csv;charset=utf-8;'
    });

    if (window.navigator.msSaveOrOpenBlob) {
      navigator.msSaveOrOpenBlob(blob, filename + '.csv');
    } else {
      const link = document.createElement('a');
      link.style.display = 'none';
      document.body.appendChild(link);
      if (link.download !== undefined) {
        link.setAttribute('href', URL.createObjectURL(blob));
        link.setAttribute('download', filename + '.csv');
        link.click();
      } else {
        csv = 'data:text/csv;charset=utf-8,' + csv;
        window.open(encodeURI(csv));
      }
      document.body.removeChild(link);
    }
  }
}
